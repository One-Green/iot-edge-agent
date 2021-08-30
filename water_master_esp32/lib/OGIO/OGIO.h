//
// Created by shan on 26/08/2021.
//

class OGIO {

private:
	void sendCommand(const byte cmd, const int responseSize);
	int readInt(const byte cmd);
	void flushI2C();
	float readFloat(const byte cmd);
	int readByte(const byte cmd);

public:

	char *nodeTag;
	float pHRawADC;
	float TDSRawADC;
	float pH;
	float TDS;
	int WaterTankLevel;
	int PHTankLevel;
	int NutrientTankLevel;
	String lineProtoStr; // formatted metrics for influx db line protocol 

	void initR(char *nodeTag);

	// sensors reading 
	float getPhLevelRawADC();
	float getTDSRawADC();

	float getPhLevel();	
	float getTDS();

	int getWaterLevelCM();
	int getNutrientLevelCM();
	int getPhDownerLevelCM();

	byte OnWatrPump();
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

	String generateInfluxLineProtocol();


};

extern OGIO io_handler;
