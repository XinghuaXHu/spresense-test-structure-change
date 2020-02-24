#! /usr/bin/env python3
from api.nuttshell_device import *


class MagExampleTestDevice(NuttshellDevice):

    MAGEND = 'MAGEND'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(MagExampleTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(MAGEND,
                        ('SCU Timestamp', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
