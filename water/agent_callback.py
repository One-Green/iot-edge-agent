import sys

sys.path.insert(0, "..")
import json
import paho.mqtt.client as mqtt
from core.db import get_db, db_update_or_create
from settings import MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWORD, MQTT_CONTROLLER_TOPIC
from settings import CALLBACK_UPDATED_AT_KEY
from settings import logger
from datetime import datetime

db = get_db()


def on_connect(client, userdata, flags, rc):
    client.subscribe(MQTT_CONTROLLER_TOPIC)
    logger.info(f"MQTT Connected to Callback {MQTT_HOST=} / {MQTT_CONTROLLER_TOPIC}")


def on_message(client, userdata, message):
    for k, v in json.loads(message.payload).items():
        db_update_or_create(k, v)
    db_update_or_create(CALLBACK_UPDATED_AT_KEY, datetime.now().isoformat())


mqtt_client = mqtt.Client()
mqtt_client.username_pw_set(username=MQTT_USER, password=MQTT_PASSWORD)
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_HOST, MQTT_PORT, 60)
mqtt_client.loop_forever()
