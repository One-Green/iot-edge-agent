//
// Created by jkl on 02/11/2020.
//

#include "Arduino.h"
#include "Wire.h"
#include "OGIO.h"
#include "Config.h"
#include "I2C_Anything.h"

const int SLAVE_ADDRESS = 2;


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

void OGIO::initR(char *nodeTag) 
{
	OGIO::nodeTag = NODE_TAG;
	Wire.begin();
	delay(200);
	byte slave_addr = OGIO::sendIdle();
	if (slave_addr == 2)
	{
		DEBUG_PRINT("[I2C] Slave is connected at adr: ");DEBUG_PRINTLN(slave_addr);
	}
	else
		DEBUG_PRINTLN("[I2C] no response from slave !!");
    OGIO::safeMode();
}

void OGIO::sendCommand(const byte cmd, const int responseSize)
{
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write(cmd);
    if (Wire.endTransmission() == 0){DEBUG_PRINT("[I2C] CMD SEND SUCCESS="); DEBUG_PRINTLN(cmd);}
    Wire.requestFrom(SLAVE_ADDRESS, responseSize);
}

int OGIO::readInt(int cmd)
{
    int val;
    OGIO::sendCommand(cmd, 1);
    I2C_readAnything(val);
    return val ;
}

float OGIO::readFloat(int cmd)
{
    float val;
    OGIO::sendCommand(cmd, 4);
    I2C_readAnything(val);
    return val ;
}

byte OGIO::readByte(int cmd)
{
    byte val;
    OGIO::sendCommand(cmd, 1);
    val = Wire.read();
    return val ;
}

byte OGIO::sendIdle()
{
	OGIO::idleReturn = OGIO::readByte(CMD_IDLE);
	return OGIO::idleReturn;
}

float OGIO::getPhVoltage()
{
	OGIO::phVoltage = OGIO::readFloat(CMD_READ_PH_VOLTAGE);
	return OGIO::phVoltage;
}

float OGIO::getTDSVoltage()
{
	OGIO::TDSVoltage = OGIO::readFloat(CMD_READ_TDS_VOLTAGE);
	return OGIO::TDSVoltage;
}

float OGIO::getPhLevel()
{
	OGIO::pH = OGIO::readFloat(CMD_READ_PH);
	return OGIO::pH;
}

float OGIO::getTDS()
{
	OGIO::TDS = OGIO::readFloat(CMD_READ_TDS);
	return OGIO::TDS;
}

float OGIO::getWaterLevelCM()
{
	OGIO::WaterTankLevel = OGIO::readFloat(CMD_READ_WATER_LVL);
	return OGIO::WaterTankLevel;

}

float OGIO::getNutrientLevelCM()
{
	OGIO::NutrientTankLevel = OGIO::readFloat(CMD_READ_NUTRIENT_LVL);
	return OGIO::NutrientTankLevel;
}

float OGIO::getPhDownerLevelCM()
{
	OGIO::PHTankLevel = OGIO::readFloat(CMD_READ_PH_LVL);
	return OGIO::PHTankLevel;
}

int OGIO::getWaterLevelPercent()
{
    OGIO::getWaterLevelCM() ;
    OGIO::WaterTankLevelPercent = map( (int)OGIO::WaterTankLevel, 5, WATER_TANK_HEIGHT_CM, 100, 0 ) ;
    return OGIO::WaterTankLevelPercent ;
}

int OGIO::getNutrientLevelPercent()
{
    OGIO::getNutrientLevelCM() ;
    OGIO::NutrientTankLevelPercent = map( (int)OGIO::NutrientTankLevel, 5, NUTRIENT_TANK_HEIGHT_CM, 100, 0 ) ;
    return OGIO::NutrientTankLevelPercent ;
}

int OGIO::getPhDownerLevelPercent()
{
    OGIO::getPhDownerLevelCM() ;
    OGIO::PHTankLevelPercent = map( (int)OGIO::PHTankLevel, 5, PH_DOWNER_TANK_HEIGHT_CM, 100, 0 ) ;
    return OGIO::PHTankLevelPercent ;
}


