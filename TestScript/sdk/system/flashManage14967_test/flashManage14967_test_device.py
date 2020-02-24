#! /usr/bin/env python3
from api.nuttshell_device import *


class FlashManage14967TestDevice(NuttshellDevice):

    FLASHMANAGE14967END = 'FLASHMANAGE14967END'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(FlashManage14967TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(FLASHMANAGE14967END,
                        ('update: ret=0', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
