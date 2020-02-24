#! /usr/bin/env python3
from api.nuttshell_device import *


class DigReadHighTestDevice(NuttshellDevice):

    DIG_READ_HIGH_END = 'DIG_READ_HIGH_END'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(DigReadHighTestDevice, self).__init__(device, name)

    msg_templates = [
    ] + NuttshellDevice.msg_templates
