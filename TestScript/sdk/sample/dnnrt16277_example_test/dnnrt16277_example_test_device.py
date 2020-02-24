#! /usr/bin/env python3
from api.nuttshell_device import *


class SpresenseTestDevice(NuttshellDevice):

    HELLOWORLD = 'HELLOWORLD'

    KEY_MESSAGE = 'Message'
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
        super(SpresenseTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(HELLOWORLD,
                        ('Hello, World!!', None, MessageTemplate.validate_line),
                        ),

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
