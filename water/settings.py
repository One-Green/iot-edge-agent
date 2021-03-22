"""
Water settings

"""
import os

# ----- MQTT -----
MQTT_HOST: str = os.getenv("MQTT_HOST", "010e7d5e-3a86-4c87-a4bc-8d7a82bf5d2e.nodes.k8s.fr-par.scw.cloud")
MQTT_PORT: int = int(os.getenv("MQTT_PORT", "30181"))
MQTT_USER: str = os.getenv("MQTT_USER", "mqtt")
MQTT_PASSWORD: str = os.getenv("MQTT_PASSWORD", "anyrandompassword")
MQTT_SENSOR_TOPIC: str = "water/sensor"
MQTT_CONTROLLER_TOPIC: str = "water/controller"

# ----- INPUTS -----
# analog pins definition
PH_SENSOR_PIN = 0
TDS_SENSOR_PIN = 1

WATER_LEVEL_PIN = 2  # analog value provided by arduino nano
NUTRIENT_LEVEL = 3  # analog value provided by arduino nano
PH_DOWNER_LEVEL_PIN = 4  # analog value provided by arduino nano

# ----- OUTPUTS -----
# Digital output
WATER_PUMP_PIN = 22
NUTRIENT_PUMP_PIN = 23
PH_DOWNER_PUMP_PIN = 24
MIXER_PUMP_PIN = 25
