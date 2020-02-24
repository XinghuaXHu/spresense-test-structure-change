#! /usr/bin/env python3
import sys
import os

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.sd_create_write_test.sd_create_write_test_device import SdCreateWriteTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['sd_create_write']


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


class SdCreateWriteTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(SdCreateWriteTestTcRunner, self).__init__(conf)

        self.sd_create_write_test_device = SdCreateWriteTestDevice(
            kwargs.get('dut_device', None), 'SD_CREATE_WRITE_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/sd_create_write-defconfig')

    def get_devices_list(self):
        return [self.sd_create_write_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.sd_create_write_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.sd_create_write_test_device)

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

        sd_create_write_test_device = self.get_devices_list()[0]

        setup = [
        ]

        teardown = [
        ]

        ADN_Sdcard_15643_01 = Test(
            name='ADN_Sdcard_15643_01',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, sd_create_write_test_device, sd_create_write_test_device.WRITE_TO_TEST_DONE),
                Step(Action.WAIT_FOR, sd_create_write_test_device, sd_create_write_test_device.WRITE_TO_T1ST1_DONE),
                Step(Action.WAIT_FOR, sd_create_write_test_device, sd_create_write_test_device.WRITE_TO_T2ST2_DONE),
                Step(Action.WAIT_FOR, sd_create_write_test_device, sd_create_write_test_device.WRITE_TO_DUMMY1_DONE),
                Step(Action.WAIT_FOR, sd_create_write_test_device, sd_create_write_test_device.WRITE_TO_DUMMY2_DONE),
                Step(Action.WAIT_FOR, sd_create_write_test_device, sd_create_write_test_device.WRITE_TO_SUNNY1_DONE),
                Step(Action.WAIT_FOR, sd_create_write_test_device, sd_create_write_test_device.WRITE_TO_DUMMY3_DONE),
                Step(Action.WAIT_FOR, sd_create_write_test_device, sd_create_write_test_device.WRITE_TO_DUMMY4_DONE),
                Step(Action.WAIT_FOR, sd_create_write_test_device, sd_create_write_test_device.WRITE_TO_SUNNY2_DONE),
                Step(Action.WAIT_FOR, sd_create_write_test_device, sd_create_write_test_device.TEST_DONE),
            ],
            teardown=teardown
        )

        sd_create_write = TestGroup(
            name='sd_create_write',
            devices=[sd_create_write_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ADN_Sdcard_15643_01,
            ]
        )

        test_groups = [
            sd_create_write,
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
    tc_runner = SdCreateWriteTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
