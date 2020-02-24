#! /usr/bin/env python3
from api.nuttshell_device import *


def parse_dsp_type(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, line.split()[4].strip('?'))
    else:
        raise ParsingException()


class DspInstallerTestDevice(NuttshellDevice):

    PROMPT = 'DSP_TYPE'
    KEY_TYPE = 'DSP_TYPE'

    def __init__(self, device, name):
        super(DspInstallerTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(PROMPT,
                        ('Select where to install', KEY_TYPE, parse_dsp_type),
                        ),
    ] + NuttshellDevice.msg_templates
