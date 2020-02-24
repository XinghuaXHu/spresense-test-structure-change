#! /usr/bin/env python3
from api.nuttshell_device import *


class GnssTestDevice(NuttshellDevice):

    GNSSOK = 'GNSSOK'
 
    def __init__(self, device, name):
        super(GnssTestDevice, self).__init__(device, name)

    msg_templates = [
    ] + NuttshellDevice.msg_templates
