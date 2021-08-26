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

const int SLAVE_ADDRESS = 2;

// i2c exchange table
enum
{
	CMD_IDLE                      = 0,

  // Sensors repport commands
	CMD_READ_PH                   = 1,
	CMD_READ_TDS                  = 2,
	CMD_READ_WATER_LVL            = 3,
	CMD_READ_NUTRIENT_LVL         = 4,
	CMD_READ_PH_LVL               = 5,

	CMD_READ_WATER_PUMP           = 6,
	CMD_WRITE_LOW_WATER_PUMP      = 7,
	CMD_WRITE_HIGH_WATER_PUMP     = 8,
	
	CMD_READ_NUTRIENT_PUMP        = 9,
	CMD_WRITE_LOW_NUTRIENT_PUMP   = 10,
	CMD_WRITE_HIGH_NUTRIEN_PUMP   = 11,

	CMD_READ_PH_DOWNER_PUMP       = 12,
	CMD_WRITE_LOW_PH_DOWNER_PUMP  = 13,
	CMD_WRITE_HIGH_PH_DOWNER_PUMP = 14,

	CMD_READ_MIXER_PUMP           = 15,
	CMD_WRITE_LOW_MIXER_PUMP      = 16,
	CMD_WRITE_HIGH_MIXER_PUMP     = 17,

	// safety actions 
	CMD_SAFE_MODE 			      = 18
};

void sendCommand (const byte cmd, const int responseSize)
{
	Serial.print("I2C send command >");
	Serial.println(cmd);
	Wire.beginTransmission (SLAVE_ADDRESS);
	Wire.write (cmd);
	Wire.endTransmission ();
	Wire.requestFrom(SLAVE_ADDRESS, responseSize);  
} 
  
int readInt(const byte cmd)
{
	int val;  
	sendCommand (cmd, 2);
	val = Wire.read ();
	val <<= 8;
	val |= Wire.read ();
	return val;
}

void flushI2C()
{
	while (Wire.available())
	{
		Wire.read();
	}
}

float readFloat(const byte cmd)
{
	// Receive from i2c char bytes
	// and convert to float 
	byte resp_length = 10;
	char resp_buffer[resp_length] ; 
	
	sendCommand(cmd, resp_length);

	if (Wire.available()) {
		for (byte i = 0; i < resp_length; i = i + 1) 
			{
  				resp_buffer[i] = Wire.read();
			}
	}

	flushI2C();

	// convert to float 
	float val = atof(resp_buffer);
	Serial.print("readFloat >> ");
	Serial.println(val);

	return val;

}


int readByte(const byte cmd)
{
	int val;
	sendCommand(cmd, 1);
	val = Wire.read ();
	return val;
}

void setup ()
{
	Serial.begin (9600); 
	
	Wire.begin ();   
	sendCommand (CMD_IDLE, 1);
	if (Wire.available ())
	{
		Serial.print ("Slave is ID: ");
		Serial.println (Wire.read (), DEC);
	}
	else
		Serial.println ("No response to ID request");

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
	float ph = readFloat(CMD_READ_PH);
	float tds = readFloat(CMD_READ_TDS);

	delay(500);
}
