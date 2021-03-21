import sys
import glob
import serial
import math


def serial_ports():
    """ Lists serial port names
        source: https://stackoverflow.com/questions/12090503/listing-available-com-ports-with-python
        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result


def detect_arduino_usb_serial():
    for _ in serial_ports():
        if "usbserial" in _:
            return _


class BasicLinearReg:

    @staticmethod
    def mean(xs):
        return sum(xs) / len(xs)

    @staticmethod
    def std(xs, m):
        normalizer = len(xs) - 1
        return math.sqrt(sum((pow(x - m, 2) for x in xs)) / normalizer)

    @staticmethod
    def pearson_r(xs, ys, m_x, m_y):
        sum_xy = 0
        sum_sq_v_x = 0
        sum_sq_v_y = 0

        for (x, y) in zip(xs, ys):
            var_x = x - m_x
            var_y = y - m_y
            sum_xy += var_x * var_y
            sum_sq_v_x += pow(var_x, 2)
            sum_sq_v_y += pow(var_y, 2)
        return sum_xy / math.sqrt(sum_sq_v_x * sum_sq_v_y)

    def fit(self, x, y):
        """
        Fit and generate model function
        :param x: list
        :param y:
        :return:
        """
        m_x = self.mean(x)
        m_y = self.mean(y)
        r = self.pearson_r(x, y, m_x, m_y)

        b = r * (self.std(y, m_y) / self.std(x, m_x))
        a = m_y - b * m_x

        def model(_x):
            return b * _x + a

        return model
