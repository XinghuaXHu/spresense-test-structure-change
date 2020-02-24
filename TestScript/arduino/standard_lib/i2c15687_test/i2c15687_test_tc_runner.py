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
from tc.i2c15687_test.i2c15687_test_device import I2C15687TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser
GPGSV_count = 0
QZGSV_count = 0
pos_count = 0

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['i2c']


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

def checkStep(test, source, data, value):
    test.log.info(value)

def call_shell(test, source, data, command):
    test.log.info("start")
    os.system(command)
    test.log.info("end")

class I2CTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(I2CTestTcRunner, self).__init__(conf)

        self.I2C15687_test2_device = I2C15687TestDevice(
            kwargs.get('dut2_device', None), 'I2C15687_TEST2_DEVICE') if kwargs.get('dut2_device', None)\
            else None
        self.I2C15687_test_device = I2C15687TestDevice(
            kwargs.get('dut_device', None), 'I2C15687_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')


        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/i2c-defconfig')

    def get_devices_list(self):
        return [self.I2C15687_test2_device, self.I2C15687_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.I2C15687_test2_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.I2C15687_test2_device)

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

        I2C15687_test2_device, I2C15687_test_device = self.get_devices_list()

        setup = [
            Step(Action.EXECUTE, I2C15687_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, I2C15687_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        Arduino_I2C_15687 = Test(
            name='Arduino_I2C_15687',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, I2C15687_test_device, 'test wire start'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:97------a'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:66------B'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:32------'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:0------'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:10------'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:37------%'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:48------0'),

                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:65------A'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:120------x'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:35------#'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:32------'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:110------n'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:10------'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:48------0'),

                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:65------A'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:120------x'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:35------#'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:32------'),
                Step(Action.WAIT_FOR, I2C15687_test_device, 'I receive:110------n'),
            ],
            teardown=teardown
        )

        i2c_group = TestGroup(
            name='i2c_group',
            devices=[I2C15687_test_device, I2C15687_test2_device],
            tag=[Tag.POSITIVE],
            tests=[
                Arduino_I2C_15687,
            ]
        )

        test_groups = [
            i2c_group,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()

    parser.add_argument('--master', metavar='SERIAL_PORT', help='Set master')
    parser.add_argument('--slave', metavar='SERIAL_PORT', help='Set slave')

    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    dut2_device, dut_device = dev_manager.get_devices_by_serials(args.master, args.slave)

    # Create test runner instance
    tc_runner = I2CTestTcRunner(config, dut2_device=dut2_device, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)