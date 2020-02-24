#! /usr/bin/env python3
import sys
import os

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.stdlib_test.master_device import MasterDevice
from tc.stdlib_test.slave_spresense_device import SlaveSpresenseDevice
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


class Uart2SoftSerialTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(Uart2SoftSerialTestTcRunner, self).__init__(conf)

        self.master_device = MasterDevice(kwargs.get('master_device', None), 'MASTER_DEVICE') \
            if kwargs.get('master_device', None) else None
        self.slave_spresense_device = MasterDevice(kwargs.get('slave_spresense_device', None), 'SLAVE_SPRESENSE_DEVICE') \
            if kwargs.get('slave_spresense_device', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least two device are needed!')

    def get_devices_list(self):
        return [self.master_device, self.slave_spresense_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release

    def generate_test_groups(self, arguments, log=None):
        timeout = 10

        master_device, slave_spresense_device = self.get_devices_list()

        setup = [
            Step(Action.EXECUTE, master_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, master_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        uart2_tc_01_send = Test(
            name='uart2_tc_01_send',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, slave_spresense_device, ''),
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'stdlib#'),
                Step(Action.WRITE, slave_spresense_device, 'uart2 115200'),
                Step(Action.WRITE, master_device, 'uart2 115200'),
                Step(Action.WAIT_FOR, master_device, 'UART2 buadrate is set'),
                Step(Action.WRITE, master_device, 'u2_send uvwxyz'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'uvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'uvwxyz'),
            ],
            teardown=teardown
        )

        uart2_tc_02_send = Test(
            name='uart2_tc_02_send',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, slave_spresense_device, ''),
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'stdlib#'),
                Step(Action.WRITE, slave_spresense_device, 'uart2 576000'),
                Step(Action.WRITE, master_device, 'uart2 576000'),
                Step(Action.WAIT_FOR, master_device, 'UART2 buadrate is set'),
                Step(Action.WRITE, master_device, 'u2_send uvwxyz'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'uvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'uvwxyz'),
            ],
            teardown=teardown
        )

        uart2_test_group = TestGroup(
            name='uart2_test_group',
            devices=[master_device, slave_spresense_device],
            tag=[Tag.POSITIVE],
            tests=[
                uart2_tc_01_send,
                uart2_tc_02_send,
            ]
        )

        soft_serial_send_4800_pin_20_21 = Test(
            name='soft_serial_01_send_4800_pin_20_21',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, slave_spresense_device, ''),
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'stdlib#'),
                Step(Action.WRITE, slave_spresense_device, 'soft 4800 20 21'),
                Step(Action.WRITE, master_device, 'soft 4800 20 21'),
                Step(Action.WRITE, master_device, 'sf_send uvwxyz'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'uvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'uvwxyz'),
            ],
            teardown=teardown
        )

        soft_serial_send_115200_pin_20_21 = Test(
            name='soft_serial_02_send_115200_pin_20_21',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, slave_spresense_device, ''),
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'stdlib#'),
                Step(Action.WRITE, slave_spresense_device, 'soft 115200 20 21'),
                Step(Action.WRITE, master_device, 'soft 115200 20 21'),
                Step(Action.WRITE, master_device, 'sf_send uvwxyz'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'uvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'uvwxyz'),
            ],
            teardown=teardown
        )

        soft_serial_send_250000_pin_20_21 = Test(
            name='soft_serial_send_250000_pin_20_21',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, slave_spresense_device, ''),
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'stdlib#'),
                Step(Action.WRITE, slave_spresense_device, 'soft 250000 20 21'),
                Step(Action.WRITE, master_device, 'soft 250000 20 21'),
                Step(Action.WRITE, master_device, 'sf_send uvwxyz'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'uvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'uvwxyz'),
            ],
            teardown=teardown
        )

        soft_serial_send_4800_pin_05_06 = Test(
            name='soft_serial_02_send_4800_pin_5_6',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, slave_spresense_device, ''),
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'stdlib#'),
                Step(Action.WRITE, slave_spresense_device, 'soft 4800 5 6'),
                Step(Action.WRITE, master_device, 'soft 4800 5 6'),
                Step(Action.WRITE, master_device, 'sf_send uvwxyz'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'uvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'uvwxyz'),
            ],
            teardown=teardown
        )

        soft_serial_send_250000_pin_05_06 = Test(
            name='soft_serial_02_send_250000_pin_05_06',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, slave_spresense_device, ''),
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'stdlib#'),
                Step(Action.WRITE, slave_spresense_device, 'soft 250000 5 6'),
                Step(Action.WRITE, master_device, 'soft 250000 5 6'),
                Step(Action.WRITE, master_device, 'sf_send uvwxyz'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'uvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'uvwxyz'),
            ],
            teardown=teardown
        )

        soft_serial_send_250000_pin_10_11 = Test(
            name='soft_serial_02_send_250000_pin_10_11',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, slave_spresense_device, ''),
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'stdlib#'),
                Step(Action.WRITE, slave_spresense_device, 'soft 250000 10 11'),
                Step(Action.WRITE, master_device, 'soft 250000 10 11'),
                Step(Action.WRITE, master_device, 'sf_send uvwxyz'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'uvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'uvwxyz'),
            ],
            teardown=teardown
        )

        soft_serial_send_250000_pin_12_13 = Test(
            name='soft_serial_02_send_250000_pin_12_13',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, slave_spresense_device, ''),
                Step(Action.WRITE, master_device, 'help'),
                Step(Action.WAIT_FOR, master_device, 'stdlib#'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'stdlib#'),
                Step(Action.WRITE, slave_spresense_device, 'soft 250000 12 13'),
                Step(Action.WRITE, master_device, 'soft 250000 12 13'),
                Step(Action.WRITE, master_device, 'sf_send uvwxyz'),
                Step(Action.WAIT_FOR, slave_spresense_device, 'uvwxyz'),
                Step(Action.WAIT_FOR, master_device, 'uvwxyz'),
            ],
            teardown=teardown
        )

        soft_serial_test_group = TestGroup(
            name='soft_serial_test_group',
            devices=[master_device, slave_spresense_device],
            tag=[Tag.POSITIVE],
            tests=[
                soft_serial_send_4800_pin_20_21,
                soft_serial_send_115200_pin_20_21,
                soft_serial_send_250000_pin_20_21,
                soft_serial_send_4800_pin_05_06,
                soft_serial_send_250000_pin_05_06,
                soft_serial_send_250000_pin_10_11,
                soft_serial_send_250000_pin_12_13,
            ]
        )

        test_groups = [
            uart2_test_group,
            soft_serial_test_group,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()

    parser.add_argument('--master', metavar='SERIAL_PORT', help='Set master')
    parser.add_argument('--slave_spresense', metavar='SERIAL_PORT', help='Set slave spresense')

    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    master_device, slave_spresense_device = dev_manager.get_devices_by_serials(args.master, args.slave_spresense)

    # Create test runner instance
    tc_runner = Uart2SoftSerialTestTcRunner(config, master_device=master_device, slave_spresense_device=slave_spresense_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
