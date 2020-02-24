#! /usr/bin/env python3
from api.nuttshell_device import *

def parse_still_data(lines, expected_key, user_key, msg):
    line = next(lines)
    if expected_key in line:
        msg.set_value(user_key, line.split(":")[1].strip())
    else:
        raise ParsingException()

class CameraSdkExampleTestDevice(NuttshellDevice):

    CAMERAEND = 'CAMERAEND'
    IMAGESIZE = 'IMAGESIZE'
    KEY_MESSAGE = 'Message'
    STILL_DATA = 'STILL_DATA'
    KEY_FILE_PATH = 'File path'

    def __init__(self, device, name):
        super(CameraSdkExampleTestDevice, self).__init__(device, name)

    msg_templates = [
        #MessageTemplate(CAMERAEND,
                        #('/mnt/spif/VIDEO009.YUV', None, MessageTemplate.validate_line),
                        #),
        MessageTemplate(STILL_DATA,
                        ('FILENAME', KEY_FILE_PATH, parse_still_data),
                        ),
    ] + NuttshellDevice.msg_templates