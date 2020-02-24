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
from tc.sdk_pwm_example.sdk_pwm_example_device import SdkPwmExampleDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['pwm']


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

def analysys_16283(test, source, data, dev_ch, frequency, duty, duration):
    child = pexpect.spawn('sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0')
    child.expect('>')
    child.sendline('RUN')
    child.expect('>')
    time.sleep(1)
    child.sendline('STOP')
    child.expect('>')
    dev_ch_to_oscillo_ch = [3, 2, 4, 1]
    child.sendline('MEASure:SOURce CHANnel{}'.format(str(dev_ch_to_oscillo_ch[dev_ch])))
    child.expect('>')
    child.sendline('MEASure:DUTYcycle?')
    child.expect('\+.+E(\+|\-)..')
    mea_duty = float(child.after)
    child.expect('>')
    child.sendline('MEASure:FREQuency?')
    child.expect('\+.+E(\+|\-)..')
    mea_freq = float(child.after)
    child.expect('>')

    test.log.info('mea_freq = {} mea_duty = {}'.format(mea_freq, mea_duty))

    if not frequency-10 <= mea_freq <= frequency+10:
        test.set_fail('Out of range (dev_ch = {} mea_freq = {})'.format(dev_ch, mea_freq))
    if not duty-2 <= mea_duty <= duty+2:
        test.set_fail('Out of range (dev_ch = {} mea_duty = {})'.format(dev_ch, mea_duty))

    child.close()
    test.add_user_event(source, 'analysys_16283 ok!')

class SdkPwmExampleTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(SdkPwmExampleTcRunner, self).__init__(conf)

        self.sdk_pwm_example_device = SdkPwmExampleDevice(
            kwargs.get('dut_device', None), 'SDK_PWM_EXAMPLE_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(self.config.projects[0].path, 'sdk/configs/examples/pwm-defconfig')

    def get_devices_list(self):
        return [self.sdk_pwm_example_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.sdk_pwm_example_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.sdk_pwm_example_device)

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

        sdk_pwm_example_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, sdk_pwm_example_device, sdk_pwm_example_device.NSH_PROMPT),
            Step(Action.WRITE, sdk_pwm_example_device, 'reboot'),
            Step(Action.WAIT_FOR, sdk_pwm_example_device, sdk_pwm_example_device.NUTTSHELL),
        ]

        teardown = [
        ]

        def wait_sec_steps(device, sec, description=None):
            return [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: wait_sec(test, source, data, sec, description)),
                Step(Action.WAIT_FOR, device, 'wait sec completed'),
            ]

        def analysys_16283_steps(device, dev_ch, frequency, duty, duration):
            return [
                Step(Action.WRITE, device, 'pwm -p /dev/pwm{} -t {} -f {} -d {}'.format(str(dev_ch), str(duration), str(frequency), str(duty))),
                Step(Action.WAIT_FOR, device, 'starting output'),
            ] +\
            wait_sec_steps(device, 1) +\
            [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: analysys_16283(test, source, data, dev_ch, frequency, duty, duration)),
                Step(Action.WAIT_FOR, device, 'analysys_16283 ok!'),
                Step(Action.WAIT_FOR, device, 'stopping output'),
            ]

        def test_16283_steps(device):
            return analysys_16283_steps(device, 0, 1000, 50, 10) +\
                   analysys_16283_steps(device, 1, 1000, 50, 10) +\
                   analysys_16283_steps(device, 2, 1000, 50, 10) +\
                   analysys_16283_steps(device, 3, 1000, 50, 10)

        sdk_pwm_example_16283 = Test(
            name='SDK_Example_test_16283',
            timeout=timeout,
            setup=setup,
            test=test_16283_steps(sdk_pwm_example_device),
            teardown=teardown
        )

        sdk_pwm_example_group = TestGroup(
            name='sdk_pwm_example_group',
            devices=[sdk_pwm_example_device],
            tag=[Tag.POSITIVE],
            tests=[
                sdk_pwm_example_16283,
            ]
        )

        test_groups = [
            sdk_pwm_example_group,
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
    tc_runner = SdkPwmExampleTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
