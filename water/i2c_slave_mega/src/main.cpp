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

  // Sensors report commands
	CMD_READ_PH_RAW_ADC           = 1,
	CMD_READ_TDS_RAW_ADC          = 2,
	CMD_READ_PH                   = 3,
	CMD_READ_TDS                  = 4,
	CMD_READ_WATER_LVL            = 5,
	CMD_READ_NUTRIENT_LVL         = 6,
	CMD_READ_PH_LVL               = 7,

	CMD_READ_WATER_PUMP           = 8,
	CMD_WRITE_LOW_WATER_PUMP      = 9,
	CMD_WRITE_HIGH_WATER_PUMP     = 10,
	
	CMD_READ_NUTRIENT_PUMP        = 11,
	CMD_WRITE_LOW_NUTRIENT_PUMP   = 12,
	CMD_WRITE_HIGH_NUTRIENT_PUMP  = 13,

	CMD_READ_PH_DOWNER_PUMP       = 14,
	CMD_WRITE_LOW_PH_DOWNER_PUMP  = 15,
	CMD_WRITE_HIGH_PH_DOWNER_PUMP = 16,

	CMD_READ_MIXER_PUMP           = 17,
	CMD_WRITE_LOW_MIXER_PUMP      = 18,
	CMD_WRITE_HIGH_MIXER_PUMP     = 19,

	// safety actions 
	CMD_SAFE_MODE 			      = 20
};


float water_tank_level;
float nutrient_tank_level;
float ph_downer_tank_level;

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


void receiveEvent (int howMany) { command = Wire.read ();} 

