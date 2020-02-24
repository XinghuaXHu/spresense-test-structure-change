#! /usr/bin/env python3
from api.nuttshell_device import *


class Interrupt15731TestDevice(NuttshellDevice):

    def __init__(self, device, name):
        super(Interrupt15731TestDevice, self).__init__(device, name)
