#! /usr/bin/env python3
from api.nuttshell_device import *


class ProximityExampleTestDevice(NuttshellDevice):

    PROXIMITYEND = 'PROXIMITYEND'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(ProximityExampleTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(PROXIMITYEND,
                        ('SCU Timestamp:', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
