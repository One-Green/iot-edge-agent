/*
 * Sprinkler ESP32 Arduino Firmware
 * https://github.com/One-Green/node-sprinkler-arduino
 *
 *
 * Work with this framework
 * One-Green : open source framework for plant cultivation including web server and microcontroller framework
 *
 * License : Creative Commons Legal Code - CC0 1.0 Universal
 * Author: Shanmugathas Vigneswaran
 * mail: shanmugathas.vigneswaran@outlook.fr
 *
 *
 * */


#include "WiFi.h"
#include "ArduinoJson.h"
#include "PubSubClient.h"
#include "OGDisplay.h"
#include "OGIO.h"
#include "Config.h"
#include "esp_task_wdt.h"

// State machine implementation :
enum State {
    IDLE,
    WATER,
    FORCE_NOT_WATER,
    FORCE_WATER
};
static State state = IDLE ;
unsigned int WATER_VALVE_BOUNCER_DELAY=5 ;
unsigned int MAX_WATER_PERIOD=5;
unsigned int idle_time=0;
unsigned int water_time=0;

// Watch configuration (in second)
#define WDT_TIMEOUT 3

// ADC to MAX = 0% and ADC to MIN = 100% calibration
// Used for mapping
int SOIL_MOISTURE_ADC_MAX = 4095;
int SOIL_MOISTURE_ADC_MIN = 1300;

// Actuator
bool last_water_valve_signal = false;
int soil_moisture_min_level = 0;
int soil_moisture_max_level = 0;

// controller signal
bool water_valve_signal = false;
bool f_water_valve_signal = false;
bool f_water_valve_signal_on = false;

WiFiClient espClient;
PubSubClient client(espClient);
DisplayLib displayLib;
OGIO io_handler;

// Custom functions

