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
         		float tds_voltage,
         		float tds_ppm,
         		float ph_voltage,
         		float ph,
         		bool water_pump_status,
         		bool nutrient_pump_status,
         		bool ph_downer_pump_status,
         		bool mixer_pump_status,

         		int connected_sprinkler,
         		float tds_min,
         		float tds_max,
         		float ph_min,
         		float ph_max
         		);

	void uptime();



private:

};
