import requests
import zipfile
import io
import os

dir_path = os.path.dirname(os.path.realpath(__file__))

PROTOCOL = "http"
HOST = "127.0.0.1:8080"
BUILD_PATH = "build"

api = f"{PROTOCOL}://{HOST}/{BUILD_PATH}"

src_path = "sprinkler"

build = {
    "git_repo": "https://github.com/One-Green/iot-edge-agent.git",
    "git_ref": "main",
    "git_username": None,
    "git_password": None,
    "src_path": src_path,
    "platformio_build_flags": [
        {"WIFI_SSID": "wifi_ssid"},
        {"WIFI_PASSWORD": "wifi_password"},
        {"NODE_TAG": "node_tag"},
        {"MQTT_SERVER": "mqtt_server"},
        {"MQTT_PORT": 10021},
        {"MQTT_USER": "mqtt_user"},
        {"MQTT_PASSWORD": "mqtt_password"},
        {"CONTROLLER_TOPIC": "sprinkler/controller/node_tag"},
        {"test": 1},
    ]
}

r = requests.post(api, json=build)
z = zipfile.ZipFile(io.BytesIO(r.content))
z.extractall(
    os.path.join(dir_path, "binary", src_path)
             )
