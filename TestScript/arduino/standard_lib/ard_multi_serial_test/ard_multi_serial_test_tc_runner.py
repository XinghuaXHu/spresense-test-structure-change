#! /usr/bin/env python3
import sys
import os
import re
import time

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.ard_multi_serial_test.ard_multi_serial_test_device import ArdMultiSerialTestDevice
from tc.ard_multi_serial_test.ext_serial_test_device import ArdMultiSerialTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['ard_multi_serial']
PINS = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28]

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

def wait_sec(test, source, data, sec):
    time.sleep(3)
    test.add_user_event(source, 'wait sec completed')

class ArdMultiSerialTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(ArdMultiSerialTestTcRunner, self).__init__(conf)

        self.ard_multi_serial_test_device = ArdMultiSerialTestDevice(
            kwargs.get('dut_device', None), 'ARD_MULTI_SERIAL_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if self.ard_multi_serial_test_device:
            self.ard_multi_serial_test_device.set_baudrate(9600)

        self.ext_serial_test_device = ArdMultiSerialTestDevice(
            kwargs.get('ref_device', None), 'EXT_SERIAL_TEST_DEVICE') if kwargs.get('ref_device', None)\
            else None

        if self.ext_serial_test_device:
            self.ext_serial_test_device.set_baudrate(9600)

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.getcwd(), 'defconfig/ard_multi_serial-defconfig')

    def get_devices_list(self):
        return [self.ard_multi_serial_test_device, self.ext_serial_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, ref_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None
        #binaries['ref_bin'] = os.path.normpath(arguments.ref_bin) if arguments.ref_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.ard_multi_serial_test_device, binaries['dut_bin'], log)
        #self.flash_n_check(self.ext_serial_test_device, binaries['ref_bin'], log)

    # noinspection PyUnresolvedReferences
    def build(self, debug=True, log=None, **kwargs):
        dut_bin = kwargs.get('dut_bin', None)
        ref_bin = kwargs.get('ref_bin', None)
        toolbox = None

        if not all([dut_bin, ref_bin]):
            if log:
                log.info('Building project sources')

            toolbox = Toolbox(self.config, log)
            toolbox.init_builder_module()

        if not dut_bin:
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.ard_multi_serial_test_device)

        if not ref_bin:
            ref_bin = self.__build_binary(toolbox, self.defconfig_path, self.ext_serial_test_device)


        return dict(dut_bin=dut_bin, ref_bin=ref_bin)

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

        ard_multi_serial_test_device, ext_serial_test_device = self.get_devices_list()
        setup = [
        ]

        teardown = [
        ]

        ADN_Sample_sketch_15883 = Test(
            name='ADN_Sample_sketch_15883',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, ard_multi_serial_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3)),
                Step(Action.WAIT_FOR, ard_multi_serial_test_device, 'wait sec completed'),
                Step(Action.WRITE, ard_multi_serial_test_device, 'to EXT'),
                Step(Action.WAIT_FOR, ext_serial_test_device, 'to EXT'),
                Step(Action.WRITE, ext_serial_test_device, 'to DUT'),
                Step(Action.WAIT_FOR, ard_multi_serial_test_device, 'to DUT'),
            ],
            teardown=teardown
        )

        ard_multi_serial = TestGroup(
            name='ard_multi_serial',
            devices=[ard_multi_serial_test_device, ext_serial_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ADN_Sample_sketch_15883,
            ]
        )

        test_groups = [
            ard_multi_serial,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()
    parser.add_argument('--ref_device', metavar='SERIAL_PORT', help='Set reference device')
    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    dut_device, ref_device = dev_manager.get_devices_by_serials(args.dut_device, args.ref_device)

    # Create test runner instance
    tc_runner = ArdMultiSerialTestTcRunner(config, dut_device=dut_device, ref_device=ref_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
