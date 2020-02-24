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
from tc.lowpower_16707_test.lowpower_test_device import LowPowerTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['lowpower']

def wait_sec(test, source, data, sec, description):
    if description is not None:
        test.log.info("")
        test.log.info("#############################################")
        test.log.info("## [" + str(sec) + " sec] "+ description)
        test.log.info("#############################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

class LowerPowerTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(LowerPowerTestTcRunner, self).__init__(conf)

        self.lowpower_test_device = LowPowerTestDevice(
            kwargs.get('dut_device', None), 'LOWPOWER_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')


        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/lowpower-defconfig')

    def get_devices_list(self):
        return [self.lowpower_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.lowpower16700_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.lowpower_test_device)

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
        timeout = 90

        lowpower_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.EXECUTE, lowpower_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, lowpower_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        ard_lowpower_16707 = Test(
            name='ard_lowpower_16707',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, lowpower_test_device, 'Start subcore power test'),
                Step(Action.WRITE, lowpower_test_device, 'low begin', None, '\r'),
                Step(Action.WAIT_FOR, lowpower_test_device, 'LowPower: begin()'),
                Step(Action.WRITE, lowpower_test_device, 'low LV', None, '\r'),
                Step(Action.WAIT_FOR, lowpower_test_device, 'clock mode: 8MHz'),

                Step(Action.WRITE, lowpower_test_device, 'core1', None, '\r'),
                Step(Action.WAIT_FOR, lowpower_test_device, 'core1_setup()'),
                Step(Action.EXECUTE, lowpower_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 10,
                                                         "Start to measure power consumption for LV")),
                Step(Action.EXECUTE, lowpower_test_device, None, lambda test, source, data: test.log.info(
                    "Please check power consumption--Main core and sub core 1")),
                Step(Action.WRITE, lowpower_test_device, 'core2', None, '\r'),
                Step(Action.WAIT_FOR, lowpower_test_device, 'core2_setup()'),
                Step(Action.EXECUTE, lowpower_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 10,
                                                         "Start to measure power consumption for LV")),
                Step(Action.EXECUTE, lowpower_test_device, None, lambda test, source, data: test.log.info(
                    "Please check power consumption--Main core and sub core 1-2")),
                Step(Action.WRITE, lowpower_test_device, 'core3', None, '\r'),
                Step(Action.WAIT_FOR, lowpower_test_device, 'core3_setup()'),
                Step(Action.EXECUTE, lowpower_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 10,
                                                         "Start to measure power consumption for LV")),
                Step(Action.EXECUTE, lowpower_test_device, None, lambda test, source, data: test.log.info(
                    "Please check power consumption--Main core and sub core 1-3")),
                Step(Action.WRITE, lowpower_test_device, 'core4', None, '\r'),
                Step(Action.WAIT_FOR, lowpower_test_device, 'core4_setup()'),
                Step(Action.EXECUTE, lowpower_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 10,
                                                         "Start to measure power consumption for LV")),
                Step(Action.EXECUTE, lowpower_test_device, None, lambda test, source, data: test.log.info(
                    "Please check power consumption--Main core and sub core 1-4")),
                Step(Action.WRITE, lowpower_test_device, 'core5', None, '\r'),
                Step(Action.WAIT_FOR, lowpower_test_device, 'core5_setup()'),
                Step(Action.EXECUTE, lowpower_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 10,
                                                         "Start to measure power consumption for LV")),
                Step(Action.EXECUTE, lowpower_test_device, None, lambda test, source, data: test.log.info(
                    "Please check power consumption--Main core and sub core 1-5")),
            ],
            teardown=teardown
        )


        lowpower_16707 = TestGroup(
            name='lowpower_16707',
            devices=[lowpower_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ard_lowpower_16707,
            ]
        )

        test_groups = [
            lowpower_16707,
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
    tc_runner = LowerPowerTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)