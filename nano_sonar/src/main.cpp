/*
* Purpose: 3 sonars to MCP4725 DAC 0-5V analog output
*
* Author: Shanmugathas Vigneswaran
* email: shanmugathas.vigneswaran@outlook.fr
*/


#include <Arduino.h>
#include <Wire.h>
#include "MCP4725.h"

#define DEBUG 0
#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.print (x)
 #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTLN(x)
#endif

#define WATER_LEVEL_TRIGGER_PIN         2
#define WATER_LEVEL_ECHO_PIN            3
#define WATER_LEVEL_MIN_DISTANCE        0
#define WATER_LEVEL_MAX_DISTANCE        450

#define NUTRIENT_LEVEL_TRIGGER_PIN      4
#define NUTRIENT_LEVEL_ECHO_PIN         5
#define NUTRIENT_LEVEL_MIN_DISTANCE     0
#define NUTRIENT_LEVEL_MAX_DISTANCE     450

#define PH_DOWNER_LEVEL_TRIGGER_PIN     6
#define PH_DOWNER_LEVEL_ECHO_PIN        7
#define PH_DOWNER_LEVEL_MIN_DISTANCE    0
#define PH_DOWNER_LEVEL_MAX_DISTANCE    450

#define TCA9548A_ADDRESS                0x70
#define MCP4725_ADDRESS                 0x60

int dWater ;
int dWaterMapped ;
int dNutrient ;
int dNutrientMapped ;
int dPHDowner ;
int dPHDownerMapped ;
MCP4725 MCP_1(MCP4725_ADDRESS);
MCP4725 MCP_2(MCP4725_ADDRESS);
MCP4725 MCP_3(MCP4725_ADDRESS);

void TCA9548A(uint8_t bus)
{
  Wire.beginTransmission(TCA9548A_ADDRESS);
  Wire.write(1 << bus);
  Wire.endTransmission();
}

int baseUltrasonicReader(int trigger, int echo) {
    long duration;
    int distance;
    // Clear the trigPin by setting it LOW:
    digitalWrite(trigger, LOW);
    delayMicroseconds(5);
    // Trigger the sensor by setting the trigPin high for 10 microseconds:
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);
    // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds:
    duration = pulseIn(echo, HIGH);
    // Calculate the distance:
    distance = duration * 0.034 / 2;
    return distance;
}


void setup() {
    pinMode(WATER_LEVEL_TRIGGER_PIN, OUTPUT);
    pinMode(WATER_LEVEL_ECHO_PIN, INPUT);

    pinMode(NUTRIENT_LEVEL_TRIGGER_PIN, OUTPUT);
    pinMode(NUTRIENT_LEVEL_ECHO_PIN, INPUT);

    pinMode(PH_DOWNER_LEVEL_TRIGGER_PIN, OUTPUT);
    pinMode(PH_DOWNER_LEVEL_ECHO_PIN, INPUT);

    TCA9548A(0);
    MCP_1.begin();

    TCA9548A(1);
    MCP_2.begin();

    TCA9548A(2);
    MCP_3.begin();


    Serial.begin(9600);
}

void loop() {

    int dWater = baseUltrasonicReader(WATER_LEVEL_TRIGGER_PIN, WATER_LEVEL_ECHO_PIN);
    int dWaterMapped = map(dWater, WATER_LEVEL_MIN_DISTANCE, WATER_LEVEL_MAX_DISTANCE , 0, 4095 ) ; // analogWrite values from 0 to 255
    TCA9548A(0); MCP_1.setValue(dWaterMapped);
    DEBUG_PRINT("dWater="); DEBUG_PRINT(dWater); DEBUG_PRINT(" ; ");

    int dNutrient = baseUltrasonicReader(NUTRIENT_LEVEL_TRIGGER_PIN, NUTRIENT_LEVEL_ECHO_PIN);
    int dNutrientMapped = map(dNutrient, NUTRIENT_LEVEL_MIN_DISTANCE, NUTRIENT_LEVEL_MAX_DISTANCE , 0, 4095 ) ; // analogWrite values from 0 to 255
    TCA9548A(1); MCP_2.setValue(dNutrientMapped);
    DEBUG_PRINT("dNutrient="); DEBUG_PRINT(dNutrient); DEBUG_PRINT(" ; ");

    int dPHDowner = baseUltrasonicReader(PH_DOWNER_LEVEL_TRIGGER_PIN, PH_DOWNER_LEVEL_ECHO_PIN);
    int dPHDownerMapped = map(dPHDowner, PH_DOWNER_LEVEL_MIN_DISTANCE, PH_DOWNER_LEVEL_MAX_DISTANCE , 0, 4095 ) ; // analogWrite values from 0 to 255
    TCA9548A(2); MCP_3.setValue(dPHDownerMapped);
    DEBUG_PRINT("dPHDowner="); DEBUG_PRINT(dPHDowner); DEBUG_PRINTLN();

    DEBUG_PRINT("dWaterMapped="); DEBUG_PRINT(dWaterMapped); DEBUG_PRINT(" ; ");
    DEBUG_PRINT("dNutrientMapped="); DEBUG_PRINT(dNutrientMapped); DEBUG_PRINT(" ; ");
    DEBUG_PRINT("dPHDownerMapped="); DEBUG_PRINT(dPHDownerMapped); DEBUG_PRINTLN();

}
