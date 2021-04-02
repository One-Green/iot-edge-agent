import sys

sys.path.insert(0, "..")
from pyfirmata import ArduinoMega, util
from core.utils import detect_arduino_usb_serial
from core.sensor_processing.sampling import mean_sample
from settings import PH_SENSOR_PIN, TDS_SENSOR_PIN, WATER_LEVEL_PIN, NUTRIENT_LEVEL, PH_DOWNER_LEVEL_PIN
from pprint import pprint

print(f"[UART][INFO] Connecting Arduino Mega Board ...")
board = ArduinoMega(detect_arduino_usb_serial())
it = util.Iterator(board)
it.start()
for pin in [PH_SENSOR_PIN, TDS_SENSOR_PIN, WATER_LEVEL_PIN, NUTRIENT_LEVEL, PH_DOWNER_LEVEL_PIN]:
    board.analog[pin].enable_reporting()
print(f"[UART][OK] Connected to this board: {board}")


def read_tds(board: ArduinoMega, _pipeline_dict: dict, _temperature=25) -> dict:
    d = mean_sample(board=board, pin=TDS_SENSOR_PIN)
    # temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    temperature_compensation = 1.0 + 0.02 * (_temperature - 25.0)
    # temperature compensation
    voltage_compensation = d["voltage"] / temperature_compensation
    tds = (
                  133.42 * voltage_compensation * voltage_compensation * voltage_compensation
                  - 255.86 * voltage_compensation * voltage_compensation
                  + 857.39 * voltage_compensation
          ) * 0.5  # convert voltage value to tds value
    _pipeline_dict["tds_average_adc"] = d["adc"]
    _pipeline_dict["tds_average_voltage"] = d["voltage"]
    _pipeline_dict["tds_average_ppm"] = tds
    return _pipeline_dict


while True:
    d = {}
    pprint(read_tds(board, d))
