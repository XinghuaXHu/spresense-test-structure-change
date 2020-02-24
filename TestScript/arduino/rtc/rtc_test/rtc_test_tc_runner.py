#! /usr/bin/env python3
import sys
import os
import time

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.rtc_test.rtc_test_device import RtcTestDevice
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


# noinspection PyUnusedLocal
def print_promt(test, source, data):
    print(data)


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
        timeout = 30

        rtc_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, rtc_test_device, 'Start RTC test'),
        ]

        teardown = [
        ]

        tc_16391 = Test(
            name='tc_16391',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, rtc_test_device, 'rtc_begin', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: begin()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_settime', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: settime()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_gettime', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: gettime()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_end', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: end()'),
            ],
            teardown=teardown
        )

        tc_16392 = Test(
            name='tc_16392',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, rtc_test_device, 'rtc_begin', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: begin()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_settime', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: settime()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_attach', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: attachAlarm()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_setalarm 10', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: setAlarm()'),
                Step(Action.WAIT_FOR, rtc_test_device, 'alarm after 10 seconds'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WRITE, rtc_test_device, 'rtc_end', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: end()'),
            ],
            teardown=teardown
        )

        tc_16393 = Test(
            name='tc_16393',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, rtc_test_device, 'rtc_begin', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: begin()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_attach', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: attachAlarm()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_setalarm 10', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, '1970/01/01 00:0'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: setAlarm()'),
                Step(Action.WAIT_FOR, rtc_test_device, 'alarm after 10 seconds'),
                Step(Action.WAIT_FOR, rtc_test_device, '1970/01/01 00:0'),
                Step(Action.WRITE, rtc_test_device, 'rtc_end', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: end()'),
            ],
            teardown=teardown
        )

        tc_16394 = Test(
            name='tc_16394',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, rtc_test_device, 'rtc_begin', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: begin()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_settime', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: settime()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_attach', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: attachAlarm()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_setalarmsec 10', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: setAlarmSeconds()'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'alarm after 10 seconds'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WRITE, rtc_test_device, 'rtc_end', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: end()'),
            ],
            teardown=teardown
        )

        tc_16395 = Test(
            name='tc_16395',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, rtc_test_device, 'rtc_begin', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: begin()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_settime', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: settime()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_attach', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: attachAlarm()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_setalarm 10', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: setAlarm()'),
                Step(Action.WAIT_FOR, rtc_test_device, 'alarm after 10 seconds'),
                Step(Action.WRITE, rtc_test_device, 'rtc_cancelalarm', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: cancelAlarm()'),
                Step(Action.EXECUTE, rtc_test_device, ('{}_alarm_mon'.format(str(rtc_test_device)), alarm_monitor),
                     Test.add_monitor),
                Step(Action.EXECUTE, rtc_test_device, 15, sleep),
                Step(Action.EXECUTE, rtc_test_device, '{}_alarm_mon'.format(str(rtc_test_device)),
                     Test.remove_monitor),
                Step(Action.WRITE, rtc_test_device, 'rtc_end', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: end()'),
            ],
            teardown=teardown
        )

        tc_16396 = Test(
            name='tc_16396',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, rtc_test_device, 'rtc_begin', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: begin()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_settime', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: settime()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_attach', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: attachAlarm()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_detach', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: detachAlarm()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_attach', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: attachAlarm()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_setalarm 10', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: setAlarm()'),
                Step(Action.WAIT_FOR, rtc_test_device, 'alarm after 10 seconds'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WRITE, rtc_test_device, 'rtc_end', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: end()'),
            ],
            teardown=teardown
        )

        tc_16397 = Test(
            name='tc_16397',
            timeout=120,
            setup=setup,
            test=[
                Step(Action.EXECUTE, rtc_test_device, 'Please put PIN_D02 to HIGH', print_promt),
                Step(Action.WRITE, rtc_test_device, 'rtc_begin', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: begin()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_settime', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: settime()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_attach', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: attachAlarm()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_setalarm 120', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: setAlarm()'),
                Step(Action.WAIT_FOR, rtc_test_device, 'alarm after 120 seconds'),
                Step(Action.WRITE, rtc_test_device, 'sleep_gpio', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'Go to sleep'),
                Step(Action.EXECUTE, rtc_test_device, 'Please put PIN_D02 to LOW', print_promt),
                Step(Action.WAIT_FOR, rtc_test_device, 'Start RTC test'),
                Step(Action.WRITE, rtc_test_device, 'rtc_begin', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: begin()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_attach', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: attachAlarm()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_setalarm 10', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: setAlarm()'),
                Step(Action.WAIT_FOR, rtc_test_device, 'alarm after 10 seconds'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WRITE, rtc_test_device, 'rtc_end', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: end()'),
            ],
            teardown=teardown
        )

        tc_16398 = Test(
            name='tc_16398',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, rtc_test_device, 'rtc_begin', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: begin()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_settime', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: settime()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_attach', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: attachAlarm()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_setalarm 10', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: setAlarm()'),
                Step(Action.WAIT_FOR, rtc_test_device, 'alarm after 10 seconds'),
                Step(Action.WRITE, rtc_test_device, 'sleep_deep 10', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'Go to sleep'),
                Step(Action.WAIT_FOR, rtc_test_device, 'Start RTC test'),
                Step(Action.WRITE, rtc_test_device, 'rtc_begin', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: begin()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_attach', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: attachAlarm()'),
                Step(Action.WRITE, rtc_test_device, 'rtc_setalarm 10', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: setAlarm()'),
                Step(Action.WAIT_FOR, rtc_test_device, 'alarm after 10 seconds'),
                Step(Action.WAIT_FOR, rtc_test_device, COMPILE_TIME),
                Step(Action.WRITE, rtc_test_device, 'rtc_end', None, '\r'),
                Step(Action.WAIT_FOR, rtc_test_device, 'RTC: end()'),
            ],
            teardown=teardown
        )

        rtc_test_group = TestGroup(
            name='rtc_test_group',
            devices=[rtc_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_16393,  # need run first
                tc_16391,
                tc_16392,
                tc_16394,
                tc_16395,
                tc_16396,
                tc_16397,
                tc_16398,
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
