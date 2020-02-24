#! /usr/bin/env python3
import sys
import os
import time
import pexpect
import subprocess

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.mbedtls.mbedtls_device import MbedtlsDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_NAME = ['mbedtls', 'rz', 'sz']
APPS_TO_BUILTIN = ['examples/lte_tls', 'device/sdcard', 'mbedtls']
CERT_FILES = [
    'addtrustexternalcaroot.crt',
    'DigiCertGlobalRootCA.crt',
    'DigiCertGlobalRootG2.crt',
    'DigiCertHighAssuaranceEVRootCA.crt',
]

# noinspection PyUnusedLocal
def error_monitor(test, source, data):
    if '[ERR]' in data:
        test.set_fail('error detected -> {}'.format(data))

def wait_sec(test, source, data, sec):
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')

def sz_send_cert_file(test, source, data):
    cwd = os.getcwd()
    os.chdir(cwd + '/cert')

    for file_name in CERT_FILES:
        command = '{} {} {} {}'.format('sz', file_name, '<' + source.serial, '>' + source.serial)
        source.write('rz')
        while True:
            pr = subprocess.Popen(command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
            (out, error) = pr.communicate()

            if 'Transfer complete' not in error.decode():
                test.log.debug('sz not completed. retry.')
                continue
            else:
                break
        error = error.decode()
        for line in error.split('\r'):
            if line.strip(' \t\n\r') != '':
                test.log.debug('[HOST PC] {}'.format(line.strip()))

    test.add_user_event(source, 'sz_send_cert_file end')
    os.chdir(cwd)
    return True

class MbedtlsTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(MbedtlsTcRunner, self).__init__(conf)

        self.mbedtls_device = MbedtlsDevice(
            kwargs.get('dut_device', None), 'MBEDTLS_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/mbedtls-defconfig')

    def get_devices_list(self):
        return [self.mbedtls_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.mbedtls_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.mbedtls_device)

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
        device.check_device_config(str(device), APPS_NAME, log)

        if os.path.basename(dut_bin).startswith(str(device)):
            try:
                os.remove(dut_bin)
            except OSError as e:
                log.error(e)
                raise

    def generate_test_groups(self, arguments, log=None):

        mbedtls_device = self.get_devices_list()[0]

        def wait_sec_steps(device, sec):
            return [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: wait_sec(test, source, data, sec)),
                Step(Action.WAIT_FOR, device, 'wait sec completed'),
            ]

        setup = wait_sec_steps(mbedtls_device, 3) + [
            Step(Action.WAIT_FOR, mbedtls_device, mbedtls_device.NSH_PROMPT),
            Step(Action.EXECUTE, mbedtls_device, ('err_mon', error_monitor), Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, mbedtls_device, 'err_mon', Test.remove_monitor),
        ]

        def mbedtls_steps(cmd_id):
            return [
                Step(Action.WRITE, mbedtls_device, 'mbedtls {}'.format(cmd_id)),
                Step(Action.WAIT_FOR, mbedtls_device, 'test {} start'.format(cmd_id)),
                Step(Action.WAIT_FOR, mbedtls_device, 'test {} end'.format(cmd_id)),
            ]

        def mbedtls_test(test_id, cmd_id, t_o):
            return Test(
                name='SDK_Mbedtls_{}'.format(test_id),
                timeout=t_o,
                setup=setup,
                test=mbedtls_steps(cmd_id),
                teardown=teardown
            )

        def mbedtls_group(test_id, cmd_id, t_o=90):
            return TestGroup(
                name='spresense-{}'.format(test_id),
                devices=[mbedtls_device],
                tag=[cmd_id],
                tests=[ mbedtls_test(test_id, cmd_id, t_o) ]
        )

        def mbedtls_prepare_steps():
            return wait_sec_steps(mbedtls_device, 3) + [
                Step(Action.WRITE, mbedtls_device, 'cd /mnt/sd0'),
                Step(Action.WAIT_FOR, mbedtls_device, mbedtls_device.NSH_PROMPT,
                     lambda test, source, data: sz_send_cert_file(test, source, data)),
                Step(Action.WAIT_FOR, mbedtls_device, 'sz_send_cert_file end'),
            ]

        def mbedtls_prepare_test():
            return Test(
                name='SDK_Mbedtls_prepare',
                timeout=60,
                setup=setup,
                test=mbedtls_prepare_steps(),
                teardown=teardown
            )

        def mbedtls_prepare_group():
            return TestGroup(
                name='prepare',
                devices=[mbedtls_device],
                tag=['prepare'],
                tests=[ mbedtls_prepare_test() ]
            )

        test_groups = [
            mbedtls_prepare_group(),
            mbedtls_group('16468', '08', 1200),
            mbedtls_group('16469', '09', 1200),
            mbedtls_group('16470', '70'),
            mbedtls_group('16471', '71'),
            mbedtls_group('16472', '72'),
            mbedtls_group('16473', '11'),
            mbedtls_group('16474', '12'),
            mbedtls_group('16475', '13'),
            mbedtls_group('16476', '14'),
            mbedtls_group('16477', '15'),
            mbedtls_group('16478', '16'),
            mbedtls_group('16479', '17'),
            mbedtls_group('16480', '18'),
            mbedtls_group('16481', '19'),
            mbedtls_group('16482', '20'),
            mbedtls_group('16483', '21'),
            mbedtls_group('16484', '22'),
            mbedtls_group('16485', '23'),
            mbedtls_group('16486', '24'),
            mbedtls_group('16487', '25'),
            mbedtls_group('16488', '26'),
            mbedtls_group('16489', '27'),
            mbedtls_group('16490', '28'),
            mbedtls_group('16491', '99'),
            mbedtls_group('16492', '29'),
            mbedtls_group('16493', '30'),
            mbedtls_group('16494', '31'),
            mbedtls_group('16495', '32'),
            mbedtls_group('16496', '33'),
            mbedtls_group('16497', '34'),
            mbedtls_group('16498', '35'),
            mbedtls_group('16499', '36'),
            mbedtls_group('16500', '37'),
            mbedtls_group('16501', '38'),
            mbedtls_group('16502', '39'),
            mbedtls_group('16503', '40'),
            mbedtls_group('16504', '41'),
            mbedtls_group('16505', '42'),
            mbedtls_group('16506', '43'),
            mbedtls_group('16507', '44'),
            mbedtls_group('16508', '45'),
            mbedtls_group('16509', '46'),
            mbedtls_group('16510', '47'),
            mbedtls_group('16468', '48'),
            mbedtls_group('16512', '98'),
            mbedtls_group('16513', '50'),
            mbedtls_group('16514', '51'),
            mbedtls_group('16515', '52'),
            mbedtls_group('16516', '53'),
            mbedtls_group('16517', '54'),
            mbedtls_group('16518', '55'),
            mbedtls_group('16519', '56'),
            mbedtls_group('16520', '57'),
            mbedtls_group('16521', '58'),
            mbedtls_group('16522', '60'),
            mbedtls_group('16523', '61'),
            mbedtls_group('16524', '62'),
            mbedtls_group('16525', '63'),
            mbedtls_group('16526', '64'),
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
    tc_runner = MbedtlsTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
