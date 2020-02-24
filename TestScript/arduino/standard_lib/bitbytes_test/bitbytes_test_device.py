#! /usr/bin/env python3
from api.nuttshell_device import *


class BitBytesTestDevice(NuttshellDevice):

    BITBYTESOK = 'BITBYTESOK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(BitBytesTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(BITBYTESOK,
                        ('the task BitandBytes test ok!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
