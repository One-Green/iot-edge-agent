/*
 *  One-Green node type: water+nutrient+pH
 *  Water supplier sprinkler nodes
 *  ESP32 board firmware
 *  Purpose:
 *      - Interact with mega-2560 board for Inputs/Outputs
 *      - Interact with master trough MQTT
 *      - Interact with ST7735 screen
 *
 * ----------- ESP32 UART ----------------
 * UART	    RX IO	TX IO	CTS	    RTS
 * UART0	GPIO3	GPIO1	N/A	    N/A
 * UART1	GPIO9	GPIO10	GPIO6	GPIO11
 * UART2	GPIO16	GPIO17	GPIO8	GPIO7
 * source : https://circuits4you.com/2018/12/31/esp32-hardware-serial2-example/
 * ----------------------------------------
 * Wiring :
 *  ESP32           Mega
 *  GPIO16 (RX) ->  D11 (TX)
 *  GPIO17 (TX) ->  D10 (RX)
 *
 * Author: Shanmugathas Vigneswaran
 * email: shanmugathas.vigneswaran@outlook.fr
 * */

#include "ArduinoJson.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "PubSubClient.h"
#include "Config.h"
#include "OGIO.h"
#include "OGDisplay.h"


unsigned long pubSensorTimer;
unsigned long subControllerTimer;
const int sensorPubRateSec = 2; //send sensor values each 10 sec
const int safetyCloseActuatorSec = 5; //close actuators if no response from Master
bool mqttCallbackInError = false; // if callback received = false, if safetyCloseActuatorSec = true

// Parameter from Master, provided by MQTT JSON
bool ctl_water_pump = false;
bool ctl_nutrient_pump = false;
bool ctl_ph_downer_pump = false;
bool ctl_mixer_pump = false;
int ctl_ph_level_min = 0;
int ctl_ph_level_max = 0;
int ctl_tds_level_min = 0;
int ctl_tds_level_max = 0;

// ----------------------------------   // Sensors Regs Values
int water_level_cm = 0;
int nutrient_level_cm = 0;
int ph_downer_level_cm = 0;
int ph_level = 0;
int tds_level = 0;

// ----------------------------------   // Actuators
bool last_water_pump_state = false;
bool last_nutrient_pump_state = false;
bool last_ph_downer_pump_state = false;
bool last_mixer_pump_state = false;

// ---------------------------------- // Class(es) instantiation(s)
WiFiClient espClient;
PubSubClient client(espClient);
DisplayLib displayLib;


// methods


