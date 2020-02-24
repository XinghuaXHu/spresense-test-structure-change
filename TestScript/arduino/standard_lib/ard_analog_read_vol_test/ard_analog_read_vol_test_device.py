#! /usr/bin/env python3
from api.nuttshell_device import *


class ArdAnalogReadVolTestDevice(NuttshellDevice):

    ARD_ANALOG_READ_VOLOK = 'ARD_ANALOG_READ_VOLOK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(ArdAnalogReadVolTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(ARD_ANALOG_READ_VOLOK,
                        ('the task ArdAnalogReadVol test ok!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
