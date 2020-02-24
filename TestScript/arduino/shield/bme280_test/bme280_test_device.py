#! /usr/bin/env python3
from api.nuttshell_device import *


class Bme280TestDevice(NuttshellDevice):

    BME280OK = 'BME280OK'
    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Bme280TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(BME280OK,
                        ('the task Bme280 test ok!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
