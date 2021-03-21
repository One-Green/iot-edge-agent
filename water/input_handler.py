import sys

sys.path.insert(0, "..")
import time
from pyfirmata import ArduinoMega
from core.sensor_processing.math import BasicLinearReg
from settings import PH_SENSOR_PIN, TDS_SENSOR_PIN
from settings import WATER_LEVEL_PIN, NUTRIENT_LEVEL, PH_DOWNER_LEVEL_PIN
import statistics

reg = BasicLinearReg()

# Analog Map definition
adc_to_voltage_model = reg.fit([0, 1024], [0, 5])  # convert analog 0-1024 to 0-5V
water_level_model = reg.fit([0, 1024], [0, 450])  # convert analog 0-1024 (0-5V) to 0-450cm ultrasonic range
nutrient_level_model = reg.fit([0, 1024], [0, 450])  # convert analog 0-1024 (0-5V) to 0-450cm ultrasonic range
ph_downer_level_model = reg.fit([0, 1024], [0, 450])  # convert analog 0-1024 (0-5V) to 0-450cm ultrasonic range

sonar_processing = [
    (WATER_LEVEL_PIN, water_level_model, "water_level"),
    (NUTRIENT_LEVEL, nutrient_level_model, "nutrient_level"),
    (PH_DOWNER_LEVEL_PIN, ph_downer_level_model, "ph_downer_level")
]


def sampling_adc(board: ArduinoMega, pin: int, sample_number: int = 30) -> dict:
    adc_list = []
    voltage_list = []
    for _ in range(sample_number):
        adc = board.analog[pin].read()
        time.sleep(0.01)
        print(adc)
        if type(adc) in [float, int]:
            adc_list.append(adc)
            voltage_list.append(adc_to_voltage_model(adc))
    print(adc_list)
    return {"adc_list": adc_list, "voltage_list": voltage_list}


def mean_sample(*args, **kwargs):
    d = sampling_adc(*args, **kwargs)
    print(d)
    return {
        "adc": statistics.mean(d["adc_list"]),
        "voltage": statistics.mean(d["voltage_list"])
    }


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
    return tds


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
    d = read_sonars(board=board, _pipeline_dict=d)
    return d
