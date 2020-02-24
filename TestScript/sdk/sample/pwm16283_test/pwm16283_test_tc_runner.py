#! /usr/bin/env python3
import sys
import os
import time
import pexpect

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.pwm16283_test.pwm16283_test_device import SpresenseTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['pwm']

def analysys_15773(test, source, data, write_value):
    child = pexpect.spawn('sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0')
    child.expect('>')
    child.sendline('RUN')
    child.expect('>')
    time.sleep(1)
    child.sendline('STOP')
    child.expect('>')
    ch = 'CHANnel3'

    child.sendline('MEASure:SOURce {}'.format(ch))
    child.expect('>')
    child.sendline('MEASure:DUTYcycle?')
    child.expect('\+.+E(\+|\-)..')
    duty = float(child.after)
    child.expect('>')
    child.sendline('MEASure:VPP?')
    child.expect('\+.+E(\+|\-)..')
    vpp = float(child.after)
    child.expect('>')
    if write_value == 0:
        if not vpp <= 1:
            test.set_fail('Out of range (ch = {} write_value = {} vpp = {})'.format(ch, write_value, vpp))
    elif write_value == 127:
        if not 48.0 <= duty <= 52.0:
            test.set_fail('Out of range (ch = {} write_value = {} duty = {})'.format(ch, write_value, duty))
    elif write_value == 255:
        if not 4.5 <= vpp <= 5.5:
            test.set_fail('Out of range (ch = {} write_value = {} vpp = {})'.format(ch, write_value, vpp))
    else:
        test.set_fail('Invalid parameter (write_value = {})'.format(write_value))

    child.close()
    test.add_user_event(source, 'analysys_15773 {} ok!'.format(write_value))


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


class pwmTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(pwmTestTcRunner, self).__init__(conf)

        self.pwm_test_device = SpresenseTestDevice(
            kwargs.get('dut_device', None), 'PWM16283_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/pwm-defconfig')

    def get_devices_list(self):
        return [self.pwm_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.pwm_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.pwm_test_device)

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
        timeout = 60

        pwm_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, pwm_test_device, pwm_test_device.NSH_PROMPT),
            Step(Action.WRITE, pwm_test_device, 'reboot'),
            Step(Action.WAIT_FOR, pwm_test_device, pwm_test_device.NUTTSHELL),
            Step(Action.EXECUTE, pwm_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, pwm_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        SDK_Example_test_16283 = Test(
            name='SDK_Example_test_16283',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, pwm_test_device, 'pwm'),
                Step(Action.WAIT_FOR, pwm_test_device, 'pwm_main: starting output with frequency: 1000 duty: 00008000'),
                Step(Action.WAIT_FOR, pwm_test_device, 'pwm_main: stopping output'),
                Step(Action.EXECUTE, pwm_test_device, None, lambda test, source, data: time.sleep(5)),
                Step(Action.EXECUTE, pwm_test_device, None,
                     lambda test, source, data: analysys_15773(test, source, data, 127)),
                Step(Action.WAIT_FOR, pwm_test_device, 'analysys_15773 {} ok!'.format(127)),
            ],
            teardown=teardown
        )

        pwm = TestGroup(
            name='pwm',
            devices=[pwm_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Example_test_16283,
            ]
        )

        test_groups = [
            pwm,
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
    tc_runner = pwmTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
