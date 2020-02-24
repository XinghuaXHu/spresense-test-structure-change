#! /usr/bin/env python3
from api.nuttshell_device import *


class Serialread15702TestDevice(NuttshellDevice):

    SERIALREAD15702OK = 'SERIALREAD15702OK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Serialread15702TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SERIALREAD15702OK,
                        ('OKFARM', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
