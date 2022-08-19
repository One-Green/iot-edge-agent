#include "SPI.h"
#include "TFT_eSPI.h"
#include "OGDisplay.h"

#define SCREENWIDTH 480
#define SCREENHEIGHT 320

#define BGCOLOR    0xAD75
#define GRIDCOLOR  0xA815
#define BGSHADOW   0x5285
#define GRIDSHADOW 0x600C
#define RED        0xF800
#define WHITE      0xFFFF

// SPI Configuration is defined in:
// water/master_esp32/.pio/libdeps/esp32dev/TFT_eSPI/User_Setup.h
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

void DisplayLib::initR() {
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(WHITE);

    tft.setCursor(0, 0, 2);
    tft.setTextColor(TFT_BLACK,WHITE);
    tft.setTextSize(2);
    tft.println("Screen initialised");
}


void DisplayLib::initWifi() {
    tft.println("Connecting to WIFI");
}


void DisplayLib::connectedWifi() {
	tft.println("Connected to WIFI");
}


void DisplayLib::printHeader(
		char *wifiSsid,
		IPAddress ip,
		char *nodeType,
		char *nodeTag
		)
{

    tft.fillRect(0, 0, SCREENWIDTH, 60, TFT_WHITE);
    tft.fillRect(0, 60, SCREENWIDTH, SCREENHEIGHT, TFT_GREEN);
    tft.setTextSize(1);
    tft.setTextColor(TFT_BLACK);

    // print wifi SSID
    tft.setCursor(2, 2);
    tft.print("WIFI: ");
    tft.setCursor(35, 2);
    String ssid = String(wifiSsid);
    tft.print(ssid);

    // print IP
    tft.setCursor(2, 15);
    tft.print("IP: " + ip2Str(ip));

    // print node type
    tft.setCursor(2, 30);
    tft.print("Node Type: ");
    tft.print(nodeType);

    // print node tag
    tft.setCursor(2, 45);
    tft.print("Node Tag: ");
    tft.print(nodeTag);

}


void DisplayLib::printTemplate() {

    tft.setCursor(2, 60);
    tft.print("Water tk (%): ");

    tft.setCursor(2, 75);
    tft.print("Nutr. tk (%): ");

    tft.setCursor(2, 90);
    tft.print("pH dwn. tk (%): ");

    tft.setCursor(2, 105);
    tft.print("TDS (V): ");
    tft.setCursor(2, 120);
    tft.print("TDS (ppm): ");

    tft.setCursor(2, 135);
    tft.print("pH (V): ");
    tft.setCursor(2, 150);
    tft.print("pH value: ");

    tft.setCursor(2, 165);
    tft.print("Water pump status: ");

    tft.setCursor(2, 180);
    tft.print("Nutrient pump status: ");

    tft.setCursor(2, 195);
    tft.print("pH Downer pump status: ");

    tft.setCursor(2, 210);
    tft.print("Mixer pump status: ");

    tft.drawLine(SCREENWIDTH/2,60,SCREENWIDTH/2,SCREENHEIGHT,TFT_BLACK);

    tft.setCursor(245, 60);
    tft.print("Connected sprinklers: ");

    tft.setCursor(245, 75);
    tft.print("TDS config min (ppm): ");
    tft.setCursor(245, 90);
    tft.print("TDS config max (ppm): ");

    tft.setCursor(245, 105);
    tft.print("pH config min: ");
    tft.setCursor(245, 120);
    tft.print("pH config max: ");

}


void DisplayLib::updateDisplay(
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
		)
		{

    // flush values
    tft.fillRect(150, 60, 85, SCREENHEIGHT, TFT_GREEN);
    tft.fillRect(390, 60, 85, SCREENHEIGHT, TFT_GREEN);

    // left pan
    tft.setCursor(152, 60);
    tft.print(water_tank_lvl_cm);

    tft.setCursor(152, 75);
    tft.print(nutrient_tank_lvl_cm);

    tft.setCursor(152, 90);
    tft.print(ph_downer_tank_lvl_cm);

    tft.setCursor(152, 105);
    tft.print(tds_voltage);
    tft.setCursor(152, 120);
    tft.print(tds_ppm);

    tft.setCursor(152, 135);
    tft.print(ph_voltage);
    tft.setCursor(152, 150);
    tft.print(ph);

    tft.setCursor(152, 165);
    if (water_pump_status){
        tft.print("OPEN");
    }
    else{
        tft.print("CLOSE");
    }

    tft.setCursor(152, 180);
    if (nutrient_pump_status){
        tft.print("OPEN");
    }
    else{
        tft.print("CLOSE");
    }

    tft.setCursor(152, 195);
    if (ph_downer_pump_status){
        tft.print("OPEN");
    }
    else{
        tft.print("CLOSE");
    }

    tft.setCursor(152, 210);
    if (mixer_pump_status){
        tft.print("OPEN");
    }
    else{
        tft.print("CLOSE");
    }

    // right pan
    tft.setCursor(392, 60);
    tft.print(connected_sprinkler);

    tft.setCursor(392, 75);
    tft.print(tds_min);
    tft.setCursor(392, 90);
    tft.print(tds_max);

    tft.setCursor(392, 105);
    tft.print(ph_min);
    tft.setCursor(392, 120);
    tft.print(ph_max);

}


String DisplayLib::ip2Str(IPAddress ip) {
	// took from https://gist.github.com/loosak/76019faaefd5409fca67
	String s = "";
	for (int i = 0; i < 4; i++) {
		s += i ? "." + String(ip[i]) : String(ip[i]);
	}
	return s;
}


void DisplayLib::uptime()
{
	long days=0;
	long hours=0;
	long mins=0;
	long secs=0;
	secs = millis()/1000;
	mins=secs/60; //convert seconds to minutes
	hours=mins/60; //convert minutes to hours
	days=hours/24; //convert hours to days
	secs=secs-(mins*60); //subtract the converted seconds to minutes in order to display 59 secs max
	mins=mins-(hours*60); //subtract the converted minutes to hours in order to display 59 minutes max
	hours=hours-(days*24); //subtract the converted hours to days in order to display 23 hours max

}
