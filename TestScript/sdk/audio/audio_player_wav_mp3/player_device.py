#! /usr/bin/env python3
from api.nuttshell_device import *


def parse_running_time(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, line.split()[3].strip() + ' s')
    else:
        raise ParsingException()


class PlayerDevice(NuttshellDevice):

    PLAYER_START = 'PLAYER_START'
    PLAYER_EXIT = 'PLAYER_EXIT'
    ACAPULCO = 'ACAPULCO'
    RUNNING_TIME = 'RUNNING_TIME'

    KEY_DURATION = 'Duration'

    def __init__(self, device, name):
        super(PlayerDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(PLAYER_START,
                        ('Start AudioPlayer example', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(PLAYER_EXIT,
                        ('Exit spr_sdk_15451', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(ACAPULCO,
                        ('AcaPulco ES4(4Ch)', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(RUNNING_TIME,
                        ('Running time', KEY_DURATION, parse_running_time),
                        ),
    ] + NuttshellDevice.msg_templates
