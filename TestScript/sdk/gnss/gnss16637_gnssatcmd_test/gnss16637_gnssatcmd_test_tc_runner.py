#! /usr/bin/env python3
import sys
import os
import re
import time
import serial

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.gnss16637_gnssatcmd_test.gnss16637_gnssatcmd_test_device import Gnss16637TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['gnss_atcmd']

def wait_sec(test, source, data, sec, description):
    if description is not None:
        test.log.info("")
        test.log.info("#############################################")
        test.log.info("## [" + str(sec) + " sec] "+ description)
        test.log.info("#############################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    global pos_count
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

def call_shell(test, source, data, command):
    test.log.info("start")
    os.system(command)
    test.log.info("end")

class GnssTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(GnssTestTcRunner, self).__init__(conf)

        self.gnss16637_test2_device = Gnss16637TestDevice(
            kwargs.get('dut2_device', None), 'GNSS16637_TEST2_DEVICE') if kwargs.get('dut2_device', None)\
            else None
        self.gnss16637_test_device = Gnss16637TestDevice(
            kwargs.get('dut_device', None), 'GNSS16637_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')


        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/gnss_atcmd-defconfig')

    def get_devices_list(self):
        return [self.gnss16637_test2_device, self.gnss16637_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.gnss16637_test2_device, binaries['dut_bin'], log)
        usb0 = serial.Serial(self.gnss16637_test2_device.serial, timeout=1, baudrate=115200)
        time.sleep(1)
        data = "reboot"
        usb0.write(data.encode('utf-8') + b'\r\n')
        time.sleep(5)
        data = "gnss_atcmd"
        usb0.write(data.encode('utf-8') + b'\r\n')
        time.sleep(5)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        if 'Successfully registered the serial driver' in data:
            log.info('open main tty OK')
            time.sleep(20)
        else:
            log.info('open main tty NG')

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.gnss16637_test2_device)

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

        gnss16637_test2_device, gnss16637_test_device = self.get_devices_list()

        setup = [
            Step(Action.EXECUTE, gnss16637_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
            Step(Action.EXECUTE, gnss16637_test_device, None,
                lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
            Step(Action.EXECUTE, gnss16637_test_device, None,
                lambda test, source, data: wait_sec(test, source, data, 5, "waiting")),
            Step(Action.EXECUTE, gnss16637_test_device, None,
                lambda test, source, data: call_shell(test, source, data, "./labsat_start.sh")),
        ]

        teardown = [
            Step(Action.EXECUTE, gnss16637_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]


        SDK_Gnss_16637 = Test(
            name='SDK_Gnss_16637',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, gnss16637_test_device, '@GNS 0x29'),
                Step(Action.WAIT_FOR, gnss16637_test_device, 'Done'),
                Step(Action.WRITE, gnss16637_test_device, '@BSSL 0x40ef'),
                Step(Action.WAIT_FOR, gnss16637_test_device, 'Done'),
                Step(Action.WRITE, gnss16637_test_device, '@GCD'),
                Step(Action.WAIT_FOR, gnss16637_test_device, 'Done'),
                Step(Action.WAIT_FOR, gnss16637_test_device, 'QZQSM'),
                Step(Action.WAIT_FOR, gnss16637_test_device, 'QZQSM'),
                Step(Action.WAIT_FOR, gnss16637_test_device, 'QZQSM'),
                Step(Action.WRITE, gnss16637_test_device, '@GSTP'),
                Step(Action.WAIT_FOR, gnss16637_test_device, 'Done'),
            ],
            teardown=teardown
        )

        gnss = TestGroup(
            name='gnss',
            devices=[gnss16637_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Gnss_16637,
            ]
        )

        test_groups = [
            gnss,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()

    parser.add_argument('--master', metavar='SERIAL_PORT', help='Set master')
    parser.add_argument('--slave', metavar='SERIAL_PORT', help='Set slave')

    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    dut2_device, dut_device = dev_manager.get_devices_by_serials(args.master, args.slave)

    # Create test runner instance
    tc_runner = GnssTestTcRunner(config, dut2_device=dut2_device, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)