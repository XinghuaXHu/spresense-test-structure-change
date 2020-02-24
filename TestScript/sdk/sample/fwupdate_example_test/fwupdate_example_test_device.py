#! /usr/bin/env python3
from api.nuttshell_device import *


class FwupdateExampleTestDevice(NuttshellDevice):

    FWUPDATEEND = 'FWUPDATEEND'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(FwupdateExampleTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(FWUPDATEEND,
                        ('update: ret=0', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
