#! /usr/bin/env python3
from api.nuttshell_device import *


class I2CScanner15884TestDevice(NuttshellDevice):

    I2CScannerOK = 'I2CScannerOK'

    def __init__(self, device, name):
        super(I2CScanner15884TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(I2CScannerOK,
                        ('SUCCESS', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
