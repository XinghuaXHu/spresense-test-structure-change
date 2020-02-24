#! /usr/bin/env python3
from api.nuttshell_device import *


class RtcTestDevice(NuttshellDevice):

    def __init__(self, device, name):
        super(RtcTestDevice, self).__init__(device, name)

    msg_templates = [
    ] + NuttshellDevice.msg_templates
