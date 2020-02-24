#! /usr/bin/env python3
from api.nuttshell_device import *


class Adt7310TestDevice(NuttshellDevice):

    ADT7310OK = 'ADT7310OK'
    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(Adt7310TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(ADT7310OK,
                        ('the task Adt7310 test ok!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
