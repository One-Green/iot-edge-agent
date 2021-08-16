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


#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "OGDisplay.h"
#include "OGApiHandler.h"
#include "OGIO.h"
#include "Config.h"

// ADC to MAX = 100% and ADC tp MIN = 0% calibration
// Used for mapping
int SOIL_MOISTURE_ADC_MAX = 3550;
int SOIL_MOISTURE_ADC_MIN = 1830;

// ensure NODE_TAG is unique , use CHECK_NODE_TAG_DUPLICATE = false to bypass
bool registered = false;

// Actuator
bool last_water_valve_signal = false;
int soil_moisture_min_level = 0;
int soil_moisture_max_level = 0;


WiFiClient espClient;
PubSubClient client(espClient);
DisplayLib displayLib;
OGApiHandler apiHandler;
OGIO io_handler;

// Custom functions

void reconnect_mqtt() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("[MQTT] Attempting connection... with client name = ");
		String client_name = String(NODE_TYPE) + "-" + String(NODE_TYPE);
		int clt_len = client_name.length() + 1;
		char clt_name_char[clt_len];
		client_name.toCharArray(clt_name_char, clt_len);
		Serial.println(clt_name_char);

		// Attempt to connect
		if (client.connect(clt_name_char, MQTT_USER, MQTT_PASSWORD)) {
			Serial.println("[MQTT] Client connected");
			// Subscribe
			client.subscribe(CONTROLLER_TOPIC);
		} else {
			Serial.print("[MQTT] failed, rc=");
			Serial.print(client.state());
			Serial.println("[MQTT] try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}


void mqttCallback(char *topic, byte *message, unsigned int length) {
	Serial.print("[MQTT] Receiving << on topic: ");
	Serial.print(topic);
	Serial.print(". JSON message: ");
	String messageTemp;

	for (int i = 0; i < length; i++) {
		Serial.print((char) message[i]);
		messageTemp += (char) message[i];
	}
	Serial.println();

	DynamicJsonDocument doc(1024);
	deserializeJson(doc, messageTemp);
	JsonObject obj = doc.as<JsonObject>();

	String tag = obj[String("tag")];
	bool water_valve_signal = obj[String("water_valve_signal")];
	soil_moisture_min_level = obj[String("soil_moisture_min_level")];
	soil_moisture_max_level = obj[String("soil_moisture_max_level")];

	if (tag == NODE_TAG) {
		if (water_valve_signal != last_water_valve_signal) {
			last_water_valve_signal = water_valve_signal;
			if (water_valve_signal) {
				io_handler.openWaterValve();
				Serial.println("[I/O] Water valve has been OPENED");
			} else {
				io_handler.closeWaterValve();
				Serial.println("[I/O] Water valve has been CLOSED");
			}
		}
	}
}


void setup(void) {

	Serial.begin(115200);
	displayLib.initR();
	// set pin mode etc ...
	io_handler.initR(NODE_TAG);

	delay(1000);
	Serial.println("\n");

	displayLib.initWifi();
	Serial.print("[WIFI] Connecting to ");
	Serial.print(WIFI_SSID);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(100);
	}

	Serial.println("\n");
	displayLib.connectedWifi();
	Serial.println("[WIFI] Connected ");
	Serial.print("[WIFI] IP address: ");
	Serial.println(WiFi.localIP());

	displayLib.printHeader(WIFI_SSID, WiFi.localIP(), NODE_TYPE, NODE_TAG);
	displayLib.printTemplate();

	if (WiFi.status() == WL_CONNECTED) {
		if (CHECK_NODE_TAG_DUPLICATE) {
			registered = apiHandler.registerNodeTag(
					NODE_TAG,
					API_GATEWAY_URL,
					API_GATEWAY_BASIC_AUTH_USER,
					API_GATEWAY_BASIC_AUTH_PASSWORD
			);
		} else {
			registered = true;
		}
	}

	// MQTT connexion
	client.setServer(MQTT_SERVER, MQTT_PORT);
	client.setCallback(mqttCallback);

}

void loop() {

	// reconnect MQTT Client if not connected
	if (!client.connected()) {
		reconnect_mqtt();
	}
	client.loop();

	if (registered) {
		int rawSoilMoisture = io_handler.getMoistureLevelADC();
		int soilMoisture = io_handler.getMoistureLevel(
				SOIL_MOISTURE_ADC_MIN, SOIL_MOISTURE_ADC_MAX,
				100, 0);
		Serial.println("[I/O] Soil moisture ADC=" + String(rawSoilMoisture) + "/" + "LEVEL=" + String(soilMoisture));
		Serial.print("[MQTT] Sending >> on topic= " + String(SENSOR_TOPIC) + " Influxdb line protocol message: ");
		String line_proto = io_handler.generateInfluxLineProtocol();
		Serial.println(line_proto);
		// convert string to char and publish to mqtt
		int line_proto_len = line_proto.length() + 1;
		char line_proto_char[line_proto_len];
		line_proto.toCharArray(line_proto_char, line_proto_len);
		client.publish(SENSOR_TOPIC, line_proto_char);

		displayLib.updateDisplay(rawSoilMoisture, soilMoisture,
		                         soil_moisture_min_level, soil_moisture_max_level,
		                         last_water_valve_signal);

	} else {
		Serial.println("Not registered, "
		               "tag is already in database, "
		               "to bypass change variable  CHECK_NODE_TAG_DUPLICATE to false");
		displayLib.printRegistryError();
	}

	delay(500);
}
