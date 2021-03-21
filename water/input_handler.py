import sys

sys.path.insert(0, "..")
import time
from pyfirmata import ArduinoMega
from core.sensor_processing.math import BasicLinearReg
from core.sensor_processing.sampling import mean_sample
from settings import PH_SENSOR_PIN, TDS_SENSOR_PIN
from settings import WATER_LEVEL_PIN, NUTRIENT_LEVEL, PH_DOWNER_LEVEL_PIN

reg = BasicLinearReg()

# Analog Map definition
water_level_model = reg.fit([0, 1024], [0, 450])  # convert analog 0-1024 (0-5V) to 0-450cm ultrasonic range
nutrient_level_model = reg.fit([0, 1024], [0, 450])  # convert analog 0-1024 (0-5V) to 0-450cm ultrasonic range
ph_downer_level_model = reg.fit([0, 1024], [0, 450])  # convert analog 0-1024 (0-5V) to 0-450cm ultrasonic range

sonar_processing = [
    (WATER_LEVEL_PIN, water_level_model, "water_level"),
    (NUTRIENT_LEVEL, nutrient_level_model, "nutrient_level"),
    (PH_DOWNER_LEVEL_PIN, ph_downer_level_model, "ph_downer_level")
]


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


def read_ph(board: ArduinoMega, _pipeline_dict: dict) -> dict:
    d = mean_sample(board=board, pin=PH_SENSOR_PIN)
    ph = 7 + ((2.5 - d["voltage"]) / 0.18)
    _pipeline_dict["ph_average_adc"] = d["adc"]
    _pipeline_dict["ph_average_voltage"] = d["voltage"]
    _pipeline_dict["ph_average_value"] = ph
    return _pipeline_dict


def read_sonars(board: ArduinoMega, _pipeline_dict: dict) -> dict:
    for pin, lin_reg, name in sonar_processing:
        adc: int = board.analog[pin].read()
        if adc:
            value: float = lin_reg(adc)
            _pipeline_dict[name + "_adc"] = adc
            _pipeline_dict[name + "_cm"] = value
        else:
            _pipeline_dict[name + "_adc"] = 0
            _pipeline_dict[name + "_cm"] = 0
        time.sleep(0.02)
    return _pipeline_dict


def read_sensors(board: ArduinoMega) -> dict:
    d = {}
    d = read_tds(board=board, _pipeline_dict=d)
    d = read_ph(board=board, _pipeline_dict=d)
    d = read_sonars(board=board, _pipeline_dict=d)
    return d
