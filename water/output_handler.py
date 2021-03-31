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

digital_output_list = [
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
        return {
            "pin_name": _pin_name,
            "changed": True,
            "state": _state
        }
    else:
        return {
            "pin_name": _pin_name,
            "changed": False,
            "state": _state
        }


def off_on_digital_output(board: ArduinoMega, _callback: dict):
    for _key, _pin, _pin_name in digital_output_list:
        yield apply_if_changed(
            board=board,
            _callback=_callback,
            _key=_key,
            _pin=_pin,
            _pin_name=_pin_name
        )
