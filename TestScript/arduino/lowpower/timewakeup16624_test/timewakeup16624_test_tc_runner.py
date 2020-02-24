#! /usr/bin/env python3
import sys
import os
import re
import datetime
import time

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.timewakeup16624_test.timewakeup16624_test_device import TimeWakeup16624TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['timewakeup']
t1 = datetime.datetime.now()
t2 = t1
t3 = t1
wakeup_count = 0

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    global t1
    global t2
    global t3
    global wakeup_count
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
    elif '1970' in data:
        timedata = data[0:19]
        if t3 == t2:
            t1 = datetime.datetime.strptime(str(timedata), "%Y/%m/%d %H:%M:%S")
            t2 = t1 + datetime.timedelta(seconds=10)
            t3 = t1 + datetime.timedelta(seconds=13)
        else:
            t1 = datetime.datetime.strptime(str(timedata), "%Y/%m/%d %H:%M:%S")
            if t2 < t1 and t1 < t3:
                wakeup_count = wakeup_count + 1
            else:
                test.set_fail('timewakeup NG')
            t2 = t1 + datetime.timedelta(seconds=10)
            t3 = t1 + datetime.timedelta(seconds=13)
            if wakeup_count > 15:
                test.add_user_event(source, 'Timewakeup Cold Test Done')

def call_shell(test, source, data, command):
    test.log.info("start")
    os.system(command)
    test.log.info("end")

class TimeWakeupTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(TimeWakeupTestTcRunner, self).__init__(conf)

        self.timewakeup16624_test_device = TimeWakeup16624TestDevice(
            kwargs.get('dut_device', None), 'TIMEWAKEUP16624_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/timewakeup-defconfig')
    def get_devices_list(self):
        return [self.timewakeup16624_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.timewakeup16624_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.timewakeup16624_test_device)

        return dict(dut_bin=dut_bin)

    @staticmethod
    def __build_binary(toolbox, defconfig, device):
        bin_file = toolbox.builder.build_project(PROJECT, APPS_TO_BUILTIN, device, defconfig)
        return os.path.normpath(bin_file)

    # noinspection PyMethodMayBeStatic
    def flash_n_check(self, device, dut_bin, log=None):
        if log:
            log.info('Flashing ' + str(device))

        device.flash(str(device), log, dut_bin)
        device.check_device_config(str(device), APPS_TO_BUILTIN, log)

        if os.path.basename(dut_bin).startswith(str(device)):
            try:
                os.remove(dut_bin)
            except OSError as e:
                log.error(e)
                raise

    def generate_test_groups(self, arguments, log=None):
        timeout = 120
        #48 hours and a little margin

        timewakeup16624_test_device = self.get_devices_list()[0]


        setup = [
            Step(Action.EXECUTE, timewakeup16624_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, timewakeup16624_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        Arduino_TimeWakeup_16624 = Test(
            name='Arduino_TimeWakeup_16624',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Example for RTC wakeup from cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Go to cold sleep...'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'wakeup from cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Boot Cause: RTC Alarm0 expired in cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Go to cold sleep...'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'wakeup from cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Boot Cause: RTC Alarm0 expired in cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Go to cold sleep...'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'wakeup from cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Boot Cause: RTC Alarm0 expired in cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Go to cold sleep...'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'wakeup from cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Boot Cause: RTC Alarm0 expired in cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Go to cold sleep...'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'wakeup from cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Boot Cause: RTC Alarm0 expired in cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Go to cold sleep...'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'wakeup from cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Boot Cause: RTC Alarm0 expired in cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Go to cold sleep...'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'wakeup from cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Boot Cause: RTC Alarm0 expired in cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Go to cold sleep...'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'wakeup from cold sleep'),
                Step(Action.WAIT_FOR, timewakeup16624_test_device, 'Boot Cause: RTC Alarm0 expired in cold sleep'),
            ],
            teardown=teardown
        )

        timewakeup = TestGroup(
            name='timewakeup',
            devices=[timewakeup16624_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                Arduino_TimeWakeup_16624,
            ]
        )

        test_groups = [
            timewakeup,
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
    tc_runner = TimeWakeupTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
