#! /usr/bin/env python3
import sys
import os
import time

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.stdlib_test.master_device import MasterDevice
from tc.stdlib_test.slave_device import SlaveDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


class StdlibTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(StdlibTestTcRunner, self).__init__(conf)

        self.master_device = MasterDevice(kwargs.get('master_device', None), 'MASTER_DEVICE') \
            if kwargs.get('master_device', None) else None
        self.slave_device = SlaveDevice(kwargs.get('slave_device', None), 'SLAVE_DEVICE') \
            if kwargs.get('slave_device', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least two device are needed!')

    def get_devices_list(self):
        return [self.master_device, self.slave_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release

    def generate_test_groups(self, arguments, log=None):
        timeout = 15

        master_device, slave_device = self.get_devices_list()

        setup = [
            Step(Action.EXECUTE, master_device, ('master_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
            Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(1)),
            Step(Action.EXECUTE, slave_device, None, lambda test, source, data: time.sleep(1)),
            Step(Action.WRITE, slave_device, 'i2c'),
            Step(Action.WAIT_FOR, slave_device, 'I2C init'),
            Step(Action.WRITE, slave_device, 'spi'),
            Step(Action.WAIT_FOR, slave_device, 'SPI init'),
            Step(Action.WRITE, master_device, 'spiclock 80000'),
            Step(Action.WRITE, master_device, 'spibit 1'),
            Step(Action.WRITE, master_device, 'spitxm 0'),
            Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(1)),
            Step(Action.EXECUTE, slave_device, None, lambda test, source, data: time.sleep(1)),
        ]

        teardown = [
            Step(Action.EXECUTE, master_device, 'master_reboot_mon', Test.remove_monitor),
            Step(Action.WRITE, slave_device, 'i2c_end'),
            Step(Action.WRITE, slave_device, 'spi_end'),
        ]

        Arduino_Example_test_i2c_by_lower_case_letter = Test(
            name='Arduino_Example_test_i2c_by_lower_case_letter',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WRITE, master_device, 'clock 1000000'),
                Step(Action.WRITE, master_device, 'i2c_send ' + 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),

                Step(Action.WRITE, master_device, 'i2c_send ' + 'uvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'uvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'uvwxyz'),

                Step(Action.WRITE, master_device, 'i2c_send ' + 'abcdef'),
                Step(Action.WAIT_FOR, master_device, 'abcdef'),
                Step(Action.WAIT_FOR, master_device, 'abcdef'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_i2c_by_upper_case_letter = Test(
            name='Arduino_Example_test_i2c_by_upper_case_letter',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WRITE, master_device, 'clock 100000'),
                Step(Action.WRITE, master_device, 'i2c_send ' + 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),

                Step(Action.WRITE, master_device, 'i2c_send ' + 'ABCDEF'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEF'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEF'),

                Step(Action.WRITE, master_device, 'i2c_send ' + 'UVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'UVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'UVWXYZ'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_i2c_by_alphabet = Test(
            name='Arduino_Example_test_i2c_by_alphabet',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WRITE, master_device, 'clock 400000'),
                Step(Action.WRITE, master_device, 'i2c_send ' + 'LJLJLJLJLADFASdafdf'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASdafdf'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASdafdf'),

                Step(Action.WRITE, master_device, 'i2c_send ' + 'abcABC'),
                Step(Action.WAIT_FOR, master_device, 'abcABC'),
                Step(Action.WAIT_FOR, master_device, 'abcABC'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_i2c_by_number = Test(
            name='Arduino_Example_test_i2c_by_number',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WRITE, master_device, 'clock 1000000'),
                Step(Action.WRITE, master_device, 'i2c_send ' + '0123456789'),
                Step(Action.WAIT_FOR, master_device, '0123456789'),
                Step(Action.WAIT_FOR, master_device, '0123456789'),

                Step(Action.WRITE, master_device, 'i2c_send ' + '9876543210'),
                Step(Action.WAIT_FOR, master_device, '9876543210'),
                Step(Action.WAIT_FOR, master_device, '9876543210'),

                Step(Action.WRITE, master_device, 'i2c_send ' + '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_i2c_by_punctuation = Test(
            name='Arduino_Example_test_i2c_by_punctuation',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WRITE, master_device, 'clock 100000'),
                Step(Action.WRITE, master_device, 'i2c_send ' + '|,./;:[]{}~!@#$%^&*()_+=-~'),
                Step(Action.WAIT_FOR, master_device, '|,./;:[]{}~!@#$%^&*()_+=-~'),
                Step(Action.WAIT_FOR, master_device, '|,./;:[]{}~!@#$%^&*()_+=-~'),

                Step(Action.WRITE, master_device, 'i2c_send ' + '\"\',./'),
                Step(Action.WAIT_FOR, master_device, '\"\',./'),
                Step(Action.WAIT_FOR, master_device, '\"\',./'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_i2c_by_mix_mode = Test(
            name='Arduino_Example_test_i2c_by_mix_mode',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WRITE, master_device, 'clock 400000'),
                Step(Action.WRITE, master_device, 'i2c_send ' + '56789%@@$^&*Pjj(9}z?!~`89.,./\\'),
                Step(Action.WAIT_FOR, master_device, '56789%@@$^&*Pjj(9}z?!~`89.,./\\'),
                Step(Action.WAIT_FOR, master_device, '56789%@@$^&*Pjj(9}z?!~`89.,./\\'),
            ],
            teardown=teardown
        )


        Arduino_Example_test_spiclock4Mhz = Test(
            name='Arduino_Example_test_spiclock4Mhz',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WRITE, slave_device, 'spi clock:4000000'),
                Step(Action.WAIT_FOR, slave_device, 'SPI init'),
                Step(Action.WRITE, master_device, 'spiclock 80000'),

                Step(Action.WRITE, master_device, 'spi_send ' + 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_sendArr ' + 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16 ' + 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16Arr ' + 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_spiclock2Mhz = Test(
            name='Arduino_Example_test_spiclock2Mhz',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WRITE, slave_device, 'spi clock:2000000'),
                Step(Action.WAIT_FOR, slave_device, 'SPI init'),
                Step(Action.WRITE, master_device, 'spiclock 40000'),

                Step(Action.WRITE, master_device, 'spi_send ' + '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_sendArr ' + '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16 ' + '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16Arr ' + '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_spimode0 = Test(
            name='Arduino_Example_test_spimode0',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),

                Step(Action.WRITE, slave_device, 'spi mode0'),
                Step(Action.WAIT_FOR, slave_device, 'SPI init'),
                Step(Action.WRITE, master_device, 'spitxm 0'),

                Step(Action.WRITE, master_device, 'spi_send ' + 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_sendArr ' + 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16 ' + 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16Arr ' + 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_spimode1 = Test(
            name='Arduino_Example_test_spimode1',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),

                Step(Action.WRITE, slave_device, 'spi mode1'),
                Step(Action.WAIT_FOR, slave_device, 'SPI init'),
                Step(Action.WRITE, master_device, 'spitxm 1'),

                Step(Action.WRITE, master_device, 'spi_send ' + 'LJLJLJLJLADFASdafd'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASdafd'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASdafd'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_sendArr ' + 'LJLJLJLJLADFASdafd'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASdafd'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASdafd'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16 ' + 'LJLJLJLJLADFASdafd'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASdafd'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASdafd'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16Arr ' + 'LJLJLJLJLADFASdafd'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASdafd'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASdafd'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_spimode2 = Test(
            name='Arduino_Example_test_spimode2',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),

                Step(Action.WRITE, slave_device, 'spi mode2'),
                Step(Action.WAIT_FOR, slave_device, 'SPI init'),
                Step(Action.WRITE, master_device, 'spitxm 2'),
                Step(Action.WRITE, master_device, 'spi_send ' + 'LJLJLJLJLADFASd1243434af34dx'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASd1243434af34dx'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASd1243434af34dx'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_sendArr ' + 'LJLJLJLJLADFASd1243434af34dx'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASd1243434af34dx'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASd1243434af34dx'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16 ' + 'LJLJLJLJLADFASd1243434af34dx'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASd1243434af34dx'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASd1243434af34dx'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16Arr ' + 'LJLJLJLJLADFASd1243434af34dx'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASd1243434af34dx'),
                Step(Action.WAIT_FOR, master_device, 'LJLJLJLJLADFASd1243434af34dx'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_spimode3 = Test(
            name='Arduino_Example_test_spimode3',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),

                Step(Action.WRITE, slave_device, 'spi mode3'),
                Step(Action.WAIT_FOR, slave_device, 'SPI init'),
                Step(Action.WRITE, master_device, 'spitxm 3'),

                Step(Action.WRITE, master_device, 'spi_send ' + 'abc234hK2H3423&^$#KH'),
                Step(Action.WAIT_FOR, master_device, 'abc234hK2H3423&^$#KH'),
                Step(Action.WAIT_FOR, master_device, 'abc234hK2H3423&^$#KH'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_sendArr ' + 'abc234hK2H3423&^$#KH'),
                Step(Action.WAIT_FOR, master_device, 'abc234hK2H3423&^$#KH'),
                Step(Action.WAIT_FOR, master_device, 'abc234hK2H3423&^$#KH'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16 ' + 'abc234hK2H3423&^$#KH'),
                Step(Action.WAIT_FOR, master_device, 'abc234hK2H3423&^$#KH'),
                Step(Action.WAIT_FOR, master_device, 'abc234hK2H3423&^$#KH'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16Arr ' + 'abc234hK2H3423&^$#KH'),
                Step(Action.WAIT_FOR, master_device, 'abc234hK2H3423&^$#KH'),
                Step(Action.WAIT_FOR, master_device, 'abc234hK2H3423&^$#KH'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_spibitorder0 = Test(
            name='Arduino_Example_test_spibitorder0',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),

                Step(Action.WRITE, slave_device, 'spi bit0'),
                Step(Action.WAIT_FOR, slave_device, 'SPI init'),
                Step(Action.WRITE, master_device, 'spibit 0'),
                Step(Action.WRITE, master_device, 'spi_send ' + '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_sendArr ' + 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16 ' + '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16Arr ' + 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_spi_bit0_2Mhz_mode2 = Test(
            name='Arduino_Example_test_spi_bit0_2Mhz_mode2',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),

                Step(Action.WRITE, slave_device, 'spi bit0 mode2 clock:2000000'),
                Step(Action.WAIT_FOR, slave_device, 'SPI init'),
                Step(Action.WRITE, master_device, 'spibit 0'),
                Step(Action.WRITE, master_device, 'spitxm 2'),
                Step(Action.WRITE, master_device, 'spiclock 40000'),

                Step(Action.WRITE, master_device, 'spi_send ' + '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_sendArr ' + 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16 ' + '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.WAIT_FOR, master_device, '12379274923479172934792374'),
                Step(Action.EXECUTE, master_device, None, lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, master_device, 'spi_send16Arr ' + 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, master_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
            ],
            teardown=teardown
        )


        i2c_mode = TestGroup(
            name='i2c_mode',
            devices=[master_device, slave_device],
            tag=[Tag.POSITIVE],
            tests=[
                Arduino_Example_test_i2c_by_lower_case_letter,
                Arduino_Example_test_i2c_by_upper_case_letter,
                Arduino_Example_test_i2c_by_alphabet,
                Arduino_Example_test_i2c_by_number,
                Arduino_Example_test_i2c_by_punctuation,
                Arduino_Example_test_i2c_by_mix_mode,
            ]
        )

        spi_mode = TestGroup(
            name='spi_mode',
            devices=[master_device, slave_device],
            tag=[Tag.POSITIVE],
            tests=[
                Arduino_Example_test_spibitorder0,
                Arduino_Example_test_spiclock2Mhz,
                Arduino_Example_test_spiclock4Mhz,
                Arduino_Example_test_spimode0,
                Arduino_Example_test_spimode1,
                Arduino_Example_test_spimode2,
                Arduino_Example_test_spimode3,
                Arduino_Example_test_spi_bit0_2Mhz_mode2,
            ]
        )

        test_groups = [
            i2c_mode,
            spi_mode,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()

    parser.add_argument('--master', metavar='SERIAL_PORT', help='Set master')
    parser.add_argument('--slave', metavar='SERIAL_PORT', help='Set slave')

    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    master_device, slave_device = dev_manager.get_devices_by_serials(args.master, args.slave)

    # Create test runner instance
    tc_runner = StdlibTestTcRunner(config, master_device=master_device, slave_device=slave_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)

