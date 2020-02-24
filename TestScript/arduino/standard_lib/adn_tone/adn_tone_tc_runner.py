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
from tc.adn_tone.adn_tone_device import AdnToneDevice
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
            if not 48.0 < duty < 52.0:
                test.set_fail('Out of range (ch = {} write_value = {} duty = {})'.format(ch, write_value, duty))
        elif write_value == 255:
            if not 4.5 < vpp < 5.5:
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

def analysys_15783(test, source, data, pin_no, frequency):
    oscillo_ch = pin_no % 4 + 1
    child = pexpect.spawn('sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0')
    child.expect('>')
    child.sendline('RUN')
    child.expect('>')
    time.sleep(1)
    child.sendline('STOP')
    child.expect('>')
    child.sendline('MEASure:SOURce CHANnel{}'.format(str(oscillo_ch)))
    child.expect('>')
    child.sendline('MEASure:FREQuency?')
    child.expect('\+.+E(\+|\-)..')
    mea_freq = float(child.after)
    child.expect('>')

    test.log.info('mea_freq = {}'.format(mea_freq))
    if frequency == 31:
        if not frequency-2 <= mea_freq <= frequency+2:
            test.set_fail('Out of range (pin_no = {} oscillo_ch = {} mea_freq = {})'.format(pin_no, oscillo_ch, mea_freq))
    elif frequency == 10000:
        if not frequency-1000 <= mea_freq <= frequency+1000:
            test.set_fail('Out of range (pin_no = {} oscillo_ch = {} mea_freq = {})'.format(pin_no, oscillo_ch, mea_freq))
    elif frequency == 65535:
        if not 50000 <= mea_freq <= 53000:
            test.set_fail('Out of range (pin_no = {} oscillo_ch = {} mea_freq = {})'.format(pin_no, oscillo_ch, mea_freq))
    elif frequency == 0:
        if mea_freq != 9.9e+37:
            test.set_fail('Out of range (pin_no = {} oscillo_ch = {} mea_freq = {})'.format(pin_no, oscillo_ch, mea_freq))
    child.close()
    test.add_user_event(source, 'analysys_15783 ok!')

def analysys_15784(test, source, data, pin_no, frequency):
    oscillo_ch = pin_no % 3 + 1
    
    child = pexpect.spawn('sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0')
    child.expect('>')
    child.sendline('RUN')
    child.expect('>')
    time.sleep(1)
    child.sendline('STOP')
    child.expect('>')
    child.sendline('MEASure:SOURce CHANnel{}'.format(str(oscillo_ch)))
    child.expect('>')
    child.sendline('MEASure:FREQuency?')
    child.expect('\+.+E(\+|\-)..')
    mea_freq = float(child.after)
    child.expect('>')
    child.sendline('MEASure:SOURce CHANnel{}'.format(str(oscillo_ch+1)))
    child.expect('>')
    child.sendline('MEASure:FREQuency?')
    child.expect('\+.+E(\+|\-)..')
    mea_freq_next = float(child.after)

    test.log.info('mea_freq = {} mea_freq_next = {}'.format(mea_freq, mea_freq_next))

    if not frequency-1000 <= mea_freq <= frequency+1000:
        test.set_fail('Out of range (pin_no = {} oscillo_ch = {} mea_freq = {})'.format(pin_no, oscillo_ch, mea_freq))

    if mea_freq_next != 9.9e+37 and mea_freq_next >= 1000:
        test.set_fail('Out of range (pin_no = {} oscillo_ch = {} mea_freq_next = {})'.format(pin_no, oscillo_ch, mea_freq_next))

    child.close()
    test.add_user_event(source, 'analysys_15784 ok!')

def analysys_15786(test, source, data, pin_no, frequency):
    oscillo_ch = pin_no % 4 + 1
    child = pexpect.spawn('sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0')
    child.expect('>')
    child.sendline('RUN')
    child.expect('>')
    time.sleep(1)
    child.sendline('STOP')
    child.expect('>')
    child.sendline('MEASure:SOURce CHANnel{}'.format(str(oscillo_ch)))
    child.expect('>')
    child.sendline('MEASure:FREQuency?')
    child.expect('\+.+E(\+|\-)..')
    mea_freq = float(child.after)
    child.expect('>')

    test.log.info('mea_freq = {}'.format(mea_freq))
    if frequency == 31:
        if not frequency-2 <= mea_freq <= frequency+2:
            test.set_fail('Out of range (pin_no = {} oscillo_ch = {} mea_freq = {})'.format(pin_no, oscillo_ch, mea_freq))
    elif frequency == 0:
        if mea_freq != 9.9e+37:
            test.set_fail('Out of range (pin_no = {} oscillo_ch = {} mea_freq = {})'.format(pin_no, oscillo_ch, mea_freq))
    child.close()
    test.add_user_event(source, 'analysys_15786 ok!')

