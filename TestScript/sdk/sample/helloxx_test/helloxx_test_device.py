#! /usr/bin/env python3
from api.nuttshell_device import *


class HelloxxTestDevice(NuttshellDevice):

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(HelloxxTestDevice, self).__init__(device, name)

    msg_templates = [
    ] + NuttshellDevice.msg_templates
