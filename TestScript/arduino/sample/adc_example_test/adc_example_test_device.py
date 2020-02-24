#! /usr/bin/env python3
from api.nuttshell_device import *


class AdcExampleTestDevice(NuttshellDevice):

    ADCEND = 'ADCEND'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(AdcExampleTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(ADCEND,
                        ('ADC example end', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
