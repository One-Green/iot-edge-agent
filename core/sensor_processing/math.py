"""
Avoid using heavy package like (pandas+numpy)
function/method => mean, std, pearson_r, fit :
    Generate model to scale analog data to user defined range
    Source: http://code.activestate.com/recipes/578914-simple-linear-regression-with-pure-python/
    Author : Chaobin Tang  http://code.activestate.com/recipes/users/4174076/
    License : MIT
"""

import math


class BasicLinearReg:
    """
    Set of method to make linear regression
    """

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
