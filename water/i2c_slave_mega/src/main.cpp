/*
Arduino Mega i2c slave
https://one-green.io
Author: Shanmugathas Vigneswaran
*/


#include "Arduino.h"
#include "Wire.h"
#include "OGIO.h"
#include "Config.h"

const byte I2C_ADDRESS = 2;
int val ;
char command;

// Safety consideration: if no I2C receiveEvent after safetyCloseActuatorSec second
unsigned long lastReceiveEvent;
const int safetyCloseActuatorSec = 2; // close actuators if no request from I2C Master

// i2c exchange table
enum
{
	CMD_IDLE                      = 0,

  // Sensors report commands
	CMD_READ_PH_VOLTAGE           = 1,
	CMD_READ_TDS_VOLTAGE          = 2,
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


void receiveEvent (int howMany) 
{ 
  lastReceiveEvent = millis();
  command = Wire.read ();
} 

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
      DEBUG_PRINTLN("I2C received CMD_IDLE");
      break;

    case CMD_READ_PH_VOLTAGE:
      DEBUG_PRINT("I2C received CMD_READ_PH_VOLTAGE, pH voltage=");
      val = io_handler.getPhVoltage();    
      DEBUG_PRINTLN(val);
      str.concat(val);
      Wire.write(str.c_str());
      break;  
    
    case CMD_READ_TDS_VOLTAGE:
      DEBUG_PRINT("I2C received CMD_READ_TDS_VOLTAGE, tds voltage=");
      val = io_handler.getTDSVoltage();    
      DEBUG_PRINTLN(val);
      str.concat(val);
      Wire.write(str.c_str());
      break;  

    case CMD_READ_PH: 
      DEBUG_PRINT("I2C received CMD_READ_PH, pH=");
      val = io_handler.getPhLevel();    
      DEBUG_PRINTLN(val);
      str.concat(val);
      Wire.write(str.c_str());
      break;  

    case CMD_READ_TDS:
      DEBUG_PRINT("I2C received CMD_READ_TDS, tds=");
      val = io_handler.getTDS();    
      DEBUG_PRINTLN(val);
      str.concat(val);
      Wire.write(str.c_str());
      break;  
    
    case CMD_READ_WATER_LVL:
      DEBUG_PRINT("I2C received CMD_READ_WATER_LVL, lvl=");
      DEBUG_PRINTLN(water_tank_level);
      str.concat(water_tank_level);
      Wire.write(str.c_str());
      break;
    
    case CMD_READ_NUTRIENT_LVL:
      DEBUG_PRINT("I2C received CMD_READ_NUTRIENT_LVL, lvl=");
      DEBUG_PRINTLN(nutrient_tank_level);
      str.concat(nutrient_tank_level);
      Wire.write(str.c_str());
      break;  

    case CMD_READ_PH_LVL:
      DEBUG_PRINT("I2C received CMD_READ_PH_LVL, lvl=");
      DEBUG_PRINTLN(ph_downer_tank_level);
      str.concat(ph_downer_tank_level);
      Wire.write(str.c_str());
      break;

    // ------------------- water pump cases 
    case CMD_READ_WATER_PUMP:
      DEBUG_PRINT("I2C received CMD_READ_WATER_PUMP, state=");
      bVal = io_handler.getWaterPumpStatus();
      DEBUG_PRINTLN(bVal);
      Wire.write(bVal);
      break;
    
    case CMD_WRITE_LOW_WATER_PUMP:
      DEBUG_PRINT("I2C received CMD_WRITE_LOW_WATER_PUMP, state=");
      io_handler.setWaterPump(0);
      bVal = io_handler.getWaterPumpStatus();
      DEBUG_PRINTLN(bVal);      
      Wire.write(bVal);
      break;

    case CMD_WRITE_HIGH_WATER_PUMP:
      DEBUG_PRINT("I2C received CMD_WRITE_HIGH_WATER_PUMP, state=");
      io_handler.setWaterPump(1);
      bVal = io_handler.getWaterPumpStatus();
      DEBUG_PRINTLN(bVal);      
      Wire.write(bVal);
      break;

    // ------------------- nutrient pump cases
    case CMD_READ_NUTRIENT_PUMP:
      DEBUG_PRINT("I2C received CMD_READ_NUTRIENT_PUMP, state=");
      bVal = io_handler.getNutrientPumpStatus();
      DEBUG_PRINTLN(bVal);
      Wire.write(bVal);
      break;
    
