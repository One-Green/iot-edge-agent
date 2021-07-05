//
// Created by shanisma on 04/07/2021.
//

// Expands macro for comparision
// see use below
#define DO_EXPAND(VAL)  VAL ## 1
#define EXPAND(VAL)     DO_EXPAND(VAL)


#define NODE_TYPE "sprinkler"

// WIFI PARAMETERS

// PlatformIO already defines WIFI_SSID and related from build_flags
// Redefining again will cause warning and empty value will be used
// so comment them

// #define WIFI_SSID ""
// #define WIFI_PASSWORD ""


// For additional info
// Throw compile time error if WIFI_SSID is not defined (if sys env is not set)
#ifndef WIFI_SSID
#error "WIFI_SSID is not defined"
#endif


// CORE API PARAMETERS
// #define API_GATEWAY_URL ""
// #define API_GATEWAY_BASIC_AUTH_USER ""
// #define API_GATEWAY_BASIC_AUTH_PASSWORD ""
// REGISTER TAG TO CORE
// #define NODE_TAG ""



// IMPORTANT: Rare case as it's neither int nor string
// Uncomment below lines if you're sure you'll pass false or true ( without qoutes )
// #ifndef CHECK_NODE_TAG_DUPLICATE
#define CHECK_NODE_TAG_DUPLICATE false
// #endif

// MQTT SERVER PARAMETERS
// #define MQTT_SERVER ""

// Use default 1883 port if MQTT_PORT is not defined
#if !defined(MQTT_PORT) || (EXPAND(MQTT_PORT) == 1)
#define MQTT_PORT 1883
#endif

// #define MQTT_USER ""
// #define MQTT_PASSWORD ""

// MQTT TOPICS
#define SENSOR_TOPIC "sprinkler/sensor"
#define SENSOR_CONTROLLER "sprinkler/controller"