import sys

sys.path.insert(0, "..")

import time
from datetime import datetime
from core.db import get_state, db_update_or_create
from settings import CALLBACK_UPDATED_AT_KEY, CALLBACK_MAX_TIME_DELTA
from settings import SAFETY_MODE_KEY
from settings import logger

while True:
    if (
        datetime.now()
        - datetime.fromisoformat(get_state()[CALLBACK_UPDATED_AT_KEY].decode("utf-8"))
    ) > CALLBACK_MAX_TIME_DELTA:
        safe = True
        logger.error("Safe mode is activated")
    else:
        safe = False
        logger.info("Safe mode is deactivated")
    db_update_or_create(SAFETY_MODE_KEY, safe)
    time.sleep(0.5)
