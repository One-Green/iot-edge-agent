/*
Water ESP32 master firmware 

*/


#include "Arduino.h"
#include "WiFi.h"
#include "Wire.h"
#include "ArduinoJson.h"
#include "PubSubClient.h"
#include "OGDisplay.h"
#include "OGApiHandler.h"
#include "OGIO.h"
#include "Config.h"


void setup()
{
	Serial.begin (9600); 
	io_handler.initR(NODE_TAG);
} 

void loop()
{

/*  int val;
  
  sendCommand (CMD_READ_PH, 2);
  val = Wire.read ();
  val <<= 8;
  val |= Wire.read ();
  Serial.print ("Value of pH pin: ");
  Serial.println (val, DEC);

  sendCommand (CMD_READ_TDS, 2);
  val = Wire.read ();
  Serial.print ("Value of D8: ");
  Serial.println (val, DEC);
*/
	// float raw_ph_adc = readFloat(CMD_READ_PH);
	// delay (100);   
	// Serial.print("raw_ph_adc=") ; Serial.println(raw_ph_adc);

	// int raw_tds_adc = readInt(CMD_READ_TDS);
	// Serial.print("raw_tds_adc=") ; Serial.println(raw_tds_adc);
	// delay (500);
	delay(500);
}
