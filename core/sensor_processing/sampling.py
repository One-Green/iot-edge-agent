import time
import statistics
from pyfirmata import ArduinoMega
from core.sensor_processing.math import BasicLinearReg

adc_to_voltage_model = BasicLinearReg().fit([0, 1], [0, 5])  # convert analog 0-1024 (0-1 with firmata) to 0-5V


def sampling_adc(board: ArduinoMega, pin: int, sample_number: int = 30) -> dict:
    adc_list = []
    voltage_list = []
    for _ in range(sample_number):
        adc = board.analog[pin].read()
        time.sleep(0.01)
        if type(adc) in [float, int]:
            adc_list.append(adc)
            voltage_list.append(adc_to_voltage_model(adc))
    return {"adc_list": adc_list, "voltage_list": voltage_list}


def mean_sample(*args, **kwargs) -> dict:
    d = sampling_adc(*args, **kwargs)
    return {
        "adc": statistics.mean(d["adc_list"]),
        "voltage": statistics.mean(d["voltage_list"])
    }
