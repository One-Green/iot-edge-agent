import time
from pyfirmata import ArduinoMega, util
from pyfirmata.util import ping_time_to_distance

"""
int OGIO::baseUltrasonicReader(int trigger, int echo) {
    long duration;
int distance;
// Clear the trigPin by setting it LOW: \
digitalWrite(trigger, LOW);
delayMicroseconds(5);
// Trigger the sensor by setting the trigPin high for 10 microseconds:
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);
    // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds: \
duration = pulseIn(echo, HIGH);
// Calculate the distance: \
distance = duration * 0.034 / 2;
return distance;

"""

# append common package
import sys

sys.path.insert(0, "..")
from core.utils import detect_arduino_usb_serial

board = ArduinoMega(detect_arduino_usb_serial())
it = util.Iterator(board)
it.start()
echo_pin = board.get_pin('d:3:o')

while True:
    board.digital[2].write(0)
    time.sleep(0.05)
    board.digital[2].write(1)
    time.sleep(0.1)
    board.digital[2].write(0)
    print(ping_time_to_distance(echo_pin.ping()))
