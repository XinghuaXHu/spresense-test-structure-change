#! /usr/bin/env python3
import sys
import os
import time
import subprocess
import io

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))
MSC_DEV = '/dev/sdb1'
MOUNT_POINT = '/mnt/autotest_sd_util'
SD_PATH = '/mnt/sd0/'

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.sd_util.sd_util_device import SdUtilDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['sd_util']


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

def remove_monitor(test, source, data):
    if isinstance(data, str):
        return

    ls = data.get_value(SdUtilDevice.KEY_LS)
    if ls is None:
        return

    path = ls['path']

    for file in ls['files']:
        source.write('rm \"{}{}\"'.format(path, file))

    if path != SD_PATH:
        source.write('rmdir \"{}\"'.format(path[:-1]))
        source.write('ls {}'.format(SD_PATH))
    else:
        if len(ls['dirs']) > 0:
            source.write('ls \"{}{}\"'.format(path, ls['dirs'][0]))
        else:
            test.add_user_event(source, 'Operation completed')

def cmd_monitor(test, source, data):
    if isinstance(data, str):
        return

    failed = data.get_value(SdUtilDevice.KEY_EXECUTION_FAILED)
    if failed is not None:
        test.set_fail('{} {}'.format(SdUtilDevice.KEY_EXECUTION_FAILED, failed))
        return

def wait_sec(test, source, data, sec):
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')

def do_operation(test, source, data, arguments):
    if arguments.src is None:
        test.set_fail('src is None')
        return

    time.sleep(5)
    cmds = [
        'sudo mkdir -p {}'.format(MOUNT_POINT),
        'sudo sync',
        'sudo mount {} {}'.format(MSC_DEV, MOUNT_POINT),
        'sudo sync',
        'sudo rm -rf {}/*'.format(MOUNT_POINT),
        'sudo sync',
        'sudo cp -r {}/* {}/'.format(arguments.src, MOUNT_POINT),
        'sudo sync',
        'sudo umount {}'.format(MOUNT_POINT),
        'sudo sync',
        'sudo rm -rf {}'.format(MOUNT_POINT),
        'sudo sync',
    ]
    for cmd in cmds:
        test.log.debug(cmd)
        subprocess.run(cmd, shell=True)
        time.sleep(3)

def sz_send_file(test, source, data, path):
    test.log.debug('[HOST PC] {}{}'.format('path = ', path))
    command = '{} {} {} {}'.format('sz', path, '<' + source.serial, '>' + source.serial)

    while True:
        pr = subprocess.Popen(command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE, bufsize=io.DEFAULT_BUFFER_SIZE * 2)
        (out, error) = pr.communicate()
        if 'Transfer complete' not in error.decode():
            test.log.debug('sz not completed. retry.')
            continue
        else:
            break

    if error:
        error = error.decode()
        for line in error.split('\r'):
            if line.strip(' \t\n\r') != '':
                test.log.debug('[HOST PC] {}'.format(line.strip()))

    # Wait for target
    time.sleep(1)

def do_copy(test, source, data, arguments):
    if not os.path.isdir(arguments.copy):
        test.set_fail('Directory \"{}\" is not exist'.format(arguments.copy))

    path = arguments.copy
    cwd = os.getcwd()
    os.chdir(path)
    for name in os.listdir(path):
        path_and_name = os.path.join(path, name)
        test.log.info(path_and_name)
        if os.path.isfile(path_and_name):
            sz_send_file(test, source, data, path_and_name)
        if os.path.isdir(path_and_name):
            source.write('mkdir {}'.format(name))
            time.sleep(1)
    os.chdir(cwd)
    test.add_user_event(source, 'Operation completed')

class SdUtilTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(SdUtilTcRunner, self).__init__(conf)

        self.sd_util_device = SdUtilDevice(
            kwargs.get('dut_device', None), 'SD_UTIL_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.getcwd(), 'defconfig/sd_util-defconfig')

    def get_devices_list(self):
        return [self.sd_util_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.sd_util_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.sd_util_device)

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
        device.check_device_config(str(device), [ 'msconn', 'msdis' ], log)

        if os.path.basename(dut_bin).startswith(str(device)):
            try:
                os.remove(dut_bin)
            except OSError as e:
                log.error(e)
                raise

    def generate_test_groups(self, arguments, log=None):
        timeout = 120

        sd_util_device = self.get_devices_list()[0]

        def setup_steps(device, arguments):
            return [
                Step(Action.EXECUTE, device, ('cmd_mon', cmd_monitor), Test.add_monitor),
                Step(Action.WAIT_FOR, sd_util_device, device.NSH_PROMPT),
                Step(Action.WRITE, sd_util_device, 'reboot'),
                Step(Action.WAIT_FOR, sd_util_device, device.NUTTSHELL),
                Step(Action.EXECUTE, sd_util_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 2)),
                Step(Action.WRITE, sd_util_device, 'cd /mnt/sd0'),
                Step(Action.WAIT_FOR, sd_util_device, 'cd /mnt/sd0'),
                Step(Action.WAIT_FOR, sd_util_device, device.NSH_PROMPT),
            ]

        def clear_steps(device, arguments):
            return [
                Step(Action.EXECUTE, device, ('rm_mon', remove_monitor), Test.add_monitor),
                Step(Action.WRITE, device, 'ls'),
                Step(Action.WAIT_FOR, device, 'Operation completed'),
            ]

        def copy_steps(device, arguments):
            return [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: do_copy(test, source, data, arguments)),
                Step(Action.WAIT_FOR, device, 'Operation completed'),
            ]

        def test_steps(device, arguments):
            if arguments.clear == True:
                return clear_steps(device, arguments)
            elif arguments.copy is not None:
                return copy_steps(device, arguments)
            else:
                return [
                    Step(Action.EXECUTE, device, None,
                         lambda test, source, data: test.set_fail('Invalid argument')),
                ]

        def teardown_steps(device, arguments):
            r = [
                Step(Action.EXECUTE, device, 'cmd_mon', Test.remove_monitor),
            ]
            if arguments.clear == True:
                r = r + [
                    Step(Action.EXECUTE, device, 'rm_mon', Test.remove_monitor),
                ]
            return r

        def test_name(device, arguments):
            if arguments.clear == True:
                return 'SDK_SdUtil_clear'
            elif arguments.copy is not None:
                return 'SDK_SdUtil_copy'
            else:
                return 'SDK_SdUtil_invalid'

        sd_util_test = Test(
            name=test_name(sd_util_device, arguments),
            timeout=timeout,
            setup=setup_steps(sd_util_device, arguments),
            test=test_steps(sd_util_device, arguments),
            teardown=teardown_steps(sd_util_device, arguments),
        )

        sd_util_group = TestGroup(
            name='sd_util_group',
            devices=[sd_util_device],
            tag=[Tag.POSITIVE],
            tests=[
                sd_util_test,
            ]
        )

        test_groups = [
            sd_util_group,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()

    parser.add_argument('--clear', action='store_true', help='Remove all')
    parser.add_argument('--copy', help='Copy to SD')
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
    tc_runner = SdUtilTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
