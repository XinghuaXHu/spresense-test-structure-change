#! /usr/bin/env python3
from api.nuttshell_device import *


class Serialwrite15697TestDevice(NuttshellDevice):

    SERIALWRITE15697OK = 'SERIALWRITE15697OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Serialwrite15697TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SERIALWRITE15697OK,
                        ('nA0\n$He5\n4aG', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
