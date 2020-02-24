#! /usr/bin/env python3
from api.nuttshell_device import *


class SDHCI16294TestDevice(NuttshellDevice):

    GNSSOK = 'GNSSOK'

    def __init__(self, device, name):
     super(SDHCI16294TestDevice, self).__init__(device, name)

    msg_templates = [
     MessageTemplate(GNSSOK,
              ('SUCCESS', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
