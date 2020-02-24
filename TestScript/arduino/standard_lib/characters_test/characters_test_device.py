#! /usr/bin/env python3
from api.nuttshell_device import *


class CharactersTestDevice(NuttshellDevice):

    CHARACTERSOK = 'CHARACTERSOK'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(CharactersTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(CHARACTERSOK,
                        ('the task Characters test ok!', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
