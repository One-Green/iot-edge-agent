//
// Created by Shan on 16/08/2021
//

#ifndef NODE_LIGHT_ARDUINO_OGIO_H
#define NODE_LIGHT_ARDUINO_OGIO_H


class OGIO {
public:
	void initR(char *nodeTag);
	void activateLightRelay();
	void deactivateLightRelay();
	String generateInfluxLineProtocol();
	char *nodeTag;
};


#endif //NODE_SPRINKLER_ARDUINO_OGIO_H
