#! /usr/bin/env python3
from api.nuttshell_device import *


class SdReadTestDevice(NuttshellDevice):

    def __init__(self, device, name):
        super(SdReadTestDevice, self).__init__(device, name)

    SD_READ_START = 'SD_READ_START'
    ARBIT_START = 'ARBIT_START'
    ARBIT_BUF1 = 'ARBIT_BUF1'
    ARBIT_BUF2 = 'ARBIT_BUF2'
    ARBIT_DONE = 'ARBIT_DONE'
    SIMUL_START = 'SIMUL_START'
    SIMUL_BUF1 = 'SIMUL_BUF1'
    SIMUL_BUF2 = 'SIMUL_BUF2'
    SIMUL_BUF3 = 'SIMUL_BUF3'
    SIMUL_BUF4 = 'SIMUL_BUF4'
    SIMUL_BUF5 = 'SIMUL_BUF5'
    SIMUL_BUF6 = 'SIMUL_BUF6'
    SIMUL_BUF7 = 'SIMUL_BUF7'
    SIMUL_BUF8 = 'SIMUL_BUF8'
    SIMUL_DONE = 'SIMUL_DONE'
    NEWLY_START = 'NEWLY_START'
    NEWLY_BUF1 = 'NEWLY_BUF1'
    NEWLY_BUF2 = 'NEWLY_BUF2'
    NEWLY_DONE = 'NEWLY_DONE'
    SD_READ_DONE = 'SD_READ_DONE'

    msg_templates = [
        MessageTemplate(SD_READ_START,
                        ('sd_read test start', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(ARBIT_START,
                        ('read_arbitrary test start', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(ARBIT_BUF1,
                        ('read buffer 1:t', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(ARBIT_BUF2,
                        ('read buffer 3:s \n', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(ARBIT_DONE,
                        ('read_arbitrary test done', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(SIMUL_START,
                        ('read_simultaneously test start', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(SIMUL_BUF1,
                        ('Dummy read buffer1:t3E', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(SIMUL_BUF2,
                        ('Sunny read buffer2:82', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(SIMUL_BUF3,
                        ('Sunny read buffer2:32', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(SIMUL_BUF4,
                        ('Sunny read buffer2:115', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(SIMUL_BUF5,
                        ('Sunny read buffer2:40', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(SIMUL_BUF6,
                        ('Sunny read buffer2:41', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(SIMUL_BUF7,
                        ('Sunny read buffer2:13', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(SIMUL_BUF8,
                        ('Sunny read buffer2:10', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(SIMUL_DONE,
                        ('read_simultaneously test done', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(NEWLY_START,
                        ('read_newly test start', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(NEWLY_BUF1,
                        ('read buf:\n', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(NEWLY_BUF2,
                        ('1dH #$', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(NEWLY_DONE,
                        ('read_newly test done', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(SD_READ_DONE,
                        ('sd_read test done', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
