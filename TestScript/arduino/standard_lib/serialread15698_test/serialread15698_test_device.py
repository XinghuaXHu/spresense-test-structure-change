#! /usr/bin/env python3
from api.nuttshell_device import *


class Serialread15698TestDevice(NuttshellDevice):

    SERIALWRITE15698OK = 'SERIALWRITE15698OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Serialread15698TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SERIALWRITE15698OK,
                        ('nA0\n$He5\n4aG', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
