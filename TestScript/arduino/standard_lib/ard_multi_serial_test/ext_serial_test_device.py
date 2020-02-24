#! /usr/bin/env python3
from api.nuttshell_device import *


class ArdMultiSerialTestDevice(NuttshellDevice):

    EXT_SERIAL_START = 'EXT_SERIAL_START'
    EXT_SERIAL_DELAY = 'EXT_SERIAL_DELAY'
    EXT_SERIAL_MONITOR_COMPLETED = 'EXT_SERIAL_MONITOR_COMPLETED'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(ArdMultiSerialTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(EXT_SERIAL_START,
                        ('read D00-D28 pins', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(EXT_SERIAL_DELAY,
                        ('delay 3sec', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
