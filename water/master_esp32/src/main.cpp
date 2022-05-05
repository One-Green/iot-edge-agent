/*
 *  One-Green node type: water+nutrient+pH
 *  Water supplier sprinkler nodes
 *  ESP32 board firmware
 *  Purpose:
 *      - Interact with mega-2560 board for Inputs/Outputs
 *      - Interact with One-Green Core trough MQTT
 *      - Interact with ST7735 screen
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
#include "esp_task_wdt.h"

// State machine implementation
enum State {
    IDLE,
    WATER_PUMP_ON,
    UP_NUTRIENT_LEVEL,
    MIX_NUTRIENT_LIQUID,
    DOWN_PH_LEVEL,
    MIX_PH_DOWNER_LIQUID,
    FORCE_SIGNAL,
    FORCE_SIGNAL_ON
};
static State state = IDLE ;
// state timer
unsigned int nutrient_time ;     // nutrient injection timer
unsigned int ph_downer_time ;    // ph downer injection timer
unsigned int mix_time ;          // mixer time
unsigned int MAX_NUTRIENT_INJECTION  = 3 ;// value in seconds
unsigned int MAX_PH_DOWNER_INJECTION = 3 ;// value in seconds
unsigned int MIX_LOCK = 15 ;               // value in seconds
// Watch configuration (in second)
#define WDT_TIMEOUT 5

unsigned long pubSensorTimer;
unsigned long subControllerTimer;
const int sensorPubRateSec       = 1; //send sensor values each x seconds
const int safetyCloseActuatorSec = 3; //close actuators if no response from One-Green Core
bool mqttCallbackInError         = false; // if callback received = false, if safetyCloseActuatorSec = true

// Parameter from Core
bool  ctl_water_pump              = false;
float ctl_ph_level_min            = 0.0;
float ctl_ph_level_max            = 0.0;
float ctl_tds_level_min           = 0.0;
float ctl_tds_level_max           = 0.0;
int   linked_sprinkler            = 0;
bool  force_water_pump_signal     = false;
bool  force_nutrient_pump_signal  = false;
bool  force_ph_downer_pump_signal = false;
bool  force_mixer_pump_signal     = false;
bool  water_pump_signal           = false;
bool  nutrient_pump_signal        = false;
bool  ph_downer_pump_signal       = false;
bool  mixer_pump_signal           = false;

// ----------------------------------   // Sensors Regs Values
int water_level_cm     = 0;
int nutrient_level_cm  = 0;
int ph_downer_level_cm = 0;

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
    //Parsing Water Pumps param
    ctl_water_pump = obj[String("p1")].as<bool>();
    // pH min/max config
    ctl_ph_level_min = obj[String("pmin")].as<float>();
    ctl_ph_level_max = obj[String("pmax")].as<float>();
    // tds nutrient min/max config
    ctl_tds_level_min = obj[String("tmin")].as<float>();
    ctl_tds_level_max = obj[String("tmax")].as<float>();
    // connected sprinkler count
    linked_sprinkler = obj[String("spc")].as<int>();
    // force control
    force_water_pump_signal =  obj[String("fps1")].as<bool>();
    force_nutrient_pump_signal = obj[String("fps2")].as<bool>();
    force_ph_downer_pump_signal = obj[String("fps3")].as<bool>();
    force_mixer_pump_signal = obj[String("fps4")].as<bool>();
    water_pump_signal = obj[String("fp1")].as<bool>();
    nutrient_pump_signal = obj[String("fp2")].as<bool>();
    ph_downer_pump_signal = obj[String("fp3")].as<bool>();
    mixer_pump_signal = obj[String("fp4")].as<bool>();
}

// Helpers functions
bool checkForceSignal()
{
    if (
        force_water_pump_signal
        ||
        force_nutrient_pump_signal
        ||
        force_ph_downer_pump_signal
        ||
        force_mixer_pump_signal
    )
    {
        return true;
    } else return false;
}

bool checkForceOn()
{
    if (
        water_pump_signal
        ||
        nutrient_pump_signal
        ||
        ph_downer_pump_signal
        ||
        mixer_pump_signal
    )
    {
        return true;
   } else return false;
}

bool pHValAvailable()
{
    if (
        ctl_ph_level_min != 0.0
        &
        ctl_ph_level_max != 0.0
        &
        io_handler.pH != 0.0
    )
    {
        return true ;
    } else return false;
}

bool TDSValAvailable()
{
    if (
        ctl_tds_level_min != 0.0
        &
        ctl_tds_level_max != 0.0
        &
        io_handler.TDS != 0.0
    )
    {
        return true ;
    } else return false;
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

void testI2CSlave()
{
    // idle
    io_handler.sendIdle();
    delay(200);

    // ph voltage
    io_handler.getPhVoltage();
    delay(1000);
    // tds voltage
    io_handler.getTDSVoltage();
    delay(1000);
    // ph real value
    io_handler.getPhLevel();
    delay(1000);
    // tds real value
    io_handler.getTDS();
    delay(1000);

    // water tank level
    io_handler.getWaterLevelCM();
    delay(1000);
    // nutrient tank level
    io_handler.getNutrientLevelCM();
    delay(1000);
    // ph downer tank level
    io_handler.getPhDownerLevelCM();
    delay(1000);
    // on+off water pump
    io_handler.getWaterPumpStatus();
    delay(1000);
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

void setup()
{

    // Serial Ports Init 
    INIT_SERIAL(115200);
    DEBUG_PRINTLN("Serial Begin OK");

    // init i/o handler
    io_handler.initR(NODE_TAG);
    io_handler.safeMode();

    // Display Init
    displayLib.initR();
    displayLib.initWifi();

    // Connect to WiFi 
    connectToWiFiNetwork();
    displayLib.connectedWifi();

    // Display WiFi Info
    displayLib.printHeader(WIFI_SSID, WiFi.localIP(), NODE_TYPE, NODE_TAG);
    displayLib.printTemplate();

    /* MQTT connexion */
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);

    /* Init MQTT Pub Sensor Timer */
    pubSensorTimer = millis();

    //enable panic so ESP32 restarts
    esp_task_wdt_init(WDT_TIMEOUT, true);
    //add current thread to WDT watch
    esp_task_wdt_add(NULL);
}