String OGIO::generateInfluxLineProtocol()
{
	
	// read value and save as attribute
	this->getPhVoltage();
	this->getTDSVoltage();
	
	this->getPhLevel();
	this->getTDS();

	this->getWaterLevelPercent();
	this->getNutrientLevelPercent();
	this->getPhDownerLevelPercent();

	String lineProtoStr =
		"water,tag=" + OGIO::nodeTag
		+ " ph_voltage=" + String(OGIO::phVoltage)+","    			                // raw adc 
		+ "tds_voltage=" + String(OGIO::TDSVoltage)+"," 				 		    // raw adc
		+ "ph_level=" + String(OGIO::pH)+","                 					    // real value
		+ "tds_level=" + String(OGIO::TDS)+","										// real value
		+ "water_tk_lvl=" + String((unsigned int) OGIO::WaterTankLevelPercent)+"i," 		// real level in cm
		+ "nutrient_tk_lvl=" + String((unsigned int) OGIO::NutrientTankLevelPercent)+"i,"  // real level in cm
		+ "ph_downer_tk_lvl=" + String((unsigned int )OGIO::PHTankLevelPercent)+"i";		// real level in cm
	return lineProtoStr ;
}

// Water pump methods
byte OGIO::OnWaterPump()
{
	OGIO::WaterPumpStatus = OGIO::readByte(CMD_WRITE_HIGH_WATER_PUMP);
	return OGIO::WaterPumpStatus;
}

byte OGIO::OffWaterPump()
{
	OGIO::WaterPumpStatus = OGIO::readByte(CMD_WRITE_LOW_WATER_PUMP);
	return OGIO::WaterPumpStatus;
}

byte OGIO::getWaterPumpStatus()
{
	OGIO::WaterPumpStatus = OGIO::readByte(CMD_READ_WATER_PUMP);
	return OGIO::WaterPumpStatus;
}

// Nutirent pump methods
byte OGIO::OnNutrientPump()
{
	OGIO::NutrientPumpStatus = OGIO::readByte(CMD_WRITE_HIGH_NUTRIENT_PUMP);
	return OGIO::NutrientPumpStatus;
}

byte OGIO::OffNutrientPump()
{
	OGIO::NutrientPumpStatus = OGIO::readByte(CMD_WRITE_LOW_NUTRIENT_PUMP);
	return OGIO::NutrientPumpStatus;
}

byte OGIO::getNutrientPumpStatus()
{
	OGIO::NutrientPumpStatus = OGIO::readByte(CMD_READ_NUTRIENT_PUMP);
	return OGIO::NutrientPumpStatus;
}

// Ph downer pump methods
byte OGIO::OnPhDownerPump()
{
	OGIO::PhDownerPumpSatus = OGIO::readByte(CMD_WRITE_HIGH_PH_DOWNER_PUMP);
	return OGIO::PhDownerPumpSatus;
}

byte OGIO::OffPhDownerPump()
{
	OGIO::PhDownerPumpSatus = OGIO::readByte(CMD_WRITE_LOW_PH_DOWNER_PUMP);
	return OGIO::PhDownerPumpSatus;
}

byte OGIO::getPhDownerPumpStatus()
{
	OGIO::PhDownerPumpSatus = OGIO::readByte(CMD_READ_PH_DOWNER_PUMP);
	return OGIO::PhDownerPumpSatus;
}

// mixer pump methods
byte OGIO::OnMixerPump()
{
	OGIO::MixerPumpStatus = OGIO::readByte(CMD_WRITE_HIGH_MIXER_PUMP);
	return OGIO::MixerPumpStatus;
}

byte OGIO::OffMixerPump()
{
	OGIO::MixerPumpStatus = OGIO::readByte(CMD_WRITE_LOW_MIXER_PUMP);
	return OGIO::MixerPumpStatus;
}

byte OGIO::getMixerPumpStatus()
{
	OGIO::MixerPumpStatus = OGIO::readByte(CMD_READ_MIXER_PUMP);
	return OGIO::MixerPumpStatus;
}

byte OGIO::safeMode()
{
	OGIO::safeModeStatus = OGIO::readByte(CMD_SAFE_MODE);
	return OGIO::safeModeStatus;
}

OGIO io_handler;
