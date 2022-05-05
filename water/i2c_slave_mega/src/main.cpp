/*
Arduino Mega i2c slave
https://one-green.io
Author: Shanmugathas Vigneswaran
*/


#include "Arduino.h"
#include "Wire.h"
#include "OGIO.h"
#include "I2C_Anything.h"
#include "Config.h"
#include "avr/wdt.h"

// Safety consideration: if no I2C receiveEvent after safetyCloseActuatorSec second
unsigned long lastReceiveEvent;
const int safetyCloseActuatorSec = 2; // close actuators if no request from I2C Master

// refresh sensors timer
unsigned int refresh_bouncer=millis() ; // millis() holder
const int REFRESH_TIME=500 ; // refresh every 500 millis

// i2c config
const byte I2C_ADDRESS = 2;
volatile boolean haveData = false;
int command;
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


// og flow variables
float waterTankLevel;
float nutrientTankLevel;
float phDownerTankLevel;
float phVoltage;
float tdsVoltage;
float phLevel;
float tdsLevel;
bool  waterPumpStatus;
bool  nutrientPumpStatus;
bool  phDownerPumpStatus;
bool  mixerPumpStatus;

void receiveEvent(int howMany)
{
    command = Wire.read();
    haveData = true;
    lastReceiveEvent = millis();
}



void requestEvent()
{
    switch (command)
    {
    // ------------------- sensors cases
    case CMD_IDLE: Wire.write(2); break;
    case CMD_READ_PH_VOLTAGE: I2C_writeAnything(phVoltage); break;
    case CMD_READ_TDS_VOLTAGE: I2C_writeAnything(tdsVoltage); break;
    case CMD_READ_PH: I2C_writeAnything(phLevel); break;
    case CMD_READ_TDS: I2C_writeAnything(tdsLevel); break;
    case CMD_READ_WATER_LVL: I2C_writeAnything(waterTankLevel); break;
    case CMD_READ_NUTRIENT_LVL: I2C_writeAnything(nutrientTankLevel); break;
    case CMD_READ_PH_LVL: I2C_writeAnything(phDownerTankLevel); break;

    // ------------------- water pump cases
    case CMD_READ_WATER_PUMP: I2C_writeAnything(io_handler.getWaterPumpStatus()); break;
    case CMD_WRITE_LOW_WATER_PUMP:
        io_handler.setWaterPump(0);
        I2C_writeAnything(io_handler.getWaterPumpStatus());
        break;
    case CMD_WRITE_HIGH_WATER_PUMP:
        io_handler.setWaterPump(1);
        I2C_writeAnything(io_handler.getWaterPumpStatus());
        break;

    // ------------------- nutrient pump cases
    case CMD_READ_NUTRIENT_PUMP: I2C_writeAnything(io_handler.getNutrientPumpStatus()); break;
    case CMD_WRITE_LOW_NUTRIENT_PUMP:
        io_handler.setNutrientPump(0);
        I2C_writeAnything(io_handler.getNutrientPumpStatus());
        break;
    case CMD_WRITE_HIGH_NUTRIENT_PUMP:
        io_handler.setNutrientPump(1);
        I2C_writeAnything(io_handler.getNutrientPumpStatus());
        break;

    // -------------------  ph downer pump cases
    case CMD_READ_PH_DOWNER_PUMP: I2C_writeAnything(io_handler.getPhDownerPumpStatus()); break;
    case CMD_WRITE_LOW_PH_DOWNER_PUMP:
        io_handler.setPHDownerPump(0);
        I2C_writeAnything(io_handler.getPhDownerPumpStatus());
        break;
    case CMD_WRITE_HIGH_PH_DOWNER_PUMP:
        io_handler.setPHDownerPump(1);
        I2C_writeAnything(io_handler.getPhDownerPumpStatus());
        break;

    // ------------------- mixer pump cases
    case CMD_READ_MIXER_PUMP: I2C_writeAnything(io_handler.getMixerPumpStatus()); break;
    case CMD_WRITE_LOW_MIXER_PUMP:
        io_handler.setMixerPump(0);
        I2C_writeAnything(io_handler.getMixerPumpStatus());
        break;
    case CMD_WRITE_HIGH_MIXER_PUMP:
        io_handler.setMixerPump(1);
        I2C_writeAnything(io_handler.getMixerPumpStatus());
        break;

    case CMD_SAFE_MODE:
        io_handler.setWaterPump(0);
        io_handler.setNutrientPump(0);
        io_handler.setPHDownerPump(0);
        io_handler.setMixerPump(0);
        I2C_writeAnything(1);
        break;
    }

}  


void setup() 
{
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

    wdt_enable(WDTO_2S);
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

    if ( (millis() - refresh_bouncer) > REFRESH_TIME )
    {
        phVoltage  = io_handler.getPhVoltage();
        tdsVoltage = io_handler.getTDSVoltage();
        phLevel    = io_handler.getPhLevel();
        tdsLevel   = io_handler.getTDS();

        waterTankLevel = io_handler.getWaterLevelCM(); delay(50);
        nutrientTankLevel = io_handler.getNutrientLevelCM(); delay(50);
        phDownerTankLevel = io_handler.getPhDownerLevelCM(); delay(50);

        refresh_bouncer = millis();
        DEBUG_PRINTLN("REFRESH SENSORS");
    }

    if (haveData)
    {
        DEBUG_PRINT("RECEIVED COMMAND=");DEBUG_PRINTLN(command);
        haveData = false;
    }

    wdt_reset();
}
