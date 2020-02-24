#! /usr/bin/env python3
from api.nuttshell_device import *


class Serialread15699TestDevice(NuttshellDevice):

    SERIALREAD15699OK = 'SERIALREAD15699OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Serialread15699TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SERIALREAD15699OK,
                        ('OKFARM', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
