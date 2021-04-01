import sys

sys.path.insert(0, "..")
import time
import json
import paho.mqtt.client as mqtt
from influx_line_protocol import Metric
from pyfirmata import ArduinoMega, util
from input_handler import read_sensors
from output_handler import off_on_digital_output
from core.utils import detect_arduino_usb_serial
from settings import PH_SENSOR_PIN, TDS_SENSOR_PIN, WATER_LEVEL_PIN, NUTRIENT_LEVEL, PH_DOWNER_LEVEL_PIN
from settings import MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWORD, MQTT_SENSOR_TOPIC
from core.db import get_db, db_update_or_create

print(f"[UART][INFO] Connecting Arduino Mega Board ...")
board = ArduinoMega(detect_arduino_usb_serial())
it = util.Iterator(board)
it.start()
for pin in [PH_SENSOR_PIN, TDS_SENSOR_PIN, WATER_LEVEL_PIN, NUTRIENT_LEVEL, PH_DOWNER_LEVEL_PIN]:
    board.analog[pin].enable_reporting()
print(f"[UART][OK] Connected to this board: {board}")
inputs = {}  # input dict for sensors values from arduino mega 2560

db = get_db()
db_update_or_create("node_type", "water")


def on_connect(client, userdata, flags, rc):
    print(f"[MQTT][OK] Connected to {MQTT_HOST=} / {MQTT_SENSOR_TOPIC}")


def generate_influx_protocol(_sensor_dict: dict) -> str:
    metric = Metric("water")
    metric.add_tag('water', 'water')
    metric.add_value('nutrient_level_cm', _sensor_dict["nutrient_level_cm"])
    metric.add_value('ph_downer_level_cm', _sensor_dict["ph_downer_level_cm"])
    metric.add_value('ph_level', _sensor_dict["ph_average_value"])
    metric.add_value('tds_level', _sensor_dict["tds_average_ppm"])
    return str(metric)


def read_callback() -> dict:
    try:
        with open("callback.json", "r+") as infile:
            return json.load(infile)
    except FileNotFoundError:
        return {}


mqtt_client = mqtt.Client()
mqtt_client.username_pw_set(username=MQTT_USER, password=MQTT_PASSWORD)
mqtt_client.on_connect = on_connect
mqtt_client.connect(MQTT_HOST, MQTT_PORT, 60)

while True:
    inputs = read_sensors(board)
    # save to local db for state-exporter.py
    for k, v in inputs.items():
        db_update_or_create(k, v)
    mqtt_client.publish(MQTT_SENSOR_TOPIC, generate_influx_protocol(inputs))
    controller_callback = read_callback()
    r = list(off_on_digital_output(board=board, _callback=controller_callback))
    for _ in r:
        if _["changed"]:
            print(_)
    time.sleep(0.5)