void loop() {
    //reconnect MQTT Client if not connected
    if (!client.connected()) {
        reconnect_mqtt();
    }
    
    client.loop();
    
    // Safe mode if no mqtt callback 
//    if (client.connected() && (millis() - subControllerTimer > (safetyCloseActuatorSec * 1000)) && mqttCallbackInError == false )
//    {
//        mqttCallbackInError = true;
//        DEBUG_PRINTLN("[MQTT] Warning callback time reached: switch OFF all actuators");
//        io_handler.safeMode();
//        DEBUG_PRINTLN("[MQTT] all actuators OFF");
//    }
    
    // Pub sensors every publish_evey (m) and only if the client is connected
    if (client.connected() && (millis() - pubSensorTimer > (sensorPubRateSec * 1000)))
    {
        //send sensors to MQTT Broker
        pubSensorsVals();
        //update timer
        pubSensorTimer = millis();
        Serial.println(CONTROLLER_TOPIC);
    }

    displayLib.updateDisplay(
        io_handler.WaterTankLevel,
        io_handler.NutrientTankLevel,
        io_handler.PHTankLevel,
        io_handler.TDSVoltage,
        io_handler.TDS,
        io_handler.phVoltage,
        io_handler.pH,
        io_handler.getWaterPumpStatus(),
        io_handler.getNutrientPumpStatus(),
        io_handler.getPhDownerPumpStatus(),
        io_handler.getMixerPumpStatus(),
        linked_sprinkler,
        ctl_tds_level_min,
        ctl_tds_level_max,
        ctl_ph_level_min,
        ctl_ph_level_max
        );

    // State handler
    switch(state)
    {
        case IDLE:
            DEBUG_PRINTLN("STATE=IDLE");
            // T1
            if ( (io_handler.TDS <= ctl_tds_level_min) & TDSValAvailable() )
            {
                nutrient_time = millis();
                mix_time = millis();
                io_handler.OnNutrientPump();
                io_handler.OnMixerPump();
                state = UP_NUTRIENT_LEVEL;
            }
            // T6
            if ( (io_handler.pH >= ctl_ph_level_max) & pHValAvailable() )
            {
                ph_downer_time = millis();
                mix_time = millis();
                io_handler.OnPhDownerPump();
                io_handler.OnMixerPump();
                state = DOWN_PH_LEVEL ;
            }
            // T11
            if (ctl_water_pump)
            {
                io_handler.OnWaterPump();
                state = WATER_PUMP_ON;
            }

            // T13
            if (checkForceSignal())
            {
                DEBUG_PRINTLN("checkForceSignal==true");
                state = FORCE_SIGNAL;
            }
        break;

        case WATER_PUMP_ON:
            DEBUG_PRINTLN("STATE=WATER_PUMP_ON");
            // T12
            if (ctl_water_pump)
            {
                io_handler.OffWaterPump();
                state = IDLE;
            }
        break;

        case UP_NUTRIENT_LEVEL:
            DEBUG_PRINTLN("STATE=UP_NUTRIENT_LEVEL");
            // T2
            if (checkForceSignal())
            {
                io_handler.OffNutrientPump();
                io_handler.OffMixerPump();
                state = IDLE;
            }
            // T3
            if ( (millis() - nutrient_time) > MAX_NUTRIENT_INJECTION * 1000 )
            {
                io_handler.OffNutrientPump();
                state = MIX_NUTRIENT_LIQUID;
            }
        break;

        case MIX_NUTRIENT_LIQUID:
            DEBUG_PRINTLN("MIX_NUTRIENT_LIQUID");
            // T4
            if ( (millis() - mix_time) > MIX_LOCK * 1000 )
            {
                nutrient_time = millis();
                mix_time = millis();
                io_handler.OnNutrientPump();
                state = UP_NUTRIENT_LEVEL;
            }
            // T5
            if (
                io_handler.TDS >= ctl_tds_level_max
                ||
                checkForceSignal()
            )
            {
                io_handler.OffNutrientPump();
                io_handler.OffMixerPump();
                state = IDLE;
            }
        break;

        case DOWN_PH_LEVEL:
            DEBUG_PRINTLN("DOWN_PH_LEVEL");
            // T7
            if (checkForceSignal())
            {
                io_handler.OffPhDownerPump();
                io_handler.OffMixerPump();
                state = IDLE;
            }
            // T8
            if ( (millis() - ph_downer_time) > MAX_PH_DOWNER_INJECTION * 1000 )
            {
                io_handler.OffPhDownerPump();
                state = MIX_PH_DOWNER_LIQUID;
            }
        break;

        case MIX_PH_DOWNER_LIQUID:
            DEBUG_PRINTLN("MIX_PH_DOWNER_LIQUID");
            // T9
            if ( (millis() - mix_time ) > MIX_LOCK * 1000 )
            {
                ph_downer_time = millis();
                mix_time = millis();
                io_handler.OnPhDownerPump();
                state = DOWN_PH_LEVEL;
            }
            // T10
            if ( io_handler.pH <= ctl_ph_level_min & checkForceSignal())
            {
                io_handler.OffPhDownerPump();
                io_handler.OffMixerPump();
                state = IDLE;
            }
        break;

        case FORCE_SIGNAL:
            DEBUG_PRINTLN("FORCE_SIGNAL");
            // T14
            if (!checkForceSignal())
            {
                io_handler.OffWaterPump();
                io_handler.OffNutrientPump();
                io_handler.OffPhDownerPump();
                io_handler.OffMixerPump();
                state = IDLE;
            }
            // handle water pump
            if (force_water_pump_signal & water_pump_signal) { io_handler.OnWaterPump(); }
            if (force_water_pump_signal & !water_pump_signal) { io_handler.OffWaterPump(); }
            // handle nutrient pump
            if (force_nutrient_pump_signal & nutrient_pump_signal) { io_handler.OnNutrientPump(); }
            if (force_nutrient_pump_signal & !nutrient_pump_signal) { io_handler.OffNutrientPump(); }
            // handle pH downer pump
            if (force_ph_downer_pump_signal & ph_downer_pump_signal) { io_handler.OnPhDownerPump(); }
            if (force_ph_downer_pump_signal & !ph_downer_pump_signal) { io_handler.OffPhDownerPump(); }
            // handle mixer pump
            if (force_mixer_pump_signal & mixer_pump_signal) { io_handler.OnMixerPump(); }
            if (force_mixer_pump_signal & !mixer_pump_signal) { io_handler.OffMixerPump(); }
        break;

    }

    // Clear Watch dog
    esp_task_wdt_reset();
    delay(200);
}
