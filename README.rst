IoT nodes firmware repository
============================

- All firmware are created with Platformio and Arduino framework (compatible with Atmel AVR, Espressif 32)
- Almost used board : ESP32. For complex case ESP32  + i2C (one or many) Arduino Mega are used.
- Read sensors from GPIO and write to MQTT topic
- Read controller MQTT topic (=callback) action from One-green Core (https://github.com/One-Green/core-api-gateway)
  and write to GPIO
- IoT nodes can not take actions, all actions are ordered by One-green Core
- If communication is loss (MQTT down, Wifi connexion down ...) IoT nodes will perform **Safe Mode**.

GUI Based deployment
--------------------

Use this project to deploy with an graphical interface : https://github.com/One-Green/iot-edge-wizard
