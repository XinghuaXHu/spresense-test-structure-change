#! /usr/bin/env python3
import sys
import os

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.spi_i2c_test.spi_i2c_test_device import SPI_I2C_TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['spi_i2c']


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


class SPI_I2C_TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(SPI_I2C_TestTcRunner, self).__init__(conf)

        self.spi_i2c_test_device = SPI_I2C_TestDevice(
            kwargs.get('dut_device', None), 'SPI_I2C_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')

    def get_devices_list(self):
        return [self.spi_i2c_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None


    def generate_test_groups(self, arguments, log=None):
        timeout = 10

        spi_i2c_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.EXECUTE, spi_i2c_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, spi_i2c_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        Arduino_Example_test_i2c_by_lower_case_letter = Test(
            name='Arduino_Example_test_i2c_by_lower_case_letter',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'abcdefghijklmnopqrstuvwxyz'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'uvwxyz'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'uvwxyz'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'abcdef'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'abcdef'),

            ],
            teardown=teardown
        )

        Arduino_Example_test_i2c_by_upper_case_letter = Test(
            name='Arduino_Example_test_i2c_by_upper_case_letter',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'ABCDEF'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'ABCDEF'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'UVWXYZ'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'UVWXYZ'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_i2c_by_alphabet = Test(
            name='Arduino_Example_test_i2c_by_alphabet',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'LJLJLJLJLADFASdafdf'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'LJLJLJLJLADFASdafdf'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'abcABC'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'abcABC'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_i2c_by_number = Test(
            name='Arduino_Example_test_i2c_by_number',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'send ' + '0123456789'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '0123456789'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + '9876543210'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '9876543210'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + '12379274923479172934792374'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '12379274923479172934792374'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_i2c_by_punctuation = Test(
            name='Arduino_Example_test_i2c_by_punctuation',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'send ' + '|,./;:[]{}~!@#$%^&*()_+=-~'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '|,./;:[]{}~!@#$%^&*()_+=-~'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + '\"\',./'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '\"\',./'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_i2c_by_mix_mode = Test(
            name='Arduino_Example_test_i2c_by_mix_mode',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'send ' + '56789%@@$^&*Pjj(9}z?!~`89.,./\\'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '56789%@@$^&*Pjj(9}z?!~`89.,./\\'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_i2c_mode_by_comand = Test(
            name='Arduino_Example_test_i2c_mode_by_comand',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'err'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '#'),

                Step(Action.WRITE, spi_i2c_test_device, 'i2c'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'I2C#'),
            ],
            teardown=teardown
        )


        Arduino_Example_test_spi_by_lower_case_letter = Test(
            name='Arduino_Example_test_spi_by_lower_case_letter',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'abcdefghijklmnopqrstuvwxyz'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'abcdefghijklmnopqrstuvwxyz'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'uvwxyz'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'uvwxyz'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'abcdef'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'abcdef'),

            ],
            teardown=teardown
        )

        Arduino_Example_test_spi_by_upper_case_letter = Test(
            name='Arduino_Example_test_spi_by_upper_case_letter',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'ABCDEF'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'ABCDEF'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'UVWXYZ'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'UVWXYZ'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_spi_by_alphabet = Test(
            name='Arduino_Example_test_spi_by_alphabet',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'LJLJLJLJLADFASdafdf'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'LJLJLJLJLADFASdafdf'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + 'abcABC'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'abcABC'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_spi_by_number = Test(
            name='Arduino_Example_test_spi_by_number',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'send ' + '0123456789'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '0123456789'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + '9876543210'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '9876543210'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + '12379274923479172934792374'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '12379274923479172934792374'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_spi_by_punctuation = Test(
            name='Arduino_Example_test_spi_by_punctuation',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'send ' + '|,./;:[]{}~!@#$%^&*()_+=-~'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '|,./;:[]{}~!@#$%^&*()_+=-~'),

                Step(Action.WRITE, spi_i2c_test_device, 'send ' + '\"\',./'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '\"\',./'),
            ],
            teardown=teardown
        )

        Arduino_Example_test_spi_by_mix_mode = Test(
            name='Arduino_Example_test_spi_by_mix_mode',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'send ' + '56789%@@$^&*Pjj(9}z?!~`89.,./\\'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '56789%@@$^&*Pjj(9}z?!~`89.,./\\'),
            ],
            teardown=teardown
        )


        Arduino_Example_test_spi_mode_by_comand = Test(
            name='Arduino_Example_test_spi_mode_by_comand',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, spi_i2c_test_device, 'err'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, '#'),

                Step(Action.WRITE, spi_i2c_test_device, 'spi'),
                Step(Action.WAIT_FOR, spi_i2c_test_device, 'SPI#'),
            ],
            teardown=teardown
        )

        i2c_mode = TestGroup(
            name='i2c_mode',
            devices=[spi_i2c_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                Arduino_Example_test_i2c_mode_by_comand,
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
            devices=[spi_i2c_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                Arduino_Example_test_spi_mode_by_comand,
                Arduino_Example_test_spi_by_lower_case_letter,
                Arduino_Example_test_spi_by_upper_case_letter,
                Arduino_Example_test_spi_by_alphabet,
                Arduino_Example_test_spi_by_number,
                Arduino_Example_test_spi_by_punctuation,
                Arduino_Example_test_spi_by_mix_mode,
            ]
        )

        test_groups = [
            i2c_mode,
            spi_mode,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()
    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    dut_device = dev_manager.get_devices_by_serials(args.dut_device)[0]

    # Create test runner instance
    tc_runner = SPI_I2C_TestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
