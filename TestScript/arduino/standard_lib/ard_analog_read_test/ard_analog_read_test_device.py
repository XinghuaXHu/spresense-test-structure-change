#! /usr/bin/env python3
from api.nuttshell_device import *


class ArdAnalogReadTestDevice(NuttshellDevice):

    ARD_ANALOG_READOK = 'ARD_ANALOG_READOK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(ArdAnalogReadTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(ARD_ANALOG_READOK,
                        ('the task ArdAnalogRead test ok!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
