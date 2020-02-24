#! /usr/bin/env python3
from api.nuttshell_device import *


class DigWriteHighTestDevice(NuttshellDevice):

    DIG_WRITE_COMPLETED = 'DIG_WRITE_COMPLETED'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(DigWriteHighTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(DIG_WRITE_COMPLETED,
                        ('wrote D00-D28 pins HIGH', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
