#! /usr/bin/env python3
from api.nuttshell_device import *


class RandomTestDevice(NuttshellDevice):

    RANDOMOK = 'RANDOMOK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(RandomTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(RANDOMOK,
                        ('Random Numbers the task Conversion test ok!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
