#! /usr/bin/env python3
from api.nuttshell_device import *


class SlaveSpresenseDevice(NuttshellDevice):

    def __init__(self, device, name):
        super(SlaveSpresenseDevice, self).__init__(device, name)

