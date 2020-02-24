#! /usr/bin/env python3
from api.nuttshell_device import *


class RctGnss16628TestDevice(NuttshellDevice):

    RctGnssOK = 'RctGnssOK'

    def __init__(self, device, name):
        super(RctGnss16628TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(RctGnssOK,
                        ('SUCCESS', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
