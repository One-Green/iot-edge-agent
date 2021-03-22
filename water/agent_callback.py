import sys

sys.path.insert(0, "..")
import paho.mqtt.client as mqtt
from settings import MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWORD, MQTT_CONTROLLER_TOPIC


def on_connect(client, userdata, flags, rc):
    client.subscribe(MQTT_CONTROLLER_TOPIC)
    print(f"[MQTT][OK] Connected to Callback {MQTT_HOST=} / {MQTT_CONTROLLER_TOPIC}")


def on_message(client, userdata, message):
    with open('callback.json', 'w') as outfile:
        outfile.write(message.payload.decode("utf-8"))


mqtt_client = mqtt.Client()
mqtt_client.username_pw_set(username=MQTT_USER, password=MQTT_PASSWORD)
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_HOST, MQTT_PORT, 60)
mqtt_client.loop_forever()
