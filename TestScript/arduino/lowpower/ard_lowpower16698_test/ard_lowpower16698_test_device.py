#! /usr/bin/env python3
from api.message_device import *


class SpresenseTestDevice(MessageDevice):

    DELAYMS = 'DELAYMS'
    DELAYUS = 'DELAYUS'

    def __init__(self, device, name):
        super(SpresenseTestDevice, self).__init__(device, name)

    msg_templates = [
    ] + MessageDevice.msg_templates

