#! /usr/bin/env python3
import sys
import os
import re
import time
import serial

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.lowpower16354_test.lowpower_test_device import LowPowerTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['lowpower']
sleepTime = 0

def wait_sec(test, source, data, sec, description=None):
    if description is not None:
        test.log.info("")
        test.log.info("#############################################")
        test.log.info("## [" + str(sec) + " sec] "+ description)
        test.log.info("#############################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')
    return True

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    global sleepTime
    if 'Boot Cause: (5) RTC Alarm expired in deep sleep' in data:
        test.set_fail('Test error: {}'.format(data))
    if 'sleep time =' in data:
        dataArr = data.split()
        sleepTime = int(dataArr[3])

# noinspection PyUnusedLocal
def check_deep_sleep(test, source, data):
    global sleepTime
    if sleepTime == 1 or sleepTime == 10 or sleepTime == 30:
        test.log.info('start time for sleep: {} second'.format(str(sleepTime)))
        time.sleep(sleepTime + 1)
        test.log.info('end time for sleep, please press reset button')
        return True
    else:
        test.log.info('sleep time is not right')
        return False

class LowerPowerTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(LowerPowerTestTcRunner, self).__init__(conf)

        self.lowpower16354_test_device = LowPowerTestDevice(
            kwargs.get('dut_device', None), 'LOWPOWER16354_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None
        self.mySerial2_test_device = LowPowerTestDevice(
            kwargs.get('dut2_device', None), 'MYSERIAL2_TEST_DEVICE') if kwargs.get('dut2_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least two device is needed!')

        if self.mySerial2_test_device:
            self.mySerial2_test_device.set_baudrate(115200)

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/lowpower-defconfig')

    def get_devices_list(self):
        return [self.lowpower16354_test_device, self.mySerial2_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.gnss16354_test2_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.gnss16354_test2_device)

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
        timeout = 600

        lowpower16354_test_device, mySerial2_test_device = self.get_devices_list()

        setup = [
            Step(Action.EXECUTE, mySerial2_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, mySerial2_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        arduino_lowpower_16354 = Test(
            name='arduino_lowpower_16354',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Example for RTC wakeup from deep sleep'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'count ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (0) Power On Reset with Power Supplied'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'sleep time ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Go to deep sleep...',
                     lambda test, source, data: check_deep_sleep(test, source, data)),

                Step(Action.WAIT_FOR, mySerial2_test_device, 'Example for RTC wakeup from deep sleep'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'count ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (0) Power On Reset with Power Supplied'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'sleep time ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Go to deep sleep...',
                     lambda test, source, data: check_deep_sleep(test, source, data)),

                Step(Action.WAIT_FOR, mySerial2_test_device, 'Example for RTC wakeup from deep sleep'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'count ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (0) Power On Reset with Power Supplied'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'sleep time ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Go to deep sleep...',
                     lambda test, source, data: check_deep_sleep(test, source, data)),

                Step(Action.WAIT_FOR, mySerial2_test_device, 'Example for RTC wakeup from deep sleep'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'count ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (0) Power On Reset with Power Supplied'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'sleep time ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Go to deep sleep...',
                     lambda test, source, data: check_deep_sleep(test, source, data)),

                Step(Action.WAIT_FOR, mySerial2_test_device, 'Example for RTC wakeup from deep sleep'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'count ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (0) Power On Reset with Power Supplied'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'sleep time ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Go to deep sleep...',
                     lambda test, source, data: check_deep_sleep(test, source, data)),

                Step(Action.WAIT_FOR, mySerial2_test_device, 'Example for RTC wakeup from deep sleep'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'count ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (0) Power On Reset with Power Supplied'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'sleep time ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Go to deep sleep...',
                     lambda test, source, data: check_deep_sleep(test, source, data)),

                Step(Action.WAIT_FOR, mySerial2_test_device, 'Example for RTC wakeup from deep sleep'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'count ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (0) Power On Reset with Power Supplied'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'sleep time ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Go to deep sleep...',
                     lambda test, source, data: check_deep_sleep(test, source, data)),

                Step(Action.WAIT_FOR, mySerial2_test_device, 'Example for RTC wakeup from deep sleep'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'count ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (0) Power On Reset with Power Supplied'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'sleep time ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Go to deep sleep...',
                     lambda test, source, data: check_deep_sleep(test, source, data)),

                Step(Action.WAIT_FOR, mySerial2_test_device, 'Example for RTC wakeup from deep sleep'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'count ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (0) Power On Reset with Power Supplied'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'sleep time ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Go to deep sleep...',
                     lambda test, source, data: check_deep_sleep(test, source, data)),

                Step(Action.WAIT_FOR, mySerial2_test_device, 'Example for RTC wakeup from deep sleep'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'count ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (0) Power On Reset with Power Supplied'),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'sleep time ='),
                Step(Action.WAIT_FOR, mySerial2_test_device, 'Go to deep sleep...',
                     lambda test, source, data: check_deep_sleep(test, source, data)),
            ],
            teardown=teardown
        )

        lowpower_16354 = TestGroup(
            name='lowpower_16354',
            devices=[lowpower16354_test_device, mySerial2_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                arduino_lowpower_16354,
            ]
        )

        test_groups = [
            lowpower_16354,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()

    parser.add_argument('--dut2_device', metavar='SERIAL_PORT', help='Set uart2')

    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    dut_device, dut2_device = dev_manager.get_devices_by_serials(args.dut_device, args.dut2_device)

    # Create test runner instance
    tc_runner = LowerPowerTestTcRunner(config, dut2_device=dut2_device, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)