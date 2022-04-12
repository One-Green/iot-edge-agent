//
// Created by Shanmugathas Vigneswaran on 07/03/2022.
//

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
