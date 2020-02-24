#! /usr/bin/env python3
from api.message_device import *


class RecorderDevice(MessageDevice):

    def __init__(self, device, name):
        super(RecorderDevice, self).__init__(device, name)
