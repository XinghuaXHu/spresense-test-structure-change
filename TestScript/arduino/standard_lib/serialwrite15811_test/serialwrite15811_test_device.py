#! /usr/bin/env python3
from api.nuttshell_device import *


class Serialwrite15811TestDevice(NuttshellDevice):

    SERIALWRITE15811OK = 'SERIALWRITE15811OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Serialwrite15811TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SERIALWRITE15811OK,
                        ('nA0\n$He5\n4aG', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
