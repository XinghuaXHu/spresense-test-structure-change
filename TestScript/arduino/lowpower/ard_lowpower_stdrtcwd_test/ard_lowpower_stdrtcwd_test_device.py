#! /usr/bin/env python3
from api.message_device import *


class SpresenseTestDevice(MessageDevice):

    DELAYMS = 'DELAYMS'
    DELAYUS = 'DELAYUS'

    def __init__(self, device, name):
        super(SpresenseTestDevice, self).__init__(device, name)

    msg_templates = [
                        MessageTemplate(DELAYMS,
                                        ('delay 1000 ms', None, MessageTemplate.validate_line),
                                        ),
                        MessageTemplate(DELAYUS,
                                        ('delay 1000000 us', None, MessageTemplate.validate_line),
                                        ),
    ] + MessageDevice.msg_templates

