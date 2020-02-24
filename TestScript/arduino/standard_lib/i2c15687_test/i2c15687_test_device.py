#! /usr/bin/env python3
from api.nuttshell_device import *


class I2C15687TestDevice(NuttshellDevice):

    I2COK = 'I2COK'

    def __init__(self, device, name):
        super(I2C15687TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(I2COK,
                        ('SUCCESS', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
