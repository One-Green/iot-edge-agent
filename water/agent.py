import sys
sys.path.insert(0, "..")
import time
from core.utils import detect_arduino_usb_serial
from pyfirmata import ArduinoMega, util


board = ArduinoMega(detect_arduino_usb_serial())
it = util.Iterator(board)
it.start()
