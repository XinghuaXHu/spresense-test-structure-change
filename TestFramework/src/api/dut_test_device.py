#! /usr/bin/env python3
from api.nuttshell_device import *


class DutTestDevice(NuttshellDevice):
    def __init__(self, device, name):
        super(DutTestDevice, self).__init__(device, name)

