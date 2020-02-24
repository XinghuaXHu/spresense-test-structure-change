#! /usr/bin/env python3
from api.nuttshell_device import *


class SlaveDevice(NuttshellDevice):

    def __init__(self, device, name):
        super(SlaveDevice, self).__init__(device, name)

