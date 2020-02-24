#! /usr/bin/env python3
from api.nuttshell_device import *


class ColorsensorExampleTestDevice(NuttshellDevice):

    COLORSENSOREND = 'COLORSENSOREND'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(ColorsensorExampleTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(COLORSENSOREND,
                        ('SCU Timestamp', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
