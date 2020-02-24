#! /usr/bin/env python3
import sys
import os
import time

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.rtc_test_16399.rtc_test_device import RtcTestDevice
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
COMPILE_TIME = '2019'

# noinspection PyUnusedLocal
def alarm_monitor(test, source, data):
    if COMPILE_TIME in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'alarm!']))


# noinspection PyUnusedLocal
def sleep(test, source, data):
    time.sleep(data)


class RtcTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(RtcTestTcRunner, self).__init__(conf)

        self.rtc_test_device = RtcTestDevice(
            kwargs.get('dut_device', None), 'RTC_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

    def get_devices_list(self):
        return [self.rtc_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release

    def generate_test_groups(self, arguments, log=None):
        timeout = 120

        rtc_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, rtc_test_device, 'Start RTC test'),
        ]

        teardown = [
        ]

        tc_16399 = Test(
            name='tc_16399',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'set alarm 10 seconds'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'loop()'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC.end()'),
            ],
            teardown=teardown
        )

        rtc_test_group = TestGroup(
            name='rtc_test_group',
            devices=[rtc_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_16399,
            ]
        )

        test_groups = [
            rtc_test_group,
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
    tc_runner = RtcTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