void connectToWiFiNetwork()
{
    Serial.print("[WIFI] Connecting to ");
    Serial.print(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println("\n");
    displayLib.connectedWifi();
    Serial.println("[WIFI] Connected ");
    Serial.print("[WIFI] IP address: ");
    Serial.println(WiFi.localIP());
}

/* MQTT Functions */

void reconnect_mqtt() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("[MQTT] Attempting connection... with client name = ");
        String client_name = String(NODE_TYPE) + "-" + String(NODE_TYPE);
        int clt_len = client_name.length() + 1;
        char clt_name_char[clt_len];
        client_name.toCharArray(clt_name_char, clt_len);
        Serial.println(clt_name_char);

        // Attempt to connect
        if (client.connect(clt_name_char, MQTT_USER, MQTT_PASSWORD)) {
            Serial.println("[MQTT] Client connected");
            // Subscribe
            client.subscribe(CONTROLLER_TOPIC);
        } else {
            Serial.print("[MQTT] failed, rc=");
            Serial.print(client.state());
            Serial.println("[MQTT] try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}


void mqttCallback(char *topic, byte *message, unsigned int length) {
    /*
     * Read JSON from controller through MQTT
     * and parse to Arduino datatype
     *
     * */
    mqttCallbackInError = false;
    subControllerTimer = millis();
    Serial.print("[MQTT] Receiving << on topic: ");
    Serial.print(topic);
    Serial.print(". JSON message: ");
    String tmpMessage;

    for (int i = 0; i < length; i++) {
        tmpMessage += (char) message[i];
    }

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, tmpMessage);
    JsonObject obj = doc.as<JsonObject>();
    serializeJsonPretty(doc, Serial);
    Serial.println();

    /* Parse params from MQTT Payload*/

    //Parsing Tag param
    String tag = obj[String("tag")];

    //Parsing Water Pumps param
    ctl_water_pump = obj[String("water_pump_signal")];
    //Parsing Nutrient Pumps param
    ctl_nutrient_pump = obj[String("nutrient_pump_signal")];
    //Parsing PH Downer Pumps param
    ctl_ph_downer_pump = obj[String("ph_downer_pump_signal")];

    // TODO: how to handle mixer ?
    // ctl_mixer_pump = obj[String("mixer_pump")];

    ctl_ph_level_min = obj[String("ph_min_level")];
    ctl_ph_level_max = obj[String("ph_max_level")];
    ctl_tds_level_min = obj[String("tds_min_level")];
    ctl_tds_level_max = obj[String("tds_max_level")];

    // TODO : add actuator flow below
    if (tag != NODE_TAG) return;
    
    unsigned long timeoutCount = millis();
    bool setPumpState = false;
    bool showMessage = true;

    // Handle water pump
    if (ctl_water_pump != last_water_pump_state) {
        last_water_pump_state = ctl_water_pump;
        if (ctl_water_pump) //turn-ON and confirm 
        { 
            while ( (millis() - timeoutCount < 3000 ) && !setPumpState)
            {
                setPumpState = io_handler.OnWaterPump();
                if (showMessage)Serial.println("[I/O] Waiting for water pump activation");
                showMessage = false;
                delay(500);
            }
            Serial.println("[I/O] Water pump is OPENED");
        } 
        else //turn-OFF and confirm 
        {
            timeoutCount = millis();
            setPumpState = false;
            showMessage = true;
            while ((millis() - timeoutCount < 3000 ) && !setPumpState) 
            { 
                setPumpState = io_handler.OffWaterPump();
                if (showMessage)Serial.println("[I/O] Waiting for water pump closing");
                showMessage = false;
                delay(500);
            }
            Serial.println("[I/O] Water pump is CLOSED");
        }
    }
    
    // handle nutrient pump
    if (ctl_nutrient_pump != last_nutrient_pump_state) {
        last_nutrient_pump_state = ctl_nutrient_pump;
        if (ctl_nutrient_pump) {
            timeoutCount = millis();
            setPumpState = false;
            showMessage = true;
            while ((millis() - timeoutCount < 3000 ) && !setPumpState)
            {
                setPumpState = io_handler.OnNutrientPump();
                if (showMessage)Serial.println("[I/O] Waiting for nutrient pump activation");
                showMessage = false;
                delay(500);
            }
            Serial.println("[I/O] Nutrient pump is OPENED");
        } else {
            timeoutCount = millis();
            setPumpState = false;
            showMessage = true;
            while ((millis() - timeoutCount < 3000 ) && !setPumpState)
            {
                setPumpState = io_handler.OffNutrientPump();
                if (showMessage)Serial.println("[I/O] Waiting for nutrient pump closing");
                showMessage = false;
                delay(500);
            }
            Serial.println("[I/O] Nutrient pump is CLOSED");
        }
    }

    // handle pH downer pump
    if (ctl_ph_downer_pump != last_ph_downer_pump_state) 
    {
        last_ph_downer_pump_state = ctl_ph_downer_pump;

        if (ctl_ph_downer_pump) 
        {
            timeoutCount = millis();
            setPumpState = false;
            showMessage = true;
            while ((millis() - timeoutCount < 3000 ) && !setPumpState)
            {
                setPumpState = io_handler.OnPhDownerPump();
                if (showMessage) Serial.println("[I/O] Waiting for pH downer pump activation");
                showMessage = false;
                delay(500);
            }
            Serial.println("[I/O] pH downer pump is OPENED");
        } 
        else
        {
            timeoutCount = millis();
            setPumpState = false;
            showMessage = true;
            while ((millis() - timeoutCount < 3000 ) && !setPumpState)
            {
                setPumpState = io_handler.OffPhDownerPump();
                if (showMessage) Serial.println("[I/O] Waiting for pH downer pump closing");
                showMessage = false;
                delay(500);
            }
            Serial.println("[I/O] pH downer pump is CLOSED");
        }
    }

    // handle mixer pump
    if (ctl_mixer_pump != last_mixer_pump_state)
    {
        last_mixer_pump_state = ctl_mixer_pump;

        if (ctl_mixer_pump)
        {
            timeoutCount = millis();
            setPumpState = false;
            showMessage = true;
            while ((millis() - timeoutCount < 3000 ) && !setPumpState) 
            {
                setPumpState = io_handler.OnMixerPump();
                if (showMessage)Serial.println("[I/O] Waiting for mixer pump activation");
                showMessage = false;
                delay(500);
            }
            Serial.println("[I/O] mixer pump is OPENED");
        }
        else
        {
            timeoutCount = millis();
            setPumpState = false;
            showMessage = true;
            while ((millis() - timeoutCount < 3000 ) && !setPumpState)
            {
                setPumpState = io_handler.OffMixerPump();
                if (showMessage)Serial.println("[I/O] Waiting for mixer pump closing");
                showMessage = false;
                delay(500);
            }
            Serial.println("[I/O] mixer pump is CLOSED");
        }
    }
}

void pubSensorsVals()
{
    String line_proto = io_handler.generateInfluxLineProtocol();
    Serial.println(line_proto);
    // convert string to char and publish to mqtt
    int line_proto_len = line_proto.length() + 1;
    char line_proto_char[line_proto_len];
    line_proto.toCharArray(line_proto_char, line_proto_len);
    client.publish(SENSOR_TOPIC, line_proto_char);
}


void setup() {

    // Serial Ports Init 
    Serial.begin(9600);
    Serial.println("Serial Begin OK");

    io_handler.initR(NODE_TAG);
    io_handler.getWaterLevelCM();

    // Display Init
    // displayLib.initR();
    // displayLib.initWifi();

    // Connect to WiFi 
    // connectToWiFiNetwork();

    // Display WiFi Info
    // displayLib.printHeader(WIFI_SSID, WiFi.localIP(), NODE_TYPE, NODE_TAG);
    // displayLib.printTemplate();

    /* MQTT connexion */
    // client.setServer(MQTT_SERVER, MQTT_PORT);
    // client.setCallback(mqttCallback);

    /* Init MQTT Pub Sensor Timer */
    // pubSensorTimer = millis();
}


void loop() {

    // WIP I2C ISSUE
    delay(1000000);
    // WIP I2C ISSUE

    //reconnect MQTT Client if not connected
    if (!client.connected()) {
        reconnect_mqtt();
    }

    client.loop();
    if (client.connected() && (millis() - subControllerTimer > (safetyCloseActuatorSec * 1000)) && mqttCallbackInError == false )
    {
        mqttCallbackInError = true;
        Serial.println("[MQTT] Warning callback time reached: switch OFF all actuators");
        io_handler.safeMode();
        Serial.println("[MQTT] all actuators OFF");
    }
    
    //Pub sensors every 10 secs and only if the client is connected
    if (client.connected() && (millis() - pubSensorTimer > (sensorPubRateSec * 1000)))
    {
        //send sensors to MQTT Broker
        pubSensorsVals();
        //update timer
        pubSensorTimer = millis();
    }


    // update TFT screen
    //displayLib.updateDisplay(
    //  water_level_cm, nutrient_level_cm, ph_downer_level_cm,
    //   ph_level, tds_level, last_water_pump_state, last_nutrient_pump_state,
    //  last_ph_downer_pump_state, last_mixer_pump_state,
    //  ctl_ph_level_min, ctl_ph_level_max, ctl_tds_level_min, ctl_tds_level_max
    // );

    delay(300);
}