def analysys_15788(test, source, data, pin_no, frequency):
    oscillo_ch = pin_no % 4 + 1
    child = pexpect.spawn('sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0')
    child.expect('>')
    child.sendline('RUN')
    child.expect('>')
    time.sleep(1)
    child.sendline('STOP')
    child.expect('>')
    child.sendline('MEASure:SOURce CHANnel{}'.format(str(oscillo_ch)))
    child.expect('>')
    child.sendline('MEASure:FREQuency?')
    child.expect('\+.+E(\+|\-)..')
    mea_freq = float(child.after)
    child.expect('>')

    test.log.info('mea_freq = {}'.format(mea_freq))
    if frequency == 31:
        if not frequency-2 <= mea_freq <= frequency+2:
            test.set_fail('Out of range (pin_no = {} oscillo_ch = {} mea_freq = {})'.format(pin_no, oscillo_ch, mea_freq))
    elif frequency == 0:
        if mea_freq != 9.9e+37:
            test.set_fail('Out of range (pin_no = {} oscillo_ch = {} mea_freq = {})'.format(pin_no, oscillo_ch, mea_freq))
    child.close()
    test.add_user_event(source, 'analysys_15788 ok!')

class AdnToneTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AdnToneTcRunner, self).__init__(conf)

        self.adn_tone_device = AdnToneDevice(
            kwargs.get('dut_device', None), 'ADN_TONE_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')

    def get_devices_list(self):
        return [self.adn_tone_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.adn_tone_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.adn_tone_device)

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

        adn_tone_device = self.get_devices_list()[0]

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

        def analysis_15783_steps(device, pin_no):
            return [
                Step(Action.WAIT_FOR, device, 'Input \"[TestID];\"'),
                Step(Action.WRITE, device, '15783;'),
                Step(Action.WAIT_FOR, device, 'Input \"[PinNO];\"'),
                Step(Action.WRITE, device, '{};'.format(str(pin_no))),
                Step(Action.WAIT_FOR, device, 'test tone 15783'),
                Step(Action.WAIT_FOR, device, 'tone set 31'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: analysys_15783(test, source, data, pin_no, 31)),
                Step(Action.WAIT_FOR, device, 'analysys_15783 ok!'),
                Step(Action.WAIT_FOR, device, 'tone set 10000'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: analysys_15783(test, source, data, pin_no, 10000)),
                Step(Action.WAIT_FOR, device, 'analysys_15783 ok!'),
                Step(Action.WAIT_FOR, device, 'tone set 65535'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: analysys_15783(test, source, data, pin_no, 65535)),
                Step(Action.WAIT_FOR, device, 'analysys_15783 ok!'),
                Step(Action.WAIT_FOR, device, 'tone set notone'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: analysys_15783(test, source, data, pin_no, 0)),
                Step(Action.WAIT_FOR, device, 'analysys_15783 ok!'),

            ]

        def analysis_15784_steps(device, pin_no):
            return [
                Step(Action.WAIT_FOR, device, 'Input \"[TestID];\"'),
                Step(Action.WRITE, device, '15784;'),
                Step(Action.WAIT_FOR, device, 'Input \"[PinNO];\"'),
                Step(Action.WRITE, device, '{};'.format(str(pin_no))),
                Step(Action.WAIT_FOR, device, 'test tone 15784'),
                Step(Action.WAIT_FOR, device, 'tone set 10000'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: analysys_15784(test, source, data, pin_no, 10000)),
                Step(Action.WAIT_FOR, device, 'analysys_15784 ok!'),
            ]

        def analysis_15786_steps(device, pin_no):
            return [
                Step(Action.WAIT_FOR, device, 'Input \"[TestID];\"'),
                Step(Action.WRITE, device, '15786;'),
                Step(Action.WAIT_FOR, device, 'Input \"[PinNO];\"'),
                Step(Action.WRITE, device, '{};'.format(str(pin_no))),
                Step(Action.WAIT_FOR, device, 'test tone 15786'),
                Step(Action.WAIT_FOR, device, 'tone set 31'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: analysys_15786(test, source, data, pin_no, 31)),
                Step(Action.WAIT_FOR, device, 'analysys_15786 ok!'),
                Step(Action.WAIT_FOR, device, 'tone duration completed'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: analysys_15786(test, source, data, pin_no, 0)),
                Step(Action.WAIT_FOR, device, 'analysys_15786 ok!'),
            ]

        def analysis_15788_steps(device, pin_no):
            return [
                Step(Action.WAIT_FOR, device, 'Input \"[TestID];\"'),
                Step(Action.WRITE, device, '15788;'),
                Step(Action.WAIT_FOR, device, 'Input \"[PinNO];\"'),
                Step(Action.WRITE, device, '{};'.format(str(pin_no))),
                Step(Action.WAIT_FOR, device, 'test tone 15788'),
                Step(Action.WAIT_FOR, device, 'tone set 31'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: analysys_15788(test, source, data, pin_no, 31)),
                Step(Action.WAIT_FOR, device, 'analysys_15788 ok!'),
                Step(Action.WAIT_FOR, device, 'tone set notone'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: analysys_15788(test, source, data, pin_no, 0)),
                Step(Action.WAIT_FOR, device, 'analysys_15788 ok!'),
            ]

        def loop_15783_steps(device, pin_no, count):
            r = []
            for i in range(pin_no, pin_no+count):
                r = r + analysis_15783_steps(device, i)
            return r

        def loop_15784_steps(device, pin_no, count):
            r = []
            for i in range(pin_no, pin_no+count):
                r = r + analysis_15784_steps(device, i)
            return r

        def loop_15786_steps(device, pin_no, count):
            r = []
            for i in range(pin_no, pin_no+count):
                r = r + analysis_15786_steps(device, i)
            return r

        def loop_15788_steps(device, pin_no, count):
            r = []
            for i in range(pin_no, pin_no+count):
                r = r + analysis_15788_steps(device, i)
            return r

        def test_15783_steps(device):
            return loop_15783_steps(device,  0, 4)
