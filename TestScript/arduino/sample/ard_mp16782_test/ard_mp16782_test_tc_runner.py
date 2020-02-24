#! /usr/bin/env python3
import sys
import os

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.ard_mp16782_test.ard_core_test_device import SpresenseTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['MP']
count = 0

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    global count
    if '24000' in data:
        dataArr = data.split()
        if float(dataArr[1]) == 0 or float(dataArr[2]) == 0 or float(dataArr[3]) == 0 or float(dataArr[4]) == 0:
            test.set_fail('value:{} should be not zero'.format(data))
        else:
            count += 1
        if count > 500:
            test.add_user_event(source, "Test Done")

class ArdMpTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(ArdMpTestTcRunner, self).__init__(conf)

        self.spresense_test_device = SpresenseTestDevice(
            kwargs.get('dut_device', None), 'MP16782_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/Spresense-defconfig')

    def get_devices_list(self):
        return [self.spresense_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.spresense_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.spresense_test_device)

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
        timeout = 300

        spresense_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.EXECUTE, spresense_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, spresense_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        ARD_MP_Example_test_16782 = Test(
            name='ARD_MP_Example_test_16782',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, spresense_test_device, 'Init Audio Library'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Init Audio Recorder'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Rec start!'),
                Step(Action.WAIT_FOR, spresense_test_device, 'Test Done'),
            ],
            teardown=teardown
        )

        Spresense = TestGroup(
            name='Spresense',
            devices=[spresense_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ARD_MP_Example_test_16782,
            ]
        )

        test_groups = [
            Spresense,
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
    tc_runner = ArdMpTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)