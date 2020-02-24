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
from tc.adn_analog_write_pwm.adn_analog_write_pwm_device import AdnAnalogWritePwmDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['']


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

def wait_sec(test, source, data, sec, description=None):
    if description is not None:
        test.log.info("")
        test.log.info("############################################################")
        test.log.info("## [{}sec] {}".format(str(sec), description))
        test.log.info("############################################################")
        test.log.info("")

    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')

def analysys_15773(test, source, data, write_value):
    child = pexpect.spawn('sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0')
    child.expect('>')
    child.sendline('RUN')
    child.expect('>')
    time.sleep(1)
    child.sendline('STOP')
    child.expect('>')

    for ch in ['CHANnel1', 'CHANnel2', 'CHANnel3', 'CHANnel4']:
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

def analysys_15774(test, source, data):
    child = pexpect.spawn('sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0')
    child.expect('>')
    child.sendline('RUN')
    child.expect('>')
    time.sleep(1)
    child.sendline('STOP')
    child.expect('>')

    for ch in ['CHANnel1', 'CHANnel2', 'CHANnel3', 'CHANnel4']:
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

        if ch == 'CHANnel1':
            if not vpp <= 1:
                test.set_fail('Out of range (ch = {} vpp = {})'.format(ch, vpp))
        elif ch == 'CHANnel2':
            if not 23.0 <= duty <= 27.0:
                test.set_fail('Out of range (ch = {} duty = {})'.format(ch, duty))
        elif ch == 'CHANnel3':
            if not 48.0 <= duty <= 52.0:
                test.set_fail('Out of range (ch = {} duty = {})'.format(ch, duty))
        elif ch == 'CHANnel4':
            if not 4.5 <= vpp <= 5.5:
                test.set_fail('Out of range (ch = {} vpp = {})'.format(ch, vpp))
        else:
            test.set_fail('Invalid parameter (ch = {})'.format(ch))

    child.close()
    test.add_user_event(source, 'analysys_15774 ok!')

class AdnAnalogWritePwmTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AdnAnalogWritePwmTcRunner, self).__init__(conf)

        self.adn_analog_write_pwm_device = AdnAnalogWritePwmDevice(
            kwargs.get('dut_device', None), 'ADN_ANALOG_WRITE_PWM_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.getcwd(), 'defconfig/ard_analog_read_vol-defconfig')

    def get_devices_list(self):
        return [self.adn_analog_write_pwm_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.adn_analog_write_pwm_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.adn_analog_write_pwm_device)

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

        adn_analog_write_pwm_device = self.get_devices_list()[0]

        setup = [
        ]

        teardown = [
        ]

        def wait_sec_steps(device, sec, description=None):
            return [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: wait_sec(test, source, data, sec, description)),
                Step(Action.WAIT_FOR, device, 'wait sec completed'),
            ]

        def analysys_15773_steps(device, write_value):
            return [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: analysys_15773(test, source, data, write_value)),
                Step(Action.WAIT_FOR, device, 'analysys_15773 {} ok!'.format(write_value)),
            ]

        def analysys_15774_steps(device):
            return [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: analysys_15774(test, source, data)),
                Step(Action.WAIT_FOR, device, 'analysys_15774 ok!'),
            ]

        def test_15773_steps(device):
            return [
                Step(Action.WAIT_FOR, device, 'Input \"[TestID];\"'),
                Step(Action.WRITE, device, '15773;'),
                Step(Action.WAIT_FOR, device, 'test analogWrite 15773'),
                Step(Action.WAIT_FOR, device, 'set analogWrite 0'),
            ] +\
            wait_sec_steps(device, 1) +\
            analysys_15773_steps(device, 0) +\
            [
                Step(Action.WAIT_FOR, device, 'set analogWrite 127'),
            ] +\
            wait_sec_steps(device, 1) +\
            analysys_15773_steps(device, 127) +\
            [
                Step(Action.WAIT_FOR, device, 'set analogWrite 255'),
            ] + wait_sec_steps(device, 1) +\
            analysys_15773_steps(device, 255)

        def test_15774_steps(device):
            return [
                Step(Action.WAIT_FOR, device, 'Input \"[TestID];\"'),
                Step(Action.WRITE, device, '15774;'),
                Step(Action.WAIT_FOR, device, 'test analogWrite 15774'),
            ] +\
            wait_sec_steps(device, 1) +\
            analysys_15774_steps(device)

        adn_analog_write_15773 = Test(
            name='ADN_Analog_io_15773',
            timeout=timeout,
            setup=setup,
            test=test_15773_steps(adn_analog_write_pwm_device),
            teardown=teardown
        )

        adn_analog_write_15774 = Test(
            name='ADN_Analog_io_15774',
            timeout=timeout,
            setup=setup,
            test=test_15774_steps(adn_analog_write_pwm_device),
            teardown=teardown
        )

        adn_analog_write_group = TestGroup(
            name='adn_analog_write_group',
            devices=[adn_analog_write_pwm_device],
            tag=[Tag.POSITIVE],
            tests=[
                adn_analog_write_15773,
                adn_analog_write_15774,
            ]
        )

        test_groups = [
            adn_analog_write_group,
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
    tc_runner = AdnAnalogWritePwmTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
