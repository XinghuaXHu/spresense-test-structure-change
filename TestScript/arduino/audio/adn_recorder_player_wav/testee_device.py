#! /usr/bin/env python3
from api.message_device import *


class TesteeDevice(MessageDevice):

    def __init__(self, device, name):
        super(TesteeDevice, self).__init__(device, name)
