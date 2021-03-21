#define WATER_LEVEL_TRIGGER_PIN  2
#define WATER_LEVEL_ECHO_PIN     3
#define WATER_LEVEL_MIN_DISTANCE 0
#define WATER_LEVEL_MAX_DISTANCE 450
#define WATER_LEVEL_PWM_PIN 9

#define NUTRIENT_LEVEL_TRIGGER_PIN 4
#define NUTRIENT_LEVEL_ECHO_PIN 5
#define NUTRIENT_LEVEL_MIN_DISTANCE 0
#define NUTRIENT_LEVEL_MAX_DISTANCE 450
#define NUTRIENT_LEVEL_PWM_PIN 10

#define PH_DOWNER_LEVEL_TRIGGER_PIN 6
#define PH_DOWNER_LEVEL_ECHO_PIN 7
#define PH_DOWNER_LEVEL_MIN_DISTANCE 0
#define PH_DOWNER_LEVEL_MAX_DISTANCE 450
#define PH_DOWNER_LEVEL_PWM_PIN 11


int baseUltrasonicReader(int trigger, int echo) {
    long duration;
    int distance;
    // Clear the trigPin by setting it LOW:
    digitalWrite(trigger, LOW);
    delayMicroseconds(5);
    // Trigger the sensor by setting the trigPin high for 10 microseconds:
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);
    // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds:
    duration = pulseIn(echo, HIGH);
    // Calculate the distance:
    distance = duration * 0.034 / 2;
    return distance;
}


void setup() {
    pinMode(WATER_LEVEL_TRIGGER_PIN, OUTPUT);
    pinMode(WATER_LEVEL_ECHO_PIN, INPUT);
    pinMode(WATER_LEVEL_PWM_PIN, OUTPUT);

    pinMode(NUTRIENT_LEVEL_TRIGGER_PIN, OUTPUT);
    pinMode(NUTRIENT_LEVEL_ECHO_PIN, INPUT);
    pinMode(NUTRIENT_LEVEL_PWM_PIN, OUTPUT);

    pinMode(PH_DOWNER_LEVEL_TRIGGER_PIN, OUTPUT);
    pinMode(PH_DOWNER_LEVEL_ECHO_PIN, INPUT);
    pinMode(PH_DOWNER_LEVEL_PWM_PIN, OUTPUT);

    Serial.begin(9600);
}

void loop() {
    int dWater = baseUltrasonicReader(WATER_LEVEL_TRIGGER_PIN, WATER_LEVEL_ECHO_PIN);
    int dWaterMapped = map(dWater, WATER_LEVEL_MIN_DISTANCE, WATER_LEVEL_MAX_DISTANCE , 0, 255 ) ; // analogWrite values from 0 to 255
    analogWrite(WATER_LEVEL_PWM_PIN, dWaterMapped);
    Serial.print("dWater=");
    Serial.print(dWater);
    Serial.print(" ; ");

    int dNutrient = baseUltrasonicReader(NUTRIENT_LEVEL_TRIGGER_PIN, NUTRIENT_LEVEL_ECHO_PIN);
    int dNutrientMapped = map(dNutrient, NUTRIENT_LEVEL_MIN_DISTANCE, NUTRIENT_LEVEL_MAX_DISTANCE , 0, 255 ) ; // analogWrite values from 0 to 255
    analogWrite(NUTRIENT_LEVEL_PWM_PIN, dNutrientMapped);
    Serial.print("dNutrient=");
    Serial.print(dNutrient);
    Serial.print(" ; ");

    int dPHDowner = baseUltrasonicReader(PH_DOWNER_LEVEL_TRIGGER_PIN, PH_DOWNER_LEVEL_ECHO_PIN);
    int dPHDownerMapped = map(dPHDowner, PH_DOWNER_LEVEL_MIN_DISTANCE, PH_DOWNER_LEVEL_MAX_DISTANCE , 0, 255 ) ; // analogWrite values from 0 to 255
    analogWrite(PH_DOWNER_LEVEL_PWM_PIN, dPHDownerMapped);
    Serial.print("dPHDowner=");
    Serial.print(dPHDowner);
    Serial.println();


    Serial.print("dWaterMapped=");
    Serial.print(dWaterMapped);
    Serial.print(" ; ");

    Serial.print("dNutrientMapped=");
    Serial.print(dNutrientMapped);
    Serial.print(" ; ");

    Serial.print("dPHDownerMapped=");
    Serial.print(dPHDownerMapped);
    Serial.println();

}
