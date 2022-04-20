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


Manual flash
------------

Set .env file with these values

.. code-block:: shell

    WIFI_SSID=
    WIFI_PASSWORD=
    MQTT_SERVER=
    MQTT_PORT=
    MQTT_USER=
    MQTT_PASSWORD=
    # use SERIAL_PRINT_ON to activate serial print
    # else use any variable to deactivate
    SERIAL_PRINT_VAR=SERIAL_PRINT_ON
    NODE_TAG=

Use Makefile cmd to flash