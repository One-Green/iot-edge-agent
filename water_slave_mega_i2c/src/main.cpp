/*
Arduino Mega i2c slave
https://one-green.io
Author: Shanmugathas Vigneswaran
*/


#include "Arduino.h"
#include "Wire.h"
#include "OGIO.h"

const byte I2C_ADDRESS = 2;
int val ;
char command;


// i2c exchange table
enum
{
	CMD_IDLE                      = 0,
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

	CMD_SAFE_MODE 			          = 15
};




void I2CwriteSensor(const byte which)
{
  val = analogRead(which);
  byte buf [2];
  buf [0] = val >> 8;
  buf [1] = val & 0xFF;
  Wire.write (buf, 2);
}  

void I2CwriteInteger(int value)
{
  byte buf [2];
  buf [0] = value >> 8;
  buf [1] = value & 0xFF;
  Wire.write (buf, 2);
}

void I2CwriteString(String value)
{
  Wire.write(value.c_str());
}


void receiveEvent (int howMany) { command = Wire.read ();} 

void requestEvent ()
{
  String str = ""; 
  float val;

  switch (command)
  {
    case CMD_IDLE: 
      Wire.write(2); 
      Serial.println("I2C recived CMD_IDLE");
      break; 

    case CMD_READ_PH: 
      Serial.print("I2C recived CMD_READ_PH, pH=");
      val = io_handler.getPhLevel();    
      Serial.println(val);
      str.concat(val);
      Wire.write(str.c_str());
      break;  

    case CMD_READ_TDS:
      Serial.print("I2C recived CMD_READ_TDS, tds=");
      val = io_handler.getTDS();    
      Serial.println(val);
      str.concat(val);
      Wire.write(str.c_str());
      break;  
    
    case CMD_READ_WATER_LVL:
      Wire.write(digitalRead(8));
      break;  
  }
}  


void setup() 
{
  command = 0;
  Serial.begin(9600);

  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);  
  Wire.onRequest(requestEvent); 
  Serial.print("I2C up , addr =");
  Serial.println(I2C_ADDRESS);

  io_handler.initR();                // I/O setup digital pin mode
  
}

void loop() {}