#            return loop_15783_steps(device,  4, 4)
#            return loop_15783_steps(device,  8, 4)
#            return loop_15783_steps(device, 12, 4)
#            return loop_15783_steps(device, 16, 4)
#            return loop_15783_steps(device, 20, 4)
#            return loop_15783_steps(device, 24, 4)
#            return loop_15783_steps(device, 28, 1)

        def test_15784_steps(device):
            return loop_15784_steps(device,  0, 3)
#            return loop_15784_steps(device,  3, 3)
#            return loop_15784_steps(device,  6, 3)
#            return loop_15784_steps(device,  9, 3)
#            return loop_15784_steps(device, 12, 3)
#            return loop_15784_steps(device, 15, 3)
#            return loop_15784_steps(device, 18, 3)
#            return loop_15784_steps(device, 21, 3)
#            return loop_15784_steps(device, 24, 3)
#            return loop_15784_steps(device, 27, 2)

        def test_15786_steps(device):
            return loop_15786_steps(device,  0, 4)
#            return loop_15786_steps(device,  4, 4)
#            return loop_15786_steps(device,  8, 4)
#            return loop_15786_steps(device, 12, 4)
#            return loop_15786_steps(device, 16, 4)
#            return loop_15786_steps(device, 20, 4)
#            return loop_15786_steps(device, 24, 4)
#            return loop_15786_steps(device, 28, 1)

        def test_15788_steps(device):
            return loop_15788_steps(device,  0, 4)
#            return loop_15788_steps(device,  4, 4)
#            return loop_15788_steps(device,  8, 4)
#            return loop_15788_steps(device, 12, 4)
#            return loop_15788_steps(device, 16, 4)
#            return loop_15788_steps(device, 20, 4)
#            return loop_15788_steps(device, 24, 4)
#            return loop_15788_steps(device, 28, 1)

        adn_tone_15783 = Test(
            name='ADN_Advanced_io_15783',
            timeout=timeout,
            setup=setup,
            test=test_15783_steps(adn_tone_device),
            teardown=teardown
        )

        adn_tone_15784 = Test(
            name='ADN_Advanced_io_15784',
            timeout=timeout,
            setup=setup,
            test=test_15784_steps(adn_tone_device),
            teardown=teardown
        )

        adn_tone_15786 = Test(
            name='ADN_Advanced_io_15786',
            timeout=timeout,
            setup=setup,
            test=test_15786_steps(adn_tone_device),
            teardown=teardown
        )

        adn_tone_15788 = Test(
            name='ADN_Advanced_io_15788',
            timeout=timeout,
            setup=setup,
            test=test_15788_steps(adn_tone_device),
            teardown=teardown
        )

        adn_tone_group = TestGroup(
            name='adn_tone_group',
            devices=[adn_tone_device],
            tag=[Tag.POSITIVE],
            tests=[
                adn_tone_15783,
                adn_tone_15784,
                adn_tone_15786,
                adn_tone_15788,
            ]
        )

        test_groups = [
            adn_tone_group,
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
    tc_runner = AdnToneTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
