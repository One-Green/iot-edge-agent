from core.utils import serial_ports

for _ in serial_ports():
    print(_)
