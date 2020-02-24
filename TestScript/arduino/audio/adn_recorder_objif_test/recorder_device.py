#! /usr/bin/env python3
from api.message_device import *


class AdnRecorderObjifTestDevice(MessageDevice):

    def __init__(self, device, name):
        super(AdnRecorderObjifTestDevice, self).__init__(device, name)
