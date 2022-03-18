//
// Created by Shan on 16/08/2021
//
// TODO: add method to read lux meter i2c
// TODO: adapt method generateInfluxLineProtocol()

#include "Arduino.h"
#include "OGIO.h"
#include "Wire.h"
#include "BH1750.h"

#define LIGHT_RELAY_PIN 19
#define PHOTO_RESISTOR_PIN 33

BH1750 lightMeter(0x23);


void OGIO::initR(char *nodeTag) {
	OGIO::nodeTag = nodeTag;
	pinMode(LIGHT_RELAY_PIN, OUTPUT);
	pinMode(PHOTO_RESISTOR_PIN, INPUT);

	Wire.begin();
	lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);

}

int OGIO::getPhotoResistorADC() {
	OGIO::PhotoResistorADC = analogRead(PHOTO_RESISTOR_PIN);
	return OGIO::PhotoResistorADC;
}

float OGIO::getBH1750LuxLevel() {

    float lux = lightMeter.readLightLevel();

//    if (lux < 0) {
//        Serial.println(F("Error condition detected")); }
//    else {
//        if (lux > 40000.0)  {
//            // reduce measurement time - needed in direct sun light
//            if (lightMeter.setMTreg(32)) {
//                Serial.println(F("Setting MTReg to low value for high light environment"));
//                                            }
//            else {
//                Serial.println(F("Error setting MTReg to low value for high light environment"));
//                }
//                }
//        else {
//            if (lux > 10.0) {
//                // typical light environment
//                if (lightMeter.setMTreg(69)) {
//                    Serial.println(F("Setting MTReg to default value for normal light environment"));
//                }
//                else {
//                    Serial.println(F("Error setting MTReg to default value for normal light environment"));
//              }
//            }
//        else {
//            if (lux <= 10.0) {
//                // very low light environment
//                if (lightMeter.setMTreg(138)) {
//                    Serial.println(F("Setting MTReg to high value for low light environment"));
//                }
//                else {
//                    Serial.println(F("Error setting MTReg to high value for low light environment"));
//                }
//              }
//            }
//          }
//        }
//    }

    OGIO::BH1750LuxLevel = lux ;
	return OGIO::BH1750LuxLevel;
}

void OGIO::activateLightRelay() {
	digitalWrite(LIGHT_RELAY_PIN, HIGH);
}

void OGIO::deactivateLightRelay() {
	digitalWrite(LIGHT_RELAY_PIN, LOW);
}

String OGIO::generateInfluxLineProtocol() {
    float lux = BH1750LuxLevel ;
    float photo_res_raw = PhotoResistorADC ;

	String lineProtoStr =
			"light,tag=" + String(nodeTag)
			+ " lux_lvl=" + String(lux)+","
			+ "photo_res_raw=" + String(photo_res_raw)+","
			+ "photo_res_perc=" + String((int) photo_res_raw)+"i";

	return lineProtoStr ;
}
