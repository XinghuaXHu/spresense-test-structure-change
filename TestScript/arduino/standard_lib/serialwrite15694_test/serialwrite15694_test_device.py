#! /usr/bin/env python3
from api.nuttshell_device import *


class Serialwrite15694TestDevice(NuttshellDevice):

    SERIALWRITE15694OK = 'SERIALWRITE15694OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Serialwrite15694TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SERIALWRITE15694OK,
                        ('nA0\n$He5\n4aG', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
