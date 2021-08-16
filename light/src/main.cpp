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

// ensure NODE_TAG is unique , use CHECK_NODE_TAG_DUPLICATE = false to bypass
bool registered = false;

WiFiClient espClient;
PubSubClient client(espClient);
DisplayLib displayLib;
OGApiHandler apiHandler;
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

    // collect one-green core information
    // https://arduinojson.org/v6/error/ambiguous-overload-for-operator-equal/
    // use as<String>() to avoid ambiguous overload
    tz = obj["tz"].as<String>();
    on_time_at = obj["on_time_at"].as<String>();
    off_time_at = obj["off_time_at"].as<String>();

	String tag = obj[String("tag")];
	bool light_signal = obj[("light_signal")];

	if (tag == NODE_TAG) {
		if (light_signal != last_light_signal) {
			last_light_signal = light_signal;
			if (light_signal) {
				io_handler.activateLightRelay();
				Serial.println("[I/O] Light power is ON");
			} else {
				io_handler.deactivateLightRelay();
				Serial.println("[I/O] Light power is OFF");
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
		String line_proto = io_handler.generateInfluxLineProtocol();
		Serial.println(line_proto);
		// convert string to char and publish to mqtt
		int line_proto_len = line_proto.length() + 1;
		char line_proto_char[line_proto_len];
		line_proto.toCharArray(line_proto_char, line_proto_len);
		client.publish(SENSOR_TOPIC, line_proto_char);

//
//		displayLib.updateDisplay();

	} else {
		Serial.println("Not registered, "
		               "tag is already in database, "
		               "to bypass change variable  CHECK_NODE_TAG_DUPLICATE to false");
		displayLib.printRegistryError();
	}

	delay(500);
}
