ESP32 firmware 
==============

One-Green Core interaction :

- MQTT Publish to topic: **SENSOR_TOPIC** = "water/sensor".
- MQTT Subscribe from topic: **CONTROLLER_TOPIC** = "water/controller".

MQTT topics configuration in **lib/Config/Config.h**


MQTT Publish
============

**SENSOR_TOPIC** message format : InfluxDb Line Protocol 

Soruce : **lib/OGIO/OGIO.cpp**

.. code-block:: cpp

    String OGIO::generateInfluxLineProtocol()
    {
        // [ ... ]  arduino mega interaction
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


MQTT Subscribe
==============

WIP


I2C Interaction with Arduino Mega 
=================================

Registry selection is defined in **lib/OGIO/OGIO.cpp** . 

The same exchange table is defined at Arduino Mega firmware **water/i2c_slave_mega/src/main.cpp**

.. code-block:: cpp

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
