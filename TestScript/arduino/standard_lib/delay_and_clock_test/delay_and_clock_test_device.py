#! /usr/bin/env python3
from api.nuttshell_device import *


class DelayAndClockTestDevice(NuttshellDevice):

    DELAYANDCLOCKOK = 'DELAYANDCLOCKOK'
    DELAYMS = 'DELAYMS'
    DELAYUS = 'DELAYUS'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(DelayAndClockTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(DELAYMS,
                        ('delay 1000 ms', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(DELAYUS,
                        ('delay 1000000 us', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
