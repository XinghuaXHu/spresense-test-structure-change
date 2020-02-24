#! /usr/bin/env python3
from api.nuttshell_device import *


class RctAlarm16627TestDevice(NuttshellDevice):

    RctAlarmOK = 'RctAlarmOK'

    def __init__(self, device, name):
        super(RctAlarm16627TestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(RctAlarmOK,
                        ('SUCCESS', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
