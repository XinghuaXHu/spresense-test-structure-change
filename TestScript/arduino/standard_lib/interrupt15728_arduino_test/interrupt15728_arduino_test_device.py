#! /usr/bin/env python3
from api.message_device import *


class Interrupt15728TestDevice(MessageDevice):

    def __init__(self, device, name):
        super(Interrupt15728TestDevice, self).__init__(device, name)
