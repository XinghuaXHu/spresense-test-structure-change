#! /usr/bin/env python3
from api.nuttshell_device import *


class SdFileVerifyTestDevice(NuttshellDevice):
    def __init__(self, device, name):
        super(SdFileVerifyTestDevice, self).__init__(device, name)

    msg_templates = [
    ] + NuttshellDevice.msg_templates
