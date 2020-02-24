#! /usr/bin/env python3
from api.nuttshell_device import *


class Serialwrite15695TestDevice(NuttshellDevice):

    SERIALWRITE15695OK = 'SERIALWRITE15695OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Serialwrite15695TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SERIALWRITE15695OK,
                        ('nA0\n$He5\n4aG', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
