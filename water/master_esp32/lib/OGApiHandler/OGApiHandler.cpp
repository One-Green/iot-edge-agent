//
// Created by shan on 02/11/2020.
//
#include "Arduino.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "base64.h"
#include "OGApiHandler.h"

String OGApiHandler::getRegistry(char *api_gateway) {

	if (String(api_gateway).endsWith("/")) {
		return String(api_gateway) + "water/registry";
	} else {
		return String(api_gateway) + "/water/registry";
	}
}


bool OGApiHandler::registerNodeTag(
		char *nodeTag,
		char *api_gateway,
		char *basic_auth,
		char *basic_pwd) {

	StaticJsonDocument<200> doc;

	HTTPClient http;
	// post to this api
	http.begin(getRegistry(api_gateway));
	// add header content type + auth if defined
	http.addHeader("Content-Type", "application/json");

	if ((String(basic_auth).length() > 0) && (String(basic_pwd).length() > 0)) {
		String auth = base64::encode(String(basic_auth) + ":" + String(basic_pwd));
		http.addHeader("Authorization", "Basic " + auth);
	}
	// send request
	Serial.println("Trying to register this tag: " + String(nodeTag));
	int httpResponseCode = http.POST("{ \"tag\": \"" + String(nodeTag) + "\"}");

	Serial.print("HTTP response code: ");
	Serial.println(httpResponseCode);

	if (httpResponseCode > 0) {
		String response = http.getString();
		// parse string to json object
		int str_len = response.length() + 1;
		char char_response[str_len];
		response.toCharArray(char_response, str_len);
		Serial.println(char_response);

		DeserializationError error = deserializeJson(doc, char_response);
		if (error) {
			Serial.print(F("deserializeJson() failed: "));
			Serial.println(error.f_str());
		} else {
			bool acknowledge = doc["acknowledge"];
			return acknowledge;
		}

	} else {
		Serial.print("Error on sending POST: ");
		Serial.println(httpResponseCode);
		return false;
	}
	http.end();
	return false;
}
