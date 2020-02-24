#! /usr/bin/env python3
import sys
import os
import time

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.adn_dsp_installer_test.dsp_installer_test_device import DspInstallerTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['hello']
dsp_type = ''


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


# noinspection PyUnusedLocal
def dsp_type_monitor(test, source, data):
    global dsp_type
    if source.PROMPT in data:
        dsp_type = data.get_value(source.KEY_TYPE)


# noinspection PyUnusedLocal
def ls_monitor(test, source, data):
    global dsp_type
    mnts = None
    if source.NSH_LS in data:
        mnts = data.get_value('ls')
        if dsp_type in mnts['files']:
            test.add_user_event(source, 'dsp installed')


def sleep(test, source, data):
    time.sleep(2)


class DspInstallerTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(DspInstallerTestTcRunner, self).__init__(conf)

        self.test_device = DspInstallerTestDevice(
            kwargs.get('dut_device', None), 'DSP_INSTALLER_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        # self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        # self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/hello-defconfig')

    def get_devices_list(self):
        return [self.test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        # binaries = self.build(debug, log, **binaries)
        # self.flash_n_check(self.test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.test_device)

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
        timeout = 30

        test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.EXECUTE, test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 ),
        ]

        teardown = [
            Step(Action.EXECUTE, test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        tc_dsp_installer_sd = Test(
            name='tc_dsp_installer_sd',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, test_device, ('{}_dsp_type_monitor'.format(str(test_device)),
                                                   dsp_type_monitor), Test.add_monitor),
                Step(Action.WAIT_FOR, test_device, test_device.PROMPT),
                Step(Action.WRITE, test_device, '1'),
                Step(Action.WAIT_FOR, test_device, 'Finished'),
                Step(Action.EXECUTE, test_device, '{}_dsp_type_monitor'.format(str(test_device)),
                     Test.remove_monitor),
            ],
            teardown=teardown
        )

        tc_dsp_installer_spif = Test(
            name='tc_dsp_installer_spif',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, test_device, test_device.PROMPT),
                Step(Action.WRITE, test_device, '2'),
                Step(Action.WAIT_FOR, test_device, 'Finished'),
            ],
            teardown=teardown
        )

        tg = TestGroup(
            name='tg',
            devices=[test_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_dsp_installer_sd,
                tc_dsp_installer_spif,
            ]
        )

        test_groups = [
            tg,
        ]

        return test_groups


class ShellCheckTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(ShellCheckTcRunner, self).__init__(conf)

        self.test_device = DspInstallerTestDevice(
            kwargs.get('dut_device', None), 'SHELL_CHECK_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/hello-defconfig')

    def get_devices_list(self):
        return [self.test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.test_device)

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

        test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.EXECUTE, test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 ),
        ]

        teardown = [
            Step(Action.EXECUTE, test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        tc_dsp_installer_shell = Test(
            name='tc_dsp_installer_shell',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, test_device, 'reboot'),
                Step(Action.WAIT_FOR, test_device, test_device.NUTTSHELL),
                Step(Action.EXECUTE, test_device, None, sleep),
                Step(Action.EXECUTE, test_device, ('{}_ls_monitor'.format(str(test_device)),
                     ls_monitor), Test.add_monitor),
                Step(Action.WRITE, test_device, 'ls /mnt/sd0/BIN'),
                Step(Action.WAIT_FOR, test_device, 'dsp installed'),
                Step(Action.WRITE, test_device, 'ls /mnt/spif/BIN'),
                Step(Action.WAIT_FOR, test_device, 'dsp installed'),
                Step(Action.EXECUTE, test_device, '{}_ls_monitor'.format(str(test_device)),
                     Test.remove_monitor),
            ],
            teardown=teardown
        )

        tg = TestGroup(
            name='tg',
            devices=[test_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_dsp_installer_shell,
            ]
        )

        test_groups = [
            tg,
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
    tc_runner = DspInstallerTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)

    tc_runner = ShellCheckTcRunner(config, dut_device=dut_device)
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)
    tc_runner.run(args, logger)
