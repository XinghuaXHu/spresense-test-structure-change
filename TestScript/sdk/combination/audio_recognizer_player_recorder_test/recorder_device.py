#! /usr/bin/env python3
from api.nuttshell_device import *


def parse_running_time(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, line.split()[3].strip() + ' s')
    else:
        raise ParsingException()


def parse_record_data(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, line.split()[-1].strip('.'))
    else:
        raise ParsingException()


def parse_cmd_failed(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        print(line)
        command = 'x'
        reason = 'y'
        record_data = {
            'command': command,
            'reason': reason,
        }
    else:
        raise ParsingException()

    msg.set_value(user_key, record_data)


class RecorderDevice(NuttshellDevice):

    RECORDER_START = 'RECORDER_START'
    RECORDER_EXIT = 'RECORDER_EXIT'
    ACAPULCO = 'ACAPULCO'
    RUNNING_TIME = 'RUNNING_TIME'
    RECORD_DATA = 'RECORD_DATA'

    KEY_DURATION = 'Duration'
    KEY_FILE_PATH = 'File path'

    def __init__(self, device, name):
        super(RecorderDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(RECORDER_START,
                        ('Start AudioRecorder example', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(RECORDER_EXIT,
                        ('Exit AudioRecorder example', None, MessageTemplate.validate_line),
        ),
        MessageTemplate(ACAPULCO,
                        ('AcaPulco ES4(4Ch)', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(RUNNING_TIME,
                        ('Running time', KEY_DURATION, parse_running_time),
                        ),
        MessageTemplate(RECORD_DATA,
                        ('Record data to', KEY_FILE_PATH, parse_record_data),
                        ),
    ] + NuttshellDevice.msg_templates
