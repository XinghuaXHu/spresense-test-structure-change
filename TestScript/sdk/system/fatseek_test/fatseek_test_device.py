#! /usr/bin/env python3
from api.nuttshell_device import *


class FatseekTestDevice(NuttshellDevice):

    FATEND = 'SUCCES'

    def __init__(self, device, name):
        super(FatseekTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(FATEND,
                        ('SUCCES', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
