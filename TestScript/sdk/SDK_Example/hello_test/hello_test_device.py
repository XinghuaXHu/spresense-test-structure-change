#! /usr/bin/env python3
from api.nuttshell_device import *


class HelloTestDevice(NuttshellDevice):

    HELLOWORLD = 'HELLOWORLD'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(HelloTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(HELLOWORLD,
                        ('Hello, World!!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
