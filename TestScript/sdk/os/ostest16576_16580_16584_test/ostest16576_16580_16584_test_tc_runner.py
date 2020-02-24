#! /usr/bin/env python3
import sys
import os
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.ostest16576_16580_16584_test.ostest_test_device import OsTestTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['ostest']

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if 'fpu_test: ERROR:' in data or 'waitpid failed with ECHILD' in data or 'PASS pthread_join failed with status=ESRCH' in data or 'nerrors=0' in data:
        test.add_user_event(source, 'skip')
    elif 'ERROR' in data or 'error' in data:
        test.set_fail(data)
    elif 'Failed' in data or 'failed' in data:
        test.set_fail(data)

def call_shell(test, source, data, command):
    test.log.info("start")
    os.system(command)
    test.log.info("end")

class OstestTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(OstestTestTcRunner, self).__init__(conf)

        self.ostest_test_device = OsTestTestDevice(
            kwargs.get('dut_device', None), 'OSTEST_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/ostest-defconfig')
        self.nuttxconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/nuttx-config')

    def get_devices_list(self):
        return [self.ostest_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.ostest_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.nuttxconfig_path, self.ostest_test_device)

        return dict(dut_bin=dut_bin)

    @staticmethod
    def __build_binary(toolbox, defconfig, nuttxconfig, device):
        bin_file = toolbox.builder.buildKernelApp_project(PROJECT, APPS_TO_BUILTIN, device, defconfig, nuttxconfig)
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
        #48 hours and a little margin

        ostest_test_device = self.get_devices_list()[0]


        setup = [
            Step(Action.WAIT_FOR, ostest_test_device, 'nsh>'),
            Step(Action.WRITE, ostest_test_device, 'reboot'),
            Step(Action.WAIT_FOR, ostest_test_device, 'NuttShell (NSH)'),
            Step(Action.EXECUTE, ostest_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, ostest_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        Nuttx_OsTest_16876_recursive_mutex = Test(
            name='Nuttx_OsTest_16876_recursive_mutex',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: recursive mutex test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'recursive_mutex_test: Complete'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16880_Named_semaphore = Test(
            name='Nuttx_OsTest_16880_Named_semaphore',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: Named semaphore test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16884_pthread_cleanup = Test(
            name='Nuttx_OsTest_16884_pthread_cleanup',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: pthread_cleanup test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'pthread_cleanup: Starting test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        ostest = TestGroup(
            name='ostest',
            devices=[ostest_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                Nuttx_OsTest_16876_recursive_mutex,
                Nuttx_OsTest_16880_Named_semaphore,
                Nuttx_OsTest_16884_pthread_cleanup,
            ]
        )

        test_groups = [
            ostest,
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
    tc_runner = OstestTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
