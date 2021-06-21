import sys

sys.path.insert(0, "..")

import time
from datetime import datetime
from core.db import get_state, db_update_or_create
from settings import CALLBACK_UPDATED_AT_KEY, CALLBACK_MAX_TIME_DELTA
from settings import SAFETY_MODE_KEY

while True:

    if (
            datetime.now() - datetime.fromisoformat(
                get_state()[CALLBACK_UPDATED_AT_KEY].decode('utf-8')
            )
    ) > CALLBACK_MAX_TIME_DELTA:
        safe = True
        print("[ERROR] Safe = True")
    else:
        safe = False
        print("[INFO] Safe = False")
    db_update_or_create(SAFETY_MODE_KEY, safe)
    time.sleep(0.5)
