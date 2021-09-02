Water firmware
=============

Features of this IoT node:

Inputs :

- ph measurement
- Tds measurement
- Ultrasonic water tank level measurement
- Ultrasonic nutrient tank level measurement
- Ultrasonic ph downer tank level measurement

Outputs :

- Sprinkler water supply pump relay
- Water Tank mixing pump relay
- Nutrient metering pump relay
- Ph downer metering pump relay
- ESP32 and Mega work through i2c protocol.

**ESP32** handle MQTT publish/subscription with One-Green Core.

**Arduino Mega** handle hardware inputs/outputs and report to **ESP32**.

Folder **master_esp32** and **i2c_slave_mega** platformio including README.rst
