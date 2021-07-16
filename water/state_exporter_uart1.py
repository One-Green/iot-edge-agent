import sys

sys.path.insert(0, "..")

from core.db import get_state
import simplejson as json
from datetime import datetime
import serial
import time

# Create serial instance
#  Rx  |  ---> UART1
#  Tx  |
#  Gnd
#  +5V
#  +5V
s = serial.Serial("/dev/ttyS1", baudrate=57_600)

while True:
    d = get_state()
    d["utc_now"] = datetime.utcnow().isoformat()
    # python dict to text, use encode() to binary
    body = json.dumps(d).encode()
    s.write(body)
    time.sleep(0.5)
