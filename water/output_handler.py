"""
Output handler

mqtt water/controller callback response example
v = {"tag": "water",
     "water_pump_signal": 0,
     "nutrient_pump_signal": 0,
     "ph_downer_pump_signal": 0,
     "mixer_pump_signal": 1,
     "tds_max_level": 0.0,
     "tds_min_level": 0.0,
     "ph_max_level": 0.0,
     "ph_min_level": 0.0
     }
Author: Shanmugathas Vigneswaran
mail : shanmugathas.vigneswaran@outlook.fr
"""
import sys

sys.path.insert(0, "..")
from pyfirmata import ArduinoMega
from settings import WATER_PUMP_PIN, NUTRIENT_PUMP_PIN, PH_DOWNER_PUMP_PIN, MIXER_PUMP_PIN
from settings import SAFETY_MODE_KEY
from settings import logger
from core.db import get_state

digital_output_list = [
    # callback key  ------  output pin -----  pin name (from settings.py)
    ("water_pump_signal", WATER_PUMP_PIN, f'{WATER_PUMP_PIN=}'.split('=')[0]),
    ("nutrient_pump_signal", NUTRIENT_PUMP_PIN, f'{NUTRIENT_PUMP_PIN=}'.split('=')[0]),
    ("ph_downer_pump_signal", PH_DOWNER_PUMP_PIN, f'{PH_DOWNER_PUMP_PIN=}'.split('=')[0]),
    ("mixer_pump_signal", MIXER_PUMP_PIN, f'{MIXER_PUMP_PIN=}'.split('=')[0])
]


def apply_if_changed(
        board: ArduinoMega,
        _callback: dict,
        _key: str,
        _pin: int,
        _pin_name: str
) -> dict:
    """
    Return true if changed
    :param board:
    :param _callback:
    :param _key:
    :param _pin:
    :param _pin_name:
    :return:
    """
    _state = bool(board.digital[_pin].read())
    _callback_requirement = bool(_callback[_key])

    if _callback_requirement != _state:
        board.digital[_pin].write(_callback[_key])
        logger.info(f"Arduino pin {_pin_name}={_pin} changed to {_state}")
        return {
            "pin_name": _pin_name,
            "changed": True,
            "state": _state,
            "pin": _pin
        }
    else:
        logger.debug(f"Arduino pin {_pin_name}={_pin} changed to {_state}")
        return {
            "pin_name": _pin_name,
            "changed": False,
            "state": _state,
            "pin": _pin
        }


def off_on_digital_output(board: ArduinoMega):
    status = get_state()
    if not status[SAFETY_MODE_KEY]:
        for _key, _pin, _pin_name in digital_output_list:
            yield apply_if_changed(
                board=board,
                _callback=status,
                _key=_key,
                _pin=_pin,
                _pin_name=_pin_name
            )


def set_actuator_safe(board: ArduinoMega, force_safe: bool = False) -> None:
    max_attempt = 10
    status = get_state()
    if status[SAFETY_MODE_KEY] or force_safe:
        logger.error("Safe mod detected")
        for _, _pin, __ in digital_output_list:
            cnt = 0
            logger.error(f"Handling {__}={_pin}")
            while bool(board.digital[_pin].read()):
                board.digital[_pin].write(0)
                logger.error(f"Arduino setting {_pin=} > OFF")
                cnt = +1
                if cnt > max_attempt:
                    logger.critical(f"Can not switch off {__}={_pin},  {max_attempt=} reached")
                    break
            else:
                logger.debug(f"Handling {__}={_pin} already OFF")
