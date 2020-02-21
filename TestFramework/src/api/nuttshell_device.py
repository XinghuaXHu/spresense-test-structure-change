#! /usr/bin/env python3

from api.message_device import *


def parse_nsh_prompt(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        command = line.strip().split('\x1b[K')[1]

        if command == '':
            command = 'No command'

        msg.set_value(user_key, command)
    else:
        raise ParsingException()


# noinspection PyUnusedLocal
def parse_nsh_ls(lines, expected_key, user_key, msg):

    dirs = []
    files = []
    path = None

    line = next(lines)

    path = line.strip()
    if path == '/:':
        path = path.strip(':')
    else:
        path = path.replace(':', '/')

    if ':' in line:
        while True:
            line = next(lines)

            if '/' in line:
                directory = str(line.strip().strip('/'))
                dirs.append(directory)
            else:
                if 'nsh>' not in line:
                    file = str(line.strip())
                    files.append(file)

            if 'nsh>' in line:
                break

        ls = {
            'path': path,
            'files': files,
            'dirs': dirs
        }

    else:
        raise ParsingException()

    msg.set_value(user_key, ls)


def parse_cmd_failed(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        command = line.split(': ')[1].strip()
        reason = line.split(': ')[3].strip()
        execution_failed = {
            'command': command,
            'reason': reason,
        }
    else:
        raise ParsingException()

    msg.set_value(user_key, execution_failed)


def parse_bytes(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, 'B' + line.split('B')[1].strip())
    else:
        raise ParsingException()


def parse_error(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, line.split(':')[1].strip())
    else:
        raise ParsingException()


def parse_app_error(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, line.split(':')[1].strip())
    else:
        raise ParsingException()


class NuttshellDevice(MessageDevice):

    NUTTSHELL = 'NUTTSHELL'
    NSH_PROMPT = 'NSH_PROMPT'
    NSH_LS = 'NSH_LS'
    CMD_FAILED = 'CMD_FAILED'
    BYTES = 'BYTES'
    ERROR = 'ERROR'
    APP_ERROR = 'APP_ERROR'

    KEY_LS = 'ls'
    KEY_COMMAND = 'Command'
    KEY_EXECUTION_FAILED = 'Execution failed'
    KEY_VALUE = 'Value'
    KEY_ERROR = 'Error'

    def __init__(self, device, name):
        super(NuttshellDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(NSH_LS,
                        ('ls', None, MessageTemplate.validate_line),
                        (None, KEY_LS, parse_nsh_ls),
                        ),
        MessageTemplate(NUTTSHELL,
                        ('NuttShell (NSH)', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(NSH_PROMPT,
                        ('nsh>', KEY_COMMAND, parse_nsh_prompt),
                        ),
        MessageTemplate(CMD_FAILED,
                        ('failed', KEY_EXECUTION_FAILED, parse_cmd_failed),
                        ),
        MessageTemplate(BYTES,
                        ('**', KEY_VALUE, parse_bytes),
                        ),
        MessageTemplate(APP_ERROR,
                        ('ERROR:', KEY_ERROR, parse_error),
                        ),
        MessageTemplate(APP_ERROR,
                        ('Error:', KEY_ERROR, parse_app_error),
                        ),
    ]
