import sys

sys.path.insert(0, "..")
from pyfirmata import ArduinoMega, util
from core.utils import detect_arduino_usb_serial
from core.sensor_processing.sampling import mean_sample
from settings import PH_SENSOR_PIN, TDS_SENSOR_PIN, WATER_LEVEL_PIN, NUTRIENT_LEVEL, PH_DOWNER_LEVEL_PIN


print(f"[UART][INFO] Connecting Arduino Mega Board ...")
board = ArduinoMega(detect_arduino_usb_serial())
it = util.Iterator(board)
it.start()
for pin in [PH_SENSOR_PIN, TDS_SENSOR_PIN, WATER_LEVEL_PIN, NUTRIENT_LEVEL, PH_DOWNER_LEVEL_PIN]:
    board.analog[pin].enable_reporting()
print(f"[UART][OK] Connected to this board: {board}")


def read_ph(board: ArduinoMega, _pipeline_dict: dict) -> dict:
    d = mean_sample(board=board, pin=PH_SENSOR_PIN)
    ph = 7 + ((2.5 - d["voltage"]) / 0.18)
    _pipeline_dict["ph_average_adc"] = d["adc"]
    _pipeline_dict["ph_average_voltage"] = d["voltage"]
    _pipeline_dict["ph_average_value"] = ph
    return _pipeline_dict


while True:
    d = {}
    print(read_ph(board, d))
