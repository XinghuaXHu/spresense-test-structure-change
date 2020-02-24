#! /usr/bin/env python3
from api.message_device import *


class PlayerDevice(MessageDevice):
    def __init__(self, device, name):
        super(PlayerDevice, self).__init__(device, name)
