#! /usr/bin/env python3
from api.nuttshell_device import *


class SdkPwmExampleDevice(NuttshellDevice):

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(SdkPwmExampleDevice, self).__init__(device, name)

    msg_templates = [
    ] + NuttshellDevice.msg_templates
