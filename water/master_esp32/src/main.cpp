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
const int sensorPubRateSec = 1; //send sensor values each x seconds
const int safetyCloseActuatorSec = 3; //close actuators if no response from Master
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
    DEBUG_PRINT("[WIFI] Connecting to ");
    DEBUG_PRINT(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINT(".");
        delay(100);
    }
    DEBUG_PRINTLN("\n");
    // displayLib.connectedWifi();
    DEBUG_PRINTLN("[WIFI] Connected ");
    DEBUG_PRINT("[WIFI] IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
}

/* MQTT Functions */

void reconnect_mqtt() {
    // Loop until we're reconnected
    while (!client.connected()) {
        DEBUG_PRINT("[MQTT] Attempting connection... with client name = ");
        String client_name = String(NODE_TYPE) + "-" + String(NODE_TAG);
        int clt_len = client_name.length() + 1;
        char clt_name_char[clt_len];
        client_name.toCharArray(clt_name_char, clt_len);
        DEBUG_PRINTLN(clt_name_char);

        // Attempt to connect
        if (client.connect(clt_name_char, MQTT_USER, MQTT_PASSWORD)) {
            DEBUG_PRINTLN("[MQTT] Client connected");
            // Subscribe
            client.subscribe(CONTROLLER_TOPIC);
        } else {
            DEBUG_PRINT("[MQTT] failed to connect, rc=");
            DEBUG_PRINT(client.state());
            DEBUG_PRINTLN("[MQTT] try again in 5 seconds");
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
    DEBUG_PRINT("[MQTT] Receiving << on topic: ");
    DEBUG_PRINT(topic);
    DEBUG_PRINT(". JSON message: ");
    String tmpMessage;

    for (int i = 0; i < length; i++) {
        tmpMessage += (char) message[i];
    }

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, tmpMessage);
    JsonObject obj = doc.as<JsonObject>();
    serializeJsonPretty(doc, Serial);
    DEBUG_PRINTLN();

    /* Parse params from MQTT Payload*/

    //Parsing Tag param
    String tag = obj[String("tag")];

    //Parsing Water Pumps param
    ctl_water_pump = obj[String("water_pump_signal")];
    //Parsing Nutrient Pumps param
    ctl_nutrient_pump = obj[String("nutrient_pump_signal")];
    //Parsing PH Downer Pumps param
    ctl_ph_downer_pump = obj[String("ph_downer_pump_signal")];
    // Parsing PH Downer Pumps param
    ctl_mixer_pump = obj[String("mixer_pump_signal")];

    ctl_ph_level_min = obj[String("ph_min_level")];
    ctl_ph_level_max = obj[String("ph_max_level")];
    ctl_tds_level_min = obj[String("tds_min_level")];
    ctl_tds_level_max = obj[String("tds_max_level")];


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
                if (showMessage) DEBUG_PRINTLN("[I/O] Waiting for water pump activation");
                showMessage = false;
                delay(500);
            }
            DEBUG_PRINTLN("[I/O] Water pump is OPENED");
        } 
        else //turn-OFF and confirm 
        {
            timeoutCount = millis();
            setPumpState = false;
            showMessage = true;
            while ((millis() - timeoutCount < 3000 ) && !setPumpState) 
            { 
                setPumpState = io_handler.OffWaterPump();
                if (showMessage) DEBUG_PRINTLN("[I/O] Waiting for water pump closing");
                showMessage = false;
                delay(500);
            }
            DEBUG_PRINTLN("[I/O] Water pump is CLOSED");
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
                if (showMessage) DEBUG_PRINTLN("[I/O] Waiting for nutrient pump activation");
                showMessage = false;
                delay(500);
            }
            DEBUG_PRINTLN("[I/O] Nutrient pump is OPENED");
        } else {
            timeoutCount = millis();
            setPumpState = false;
            showMessage = true;
            while ((millis() - timeoutCount < 3000 ) && !setPumpState)
            {
                setPumpState = io_handler.OffNutrientPump();
                if (showMessage) DEBUG_PRINTLN("[I/O] Waiting for nutrient pump closing");
                showMessage = false;
                delay(500);
            }
            DEBUG_PRINTLN("[I/O] Nutrient pump is CLOSED");
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
                if (showMessage) DEBUG_PRINTLN("[I/O] Waiting for pH downer pump activation");
                showMessage = false;
                delay(500);
            }
            DEBUG_PRINTLN("[I/O] pH downer pump is OPENED");
        } 
        else
        {
            timeoutCount = millis();
            setPumpState = false;
            showMessage = true;
            while ((millis() - timeoutCount < 3000 ) && !setPumpState)
            {
                setPumpState = io_handler.OffPhDownerPump();
                if (showMessage) DEBUG_PRINTLN("[I/O] Waiting for pH downer pump closing");
                showMessage = false;
                delay(500);
            }
            DEBUG_PRINTLN("[I/O] pH downer pump is CLOSED");
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
                if (showMessage) DEBUG_PRINTLN("[I/O] Waiting for mixer pump activation");
                showMessage = false;
                delay(500);
            }
            DEBUG_PRINTLN("[I/O] mixer pump is OPENED");
        }
        else
        {
            timeoutCount = millis();
            setPumpState = false;
            showMessage = true;
            while ((millis() - timeoutCount < 3000 ) && !setPumpState)
            {
                setPumpState = io_handler.OffMixerPump();
                if (showMessage) DEBUG_PRINTLN("[I/O] Waiting for mixer pump closing");
                showMessage = false;
                delay(500);
            }
            DEBUG_PRINTLN("[I/O] mixer pump is CLOSED");
        }
    }

}

