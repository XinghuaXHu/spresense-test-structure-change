#! /usr/bin/env python3
from api.nuttshell_device import *


class FatrepeatTestDevice(NuttshellDevice):

    FATEND = 'SUCCES'

    def __init__(self, device, name):
        super(FatrepeatTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(FATEND,
                        ('SUCCES', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
