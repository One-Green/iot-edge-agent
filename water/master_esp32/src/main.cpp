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
const int sensorPubRateSec = 1; //send sensor values each x seconds
const int safetyCloseActuatorSec = 3; //close actuators if no response from One-Green Core
bool mqttCallbackInError = false; // if callback received = false, if safetyCloseActuatorSec = true

// Parameter from Core
bool  ctl_water_pump              = false;
bool  ctl_nutrient_pump           = false;
bool  ctl_ph_downer_pump          = false;
bool  ctl_mixer_pump              = false;
float ctl_ph_level_min            = 0;
float ctl_ph_level_max            = 0;
float ctl_tds_level_min           = 0;
float ctl_tds_level_max           = 0;
int   linked_sprinkler            = 0 ;
bool  force_water_pump_signal     = false ;
bool  force_nutrient_pump_signal  = false ;
bool  force_ph_downer_pump_signal = false ;
bool  force_mixer_pump_signal     = false ;
bool  water_pump_signal           = false ;
bool  nutrient_pump_signal        = false ;
bool  ph_downer_pump_signal       = false ;
bool  mixer_pump_signal           = false ;

// ----------------------------------   // Sensors Regs Values
int water_level_cm = 0;
int nutrient_level_cm = 0;
int ph_downer_level_cm = 0;
int ph_level = 0;
int tds_level = 0;

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
    ctl_water_pump = obj[String("p1")];
    //Parsing Nutrient Pumps param
    ctl_nutrient_pump = obj[String("p2")];
    //Parsing PH Downer Pumps param
    ctl_ph_downer_pump = obj[String("p3")];
    // Parsing PH Downer Pumps param
    ctl_mixer_pump = obj[String("p4")];
    // pH min/max config
    ctl_ph_level_min = obj[String("pmin")];
    ctl_ph_level_max = obj[String("pmax")];
    // tds nutrient min/max config
    ctl_tds_level_min = obj[String("tmin")];
    ctl_tds_level_max = obj[String("tmax")];
    // connected sprinkler count
    linked_sprinkler = obj[String("spc")];
    // force control
    force_water_pump_signal =  obj[String("fps1")];
    force_nutrient_pump_signal = obj[String("fps2")];
    force_ph_downer_pump_signal = obj[String("fps3")];
    force_mixer_pump_signal = obj[String("fps4")];
    water_pump_signal =  obj[String("fp1")];
    nutrient_pump_signal = obj[String("fp2")];
    ph_downer_pump_signal = obj[String("fp3")];
    mixer_pump_signal = obj[String("fp4")];

}

bool check_force()
{
    bool _[16] = {
        force_water_pump_signal,
        force_nutrient_pump_signal,
        force_ph_downer_pump_signal,
        force_mixer_pump_signal,
        water_pump_signal,
        nutrient_pump_signal,
        ph_downer_pump_signal,
        mixer_pump_signal,
    };

    for (byte i = 0; i < sizeof(_) - 1; i++)
    {
        if (_[i]) return true ;
    }

    return false;
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


void setup() {

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
            // T1
            if (tds_level <= ctl_tds_level_min)
            {
                nutrient_time = millis();
                mix_time = millis();
                io_handler.OnNutrientPump();
                io_handler.OnMixerPump();
                state = UP_NUTRIENT_LEVEL;
            }
        break;

        case WATER_PUMP_ON:
        break;

        case UP_NUTRIENT_LEVEL:
            // T2
            if (check_force() == true)
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
                tds_level >= ctl_tds_level_max
                ||
                check_force() == true
            )
            {
                io_handler.OffNutrientPump();
                io_handler.OffMixerPump();
                state = IDLE;
            }

        break;

        case DOWN_PH_LEVEL:
        break;

        case MIX_PH_DOWNER_LIQUID:
        break;

        case FORCE_SIGNAL:
        break;

        case FORCE_SIGNAL_ON:
        break;
    }

    // Clear Watch dog
    // TODO: to reactivate
    //esp_task_wdt_reset();
    delay(200);
}
