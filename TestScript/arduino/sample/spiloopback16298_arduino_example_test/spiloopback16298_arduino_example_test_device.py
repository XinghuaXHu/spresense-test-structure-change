#! /usr/bin/env python3
from api.nuttshell_device import *


class SpiLoopback16298TestDevice(NuttshellDevice):

    GNSSOK = 'GNSSOK'

    def __init__(self, device, name):
     super(SpiLoopback16298TestDevice, self).__init__(device, name)

    msg_templates = [
     MessageTemplate(GNSSOK,
              ('SUCCESS', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
