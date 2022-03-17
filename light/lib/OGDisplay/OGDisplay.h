
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
         		bool defaultCfg,
         		bool plannerCfg,
         		String onAt,
         		String OffAt,
         		bool LightStatus);
	void printRegistryError();
	void uptime();

private:

};
