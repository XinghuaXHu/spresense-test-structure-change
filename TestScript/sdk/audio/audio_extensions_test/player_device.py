#! /usr/bin/env python3
from api.nuttshell_device import *


def parse_running_time(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, line.split()[3].strip() + ' s')
    else:
        raise ParsingException()


def parse_volume(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, line.split()[3].strip())
    else:
        raise ParsingException()


def parse_beep(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, line.split()[2].strip() + 'dB, ' + line.split()[2].strip() + 'Hz')
    else:
        raise ParsingException()


class PlayerDevice(NuttshellDevice):

    PLAYER_START = 'PLAYER_START'
    PLAYER_MUTE_ON = 'PLAYER_MUTE_ON'
    PLAYER_MUTE_OFF = 'PLAYER_MUTE_OFF'
    PLAYER_CHANGE_VOLUME = 'PLAYER_CHANGE_VOLUME'
    PLAYER_CHANGE_VOLUME_INPUT = 'PLAYER_CHANGE_VOLUME_INPUT'
    PLAYER_BEEP = 'PLAYER_BEEP'
    PLAYER_EXIT = 'PLAYER_EXIT'
    ACAPULCO = 'ACAPULCO'
    RUNNING_TIME = 'RUNNING_TIME'

    KEY_DURATION = 'Duration'
    KEY_VOLUME = 'Volume'
    KEY_BEEP = 'BEEP'

    def __init__(self, device, name):
        super(PlayerDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(PLAYER_START,
                        ('Start Player Test', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(PLAYER_MUTE_ON,
                        ('Player Mute On', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(PLAYER_MUTE_OFF,
                        ('Player Mute Off', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(PLAYER_CHANGE_VOLUME,
                        ('Player Change Volume', KEY_VOLUME, parse_volume),
                        ),
        MessageTemplate(PLAYER_CHANGE_VOLUME_INPUT,
                        ('Player Change Input', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(PLAYER_BEEP,
                        ('Player Beep', KEY_BEEP, parse_beep),
                        ),
        MessageTemplate(PLAYER_EXIT,
                        ('Exit Player Test', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(ACAPULCO,
                        ('AcaPulco ES4(4Ch)', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(RUNNING_TIME,
                        ('Running time', KEY_DURATION, parse_running_time),
                        ),
    ] + NuttshellDevice.msg_templates
