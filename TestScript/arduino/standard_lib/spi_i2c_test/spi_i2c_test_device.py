#! /usr/bin/env python3
from api.nuttshell_device import *


class SPI_I2C_TestDevice(NuttshellDevice):

    def __init__(self, device, name):
        super(SPI_I2C_TestDevice, self).__init__(device, name)

