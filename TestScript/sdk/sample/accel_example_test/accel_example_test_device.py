#! /usr/bin/env python3
from api.nuttshell_device import *


class AccelExampleTestDevice(NuttshellDevice):

    ACCELEND = 'ACCELEND'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(AccelExampleTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(ACCELEND,
                        ('ACCEL example end', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
