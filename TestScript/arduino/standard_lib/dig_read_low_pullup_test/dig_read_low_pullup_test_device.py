#! /usr/bin/env python3
from api.nuttshell_device import *


class DigReadLowPullupTestDevice(NuttshellDevice):

    DIG_READ_LOW_END = 'DIG_READ_LOW_END'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(DigReadLowPullupTestDevice, self).__init__(device, name)

    msg_templates = [
    ] + NuttshellDevice.msg_templates
