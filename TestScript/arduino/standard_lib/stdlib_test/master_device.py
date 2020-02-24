#! /usr/bin/env python3
from api.nuttshell_device import *


class MasterDevice(NuttshellDevice):

    def __init__(self, device, name):
        super(MasterDevice, self).__init__(device, name)

