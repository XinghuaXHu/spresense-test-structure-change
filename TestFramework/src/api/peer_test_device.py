#! /usr/bin/env python3
from api.nuttshell_device import *

def parse_record_data(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, line.split()[-1].strip('.'))
    else:
        raise ParsingException()

def parse_running_time(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, line.split()[3].strip() + ' s')
    else:
        raise ParsingException()

class PeerTestDevice(NuttshellDevice):
    # audio group
    RUNNING_TIME = 'RUNNING_TIME'
    RECORD_DATA = 'RECORD_DATA'

    KEY_DURATION = 'Duration'
    KEY_FILE_PATH = 'File path'
    def __init__(self, device, name):
        super(PeerTestDevice, self).__init__(device, name)
    
    msg_templates = [
        MessageTemplate(RUNNING_TIME,
                        ('Running time', KEY_DURATION, parse_running_time),
                        ),
        MessageTemplate(RECORD_DATA,
                        ('Record data to', KEY_FILE_PATH, parse_record_data),
                        ),
    ] + NuttshellDevice.msg_templates