//
// Created by shan on 06/11/2020.
//


// TDS measurement configuration
#define SCOUNT 30
#define VREF  5

class OGIO {
private:
    int getMedianNum(int bArray[], int iFilterLen);
    float baseUltrasonicReader(int trigger, int echo);

public:

    void initR();

    float getWaterLevelCM();
    float getNutrientLevelCM();
    float getPhDownerLevelCM();

    float getPhVoltage();
    float getTDSVoltage();

    float getPhLevel();
    float getTDS();
    int analogBuffer[SCOUNT];
    int analogBufferTemp[SCOUNT];
    int analogBufferIndex = 0, copyIndex = 0;
    float averageVoltage = 0, tdsValue = 0, temperature = 25;

    void setWaterPump(uint8_t state);
    void setNutrientPump(uint8_t state);
    void setPHDownerPump(uint8_t state);
    void setMixerPump(uint8_t state);

    int getWaterPumpStatus();
    int getNutrientPumpStatus();
    int getPhDownerPumpStatus();
    int getMixerPumpStatus();

};

extern OGIO io_handler;
