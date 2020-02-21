#! /usr/bin/env python3
import sys
import os

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "../../../..","TestFramework", "src"))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

sys.path.append(SCRIPT_PATH)
from hello_test_device import HelloTestDevice

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


class HelloTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(HelloTestTcRunner, self).__init__(conf)

        self.dut_device = HelloTestDevice(
            kwargs.get('dut_device', None), 'HELLO_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')

    def get_devices_list(self):
        return [self.dut_device]

    def generate_test_groups(self, arguments, log=None):
        timeout = 10

        hello_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, hello_test_device, hello_test_device.NSH_PROMPT),
            Step(Action.WRITE, hello_test_device, 'reboot'),
            Step(Action.WAIT_FOR, hello_test_device, hello_test_device.NUTTSHELL),
            Step(Action.EXECUTE, hello_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, hello_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        SDK_Example_test_15836 = Test(
            name='SDK_Example_test_15836',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, hello_test_device, 'hello'),
                Step(Action.WAIT_FOR, hello_test_device, hello_test_device.HELLOWORLD),
            ],
            teardown=teardown
        )

        hello = TestGroup(
            name='hello',
            devices=[hello_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Example_test_15836,
            ]
        )

        test_groups = [
            hello,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()
    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../../TestFramework/', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    dut_device = dev_manager.get_devices_by_serials(args.dut_device)[0]

    # Create test runner instance
    tc_runner = HelloTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
