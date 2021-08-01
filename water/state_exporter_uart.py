import sys

sys.path.insert(0, "..")

from core.db import get_state
import simplejson as json
from datetime import datetime
import serial
import time
from settings import logger
from settings import ESP32_WATCH_WATCHDOG_TTY, ALIVE_MESSAGE
from settings import ESP32_TFT_DISPLAY_TTY


watch_dog_serial = serial.Serial(ESP32_WATCH_WATCHDOG_TTY, baudrate=57_600)
logger.info(f"UART Connected to {ESP32_WATCH_WATCHDOG_TTY=}")

tft_display_serial = serial.Serial(ESP32_TFT_DISPLAY_TTY, baudrate=57_600)
logger.info(f"UART Connected to {ESP32_TFT_DISPLAY_TTY=}")

while True:
    d = get_state()
    d["utc_now"] = datetime.utcnow().isoformat()
    # python dict to text, use encode() to binary
    watch_dog_serial.write(json.dumps(ALIVE_MESSAGE).encode())
    logger.info(f"UART Alive Json sent to {ESP32_WATCH_WATCHDOG_TTY=}")

    tft_display_serial.write(json.dumps(d).encode())
    logger.info(f"UART State Json sent to {ESP32_TFT_DISPLAY_TTY=}")
    time.sleep(0.2)
