#! /usr/bin/env python3
from api.nuttshell_device import *


class System14966TestDevice(NuttshellDevice):

    SYSTEMOK = 'SYSTEMOK'

    def __init__(self, device, name):
        super(System14966TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SYSTEMOK,
                        ('SUCCESS', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
