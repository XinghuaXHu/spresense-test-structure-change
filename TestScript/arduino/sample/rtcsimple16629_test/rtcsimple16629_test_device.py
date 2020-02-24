#! /usr/bin/env python3
from api.nuttshell_device import *


class RctSimple16629TestDevice(NuttshellDevice):

    RctSimpleOK = 'RctSimpleOK'

    def __init__(self, device, name):
        super(RctSimple16629TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(RctSimpleOK,
                        ('SUCCESS', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
