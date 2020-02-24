#! /usr/bin/env python3
from api.message_device import *


class WatchdogReboot16626TestDevice(MessageDevice):

    RctToneOK = 'RctToneOK'

    def __init__(self, device, name):
        super(WatchdogReboot16626TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(RctToneOK,
                        ('SUCCESS', None, MessageTemplate.validate_line),
                        ),
    ] + MessageDevice.msg_templates
