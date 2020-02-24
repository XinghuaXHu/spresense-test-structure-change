#! /usr/bin/env python3
from api.nuttshell_device import *


class Serialwrite15693TestDevice(NuttshellDevice):

    SERIALWRITE15693OK = 'SERIALWRITE15693OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Serialwrite15693TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SERIALWRITE15693OK,
                        ('nA0\n$He5\n4aG', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
