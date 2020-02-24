#! /usr/bin/env python3
from api.nuttshell_device import *


class LteapiTestDevice(NuttshellDevice):

    LTEAPIOK = 'LTEAPIOK'
    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(LteapiTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(LTEAPIOK,
                        ('the task Lteapi test ok!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
