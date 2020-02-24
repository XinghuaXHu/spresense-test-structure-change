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
from api.tester_exception import TesterException
from tc.lowpower16355_test.lowpower_test_device import LowPowerTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['lowpower']

def wait_sec(test, source, data, sec, description=None):
    if description is not None:
        test.log.info("")
        test.log.info("###########################################################")
        test.log.info("## [" + str(sec) + " sec] " + description)
        test.log.info("###########################################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')
    return True

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if 'fail' in data:
        test.set_fail('failed: {}'.format(data))

class LowerPowerTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(LowerPowerTestTcRunner, self).__init__(conf)

        self.lowpower16355_test_device = LowPowerTestDevice(
            kwargs.get('dut_device', None), 'LOWPOWER16355_TEST_DEVICE') if kwargs.get('dut_device', None)\
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
        return [self.lowpower16355_test_device, self.mySerial2_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.gnss16355_test2_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.gnss16355_test2_device)

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
        timeout = 1800

        lowpower16355_test_device, mySerial2_test_device = self.get_devices_list()

        setup = [
            Step(Action.EXECUTE, mySerial2_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, mySerial2_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        def get_loop_test(count=9):
            r = []
            for i in range(count):
                r = r + [
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 0'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Goto cold sleep (RTC)'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup")),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'sleeptime = 1'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (9) RTC Alarm0 expired in cold sleep'),

                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 1'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Goto cold sleep (RTC)'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup")),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'sleeptime = 10'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (9) RTC Alarm0 expired in cold sleep'),

                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 2'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Goto cold sleep (RTC)'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup")),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'sleeptime = 30'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (9) RTC Alarm0 expired in cold sleep'),

                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 3'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Start watch dog'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup")),
                    Step(Action.WAIT_FOR, lowpower16355_test_device, 'up_taskdump: alarm_daemon'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (1) System WDT expired or Self Reboot'),

                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 4'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Goto cold sleep (GPIO)'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup by pin2")),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'pin = 2'),
                    Step(Action.WAIT_FOR, mySerial2_test_device,
                         'Boot Cause: (16) GPIO detected in cold sleep <- pin 2'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'bc_pin = 2'),

                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 5'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Goto cold sleep (GPIO)'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup by pin3")),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'pin = 3'),
                    Step(Action.WAIT_FOR, mySerial2_test_device,
                         'Boot Cause: (17) GPIO detected in cold sleep <- pin 3'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'bc_pin = 3'),

                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 6'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Goto cold sleep (GPIO)'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup by pin4")),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'pin = 4'),
                    Step(Action.WAIT_FOR, mySerial2_test_device,
                         'Boot Cause: (18) GPIO detected in cold sleep <- pin 4'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'bc_pin = 4'),

                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 7'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Goto cold sleep (GPIO)'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup by pin5")),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'pin = 5'),
                    Step(Action.WAIT_FOR, mySerial2_test_device,
                         'Boot Cause: (19) GPIO detected in cold sleep <- pin 5'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'bc_pin = 5'),

                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 8'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Goto cold sleep (GPIO)'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup by pin6")),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'pin = 6'),
                    Step(Action.WAIT_FOR, mySerial2_test_device,
                         'Boot Cause: (20) GPIO detected in cold sleep <- pin 6'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'bc_pin = 6'),

                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 9'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Goto cold sleep (GPIO)'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup by pin7")),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'pin = 7'),
                    Step(Action.WAIT_FOR, mySerial2_test_device,
                         'Boot Cause: (21) GPIO detected in cold sleep <- pin 7'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'bc_pin = 7'),

                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 10'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Goto cold sleep (GPIO)'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup by pin10")),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'pin = 10'),
                    Step(Action.WAIT_FOR, mySerial2_test_device,
                         'Boot Cause: (23) GPIO detected in cold sleep <- pin 10'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'bc_pin = 10'),

                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 11'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Goto cold sleep (GPIO)'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup by pin11")),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'pin = 11'),
                    Step(Action.WAIT_FOR, mySerial2_test_device,
                         'Boot Cause: (24) GPIO detected in cold sleep <- pin 11'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'bc_pin = 11'),

                    Step(Action.WAIT_FOR, mySerial2_test_device, 'count = 12'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'Goto cold sleep (GPIO)'),
                    Step(Action.EXECUTE, mySerial2_test_device, None,
                         lambda test, source, data: test.log.info("go to LowPower, then wakeup by pin12")),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'pin = 12'),
                    Step(Action.WAIT_FOR, mySerial2_test_device,
                         'Boot Cause: (25) GPIO detected in cold sleep <- pin 12'),
                    Step(Action.WAIT_FOR, mySerial2_test_device, 'bc_pin = 12'),
                ]
            return r

        arduino_lowpower_16355 = Test(
            name='arduino_lowpower_16355',
            timeout=timeout,
            setup=setup,
            test=[
                     Step(Action.WAIT_FOR, mySerial2_test_device, 'Boot Cause: (0) Power On Reset with Power Supplied'),
                     Step(Action.WAIT_FOR, mySerial2_test_device, 'spr_sdk-16355 start'),
                 ] + get_loop_test() + [
                      Step(Action.WAIT_FOR, mySerial2_test_device, 'test end'),
                 ],
            teardown=teardown
        )

        lowpower_16355 = TestGroup(
            name='lowpower_16355',
            devices=[lowpower16355_test_device, mySerial2_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                arduino_lowpower_16355,
            ]
        )

        test_groups = [
            lowpower_16355,
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