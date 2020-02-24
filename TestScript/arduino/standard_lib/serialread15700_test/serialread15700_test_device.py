#! /usr/bin/env python3
from api.nuttshell_device import *


class Serialread15700TestDevice(NuttshellDevice):

    SERIALREAD15700OK = 'SERIALREAD15700OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Serialread15700TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SERIALREAD15700OK,
                        ('OKFARM', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
