/*
 * ESP32 Arduino Firmware compatible with water board agent
 * read from UART sent by OrangePi Zero and display to screen
 * -- optional tft screen
 * code source: https://github.com/One-Green/iot-edge-agent/tree/main/water_screen
 * uart message generator  code source (json):
 * https://github.com/One-Green/iot-edge-agent/blob/main/water/state_exporter_uart1.py
 *
 * ----------- ESP32 UART ----------------
 * UART	    RX IO	TX IO	CTS	    RTS
 * UART0	GPIO3	GPIO1	N/A	    N/A
 * UART1	GPIO9	GPIO10	GPIO6	GPIO11
 * UART2	GPIO16	GPIO17	GPIO8	GPIO7
 * source : https://circuits4you.com/2018/12/31/esp32-hardware-serial2-example/
 * Work with this framework
 * One-Green : open source framework for plant cultivation including web server and microcontroller framework
 * https://www.one-green.io
 * License : Creative Commons Legal Code - CC0 1.0 Universal
 * Author: Shanmugathas Vigneswaran
 * mail: shanmugathas.vigneswaran@outlook.fr
 *
 *
 * */

#define ARDUINOJSON_USE_DOUBLE 1 // use https://arduinojson.org/v6/assistant/ to well configure ArduinoJson
#include <ArduinoJson.h>

#define ORANGE_PI_UART_BAUD_RATE 57600


void setup(){
    Serial.begin(9600); // normal debug display serial
    Serial2.begin(ORANGE_PI_UART_BAUD_RATE);

}

void loop() {
  if (Serial2.available())
  {
    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, Serial2);

    if (err == DeserializationError::Ok)
    {
      Serial.print("nutrient_level_cm = ");
      Serial.println(doc["nutrient_level_cm"].as<int>());
      Serial.print("tds_max_level = ");
      Serial.println(doc["tds_max_level"].as<int>());
    }
    else
    {
      Serial.print("deserializeJson() returned ");
      Serial.println(err.c_str());
      while (Serial2.available() > 0)
        Serial2.read();
    }
  }
}
