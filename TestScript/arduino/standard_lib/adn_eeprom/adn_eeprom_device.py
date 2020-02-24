#! /usr/bin/env python3
from api.nuttshell_device import *


class AdnEepromDevice(NuttshellDevice):

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(AdnEepromDevice, self).__init__(device, name)

    msg_templates = [
    ] + NuttshellDevice.msg_templates
