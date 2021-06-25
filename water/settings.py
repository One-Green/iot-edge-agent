"""
Water settings

"""
import os
import sys
import logging
from datetime import timedelta
from utils import get_log_level

# ----- Logging parameter ----- > to all scripts
LOG_LEVEL = get_log_level()
logger = logging.getLogger()
logger.setLevel(LOG_LEVEL)
handler = logging.StreamHandler(sys.stdout)
handler.setLevel(LOG_LEVEL)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
handler.setFormatter(formatter)
logger.addHandler(handler)
# Do not handle peewee logger
peewee_logger = logging.getLogger('peewee')
peewee_logger.addHandler(logging.StreamHandler())
peewee_logger.setLevel(logging.CRITICAL)

# ----- SAFETY ----- > watchdog.py
CALLBACK_UPDATED_AT_KEY: str = 'callback_updated_at'  # key for last callback updated at datetime
SAFETY_MODE_KEY: str = 'safety_mode'
CALLBACK_MAX_TIME_DELTA: timedelta = timedelta(
    seconds=2,
    milliseconds=0)  # turn off all actuators if timedelta is higher than this config


# ----- MQTT -----
MQTT_HOST: str = os.getenv("MQTT_HOST", "010e7d5e-3a86-4c87-a4bc-8d7a82bf5d2e.nodes.k8s.fr-par.scw.cloud")
MQTT_PORT: int = int(os.getenv("MQTT_PORT", "30181"))
MQTT_USER: str = os.getenv("MQTT_USER", "mqtt")
MQTT_PASSWORD: str = os.getenv("MQTT_PASSWORD", "anyrandompassword")
MQTT_SENSOR_TOPIC: str = "water/sensor"
MQTT_CONTROLLER_TOPIC: str = "water/controller"

# ----- INPUTS ----- > inputs_handler.py
# analog pins definition
PH_SENSOR_PIN = 0
TDS_SENSOR_PIN = 1

WATER_LEVEL_PIN = 2  # analog value provided by arduino nano
NUTRIENT_LEVEL = 3  # analog value provided by arduino nano
PH_DOWNER_LEVEL_PIN = 4  # analog value provided by arduino nano

# ----- OUTPUTS ----- > output_handler.py
# Digital output
WATER_PUMP_PIN = 22
NUTRIENT_PUMP_PIN = 23
PH_DOWNER_PUMP_PIN = 24
MIXER_PUMP_PIN = 25
