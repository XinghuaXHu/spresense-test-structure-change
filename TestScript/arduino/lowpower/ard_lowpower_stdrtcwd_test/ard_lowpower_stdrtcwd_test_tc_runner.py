#! /usr/bin/env python3
import sys
import os
import time
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.ard_lowpower_stdrtcwd_test.ard_lowpower_stdrtcwd_test_device import SpresenseTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['']
global _global_dict
_global_dict = {}
_global_dict['reboot'] = 'NO'
REBOOT_OK = 'rebootOK'
REBOOT_NG = 'rebootNG'
COMPILE_TIME = '2019'

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if 'up_taskdump: <pthread>' in data:
        _global_dict.pop('reboot')
        _global_dict['reboot'] = 'YES'

def checkReboot(test, source, data):
    if _global_dict['reboot'] == 'YES':
        _global_dict.pop('reboot')
        _global_dict['reboot'] = 'NO'
        test.add_user_event(source, REBOOT_OK)
    else:
        test.add_user_event(source, REBOOT_NG)

def check_delay_lv(test, source, data):
    ms_value = re.search(r'(delta_millis : [0-9]+)', data)
    us_value = re.search(r'(delta_micros : [0-9]+)', data)

    if not ms_value or not us_value:
        return False;

    ms_str = ms_value.group().replace('delta_millis : ', '')
    if not 1100 >= int(ms_str) >= 960:
        test.set_fail(' '.join(['Out of range. delta_millis = ', ms_str]))
        return False;

    us_str = us_value.group().replace('delta_micros : ', '')
    if not 1100000 >= int(us_str) >= 960000:
        test.set_fail(' '.join(['Out of range. delta_micros = ', us_str]))
        return False

    return True

def check_delay_mv(test, source, data):
    ms_value = re.search(r'(delta_millis : [0-9]+)', data)
    us_value = re.search(r'(delta_micros : [0-9]+)', data)

    if not ms_value or not us_value:
        return False;

    ms_str = ms_value.group().replace('delta_millis : ', '')
    if not 1100 >= int(ms_str) >= 990:
        test.set_fail(' '.join(['Out of range. delta_millis = ', ms_str]))
        return False;

    us_str = us_value.group().replace('delta_micros : ', '')
    if not 1100000 >= int(us_str) >= 990000:
        test.set_fail(' '.join(['Out of range. delta_micros = ', us_str]))
        return False

    return True

class ArdLowPowerTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(ArdLowPowerTestTcRunner, self).__init__(conf)

        self.spresense_test_device = SpresenseTestDevice(
            kwargs.get('dut_device', None), 'RTC_STD_WD_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/Spresense-defconfig')

    def get_devices_list(self):
        return [self.spresense_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.spresense_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.spresense_test_device)

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
        timeout = 180

        spresense_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.EXECUTE, spresense_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, spresense_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        ARD_LowPower_delay_LV_test1_16708_to_16711 = Test(
            name='ARD_LowPower_delay_LV_test1_16708_to_16711',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, spresense_test_device, 'Start WATCHDOG RTC STD lib test in low-power case'),
                Step(Action.WRITE, spresense_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, spresense_test_device, 'low LV', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, '8MHz'),
                Step(Action.WRITE, spresense_test_device, 'delay_test', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, spresense_test_device.DELAYMS),
                Step(Action.WAIT_FOR, spresense_test_device, spresense_test_device.DELAYMS),
                Step(Action.WAIT_FOR, spresense_test_device, 'delta_',
                     lambda test, source, data: check_delay_lv(test, source, data)),
                Step(Action.WAIT_FOR, spresense_test_device, spresense_test_device.DELAYUS),
                Step(Action.WAIT_FOR, spresense_test_device, 'delta_',
                     lambda test, source, data: check_delay_lv(test, source, data)),
            ],
            teardown=teardown
        )

        ARD_LowPower_delay_MV_test1_16708_to_16711 = Test(
            name='ARD_LowPower_delay_MV_test1_16708_to_16711',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, spresense_test_device, 'Start WATCHDOG RTC STD lib test in low-power case'),
                Step(Action.WRITE, spresense_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, spresense_test_device, 'low MV', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, '32MHz'),
                Step(Action.WRITE, spresense_test_device, 'delay_test', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, spresense_test_device.DELAYMS),
                Step(Action.WAIT_FOR, spresense_test_device, spresense_test_device.DELAYMS),
                Step(Action.WAIT_FOR, spresense_test_device, 'delta_',
                     lambda test, source, data: check_delay_mv(test, source, data)),
                Step(Action.WAIT_FOR, spresense_test_device, spresense_test_device.DELAYUS),
                Step(Action.WAIT_FOR, spresense_test_device, 'delta_',
                     lambda test, source, data: check_delay_mv(test, source, data)),
            ],
            teardown=teardown
        )

        ARD_LowPower_RTC_LV_test1_16712 = Test(
            name='ARD_LowPower_RTC_LV_test1_16712',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, spresense_test_device, 'Start WATCHDOG RTC STD lib test in low-power case'),
                Step(Action.WRITE, spresense_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, spresense_test_device, 'low LV', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, '8MHz'),
                Step(Action.WRITE, spresense_test_device, 'rtc_begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'RTC: begin()'),
                Step(Action.WRITE, spresense_test_device, 'rtc_settime', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, spresense_test_device, 'RTC: settime()'),
                Step(Action.WRITE, spresense_test_device, 'rtc_gettime', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, spresense_test_device, 'RTC: gettime()'),
                Step(Action.WRITE, spresense_test_device, 'rtc_end', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'RTC: end()'),
            ],
            teardown=teardown
        )

        ARD_LowPower_RTC_MV_test1_16712 = Test(
            name='ARD_LowPower_RTC_MV_test1_16712',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, spresense_test_device, 'Start WATCHDOG RTC STD lib test in low-power case'),
                Step(Action.WRITE, spresense_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, spresense_test_device, 'low MV', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, '32MHz'),
                Step(Action.WRITE, spresense_test_device, 'rtc_begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'RTC: begin()'),
                Step(Action.WRITE, spresense_test_device, 'rtc_settime', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'RTC: settime()'),
                Step(Action.WRITE, spresense_test_device, 'rtc_attach', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'RTC: attachAlarm()'),
                Step(Action.WRITE, spresense_test_device, 'rtc_setalarm 10', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, COMPILE_TIME),
                Step(Action.WAIT_FOR, spresense_test_device, 'RTC: setAlarm()'),
                Step(Action.WAIT_FOR, spresense_test_device, 'alarm after 10 seconds'),
                Step(Action.WAIT_FOR, spresense_test_device, COMPILE_TIME),
                Step(Action.WRITE, spresense_test_device, 'rtc_end', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'RTC: end()'),
            ],
            teardown=teardown
        )

        ARD_LowPower_WatchDog_LV_test1_16713 = Test(
            name='ARD_LowPower_WatchDog_LV_test1_16713',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, spresense_test_device, 'Start WATCHDOG RTC STD lib test in low-power case'),
                Step(Action.WRITE, spresense_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, spresense_test_device, 'low LV', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, '8MHz'),
                Step(Action.WRITE, spresense_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, spresense_test_device, 'wd_start 1000', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, spresense_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, spresense_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, spresense_test_device, 'Start WATCHDOG RTC STD lib test in low-power case'),
                Step(Action.WRITE, spresense_test_device, 'getboot', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'System WDT expired or Self Reboot'),

                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, spresense_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, spresense_test_device, 'low LV', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, '8MHz'),
                Step(Action.WRITE, spresense_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, spresense_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, spresense_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(31)),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, spresense_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, spresense_test_device, 'Start WATCHDOG RTC STD lib test in low-power case'),
                Step(Action.WRITE, spresense_test_device, 'getboot', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'System WDT expired or Self Reboot'),

                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, spresense_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, spresense_test_device, 'low LV', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, '8MHz'),
                Step(Action.WRITE, spresense_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, spresense_test_device, 'wd_start 40000', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, spresense_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(41)),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, spresense_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, spresense_test_device, 'Start WATCHDOG RTC STD lib test in low-power case'),
                Step(Action.WRITE, spresense_test_device, 'getboot', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'System WDT expired or Self Reboot'),

                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, spresense_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, spresense_test_device, 'low LV', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, '8MHz'),
                Step(Action.WRITE, spresense_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, spresense_test_device, 'wd_start 50000', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, spresense_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(51)),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, spresense_test_device, REBOOT_NG),

            ],
            teardown=teardown
        )

        ARD_LowPower_WatchDog_MV_test1_16713 = Test(
            name='ARD_LowPower_WatchDog_MV_test1_16713',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, spresense_test_device, 'Start WATCHDOG RTC STD lib test in low-power case'),
                Step(Action.WRITE, spresense_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, spresense_test_device, 'low MV', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, '32MHz'),
                Step(Action.WRITE, spresense_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, spresense_test_device, 'wd_start 1000', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, spresense_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, spresense_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, spresense_test_device, 'Start WATCHDOG RTC STD lib test in low-power case'),
                Step(Action.WRITE, spresense_test_device, 'getboot', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'System WDT expired or Self Reboot'),

                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, spresense_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, spresense_test_device, 'low MV', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, '32MHz'),
                Step(Action.WRITE, spresense_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, spresense_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, spresense_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(31)),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, spresense_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, spresense_test_device, 'Start WATCHDOG RTC STD lib test in low-power case'),
                Step(Action.WRITE, spresense_test_device, 'getboot', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'System WDT expired or Self Reboot'),

                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, spresense_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, spresense_test_device, 'low MV', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, '32MHz'),
                Step(Action.WRITE, spresense_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, spresense_test_device, 'wd_start 40000', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, spresense_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(41)),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, spresense_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, spresense_test_device, 'Start WATCHDOG RTC STD lib test in low-power case'),
                Step(Action.WRITE, spresense_test_device, 'getboot', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'System WDT expired or Self Reboot'),

                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, spresense_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, spresense_test_device, 'low MV', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, '32MHz'),
                Step(Action.WRITE, spresense_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, spresense_test_device, 'wd_start 50000', None, '\r'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, spresense_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: time.sleep(51)),
                Step(Action.EXECUTE, spresense_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, spresense_test_device, REBOOT_NG),

            ],
            teardown=teardown
        )


        Spresense = TestGroup(
            name='Spresense',
            devices=[spresense_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ARD_LowPower_delay_LV_test1_16708_to_16711,
                ARD_LowPower_delay_MV_test1_16708_to_16711,
                ARD_LowPower_RTC_LV_test1_16712,
                ARD_LowPower_RTC_MV_test1_16712,
                ARD_LowPower_WatchDog_LV_test1_16713,
                ARD_LowPower_WatchDog_MV_test1_16713,
            ]
        )

        test_groups = [
            Spresense,
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
    tc_runner = ArdLowPowerTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
