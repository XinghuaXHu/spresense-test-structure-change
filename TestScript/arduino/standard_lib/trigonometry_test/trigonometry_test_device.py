#! /usr/bin/env python3
from api.nuttshell_device import *


class TrigonometryTestDevice(NuttshellDevice):

    TRIGONOMETRYOK = 'TRIGONOMETRYOK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(TrigonometryTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(TRIGONOMETRYOK,
                        ('the task Trigonometry test ok!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
