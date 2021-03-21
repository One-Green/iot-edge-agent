import sys

sys.path.insert(0, "..")
import time
from core.utils import detect_arduino_usb_serial
from pyfirmata import ArduinoMega, util
from input_handler import read_sensors, read_tds, sampling_adc, mean_sample
from settings import PH_SENSOR_PIN, TDS_SENSOR_PIN, WATER_LEVEL_PIN, NUTRIENT_LEVEL, PH_DOWNER_LEVEL_PIN

board = ArduinoMega(detect_arduino_usb_serial())
it = util.Iterator(board)
it.start()
for pin in [PH_SENSOR_PIN, TDS_SENSOR_PIN, WATER_LEVEL_PIN, NUTRIENT_LEVEL, PH_DOWNER_LEVEL_PIN]:
    board.analog[pin].enable_reporting()

print(f"[OK] Connected to this board: {board}")
#
# while True:
#     print(mean_sample(board=board, pin=PH_SENSOR_PIN))
#     #time.sleep(0.5)
#     #adc = board.analog[PH_SENSOR_PIN].read()
#     #print(adc)
#     #time.sleep(0.5)

