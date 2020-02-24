#! /usr/bin/env python3
from api.nuttshell_device import *


class ConversionTestDevice(NuttshellDevice):

    CONVERSIONOK = 'CONVERSIONOK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(ConversionTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(CONVERSIONOK,
                        ('the task Conversion test ok!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
