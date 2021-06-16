//
// Created by shan on 02/11/2020.
//

#ifndef NODE_SPRINKLER_ARDUINO_OGAPIHANDLER_H
#define NODE_SPRINKLER_ARDUINO_OGAPIHANDLER_H

class OGApiHandler {
public:
	String getRegistry(char *api_gateway);

	bool registerNodeTag(char *nodeTag, char *api_gateway, char *basic_auth, char *basic_pwd);
};


#endif //NODE_SPRINKLER_ARDUINO_OGAPIHANDLER_H
