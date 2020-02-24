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
from tc.ard_sample_16299_test2.ard_sample_16299_test2_device import MP3TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['sd']


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


def check_file_exist(test, source, data):
    size_value = re.search(r'( [0-9]+ )', data)
    if size_value:
        size_str = size_value.group().strip()
        if int(size_str) >= 0:
            return True
        else:
            test.set_fail(' '.join(['file not find=', size_str]))
    return False

def wait_sec(test, source, data, sec, description):
    if description is not None:
        test.log.info("")
        test.log.info("#############################################")
        test.log.info("## [" + str(sec) + " sec] "+ description)
        test.log.info("#############################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')


class ArdMp3TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(ArdMp3TestTcRunner, self).__init__(conf)

        self.mp3_test_device = MP3TestDevice(
            kwargs.get('dut_device', None), 'MP3_16299_TEST2_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/sd-defconfig')

    def get_devices_list(self):
        return [self.mp3_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.mp3_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.mp3_test_device)

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
        device.check_device_config(str(device), 'mount', log)

        if os.path.basename(dut_bin).startswith(str(device)):
            try:
                os.remove(dut_bin)
            except OSError as e:
                log.error(e)
                raise

    def generate_test_groups(self, arguments, log=None):
        timeout = 10

        mp3_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, mp3_test_device, mp3_test_device.NSH_PROMPT),
            Step(Action.WRITE, mp3_test_device, 'reboot'),
            Step(Action.WAIT_FOR, mp3_test_device, mp3_test_device.NUTTSHELL),
            Step(Action.EXECUTE, mp3_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, mp3_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        ARD_Example_test_16299_MP3DEC_verify = Test(
            name='ARD_Example_test_16299_MP3DEC_verify',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, mp3_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, 'wait for sd0')),
                Step(Action.WRITE, mp3_test_device, 'ls -l /mnt/spif/BIN/MP3DEC'),
                Step(Action.WAIT_FOR, mp3_test_device, '/mnt/spif/BIN/MP3DEC',
                     lambda test, source, data: check_file_exist(test, source, data)),
                Step(Action.WRITE, mp3_test_device, 'ls -l /mnt/sd0/BIN/MP3DEC'),
                Step(Action.WAIT_FOR, mp3_test_device, '/mnt/sd0/BIN/MP3DEC',
                     lambda test, source, data: check_file_exist(test, source, data)),
            ],
            teardown=teardown
        )

        ARD_Example_test_16299_MP3DEC_clear = Test(
            name='ARD_Example_test_16299_MP3DEC_clear',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, mp3_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, 'wait for sd0')),
                Step(Action.WRITE, mp3_test_device, 'rm /mnt/spif/BIN/MP3DEC'),
                Step(Action.WRITE, mp3_test_device, 'rm /mnt/sd0/BIN/MP3DEC'),
                Step(Action.EXECUTE, mp3_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, 'wait for moving')),
                Step(Action.WRITE, mp3_test_device, 'rm /mnt/spif/BIN/MP3DEC'),
                Step(Action.WAIT_FOR, mp3_test_device, 'CMD_FAILED'),
                Step(Action.WRITE, mp3_test_device, 'rm /mnt/sd0/BIN/MP3DEC'),
                Step(Action.WAIT_FOR, mp3_test_device, 'CMD_FAILED'),
            ],
            teardown=teardown
        )

        def seleteTest(arguments):
            if arguments.clear == True:
                return [ARD_Example_test_16299_MP3DEC_clear, ]
            else:
                return [ARD_Example_test_16299_MP3DEC_verify, ]

        testGP = TestGroup(
            name='testGP',
            devices=[mp3_test_device],
            tag=[Tag.POSITIVE],
            tests=seleteTest(arguments)
        )

        test_groups = [
            testGP,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()
    parser.add_argument('--clear', action='store_true', help='Remove dsp')
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
    tc_runner = ArdMp3TestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
