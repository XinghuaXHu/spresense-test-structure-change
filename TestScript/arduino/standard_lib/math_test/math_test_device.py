#! /usr/bin/env python3
from api.nuttshell_device import *


class MathTestDevice(NuttshellDevice):

    MATHOK = 'MATHOK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(MathTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(MATHOK,
                        ('the task Math test ok!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
