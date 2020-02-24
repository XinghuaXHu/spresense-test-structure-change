#! /usr/bin/env python3
import sys
import os
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.dig_read_high_test.dig_read_high_test_device import DigReadHighTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['dig_read_high']
EXT_PINS = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28]
LTE_PINS = [2,3,5,6,7,9]
global gLTE_Flage
gLTE_Flage = False

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

def dut_pin_monitor(test, source, data):

    if gLTE_Flage is True:
        PINS = LTE_PINS
    else:
        PINS = EXT_PINS

    number = re.search(r'pin\[[0-9]+', data).group().replace('pin[', '')
    status = re.search(r'=\[[0-9]+', data).group().replace('=[', '')
    if int(number) in PINS:
        if int(status) == 1:
            return True
        else:
            test.set_fail(' '.join(['Invalid status. ' + 'Number = ' + number + ' status = ', status]))
            return False
    else:
        return True

class DigReadHighTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(DigReadHighTestTcRunner, self).__init__(conf)

        self.dig_read_high_test_device = DigReadHighTestDevice(
            kwargs.get('dut_device', None), 'DIG_READ_HIGH_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/dig_read_high-defconfig')

    def get_devices_list(self):
        return [self.dig_read_high_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.dig_read_high_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.dig_read_high_test_device)

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
        timeout = 10

        dig_read_high_test_device = self.get_devices_list()[0]

        global gLTE_Flage
        if 'lte' == dig_read_high_test_device.ext_type:
            PINS = LTE_PINS
            gLTE_Flage = True
        else:
            gLTE_Flage = False
            PINS = EXT_PINS

        setup = [
            Step(Action.EXECUTE, dig_read_high_test_device, ('dut_pin_mon', dut_pin_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, dig_read_high_test_device, 'dut_pin_mon', Test.remove_monitor),
        ]

        ADN_Digital_io_15715 = Test(
            name='ADN_Digital_io_15715',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, dig_read_high_test_device, 'pin['+ str(max(PINS)) + ']',
                     lambda test, source, data: dut_pin_monitor(test, source, data)),
            ],
            teardown=teardown
        )

        dig_read_high = TestGroup(
            name='dig_read_high',
            devices=[dig_read_high_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ADN_Digital_io_15715,
            ]
        )

        test_groups = [
            dig_read_high,
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
    tc_runner = DigReadHighTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)