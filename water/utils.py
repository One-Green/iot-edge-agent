import os
import logging


def get_log_level():

    lvl = logging.NOTSET
    log_level = os.getenv("LOG_LEVEL", "DEBUG")

    if log_level == "DEBUG":
        lvl = logging.DEBUG
    elif log_level == ["WARN", "WARNING"]:
        lvl = logging.WARNING
    elif log_level == "ERROR":
        lvl = logging.ERROR
    elif log_level in ["CRITICAL", "FATAL"]:
        lvl = logging.CRITICAL
    elif log_level == "INFO":
        lvl = logging.INFO

    return lvl
