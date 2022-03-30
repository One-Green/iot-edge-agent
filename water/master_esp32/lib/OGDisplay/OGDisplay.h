class DisplayLib {

public:
	// constructor
	void initR();

	void initWifi();

	void connectedWifi();

	void printHeader(char *wifiSsid, IPAddress ip, char *nodeType, char *nodeTag);

	String ip2Str(IPAddress ip);

	void printTemplate();

	void updateDisplay(
         		int water_tank_lvl_cm,
         		int nutrient_tank_lvl_cm,
         		int ph_downer_tank_lvl_cm,
         		bool water_pump_status);

	void uptime();



private:

};
