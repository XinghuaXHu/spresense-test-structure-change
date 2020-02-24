#! /usr/bin/env python3
from api.message_device import *


class Alarm16611TestDevice(MessageDevice):

    SYSTEMOK = 'SYSTEMOK'

    def __init__(self, device, name):
        super(Alarm16611TestDevice, self).__init__(device, name)

    msg_templates = [
    ] + MessageDevice.msg_templates
