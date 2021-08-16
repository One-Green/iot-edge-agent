//
// Created by Shan on 16/08/2021
//
// TODO: add method to read lux meter i2c
// TODO: adapt method generateInfluxLineProtocol()

#include "Arduino.h"
#include "OGIO.h"

#define LIGHT_RELAY_PIN 19


void OGIO::initR(char *nodeTag) {
	OGIO::nodeTag = nodeTag;
	pinMode(LIGHT_RELAY_PIN, OUTPUT);

}

void OGIO::activateLightRelay() {
	digitalWrite(LIGHT_RELAY_PIN, HIGH);
}

void OGIO::deactivateLightRelay() {
	digitalWrite(LIGHT_RELAY_PIN, LOW);
}

String OGIO::generateInfluxLineProtocol() {
    // TODO: remove mock value
    int int_test = 10 ;
	String lineProtoStr =
			"light,tag=" + String(nodeTag)
			+ " lux_level=" + String(int_test)+"i,"
			+ "other_level=" + String(int_test)+"i";
	return lineProtoStr ;
}
