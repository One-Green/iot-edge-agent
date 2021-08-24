//
// Created by jkl on 02/11/2020.
//

#ifndef NODE_SPRINKLER_ARDUINO_OGIO_H
#define NODE_SPRINKLER_ARDUINO_OGIO_H


class OGIO {

public:


	void initR(char *nodeTag);

	float getMoistureLevelADC();

	float getMoistureLevel(float in_min, float in_max, float out_min, float out_max);

	void openWaterValve();

	void closeWaterValve();

	String generateInfluxLineProtocol();

	char *nodeTag;
	float soilMoistureADC;
	float soilMoisture;

};


#endif //NODE_SPRINKLER_ARDUINO_OGIO_H
