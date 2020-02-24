#! /usr/bin/env python3
from api.message_device import *


class SDHCI16295TestDevice(MessageDevice):

    def __init__(self, device, name):
     super(SDHCI16295TestDevice, self).__init__(device, name)

    msg_templates = [
    ] + MessageDevice.msg_templates
