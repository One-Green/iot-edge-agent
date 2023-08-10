import requests

PROTOCOL = "http"
HOST = "127.0.0.1:8080"
BUILD_PATH = "build"

api = f"{PROTOCOL}://{HOST}/{BUILD_PATH}"


build = {
    "git_repo": "https://github.com/One-Green/iot-edge-agent.git",
    "git_ref": "main",
    "git_username": None,
    "git_password": None,
    "src_path": "sprinkler",
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
print(r.text)

