#! /usr/bin/env python3
import sys
import os
import re
import time
import datetime

startTime = datetime.datetime.now()
endTime = startTime + datetime.timedelta(hours=8)
timeou8H = 29000
checkTimeCount = 0

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from tc.fatrepeat_test.fatrepeat_test_device import FatrepeatTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['fat']

# noinspection PyUnusedLocal

def dut_reboot_monitor(test, source, data):
    global checkTimeCount
    global currentTime
    global endTime
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
    elif "Verify" in data:
        checkTimeCount = checkTimeCount + 1
        if checkTimeCount > 20:
            checkTimeCount = 0
            currentTime = datetime.datetime.now()
            if currentTime > endTime:
                test.log.info(str(startTime))
                test.log.info(str(currentTime))
                test.add_user_event(source, "Long Run OK")

def dut_fatrepeat_monitor(test, source, data):
    if 'NG' in data or 'error' in data or 'failed' in data:
        test.set_fail(' '.join(['ERROR', data]))
    else:
        test.log.debug('OK!')

def wait_sec(test, source, data, sec, description):
    if description is not None:
        test.log.info("")
        test.log.info("#############################################")
        test.log.info("## [" + str(sec) + " sec] "+ description)
        test.log.info("#############################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')



class FatrepeatTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(FatrepeatTestTcRunner, self).__init__(conf)

        self.fatrepeat_test_device = FatrepeatTestDevice(
            kwargs.get('dut_device', None), 'FATREPEAT_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/fat-defconfig')

    def get_devices_list(self):
        return [self.fatrepeat_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.fatrepeat_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.fatrepeat_test_device)

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
        timeout = timeou8H

        fatrepeat_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, fatrepeat_test_device, fatrepeat_test_device.NSH_PROMPT),
            Step(Action.WRITE, fatrepeat_test_device, 'reboot'),
            Step(Action.WAIT_FOR, fatrepeat_test_device, fatrepeat_test_device.NUTTSHELL),
            Step(Action.EXECUTE, fatrepeat_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
            Step(Action.EXECUTE, fatrepeat_test_device, ('dut_fatrepeat_mon', dut_fatrepeat_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, fatrepeat_test_device, 'dut_fatrepeat_mon', Test.remove_monitor),
            Step(Action.EXECUTE, fatrepeat_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        SDK_System_15935 = Test(
            name='SDK_System_15935',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, fatrepeat_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "fatrepeat wait")),
                Step(Action.WRITE, fatrepeat_test_device, 'fat -mt r'),
                Step(Action.WAIT_FOR, fatrepeat_test_device, 'Long Run OK')
            ],
            teardown=teardown
        )

        fatrepeat = TestGroup(
            name='fatrepeat',
            devices=[fatrepeat_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_System_15935,
            ]
        )

        test_groups = [
            fatrepeat,
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
    tc_runner = FatrepeatTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
