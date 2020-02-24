#! /usr/bin/env python3
from api.nuttshell_device import *


class AnalogReadTestDevice(NuttshellDevice):

    END = 'END'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(AnalogReadTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(END,
                        ('Read end', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
