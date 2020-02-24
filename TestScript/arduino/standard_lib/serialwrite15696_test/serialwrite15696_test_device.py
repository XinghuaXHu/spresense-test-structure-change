#! /usr/bin/env python3
from api.nuttshell_device import *


class Serialwrite15696TestDevice(NuttshellDevice):

    SERIALWRITE15696OK = 'SERIALWRITE15696OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Serialwrite15696TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SERIALWRITE15696OK,
                        ('nA0\n$He5\n4aG', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
