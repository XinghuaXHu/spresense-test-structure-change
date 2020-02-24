#! /usr/bin/env python3
from api.nuttshell_device import *


class DecimatorExampleTestDevice(NuttshellDevice):

    DECIMATOREND = 'DECIMATORND'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(DecimatorExampleTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(DECIMATOREND,
                        ('SCU Timestamp', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
