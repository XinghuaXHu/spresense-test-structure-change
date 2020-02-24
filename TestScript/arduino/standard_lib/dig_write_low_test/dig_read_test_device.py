#! /usr/bin/env python3
from api.nuttshell_device import *


class DigReadTestDevice(NuttshellDevice):

    DIG_READ_START = 'DIG_READ_START'
    DIG_READ_DELAY = 'DIG_READ_DELAY'
    DIG_READ_MONITOR_COMPLETED = 'DIG_READ_MONITOR_COMPLETED'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(DigReadTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(DIG_READ_START,
                        ('read D00-D28 pins', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(DIG_READ_DELAY,
                        ('delay 3sec', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
