#! /usr/bin/env python3
from api.nuttshell_device import *


class AudioTestDevice(NuttshellDevice):
    def __init__(self, device, name):
        super(AudioTestDevice, self).__init__(device, name)

