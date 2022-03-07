//
// Created by shanisma on 04/07/2021.
//

// Expands macro for comparison
// see use below
#define DO_EXPAND(VAL)  VAL ## 1
#define EXPAND(VAL)     DO_EXPAND(VAL)

// Serial print configuration
// from https://forum.arduino.cc/index.php?topic=64555.0
#ifdef SERIAL_PRINT_ON
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#define INIT_SERIAL(...) Serial.begin(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#define INIT_SERIAL(...)
#endif


#define NODE_TYPE "water"

// WIFI Parameters
#ifndef WIFI_SSID
#error "WIFI_SSID is not defined"
#endif

#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD is not defined"
#endif

// MQTT Parameters
#ifndef MQTT_SERVER
#error "MQTT_SERVER is not defined"
#endif

// Use default 1883 port if MQTT_PORT is not defined
#if !defined(MQTT_PORT) || (EXPAND(MQTT_PORT) == 1)
#define MQTT_PORT 1883
#endif

#ifndef MQTT_USER
#error "MQTT_USER is not defined"
#endif

#ifndef MQTT_PASSWORD
#error "MQTT_PASSWORD is not defined"
#endif

#define SENSOR_TOPIC "sprinkler/sensor"
#ifndef CONTROLLER_TOPIC
#error "CONTROLLER_TOPIC is not defined"
#endif

