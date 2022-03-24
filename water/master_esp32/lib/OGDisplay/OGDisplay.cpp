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
	// print node tag
//	tft.setCursor(2, 50);
//	tft.print("Moisture (Raw): ");
//	tft.setCursor(2, 60);
//	tft.print("Moisture (%)  : ");
//	tft.setCursor(2, 70);
//	tft.print("Config Min (%): ");
//	tft.setCursor(2, 80);
//	tft.print("Config Max (%): ");
//	tft.setCursor(2, 90);
//	tft.print("Valve status  : ");
}


void DisplayLib::updateDisplay(
		float moistureLevelADC,
		float moistureLevel,
		float configMin,
		float configMax,
		bool water_valve_signal){

	// tft.fillRect(90, 50, 128, 50, ST7735_GREEN);

	tft.setCursor(95, 50);
	tft.print((int) moistureLevelADC);

	tft.setCursor(95, 60);
	tft.print((int) moistureLevel);

	tft.setCursor(95, 70);
	tft.print((int) configMin);

	tft.setCursor(95, 80);
	tft.print((int) configMax);

	tft.setCursor(95, 90);
	if (water_valve_signal){
		tft.print("OPEN");
	}
	else{
		tft.print("CLOSE");
	}

}

void DisplayLib::drawtext(char *text, uint16_t color) {
	//tft.fillScreen(ST7735_BLACK);
	tft.setCursor(0, 0);
	tft.setTextColor(color);
	tft.setTextWrap(true);
	tft.print(text);
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
