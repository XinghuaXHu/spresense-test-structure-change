#! /usr/bin/env python3
from api.nuttshell_device import *


class GpioExampleTestDevice(NuttshellDevice):

    GPIO79OUT = 'GPIO79OUT'
    GPIO80EN = 'GPIO80EN'
    GPIO79HI = 'GPIO79HI'
    GPIO80HI = 'GPIO80HI'
    GPIO80IRQ = 'GPIO80IRQ'

    KEY_MESSAGE = 'Message'

    def __init__(self, device, name):
        super(GpioExampleTestDevice, self).__init__(device, name)

    msg_templates = [
        MessageTemplate(GPIO79OUT,
                        ('( 79)PIN_EMMC_DATA2    : 0     /O 2  --   0    -1', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(GPIO80EN,
                        ('( 80)PIN_EMMC_DATA3    : 0    I/  2  --   0    42  Rise NF EN', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(GPIO79HI,
                        ('( 79)PIN_EMMC_DATA2    : 0     /O 2  --   1    -1', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(GPIO80HI,
                        ('( 80)PIN_EMMC_DATA3    : 0    I/  2  --   1    42  Rise NF EN', None, MessageTemplate.validate_line),
                        ),
        MessageTemplate(GPIO80IRQ,
                        ('42   80     1', None, MessageTemplate.validate_line),
                        ),
    ] + NuttshellDevice.msg_templates