void reconnect_mqtt() {
	// Loop until we're reconnected
	while (!client.connected()) {
		DEBUG_PRINT("[MQTT] Attempting connection... with client name = ");
		String client_name = String(NODE_TYPE) + "-" + String(NODE_TAG);
		int clt_len = client_name.length() + 1;
		char clt_name_char[clt_len];
		client_name.toCharArray(clt_name_char, clt_len);
		DEBUG_PRINTLN(clt_name_char);

		// Attempt to connect
		if (client.connect(clt_name_char, MQTT_USER, MQTT_PASSWORD)) {
			DEBUG_PRINTLN("[MQTT] Client connected");
			// Subscribe
			client.subscribe(CONTROLLER_TOPIC);
		} else {
			DEBUG_PRINT("[MQTT] failed, rc=");
			DEBUG_PRINT(client.state());
			DEBUG_PRINTLN("[MQTT] try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}


void mqttCallback(char *topic, byte *message, unsigned int length) {
	DEBUG_PRINT("[MQTT] Receiving << on topic: ");
	DEBUG_PRINT(topic);
	DEBUG_PRINT(". JSON message: ");
	String messageTemp;

	for (int i = 0; i < length; i++) {
		DEBUG_PRINT((char) message[i]);
		messageTemp += (char) message[i];
	}
	DEBUG_PRINTLN();

	DynamicJsonDocument doc(1024);
	deserializeJson(doc, messageTemp);
	JsonObject obj = doc.as<JsonObject>();

	String tag = obj[String("tag")];
	water_valve_signal = obj[String("water_valve_signal")];
	soil_moisture_min_level = obj[String("soil_moisture_min_level")];
	soil_moisture_max_level = obj[String("soil_moisture_max_level")];
	// TODO: to implement
    f_water_valve_signal = false;
    f_water_valve_signal_on  = false;
}

void setup(void) {

    // idle all actuator
    io_handler.closeWaterValve();

	INIT_SERIAL(115200);
	displayLib.initR();
	// set pin mode etc ...
	io_handler.initR(NODE_TAG);

	delay(1000);
	DEBUG_PRINTLN("\n");

	displayLib.initWifi();
	DEBUG_PRINT("[WIFI] Connecting to ");
	DEBUG_PRINTLN(WIFI_SSID);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		DEBUG_PRINT(".");
		delay(100);
	}
	DEBUG_PRINTLN("\n");
	displayLib.connectedWifi();
	DEBUG_PRINTLN("[WIFI] Connected ");
	DEBUG_PRINT("[WIFI] IP address: ");
	DEBUG_PRINTLN(WiFi.localIP());

	displayLib.printHeader(WIFI_SSID, WiFi.localIP(), NODE_TYPE, NODE_TAG);
	displayLib.printTemplate();

	// MQTT connexion
	client.setServer(MQTT_SERVER, MQTT_PORT);
	client.setCallback(mqttCallback);

	// enable panic so ESP32 restarts
    esp_task_wdt_init(WDT_TIMEOUT, true);
    //add current thread to WDT watch
    esp_task_wdt_add(NULL);

}

void loop() {

	// reconnect MQTT Client if not connected
	if (!client.connected()) {
		reconnect_mqtt();
	}
	client.loop();
    int rawSoilMoisture = io_handler.getMoistureLevelADC();
    int soilMoisture = io_handler.getMoistureLevel(
            SOIL_MOISTURE_ADC_MIN,
            SOIL_MOISTURE_ADC_MAX,
            100, 0);
    DEBUG_PRINTLN("[I/O] Soil moisture ADC=" + String(rawSoilMoisture) + "/" + "LEVEL=" + String(soilMoisture));
    DEBUG_PRINT("[MQTT] Sending >> on topic= " + String(SENSOR_TOPIC) + " Influxdb line protocol message: ");
    String line_proto = io_handler.generateInfluxLineProtocol();
    DEBUG_PRINTLN(line_proto);
    // convert string to char and publish to mqtt
    int line_proto_len = line_proto.length() + 1;
    char line_proto_char[line_proto_len];
    line_proto.toCharArray(line_proto_char, line_proto_len);
    client.publish(SENSOR_TOPIC, line_proto_char);
    displayLib.updateDisplay(rawSoilMoisture, soilMoisture,
                             soil_moisture_min_level, soil_moisture_max_level,
                             last_water_valve_signal);

    switch(state)
    {
        case IDLE:
            io_handler.closeWaterValve();

            // transition 1
            if (
                 soilMoisture <= soil_moisture_min_level
                 &&
                 (millis() - idle_time) > WATER_VALVE_BOUNCER_DELAY
             ) {
                state = WATER;
                water_time = millis();
            }

            // transition 3
            if (
                f_water_valve_signal == true
                &&
                f_water_valve_signal_on == false
                ) {
                state = FORCE_NOT_WATER ;
            }

            // transition 5
            if (
                f_water_valve_signal == true
                &&
                f_water_valve_signal_on == true
                ) {
                state = FORCE_WATER ;
            }
            break;
        case WATER:
            io_handler.openWaterValve();

            // transition 2
            if (
                soilMoisture >= soil_moisture_max_level
                ||
                (millis() - water_time) > MAX_WATER_PERIOD
            ) {
                idle_time = millis();
                state = IDLE;
            }

            // transition 9
            if ( f_water_valve_signal == true ) { state = FORCE_NOT_WATER ;}

            break;
        case FORCE_NOT_WATER:
            io_handler.closeWaterValve();

            // transition 4
            if (f_water_valve_signal == false) { state = IDLE ;}

            // transition 8
            if (f_water_valve_signal_on == true) { state = FORCE_WATER ;}
            break;
        case FORCE_WATER:
            io_handler.openWaterValve();

            // transition 6
            if (
                f_water_valve_signal == false
                &&
                f_water_valve_signal_on == false
                ) {
                state = IDLE ;
            }

            // transition 7
            if (
                f_water_valve_signal == true
                &&
                f_water_valve_signal_on == false
                ) {
                state = FORCE_NOT_WATER ;
            }

            break;
    }


	delay(500);
    // Clear Watch dog
    esp_task_wdt_reset();
}