void requestEvent ()
{
  
  float val;          // float sensor value
  int intVal;         // integer sensor value 
  bool bVal;          // bool digital value

  String str = "";    // cast float to String to write through Wire

  switch (command)
  {
    case CMD_IDLE: 
      Wire.write(2); 
      Serial.println("I2C received CMD_IDLE");
      break;

    case CMD_READ_PH_RAW_ADC:
      Serial.print("I2C received CMD_READ_PH_RAW_ADC, pH raw adc=");
      val = io_handler.getPhLevelRawADC();    
      Serial.println(val);
      str.concat(val);
      Wire.write(str.c_str());
      break;  
    
    case CMD_READ_TDS_RAW_ADC:
      Serial.print("I2C received CMD_READ_TDS_RAW_ADC, tds raw adc=");
      val = io_handler.getTDSRawADC();    
      Serial.println(val);
      str.concat(val);
      Wire.write(str.c_str());
      break;  

    case CMD_READ_PH: 
      Serial.print("I2C received CMD_READ_PH, pH=");
      val = io_handler.getPhLevel();    
      Serial.println(val);
      str.concat(val);
      Wire.write(str.c_str());
      break;  

    case CMD_READ_TDS:
      Serial.print("I2C received CMD_READ_TDS, tds=");
      val = io_handler.getTDS();    
      Serial.println(val);
      str.concat(val);
      Wire.write(str.c_str());
      break;  
    
    case CMD_READ_WATER_LVL:
      Serial.print("I2C received CMD_READ_WATER_LVL, lvl=");
      Serial.println(water_tank_level);
      str.concat(water_tank_level);
      Wire.write(str.c_str());
      break;
    
    case CMD_READ_NUTRIENT_LVL:
      Serial.print("I2C received CMD_READ_NUTRIENT_LVL, lvl=");
      Serial.println(nutrient_tank_level);
      str.concat(nutrient_tank_level);
      Wire.write(str.c_str());
      break;  

    case CMD_READ_PH_LVL:
      Serial.print("I2C received CMD_READ_PH_LVL, lvl=");
      Serial.println(ph_downer_tank_level);
      str.concat(ph_downer_tank_level);
      Wire.write(str.c_str());
      break;

    // ------------------- water pump cases 
    case CMD_READ_WATER_PUMP:
      Serial.print("I2C received CMD_READ_WATER_PUMP, state=");
      bVal = io_handler.getWaterPumpStatus();
      Serial.println(bVal);
      Wire.write(bVal);
      break;
    
    case CMD_WRITE_LOW_WATER_PUMP:
      Serial.print("I2C received CMD_WRITE_LOW_WATER_PUMP, state=");
      io_handler.setWaterPump(0);
      bVal = io_handler.getWaterPumpStatus();
      Serial.println(bVal);      
      Wire.write(bVal);
      break;

    case CMD_WRITE_HIGH_WATER_PUMP:
      Serial.print("I2C received CMD_WRITE_HIGH_WATER_PUMP, state=");
      io_handler.setWaterPump(1);
      bVal = io_handler.getWaterPumpStatus();
      Serial.println(bVal);      
      Wire.write(bVal);
      break;

    // ------------------- nutrient pump cases
    case CMD_READ_NUTRIENT_PUMP:
      Serial.print("I2C received CMD_READ_NUTRIENT_PUMP, state=");
      bVal = io_handler.getNutrientPumpStatus();
      Serial.println(bVal);
      Wire.write(bVal);
      break;
    
    case CMD_WRITE_LOW_NUTRIENT_PUMP:
      Serial.print("I2C received CMD_WRITE_LOW_NUTRIENT_PUMP, state=");
      io_handler.setNutrientPump(0);
      bVal = io_handler.getNutrientPumpStatus();
      Serial.println(bVal);
      Wire.write(bVal);
      break;
    
    case CMD_WRITE_HIGH_NUTRIENT_PUMP:
      Serial.print("I2C received CMD_WRITE_HIGH_NUTRIENT_PUMP, state=");
      io_handler.setNutrientPump(1);
      bVal = io_handler.getNutrientPumpStatus();
      Serial.println(bVal);
      Wire.write(bVal);
      break;

    // -------------------  ph downer pump cases
    case CMD_READ_PH_DOWNER_PUMP:
      Serial.print("I2C received CMD_READ_PH_DOWNER_PUMP, state=");
      bVal = io_handler.getPhDownerPumpStatus();
      Serial.println(bVal);
      Wire.write(bVal);
      break;

    case CMD_WRITE_LOW_PH_DOWNER_PUMP:
      Serial.print("I2C received CMD_WRITE_LOW_PH_DOWNER_PUMP, state=");
      io_handler.setPHDownerPump(0);
      bVal = io_handler.getPhDownerPumpStatus();
      Serial.println(bVal);
      Wire.write(bVal);
      break;

    case CMD_WRITE_HIGH_PH_DOWNER_PUMP:
      Serial.print("I2C received CMD_WRITE_HIGH_PH_DOWNER_PUMP, state=");
      io_handler.setPHDownerPump(1);
      bVal = io_handler.getPhDownerPumpStatus();
      Serial.println(bVal);
      Wire.write(bVal);
      break;

    // ------------------- mixer pump cases
    case CMD_READ_MIXER_PUMP:
      Serial.print("I2C received CMD_WRITE_LOW_PH_DOWNER_PUMP, state=");
      bVal = io_handler.getMixerPumpStatus();
      Serial.println(bVal);
      Wire.write(bVal);
      break;
    
    case CMD_WRITE_LOW_MIXER_PUMP:
      Serial.print("I2C received CMD_WRITE_LOW_MIXER_PUMP, state=");
      io_handler.setMixerPump(0);
      bVal = io_handler.getMixerPumpStatus();
      Serial.println(bVal);
      Wire.write(bVal);
      break;

    case CMD_WRITE_HIGH_MIXER_PUMP:
      Serial.print("I2C received CMD_WRITE_HIGH_MIXER_PUMP, state=");
      io_handler.setMixerPump(1);
      bVal = io_handler.getMixerPumpStatus();
      Serial.println(bVal);
      Wire.write(bVal);
      break;

    case CMD_SAFE_MODE:
      Serial.println("I2C received CMD_WRITE_SAFE_MODE");
      io_handler.setWaterPump(0);
      io_handler.setNutrientPump(0);
      io_handler.setPHDownerPump(0);
      io_handler.setMixerPump(0);
      Wire.write(1);                      // send ack received command, do pumps status on esp32
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
  Serial.print("I2C Salve is up , addr =");
  Serial.println(I2C_ADDRESS);

  io_handler.initR();                // I/O Arduino mega setup digital pin mode
  
}

void loop() {
  water_tank_level = io_handler.getWaterLevelCM();
  delay(50);
  nutrient_tank_level = io_handler.getNutrientLevelCM();
  delay(50);
  ph_downer_tank_level = io_handler.getPhDownerLevelCM();
  delay(50);
}
