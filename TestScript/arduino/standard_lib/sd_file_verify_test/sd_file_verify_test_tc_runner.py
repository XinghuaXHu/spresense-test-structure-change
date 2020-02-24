#! /usr/bin/env python3
import sys
import os
import subprocess
import time


SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.sd_file_verify_test.sd_file_verify_test_device import SdFileVerifyTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['zmodem']
SD_PATH = '/mnt/sd0/'
SD_FILES = ['test', 'T1st1', 'T2st2', 'Dummy', 'Dummy1', 'Dummy2', 'Dummy3', 'Dummy4', 'Sunny1', 'Sunny2']
IN_FILES_STR = ['He2\n4h Q',
                '6G n\n',
                '3 ffG',
                't3Es \n 8En',
                't3Es \n 8End\n0 $0R4dG&\na PHe\n4 #\x00fn54',
                't34g()\n8En',
                ' AEs #g8En1\n',
                't3Es ak47kill',
                '4R s()\nn#$nm',
                '4R s()\nntrash',
               ]

SD_READ_SIZE = 128

# noinspection PyUnusedLocal
def rz_receive_file(test, source, data):
    cwd = os.getcwd()
    time.sleep(5)
    os.chdir(cwd + '/output')

    transfer_completed = False
    for fname in SD_FILES:
        if os.path.exists(fname):
            os.remove(fname)
        command = '{} {} {}'.format('rz', '<' + source.serial, '>' + source.serial)
        test.log.info('sz {}{}'.format(SD_PATH, fname))
        test.log.info(command)
        loop_count = 0
        while True:
            pr = subprocess.Popen(command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
            source.write('sz {}{}'.format(SD_PATH, fname))
            (out, error) = pr.communicate()
            loop_count += 1
            if 'Transfer complete' not in error.decode() and loop_count < 4:
                test.log.debug('rz not completed. retry.')
                source.write("reboot")
                time.sleep(5)
                source.write("cd /mnt/sd0")
                time.sleep(1)
                continue
            else:
                break

        file_name = 'unknown'

        if error:
            error = error.decode()
            for line in error.split('\r'):
                if 'Receiving:' in line:
                    file_name = line.split(':')[1].strip()
                if 'Transfer complete' in line:
                    transfer_completed = True
                if 'rz: not found' in line:
                    test.set_fail('The program \'rz\' is currently not installed. '
                                  'Install lrzsz package.')
                if line.strip(' \t\n\r') != '':
                    test.log.debug('[HOST PC] {}'.format(line.strip()))

        if not transfer_completed:
            os.chdir(cwd)
            test.set_fail('Failed to transfer file')
            return False

    test.add_user_event(source, 'Files successfully transferred'.format(file_name))
    os.chdir(cwd)
    return True

def verify_file(test, source, data):
    cwd = os.getcwd()
    os.chdir(cwd + '/output')

    for fname, infile in zip(SD_FILES, IN_FILES_STR):
        with open(fname, 'r') as f:
            buf = f.read(SD_READ_SIZE)
            test.log.debug('Name   : ' + fname)
            test.log.debug('In file: [' + repr(buf) + ']')
            test.log.debug('Expect : [' + repr(infile) + ']')
            if buf != infile:
                test.set_fail('File {} verify failed'.format(fname))
                os.chdir(cwd)
                return False
    test.add_user_event(source, 'Verify succeeded')
    return True

# noinspection PyUnusedLocal
def execute_command(test, source, data, command, confirm):
    source.write(command)
    if confirm:
        test.add_user_event(source, 'Command {} executed'.format(command))
    return True

class SdFileVerifyTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(SdFileVerifyTestTcRunner, self).__init__(conf)

        self.sd_file_verify_test_device = SdFileVerifyTestDevice(
            kwargs.get('dut_device', None), 'SD_FILE_VERIFY_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/zmodem-defconfig')

    def get_devices_list(self):
        return [self.sd_file_verify_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.sd_file_verify_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.sd_file_verify_test_device)

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
        timeout = 90

        sd_file_verify_test_device = self.get_devices_list()[0]

        setup = [
        ]

        teardown = [
        ]

        ADN_Sdcard_15643_02 = Test(
            name='ADN_Sdcard_15643_02',
            timeout=timeout,
            setup=setup,
            test=[
            Step(Action.WAIT_FOR, sd_file_verify_test_device, sd_file_verify_test_device.NSH_PROMPT,
                 lambda test, source, data: rz_receive_file(test, source, data)),
            Step(Action.WAIT_FOR, sd_file_verify_test_device, 'successfully transferred',
                 lambda test, source, data: verify_file(test, source, data)),
            ],
            teardown=teardown
        )

        sd_file_verify = TestGroup(
            name='sd_file_verify',
            devices=[sd_file_verify_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ADN_Sdcard_15643_02,
            ]
        )

        test_groups = [
            sd_file_verify,
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
    tc_runner = SdFileVerifyTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
