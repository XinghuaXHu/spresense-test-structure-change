#! /usr/bin/env python3
from api.message_device import *


class OsTestTestDevice(MessageDevice):


    def __init__(self, device, name):
        super(OsTestTestDevice, self).__init__(device, name)

    msg_templates = [] + MessageDevice.msg_templates
