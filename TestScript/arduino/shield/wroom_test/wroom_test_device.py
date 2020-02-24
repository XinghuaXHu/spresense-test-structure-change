#! /usr/bin/env python3
from api.nuttshell_device import *


class WroomTestDevice(NuttshellDevice):

    WROOMOK = 'WROOMOK'
    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(WroomTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(WROOMOK,
                        ('the task Wroom test ok!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
