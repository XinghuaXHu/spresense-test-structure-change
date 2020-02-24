#! /usr/bin/env python3
from api.nuttshell_device import *


class AsmpExtensionsTestDevice(NuttshellDevice):

    TC_MESSAGE = 'TC_MESSAGE'

    KEY_MESSAGE = 'Message'



    def __init__(self, device, name):
        super(AsmpExtensionsTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(TC_MESSAGE,
                        ('ASMP test!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
