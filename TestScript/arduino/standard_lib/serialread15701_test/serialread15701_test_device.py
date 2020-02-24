#! /usr/bin/env python3
from api.nuttshell_device import *


class Serialread15701TestDevice(NuttshellDevice):

    SERIALREAD15701OK = 'SERIALREAD15701OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Serialread15701TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SERIALREAD15701OK,
                        ('OKFARM', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
