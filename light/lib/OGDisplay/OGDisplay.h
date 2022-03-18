
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
	void updateDisplay(
         		String cfgType,
         		String onAt,
         		String OffAt,
         		float LuxLevel,
         		int PhotoResistorRaw,
         		int PhotoResistorPercent,
         		bool LightStatus
         		);
	void printRegistryError();
	void uptime();

private:

};
