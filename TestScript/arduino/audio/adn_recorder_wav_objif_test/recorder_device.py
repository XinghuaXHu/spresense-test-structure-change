#! /usr/bin/env python3
from api.message_device import *


class AdnRecorderWavObjifTestDevice(MessageDevice):

    def __init__(self, device, name):
        super(AdnRecorderWavObjifTestDevice, self).__init__(device, name)
