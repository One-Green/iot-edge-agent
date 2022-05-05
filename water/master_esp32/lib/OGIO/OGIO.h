//
// Created by shan on 26/08/2021.
//

class OGIO {

private:
	int readInt(int cmd);
	float readFloat(int cmd);
	byte readByte(int cmd);
	void sendCommand (const byte cmd, const int responseSize);

public:
	String nodeTag;
	float phVoltage;
	float TDSVoltage;
	float pH;
	float TDS;
	float WaterTankLevel;
	float PHTankLevel;
	float NutrientTankLevel;
	String lineProtoStr; // formatted metrics for influx db line protocol 

	byte sendIdle();
    byte idleReturn;

	void initR(char *nodeTag);

	// sensors reading 
	float getPhVoltage();
	float getTDSVoltage();

	float getPhLevel();	
	float getTDS();

	float getWaterLevelCM();
	float getNutrientLevelCM();
	float getPhDownerLevelCM();

	String generateInfluxLineProtocol();
	
	// actuators methods 
	byte WaterPumpStatus;
	byte NutrientPumpStatus;
	byte PhDownerPumpSatus;
	byte MixerPumpStatus; 

	byte OnWaterPump();
	byte OffWaterPump();
	byte getWaterPumpStatus();

	byte OnNutrientPump();
	byte OffNutrientPump();
	byte getNutrientPumpStatus();

	byte OnPhDownerPump();
	byte OffPhDownerPump();
	byte getPhDownerPumpStatus();
	
	byte OnMixerPump();
	byte OffMixerPump();
	byte getMixerPumpStatus();

	// safe mode att , methods
	byte safeModeStatus;
	byte safeMode();
};

extern OGIO io_handler;
