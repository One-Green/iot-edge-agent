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

// Watch configuration (in second)
#define WDT_TIMEOUT 3

WiFiClient espClient;
PubSubClient client(espClient);
DisplayLib displayLib;
OGIO io_handler;

bool last_light_signal = false;
// information in JSON, callback topic CONTROLLER_TOPIC
String tz;
String on_time_at;
String off_time_at;


// Custom functions

void reconnect_mqtt() {
	// Loop until we're reconnected
	while (!client.connected()) {
		DEBUG_PRINT("[MQTT] Attempting connection... with client name = ");
		String client_name = String(NODE_TYPE) + "-" + String(NODE_TYPE);
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

    // collect one-green core information
    // https://arduinojson.org/v6/error/ambiguous-overload-for-operator-equal/
    // use as<String>() to avoid ambiguous overload
    tz = obj["tz"].as<String>();
    on_time_at = obj["on_time_at"].as<String>();
    off_time_at = obj["off_time_at"].as<String>();

	bool light_signal = obj[("light_signal")];

    if (light_signal != last_light_signal) {
        last_light_signal = light_signal;
        if (light_signal) {
            io_handler.activateLightRelay();
            DEBUG_PRINTLN("[I/O] Light power is ON");
        } else {
            io_handler.deactivateLightRelay();
            DEBUG_PRINTLN("[I/O] Light power is OFF");
        }
    }

}


void setup(void) {

	INIT_SERIAL(115200);

	displayLib.initR();
	// set pin mode etc ...
	io_handler.initR(NODE_TAG);
	delay(1000);
	DEBUG_PRINTLN("\n");

	displayLib.initWifi();
	DEBUG_PRINT("[WIFI] Connecting to ");
	DEBUG_PRINT(WIFI_SSID);
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

    float lux = io_handler.getBH1750LuxLevel() ;
    int phr = io_handler.getPhotoResistorADC() ;
    DEBUG_PRINTLN("[I/O] Photo resistor ADC=" + String(phr));
    DEBUG_PRINT("[MQTT] Sending >> on topic= " + String(SENSOR_TOPIC) + " Influxdb line protocol message: ");
    String line_proto = io_handler.generateInfluxLineProtocol();
    DEBUG_PRINTLN(line_proto);

    // convert string to char and publish to mqtt
    int line_proto_len = line_proto.length() + 1;
    char line_proto_char[line_proto_len];
    line_proto.toCharArray(line_proto_char, line_proto_len);
    client.publish(SENSOR_TOPIC, line_proto_char);

    displayLib.updateDisplay(
        true,
        false,
        "12:30",
        "14:30",
        true
    );

	delay(500);
    // Clear Watch dog
    esp_task_wdt_reset();
}
