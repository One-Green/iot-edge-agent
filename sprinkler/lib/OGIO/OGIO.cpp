//
// Created by jkl on 02/11/2020.
//

#include "Arduino.h"
#include "OGIO.h"

#define SOIL_MOISTURE_PIN  34
#define WATER_VALVE_PIN 19


void OGIO::initR(char *nodeTag) {

	OGIO::nodeTag = nodeTag;
	pinMode(WATER_VALVE_PIN, OUTPUT);

}

float OGIO::getMoistureLevelADC() {
	OGIO::soilMoistureADC = analogRead(SOIL_MOISTURE_PIN);
	return OGIO::soilMoistureADC;
}


float OGIO::getMoistureLevel(float in_min, float in_max, float out_min, float out_max) {
	OGIO::soilMoisture = map(getMoistureLevelADC(), in_min, in_max, out_min, out_max);
	return OGIO::soilMoisture;
}

void OGIO::openWaterValve() {
	digitalWrite(WATER_VALVE_PIN, HIGH);
}

void OGIO::closeWaterValve() {
	digitalWrite(WATER_VALVE_PIN, LOW);
}

String OGIO::generateInfluxLineProtocol() {
	int int_soilMoistureADC = (int) soilMoistureADC ;
	int int_soilMoisture = (int) soilMoisture ;
	String lineProtoStr =
			"sprinkler,tag=" + String(nodeTag)
			+ " soil_moisture_raw_adc=" + String(int_soilMoistureADC)+"i,"
			+ "soil_moisture=" + String(int_soilMoisture)+"i";
	return lineProtoStr ;
}
