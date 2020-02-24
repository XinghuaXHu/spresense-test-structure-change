#! /usr/bin/env python3
from api.nuttshell_device import *


class AdvancedIO15860TestDevice(NuttshellDevice):

    ADVANCEDIO15860OK = 'ADVANCEDIO15860OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(AdvancedIO15860TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(ADVANCEDIO15860OK,
                        ('OKFARM', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
