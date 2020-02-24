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
from tc.watchdog_test.watchdog_test_device import WatchdogTestDevice
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
        timeout_1sec = 5
        timeout_30sec = 35
        timeout_40sec = 45
        timeout_50sec = 55
        def loop_begin_steps(count=1 ):
            r = []
            for i in range(count):
                    r = r + [
                        Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                        Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                    ]
            return r

        def loop_end_steps(count=1 ):
            r = []
            for i in range(count):
                    r = r + [
                        Step(Action.WRITE, watchdog_test_device, 'wd_end', None, '\r'),
                        Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: end()'),
                    ]
            return r

        watchdog_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            Step(Action.EXECUTE, watchdog_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, watchdog_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        WatchDog16373_Reboot_1second = Test(
            name='WatchDog16373_Reboot_1second',
            timeout=timeout_1sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 1000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            ],
            teardown=teardown
        )

        WatchDog16373_Reboot_30second = Test(
            name='WatchDog16373_Reboot_30second',
            timeout=timeout_30sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(31)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            ],
            teardown=teardown
        )

        WatchDog16373_Reboot_40second = Test(
            name='WatchDog16373_Reboot_40second',
            timeout=timeout_40sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 40000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '39999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(41)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            ],
            teardown=teardown
        )

        WatchDog16373_NoneReboot_0second = Test(
            name='WatchDog16373_NoneReboot_0second',
            timeout=timeout_30sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 0', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '27531'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(31)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
            ],
            teardown=teardown
        )

        WatchDog16373_NoneReboot_negative1second = Test(
            name='WatchDog16373_NoneReboot_negative1second',
            timeout=timeout_30sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start -1000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '27531'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(31)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
            ],
            teardown=teardown
        )

        WatchDog16373_NoneReboot_50second = Test(
            name='WatchDog16373_NoneReboot_50second',
            timeout=timeout_50sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 50000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '27531'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(51)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
            ],
            teardown=teardown
        )



        WatchDog16374_Reboot_1second = Test(
            name='WatchDog16374_Reboot_1second',
            timeout=timeout_1sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 1000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            ],
            teardown=teardown
        )

        WatchDog16374_Reboot_30second = Test(
            name='WatchDog16374_Reboot_30second',
            timeout=timeout_30sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(31)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            ],
            teardown=teardown
        )

        WatchDog16374_Reboot_40second = Test(
            name='WatchDog16374_Reboot_40second',
            timeout=timeout_40sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 40000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '39999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(41)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            ],
            teardown=teardown
        )

        WatchDog16375_Stop_30second = Test(
            name='WatchDog16375_Stop_30second',
            timeout=timeout_30sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(20)),
                Step(Action.WRITE, watchdog_test_device, 'wd_stop', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(11)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
            ],
            teardown=teardown
        )

        WatchDog16376_End_30second = Test(
            name='WatchDog16376_End_30second',
            timeout=timeout_30sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(20)),
                Step(Action.WRITE, watchdog_test_device, 'wd_end', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: end()'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(11)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
            ],
            teardown=teardown
        )

        WatchDog16377_Kick_30second = Test(
            name='WatchDog16377_Kick_30second',
            timeout=timeout_50sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(20)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(11)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
            ],
            teardown=teardown
        )


        WatchDog16378_KickReboot_30second = Test(
            name='WatchDog16378_KickReboot_30second',
            timeout=timeout_50sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(20)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(31)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            ],
            teardown=teardown
        )


        WatchDog16379_KicksReboot_30second = Test(
            name='WatchDog16379_KicksReboot_30second',
            timeout=timeout_50sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(31)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            ],
            teardown=teardown
        )


        WatchDog16380_KicksStop_30second = Test(
            name='WatchDog16380_KicksStop_30second',
            timeout=timeout_50sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(15)),
                Step(Action.WRITE, watchdog_test_device, 'wd_stop', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(15)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
            ],
            teardown=teardown
        )

        WatchDog16381_ColdSleep_40second = Test(
            name='WatchDog16381_ColdSleep_40second',
            timeout=timeout_50sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 40000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '39999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(5)),
                Step(Action.WRITE, watchdog_test_device, 'sleep_cold 5', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Go to sleep'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(41)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
            ],
            teardown=teardown
        )

        WatchDog16382_Sleepdeep_40second = Test(
            name='WatchDog16382_Sleepdeep_40second',
            timeout=timeout_50sec,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 40000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '39999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(5)),
                Step(Action.WRITE, watchdog_test_device, 'sleep_deep 5', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Go to sleep'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(41)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
            ],
            teardown=teardown
        )


        WatchDog16383_SleepdeepReboot_40second = Test(
            name='WatchDog16383_SleepdeepReboot_40second',
            timeout=100,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 40000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '39999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(5)),
                Step(Action.WRITE, watchdog_test_device, 'sleep_deep 5', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Go to sleep'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(41)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(31)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),

            ],
            teardown=teardown
        )


        WatchDog16410_NotFreeze = Test(
            name='WatchDog16410_NotFreeze',
            timeout=80,
            setup=setup,
            test=[
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(5)),
                Step(Action.WRITE, watchdog_test_device, 'wd_stop', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: stop()'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.WRITE, watchdog_test_device, 'wd_kick', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.WRITE, watchdog_test_device, 'wd_end', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: end()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_timeleft', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
            ],
            teardown=teardown
        )


        WatchDog16411_Reboot_1second = Test(
            name='WatchDog16411_Reboot_1second',
            timeout=200,
            setup=setup,
            test=loop_begin_steps(100) + [
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 1000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(2)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            ],
            teardown=teardown
        )

        WatchDog16411_Reboot_30second = Test(
            name='WatchDog16411_Reboot_30second',
            timeout=200,
            setup=setup,
            test=loop_begin_steps(100) + [
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 30000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '29999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(31)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            ],
            teardown=teardown
        )

        WatchDog16411_Reboot_40second = Test(
            name='WatchDog16411_Reboot_40second',
            timeout=200,
            setup=setup,
            test=loop_begin_steps(100) + [
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 40000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '39999'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(41)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_OK),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Start Watchdog test'),
            ],
            teardown=teardown
        )

        WatchDog16411_NoneReboot_0second = Test(
            name='WatchDog16411_NoneReboot_0second',
            timeout=200,
            setup=setup,
            test=loop_begin_steps(100) + [
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 0', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '27531'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(31)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
            ],
            teardown=teardown
        )

        WatchDog16411_NoneReboot_negative1second = Test(
            name='WatchDog16411_NoneReboot_negative1second',
            timeout=200,
            setup=setup,
            test=loop_begin_steps(100) + [
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start -1000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '27531'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(31)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
            ],
            teardown=teardown
        )

        WatchDog16411_NoneReboot_50second = Test(
            name='WatchDog16411_NoneReboot_50second',
            timeout=200,
            setup=setup,
            test=loop_begin_steps(100) + [
                Step(Action.WRITE, watchdog_test_device, 'wd_begin', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: begin()'),
                Step(Action.WRITE, watchdog_test_device, 'wd_start 50000', None, '\r'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'Watchdog: start(ms-sec)'),
                Step(Action.WAIT_FOR, watchdog_test_device, '27531'),
                Step(Action.WAIT_FOR, watchdog_test_device, 'ms left for watchdog bite'),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: time.sleep(51)),
                Step(Action.EXECUTE, watchdog_test_device, None,
                     lambda test, source, data: checkReboot(test, source, data)),
                Step(Action.WAIT_FOR, watchdog_test_device, REBOOT_NG),
            ],
            teardown=teardown
        )

        WatchDog16412_LoopEnd = Test(
            name='WatchDog16412_LoopEnd',
            timeout=200,
            setup=setup,
            test=loop_end_steps(100),
            teardown=teardown
        )

        watchdog = TestGroup(
            name='watchdog',
            devices=[watchdog_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                WatchDog16373_Reboot_1second,
                WatchDog16373_Reboot_30second,
                WatchDog16373_Reboot_40second,
                WatchDog16373_NoneReboot_0second,
                WatchDog16373_NoneReboot_negative1second,
                WatchDog16373_NoneReboot_50second,
                WatchDog16374_Reboot_1second,
                WatchDog16374_Reboot_30second,
                WatchDog16374_Reboot_40second,
                WatchDog16375_Stop_30second,
                WatchDog16376_End_30second,
                WatchDog16377_Kick_30second,
                WatchDog16378_KickReboot_30second,
                WatchDog16379_KicksReboot_30second,
                WatchDog16380_KicksStop_30second,
                WatchDog16381_ColdSleep_40second,
                WatchDog16382_Sleepdeep_40second,
                WatchDog16383_SleepdeepReboot_40second,
                WatchDog16410_NotFreeze,
                WatchDog16411_Reboot_1second,
                WatchDog16411_Reboot_30second,
                WatchDog16411_Reboot_40second,
                WatchDog16411_NoneReboot_0second,
                WatchDog16411_NoneReboot_negative1second,
                WatchDog16411_NoneReboot_50second,
                WatchDog16412_LoopEnd,
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
