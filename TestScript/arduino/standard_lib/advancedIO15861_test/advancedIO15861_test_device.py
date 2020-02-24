#! /usr/bin/env python3
from api.nuttshell_device import *


class AdvancedIO15861TestDevice(NuttshellDevice):

    ADVANCEDIO15861OK = 'ADVANCEDIO15861OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(AdvancedIO15861TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(ADVANCEDIO15861OK,
                        ('OKFARM', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
