#! /usr/bin/env python3
from api.nuttshell_device import *


class JpegDecode16614TestDevice(NuttshellDevice):

    SYSTEMOK = 'SYSTEMOK'

    def __init__(self, device, name):
        super(JpegDecode16614TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(SYSTEMOK,
                        ('SUCCESS', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
