#! /usr/bin/env python3
import sys
import os
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from tc.colorsensor_example_test.colorsensor_example_test_device import ColorsensorExampleTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['colorsensor']
colorsensor_int = 0

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))



def dut_colorsensor_monitor(test, source, data):
    #if 'start' in data:
     #   test.log.debug('testing')

#    test.log.debug(data)
    if ' R' in data:
     #   test.log.debug(type(data))
    #    test.log.debug(data.get_value(' R'))
        ary_data = data.split(":")
        color_r = int( ary_data[1][:4] ,16)
        color_g = int( ary_data[2][:4] ,16)
        color_b = int( ary_data[3][:4] ,16)
        color_c = int( ary_data[4][:4] ,16)
        if 0xffff >= color_r >= 0000 and  0xffff >= color_g >= 0000 and 0xffff >= color_b >= 0000 and 0xffff >= color_c >= 0000:
           test.log.debug('OK!')
        else:
            test.set_fail(' '.join(['Out of range. colorsensor = ', colorsensor_str]))

class ColorsensorExampleTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(ColorsensorExampleTestTcRunner, self).__init__(conf)

        self.colorsensor_example_test_device = ColorsensorExampleTestDevice(
            kwargs.get('dut_device', None), 'COLORSENSOR_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')


        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.getcwd(), 'defconfig/colorsensor-defconfig')

    def get_devices_list(self):
        return [self.colorsensor_example_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.colorsensor_example_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.colorsensor_example_test_device)

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
        timeout = 15

        colorsensor_example_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, colorsensor_example_test_device, colorsensor_example_test_device.NSH_PROMPT),
            Step(Action.WRITE, colorsensor_example_test_device, 'reboot'),
            Step(Action.WAIT_FOR, colorsensor_example_test_device, colorsensor_example_test_device.NUTTSHELL),
            Step(Action.EXECUTE, colorsensor_example_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
            Step(Action.EXECUTE, colorsensor_example_test_device, ('dut_colorsensor_mon', dut_colorsensor_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, colorsensor_example_test_device, 'dut_colorsensor_mon', Test.remove_monitor),
            Step(Action.EXECUTE, colorsensor_example_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        SDK_Example_test_15846 = Test(
            name='SDK_Example_test_15846',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, colorsensor_example_test_device, 'colorsensor'),
                Step(Action.WAIT_FOR, colorsensor_example_test_device, colorsensor_example_test_device.COLORSENSOREND),
            ],
            teardown=teardown
        )

        colorsensor = TestGroup(
            name='colorsensor',
            devices=[colorsensor_example_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Example_test_15846,
            ]
        )

        test_groups = [
            colorsensor,
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
    tc_runner = ColorsensorExampleTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)