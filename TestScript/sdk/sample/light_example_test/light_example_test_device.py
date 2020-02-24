#! /usr/bin/env python3
from api.nuttshell_device import *


class LightExampleTestDevice(NuttshellDevice):

    LIGHTEND = 'LIGHTEND'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(LightExampleTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(LIGHTEND,
                        ('SCU Timestamp:', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
