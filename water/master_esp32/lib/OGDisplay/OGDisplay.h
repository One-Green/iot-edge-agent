#include "Arduino.h"
#include "SPI.h"
#include "WiFi.h"

class DisplayLib {

public:
	// constructor
	void initR();

	void drawtext(char *text, uint16_t color);

	void initWifi();

	void connectedWifi();

	void printHeader(char *wifiSsid, IPAddress ip, char *nodeType, char *nodeTag);

	String ip2Str(IPAddress ip);

	void printTemplate();

	void updateDisplay(float moistureLevelADC, float moistureLevel,
					float configMin, float configMax ,
					bool water_valve_signal);

	void printRegistryError();

	void uptime();



private:

};
