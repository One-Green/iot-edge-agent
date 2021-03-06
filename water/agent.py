import sys

sys.path.insert(0, "..")
import time
import paho.mqtt.client as mqtt
from influx_line_protocol import Metric
from pyfirmata import ArduinoMega, util
from input_handler import read_sensors
from output_handler import off_on_digital_output
from output_handler import set_actuator_safe
from core.utils import detect_arduino_usb_serial
from settings import (
    PH_SENSOR_PIN,
    TDS_SENSOR_PIN,
    WATER_LEVEL_PIN,
    NUTRIENT_LEVEL,
    PH_DOWNER_LEVEL_PIN,
)
from settings import MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWORD, MQTT_SENSOR_TOPIC
from settings import SAFETY_MODE_KEY
from settings import logger
from core.db import get_db, db_update_or_create


logger.info(f"[UART][INFO] Connecting Arduino Mega Board ...")
board = ArduinoMega(detect_arduino_usb_serial())
it = util.Iterator(board)
it.start()
for pin in [
    PH_SENSOR_PIN,
    TDS_SENSOR_PIN,
    WATER_LEVEL_PIN,
    NUTRIENT_LEVEL,
    PH_DOWNER_LEVEL_PIN,
]:
    board.analog[pin].enable_reporting()
logger.info(f"[UART][OK] Connected to this board: {board}")
inputs = {}  # input dict for sensors values from arduino mega 2560

db = get_db()
db_update_or_create("node_type", "water")
db_update_or_create(SAFETY_MODE_KEY, False)


def on_connect(client, userdata, flags, rc):
    logger.info(f"[MQTT][OK] Connected to {MQTT_HOST=} / {MQTT_SENSOR_TOPIC}")


def generate_influx_protocol(_sensor_dict: dict) -> str:
    metric = Metric("water")
    metric.add_tag("water", "water")
    metric.add_value("nutrient_level_cm", _sensor_dict["nutrient_level_cm"])
    metric.add_value("ph_downer_level_cm", _sensor_dict["ph_downer_level_cm"])
    metric.add_value("ph_level", _sensor_dict["ph_average_value"])
    metric.add_value("tds_level", _sensor_dict["tds_average_ppm"])
    return str(metric)


mqtt_client = mqtt.Client()
mqtt_client.username_pw_set(username=MQTT_USER, password=MQTT_PASSWORD)
mqtt_client.on_connect = on_connect
mqtt_client.connect(MQTT_HOST, MQTT_PORT, 60)

while True:
    # Try to reconnect
    # if connection lost
    # set actuator in safe mod
    try:
        mqtt_client.reconnect()
        logger.debug("mqtt_client.reconnect() ok")
    except ConnectionRefusedError:
        logger.error(
            "mqtt_client.reconnect() failed, "
            "mqtt server can not be reached, calling set_actuator_safe"
        )
        set_actuator_safe(board=board, force_safe=True)

    # Read sensors through firmata
    inputs = read_sensors(board)
    # save to local db
    for k, v in inputs.items():
        db_update_or_create(k, v)
    # publish to mqtt
    mqtt_client.publish(MQTT_SENSOR_TOPIC, generate_influx_protocol(inputs))

    # check if mqtt callback is not in timeout ( water controller down)
    # if callback in timeout set safe mod while water controller is up
    set_actuator_safe(board=board)
    # apply pin UP/DOWN following water controller rule (safe mod check already in function)
    list(off_on_digital_output(board=board))
    time.sleep(0.5)
    logger.debug("Main loop successfully executed")
