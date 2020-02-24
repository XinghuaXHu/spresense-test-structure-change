#! /usr/bin/env python3
from api.nuttshell_device import *


class CameraExampleTestDevice(NuttshellDevice):

    CAMERAEND = 'CAMERAEND'
    IMAGESIZE = 'IMAGESIZE'
    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(CameraExampleTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(CAMERAEND,
                        ('FILENAME:/mnt/spif/VIDEO001.JPG', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
