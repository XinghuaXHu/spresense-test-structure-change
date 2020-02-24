#! /usr/bin/env python3
from api.message_device import *


class TesterDevice(MessageDevice):
    def __init__(self, device, name):
        super(TesterDevice, self).__init__(device, name)
