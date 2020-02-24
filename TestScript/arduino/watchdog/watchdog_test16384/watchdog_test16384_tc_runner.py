#! /usr/bin/env python3
import sys
import os
import time
import datetime

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.watchdog_test16384.watchdog_test_device import WatchdogTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser
global _global_dict
_global_dict = {}
_global_dict['reboot'] = 'NO'
PROJECT = 'spresense'
APPS_TO_BUILTIN = ['watchdog']
REBOOT_OK = 'rebootOK'
REBOOT_NG = 'rebootNG'


def checkReboot(test, source, data):
    if _global_dict['reboot'] == 'YES':
        _global_dict.pop('reboot')
        _global_dict['reboot'] = 'NO'
        test.add_user_event(source, REBOOT_OK)
    else:
        test.add_user_event(source, REBOOT_NG)



# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
    elif 'up_taskdump: <pthread>' in data:
        _global_dict.pop('reboot')
        _global_dict['reboot'] = 'YES'


class WatchdogTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(WatchdogTestTcRunner, self).__init__(conf)

        self.watchdog_test_device = WatchdogTestDevice(
            kwargs.get('dut_device', None), 'WATCHDOG_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.getcwd(), 'defconfig/watchdog-defconfig')

    def get_devices_list(self):
        return [self.watchdog_test_device]

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.watchdog_test_device)

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
        timeout_90sec = 90


        watchdog_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            Step(Action.EXECUTE, watchdog_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, watchdog_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        WatchDog16384_Kicks = Test(
            name='WatchDog16384_Kicks',
            timeout=timeout_90sec,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, watchdog_test_device, 'end kick-loop and reset watchdog'),
                Step(Action.EXECUTE, watchdog_test_device, None, lambda test, source, data: time.sleep(41)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),
            ],
            teardown=teardown
        )

        watchdog = TestGroup(
            name='watchdog',
            devices=[watchdog_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                WatchDog16384_Kicks,
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