void pubSensorsVals()
{
    String line_proto = io_handler.generateInfluxLineProtocol();
    DEBUG_PRINTLN(line_proto);
    // convert string to char and publish to mqtt
    int line_proto_len = line_proto.length() + 1;
    char line_proto_char[line_proto_len];
    line_proto.toCharArray(line_proto_char, line_proto_len);
    DEBUG_PRINTLN("[MQTT] Writing >> on topic " + String(SENSOR_TOPIC));
    client.publish(SENSOR_TOPIC, line_proto_char);
}

void testI2Cslave()
{
    // idle
    io_handler.sendIdle();
    delay(200);

    float v;
    // ph voltage
    io_handler.getPhVoltage();
    // tds voltage
    io_handler.getTDSVoltage();
    // ph real valu1e
    io_handler.getPhLevel();
    // tds real value
    io_handler.getTDS();

    // water tank level
    io_handler.getWaterLevelCM();
    // nutrient tank level
    io_handler.getNutrientLevelCM();
    // ph downer tank level
    io_handler.getPhDownerLevelCM();

    // on+off water pump
    io_handler.getWaterPumpStatus();
    io_handler.OnWaterPump();
    delay(1000);
    io_handler.OffWaterPump();
    delay(500);
    io_handler.OnWaterPump();
    delay(1000);
    io_handler.OffWaterPump();

    io_handler.getPhDownerPumpStatus();
    io_handler.OnPhDownerPump();
    delay(1000);
    io_handler.OffPhDownerPump();
    delay(500);
    io_handler.OnPhDownerPump();
    delay(1000);
    io_handler.OffPhDownerPump();

    io_handler.getNutrientPumpStatus();
    io_handler.OnNutrientPump();
    delay(1000);
    io_handler.OffNutrientPump();
    delay(500);
    io_handler.OnNutrientPump();
    delay(1000);
    io_handler.OffNutrientPump();

    io_handler.getMixerPumpStatus();
    io_handler.OnMixerPump();
    delay(1000);
    io_handler.OffMixerPump();
    delay(500);
    io_handler.OnMixerPump();
    delay(1000);
    io_handler.OffMixerPump();
    
    String tmp;
    DEBUG_PRINTLN("Generating line protocol string = ");
    tmp = io_handler.generateInfluxLineProtocol();
    DEBUG_PRINTLN(tmp);

}

void testDisplay()
{
    displayLib.initR();
}

void setup() {

    // Serial Ports Init 
    INIT_SERIAL(115200);
    DEBUG_PRINTLN("Serial Begin OK");

    // init i/o handler
    io_handler.initR(NODE_TAG);
    io_handler.safeMode();

    // Display Init
    // displayLib.initR();
    // displayLib.initWifi();

    // Connect to WiFi 
    connectToWiFiNetwork();

    // Display WiFi Info
    // displayLib.printHeader(WIFI_SSID, WiFi.localIP(), NODE_TYPE, NODE_TAG);
    // displayLib.printTemplate();

    /* MQTT connexion */
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);

    /* Init MQTT Pub Sensor Timer */
    pubSensorTimer = millis();

    // for purpose only tests
    // testI2Cslave();
    // testDisplay();
}


void loop() {
   
    //reconnect MQTT Client if not connected
    if (!client.connected()) {
        reconnect_mqtt();
    }
    
    client.loop();
    
    // Safe mode if no mqtt callback 
    if (client.connected() && (millis() - subControllerTimer > (safetyCloseActuatorSec * 1000)) && mqttCallbackInError == false )
    {
        mqttCallbackInError = true;
        DEBUG_PRINTLN("[MQTT] Warning callback time reached: switch OFF all actuators");
        io_handler.safeMode();
        DEBUG_PRINTLN("[MQTT] all actuators OFF");
    }
    
    // Pub sensors every publish_evey (m) and only if the client is connected
     if (client.connected() && (millis() - pubSensorTimer > (sensorPubRateSec * 1000)))
     {
        //send sensors to MQTT Broker
        pubSensorsVals();
        //update timer
        pubSensorTimer = millis();
    }
//    else
//        DEBUG_PRINT("...");

    // update TFT screen
    //displayLib.updateDisplay(
    //  water_level_cm, nutrient_level_cm, ph_downer_level_cm,
    //   ph_level, tds_level, last_water_pump_state, last_nutrient_pump_state,
    //  last_ph_downer_pump_state, last_mixer_pump_state,
    //  ctl_ph_level_min, ctl_ph_level_max, ctl_tds_level_min, ctl_tds_level_max
    // );

}