    case CMD_WRITE_LOW_NUTRIENT_PUMP:
      DEBUG_PRINT("I2C received CMD_WRITE_LOW_NUTRIENT_PUMP, state=");
      io_handler.setNutrientPump(0);
      bVal = io_handler.getNutrientPumpStatus();
      DEBUG_PRINTLN(bVal);
      Wire.write(bVal);
      break;
    
    case CMD_WRITE_HIGH_NUTRIENT_PUMP:
      DEBUG_PRINT("I2C received CMD_WRITE_HIGH_NUTRIENT_PUMP, state=");
      io_handler.setNutrientPump(1);
      bVal = io_handler.getNutrientPumpStatus();
      DEBUG_PRINTLN(bVal);
      Wire.write(bVal);
      break;

    // -------------------  ph downer pump cases
    case CMD_READ_PH_DOWNER_PUMP:
      DEBUG_PRINT("I2C received CMD_READ_PH_DOWNER_PUMP, state=");
      bVal = io_handler.getPhDownerPumpStatus();
      DEBUG_PRINTLN(bVal);
      Wire.write(bVal);
      break;

    case CMD_WRITE_LOW_PH_DOWNER_PUMP:
      DEBUG_PRINT("I2C received CMD_WRITE_LOW_PH_DOWNER_PUMP, state=");
      io_handler.setPHDownerPump(0);
      bVal = io_handler.getPhDownerPumpStatus();
      DEBUG_PRINTLN(bVal);
      Wire.write(bVal);
      break;

    case CMD_WRITE_HIGH_PH_DOWNER_PUMP:
      DEBUG_PRINT("I2C received CMD_WRITE_HIGH_PH_DOWNER_PUMP, state=");
      io_handler.setPHDownerPump(1);
      bVal = io_handler.getPhDownerPumpStatus();
      DEBUG_PRINTLN(bVal);
      Wire.write(bVal);
      break;

    // ------------------- mixer pump cases
    case CMD_READ_MIXER_PUMP:
      DEBUG_PRINT("I2C received CMD_WRITE_LOW_PH_DOWNER_PUMP, state=");
      bVal = io_handler.getMixerPumpStatus();
      DEBUG_PRINTLN(bVal);
      Wire.write(bVal);
      break;
    
    case CMD_WRITE_LOW_MIXER_PUMP:
      DEBUG_PRINT("I2C received CMD_WRITE_LOW_MIXER_PUMP, state=");
      io_handler.setMixerPump(0);
      bVal = io_handler.getMixerPumpStatus();
      DEBUG_PRINTLN(bVal);
      Wire.write(bVal);
      break;

    case CMD_WRITE_HIGH_MIXER_PUMP:
      DEBUG_PRINT("I2C received CMD_WRITE_HIGH_MIXER_PUMP, state=");
      io_handler.setMixerPump(1);
      bVal = io_handler.getMixerPumpStatus();
      DEBUG_PRINTLN(bVal);
      Wire.write(bVal);
      break;

    case CMD_SAFE_MODE:
      DEBUG_PRINTLN("I2C received CMD_WRITE_SAFE_MODE");
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
  INIT_SERIAL(115200);
  DEBUG_PRINT("Sampling TDS voltage");
  for (byte i=0; i <20; i++)
  {
    io_handler.getTDSVoltage();
    delay(50);
    DEBUG_PRINT(".");
  }
  DEBUG_PRINTLN(" done");

  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);  
  Wire.onRequest(requestEvent); 
  DEBUG_PRINTLN("I2C Salve is up, addr =" + String(I2C_ADDRESS));
  io_handler.initR();     // I/O Arduino mega setup digital pin mode
  lastReceiveEvent = millis();
}

void safeModHandler()
{
  /*
  Set actuator in safety mode if no receiveEvent after x seconds
  lastReceiveEvent is updated in receiveEvent();
  */
  if (millis() - lastReceiveEvent > (safetyCloseActuatorSec * 1000))
  {
      DEBUG_PRINTLN("I2C Timeout reached (" + String(safetyCloseActuatorSec) + "), safety mode activated" );
      io_handler.setWaterPump(0);
      io_handler.setNutrientPump(0);
      io_handler.setPHDownerPump(0);
      io_handler.setMixerPump(0);
      DEBUG_PRINTLN("Wait 0.5 seconds");
      delay(500);
  }
}

void loop() 
{
  safeModHandler();
  water_tank_level = io_handler.getWaterLevelCM();
  delay(50);
  nutrient_tank_level = io_handler.getNutrientLevelCM();
  delay(50);
  ph_downer_tank_level = io_handler.getPhDownerLevelCM();
  delay(50);
}
