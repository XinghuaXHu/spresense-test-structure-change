#! /usr/bin/env python3
from api.nuttshell_device import *


class SdCreateWriteTestDevice(NuttshellDevice):

    WRITE_TO_TEST_DONE = 'WRITE_TO_TEST_DONE'
    WRITE_TO_T1ST1_DONE = 'WRITE_TO_T1ST1_DONE'
    WRITE_TO_T2ST2_DONE = 'WRITE_TO_T2ST2_DONE'
    WRITE_TO_DUMMY1_DONE = 'WRITE_TO_DUMMY1_DONE'
    WRITE_TO_DUMMY2_DONE = 'WRITE_TO_DUMMY2_DONE'
    WRITE_TO_SUNNY1_DONE = 'WRITE_TO_SUNNY1_DONE'
    WRITE_TO_DUMMY3_DONE = 'WRITE_TO_DUMMY3_DONE'
    WRITE_TO_DUMMY4_DONE = 'WRITE_TO_DUMMY4_DONE'
    WRITE_TO_SUNNY2_DONE = 'WRITE_TO_SUNNY2_DONE'
    TEST_DONE = 'TEST_DONE'

    def __init__(self, device, name):
        super(SdCreateWriteTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(WRITE_TO_TEST_DONE,
                        ('write to test done', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(WRITE_TO_T1ST1_DONE,
                        ('write to T1st1 done', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(WRITE_TO_T2ST2_DONE,
                        ('write to T2st2 done', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(WRITE_TO_DUMMY1_DONE,
                        ('write to Dummy1 done', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(WRITE_TO_DUMMY2_DONE,
                        ('write to Dummy2 done', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(WRITE_TO_SUNNY1_DONE,
                        ('write to Sunny1 done', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(WRITE_TO_DUMMY3_DONE,
                        ('write to Dummy3 done', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(WRITE_TO_DUMMY4_DONE,
                        ('write to Dummy4 done', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(WRITE_TO_SUNNY2_DONE,
                        ('write to Sunny2 done', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(TEST_DONE,
                        ('sd_create_write test done', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
