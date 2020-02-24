#! /usr/bin/env python3
import sys
import os
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.watchdog16631_arduino_example_test.watchdog16631_arduino_example_test_device import Watchdog16631TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

from datetime import datetime
import time
now = time.time()
utc = datetime.utcfromtimestamp(now)

time1 = 0
time2 = 0

# noinspection PyUnusedLocal
def dut_data_monitor(test, source, data):
    global time1
    global time2
    if 'Sleep ' in data:
        split_str = " |m"
        arrayData = re.split(split_str, data)
        time1 = int(arrayData[1])
    elif 'ms left for watchdog bite' in data:
        split_str = "m"
        arrayData = re.split(split_str, data)
        time2 = int(arrayData[0])
        if time1 == 1000 and time2 <= 900:
            test.set_fail('time is not right')
        elif time1 == 1100 and time2 <= 800:
            test.set_fail('time is not right')
        elif time1 == 1200 and time2 <= 700:
            test.set_fail('time is not right')
        elif time1 == 1300 and time2 <= 600:
            test.set_fail('time is not right')
        elif time1 == 1400 and time2 <= 500:
            test.set_fail('time is not right')
        elif time1 == 1500 and time2 <= 400:
            test.set_fail('time is not right')
        elif time1 == 1600 and time2 <= 300:
            test.set_fail('time is not right')
        elif time1 == 1700 and time2 <= 200:
            test.set_fail('time is not right')
        elif time1 == 1800 and time2 <= 100:
            test.set_fail('time is not right')
        elif time1 == 1900 and time2 <= 10:
            test.set_fail('time is not right')

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['watchdog']


class WatchdogTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(WatchdogTestTcRunner, self).__init__(conf)

        self.watchdog16631_test_device = Watchdog16631TestDevice(
            kwargs.get('dut_device', None), 'WATCHDOG16631_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/watchdog-defconfig')

    def get_devices_list(self):
        return [self.watchdog16631_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

    # noinspection PyUnresolvedReferences
    def build(self, debug=True, log=None, **kwargs):
        dut_bin = kwargs.get('dut_bin', None)
        toolbox = None

        if not all([dut_bin]):
            if log:
                log.info('Building project sources')

            toolbox = Toolbox(self.config, log)
            toolbox.init_builder_module()

        if not dut_bin:
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.watchdog16631_test_device)

        return dict(dut_bin=dut_bin)

    @staticmethod
    def __build_binary(toolbox, defconfig, device):
        bin_file = toolbox.builder.build_project(PROJECT, APPS_TO_BUILTIN, device, defconfig)
        return os.path.normpath(bin_file)

    # noinspection PyMethodMayBeStatic
    def flash_n_check(self, device, dut_bin, log=None):
        device.check_device_config(str(device), APPS_TO_BUILTIN, log)

        if os.path.basename(dut_bin).startswith(str(device)):
            try:
                os.remove(dut_bin)
            except OSError as e:
                log.error(e)
                raise

    def generate_test_groups(self, arguments, log=None):
        timeout = 20

        watchdog16631_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.EXECUTE, watchdog16631_test_device, ('dut_reboot_mon', dut_data_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, watchdog16631_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        Arduino_Watchdog_16631 = Test(
            name='Arduino_Watchdog_16631',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Sleep 1000ms'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'ms left for watchdog bite'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Kick!'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Sleep 1100ms'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'ms left for watchdog bite'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Kick!'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Sleep 1200ms'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'ms left for watchdog bite'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Kick!'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Sleep 1300ms'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'ms left for watchdog bite'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Kick!'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Sleep 1400ms'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'ms left for watchdog bite'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Kick!'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Sleep 1500ms'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'ms left for watchdog bite'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Kick!'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Sleep 1600ms'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'ms left for watchdog bite'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Kick!'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Sleep 1700ms'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'ms left for watchdog bite'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Kick!'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Sleep 1800ms'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'ms left for watchdog bite'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Kick!'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Sleep 1900ms'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'ms left for watchdog bite'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Kick!'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'Sleep 2000ms'),
                Step(Action.WAIT_FOR, watchdog16631_test_device, 'up_taskdump: <pthread>: PID='),
            ],
            teardown=teardown
        )

        watchdog = TestGroup(
            name='watchdog',
            devices=[watchdog16631_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                Arduino_Watchdog_16631,
            ]
        )

        test_groups = [
            watchdog,
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
    tc_runner = WatchdogTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
