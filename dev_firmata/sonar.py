import sys

sys.path.insert(0, "..")
from pyfirmata import ArduinoMega, util
from core.utils import detect_arduino_usb_serial
from core.sensor_processing.math import BasicLinearReg
from settings import PH_SENSOR_PIN, TDS_SENSOR_PIN, WATER_LEVEL_PIN, NUTRIENT_LEVEL, PH_DOWNER_LEVEL_PIN
from pprint import pprint
import time

print(f"[UART][INFO] Connecting Arduino Mega Board ...")
board = ArduinoMega("/dev/tty.usbserial-1450")
it = util.Iterator(board)
it.start()
for pin in [PH_SENSOR_PIN, TDS_SENSOR_PIN, WATER_LEVEL_PIN, NUTRIENT_LEVEL, PH_DOWNER_LEVEL_PIN]:
    board.analog[pin].enable_reporting()
print(f"[UART][OK] Connected to this board: {board}")

reg = BasicLinearReg()

# Analog Map definition
water_level_model = reg.fit([0, 0.94], [0, 450])  # convert analog 0-1024 (0-5V) to 0-450cm ultrasonic range
nutrient_level_model = reg.fit([0, 0.94], [0, 450])  # convert analog 0-1024 (0-5V) to 0-450cm ultrasonic range
ph_downer_level_model = reg.fit([0, 0.94], [0, 450])  # convert analog 0-1024 (0-5V) to 0-450cm ultrasonic range

sonar_processing = [
    (WATER_LEVEL_PIN, water_level_model, "water_level"),
    (NUTRIENT_LEVEL, nutrient_level_model, "nutrient_level"),
    (PH_DOWNER_LEVEL_PIN, ph_downer_level_model, "ph_downer_level")
]


def read_sonars(board: ArduinoMega, _pipeline_dict: dict) -> dict:
    for pin, lin_reg, name in sonar_processing:
        # TODO : use adc mean_read instead single point
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


while True:
    # TODO : missing LOW PASS Filter on PWM > ADC
    v = board.analog[0].read()
    print(v)
    time.sleep(0.5)
