//
// Created by jkl on 02/11/2020.
//

#include "Arduino.h"
#include "Wire.h"
#include "OGIO.h"

const int SLAVE_ADDRESS = 2;


// i2c exchange table
enum
{
	CMD_IDLE                      = 0,

  // Sensors repport commands
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

void OGIO::initR(char *nodeTag) 
{
	OGIO::nodeTag = NODE_TAG;
	Wire.begin ();   
	OGIO::sendCommand(CMD_IDLE, 1);
	if (Wire.available())
	{
		Serial.print("Slave is ID: ");
		Serial.println(Wire.read(), DEC);
	}
	else
		Serial.println("No response to ID request");
}

void OGIO::sendCommand(const byte cmd, const int responseSize)
{
	Serial.print("I2C send command >");
	Serial.println(cmd);
	Wire.beginTransmission(SLAVE_ADDRESS);
	Wire.write(cmd);
	Wire.endTransmission();
	Wire.requestFrom(SLAVE_ADDRESS, responseSize);  
}

int OGIO::readInt(const byte cmd)
{
	int val;  
	OGIO::sendCommand (cmd, 2);
	val = Wire.read ();
	val <<= 8;
	val |= Wire.read ();
	return val;
}

void OGIO::flushI2C()
{
	delay(20);
	while (Wire.available())
	{
		Wire.read();
	}
}

float OGIO::readFloat(const byte cmd)
{
	// Receive from i2c char bytes
	// and convert to float 
	byte resp_length = 10;
	char resp_buffer[resp_length] ; 
	
	OGIO::sendCommand(cmd, resp_length);

	if (Wire.available()) {
		for (byte i = 0; i < resp_length; i = i + 1) 
			{
  				resp_buffer[i] = Wire.read();
			}
	}

	OGIO::flushI2C();

	// convert to float 
	float val = atof(resp_buffer);
	Serial.print("readFloat >> ");
	Serial.println(val);

	return val;

}

byte OGIO::readByte(byte cmd)
{
	int val;
	OGIO::sendCommand(cmd, 1);
	val = Wire.read();
	return val;
}

byte OGIO::sendIdle()
{
	OGIO::idleReturn = OGIO::readByte(CMD_IDLE);
	return OGIO::idleReturn;
}

float OGIO::getPhLevelRawADC()
{
	OGIO::pHRawADC = OGIO::readInt(CMD_READ_PH);
	return OGIO::pHRawADC;
}

float OGIO::getTDSRawADC()
{
	OGIO::TDSRawADC = OGIO::readInt(CMD_READ_TDS);
	return OGIO::TDSRawADC;
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
	OGIO::PHTankLevel = OGIO::readFloat(CMD_READ_PH_DOWNER_PUMP);
	return OGIO::PHTankLevel;
}


String OGIO::generateInfluxLineProtocol()
{
	
	// read value and save as attibute
	this->getPhLevelRawADC();
	this->getTDSRawADC();
	
	this->getPhLevel();
	this->getTDS();

	this->getWaterLevelCM();
	this->getNutrientLevelCM();
	this->getPhDownerLevelCM();

	String lineProtoStr =
			"water,tag=" + String(nodeTag)
			+ " ph_voltage=" + String(OGIO::pHRawADC)+","    				// raw adc 
			+ "tds_voltage=" + String(OGIO::TDSRawADC)+","; 				// raw adc
			+ "ph=" + String(OGIO::pH)+",";                 				// real value
			+ "tds=" + String(OGIO::TDS)+",";								// real value
			+ "water_tk_lvl=" + String(OGIO::WaterTankLevel)+"i,"; 			// real level in cm
			+ "nutrient_tk_lvl=" + String(OGIO::NutrientTankLevel)+"i,";    // real level in cm
			+ "ph_downer_tk_lvl=" + String(OGIO::PHTankLevel)+"i";			// real level in cm
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