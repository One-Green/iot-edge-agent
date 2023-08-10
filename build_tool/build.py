import os.path
import subprocess
import configparser
from .logger import logger
import shutil
import uuid

_pio_run = "cd {src_path} && pio run"
_str_flag = '-D "{k}=\\"{v}\\""'
_int_flag = '-D {k}={v}'
_bool_flag = '-D "{k}=1"'


def pio_generate_build_flags(_vars: list):
    flags = []
    for _ in _vars:
        if type(_) == dict:
            k = [*_][0]
            v = _[k]
            if type(v) == str:
                r = _str_flag.format(k=k, v=v)
                flags.append(r)
            if type(v) == int:
                r = _int_flag.format(k=k, v=v)
                flags.append(r)
        if type(_) == set:
            k = [*_][0]
            r = _bool_flag.format(k=k)
            flags.append(r)
    return "\n".join(flags)


def pio_update_platform_ini(_scr_path: str, _vars: str):
    platform_ini_file = "platformio.ini"
    config = configparser.ConfigParser()
    platform_ini_path = os.path.join(_scr_path, platform_ini_file)
    config.read(platform_ini_path)
    logger.debug({"platformio_ini_path": platform_ini_path})
    config["env:esp32dev"]["build_flags"] = _vars
    with open(platform_ini_path, 'w') as _:
        config.write(_)


def pio_build(_src_path: str):
    command = _pio_run.format(src_path=_src_path)
    logger.debug(msg={"build_command": command})
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
    process.wait()


def make_zip(_build_path: str):
    dist_uuid = f"dist-{uuid.uuid1()}"
    uuid_zip_path = shutil.make_archive(dist_uuid, 'zip', _build_path)
    return uuid_zip_path, f"{dist_uuid}.zip"
