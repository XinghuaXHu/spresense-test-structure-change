#! /usr/bin/env python3
from api.nuttshell_device import *


class RctTimeTone16630TestDevice(NuttshellDevice):

    RctToneOK = 'RctToneOK'

    def __init__(self, device, name):
        super(RctTimeTone16630TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(RctToneOK,
                        ('SUCCESS', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
