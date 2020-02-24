#! /usr/bin/env python3
import sys
import os
import time
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.gnss_extension_test.gnsstest_test_device import GnssTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser
from datetime import datetime

PROJECT = 'spresense'

TEST_DEVICE_APP_NAME = 'gnss_extension'
TIMEOUT_GNSS = 500 
SUCCESS = 'SUCCESS'
FAILED = 'FAILED'
global _global_dict
_global_dict = {}

NEGATIVE = True
APPS_TO_BUILTIN = ['arduino_gnss_extension']

def call_shell(test, source, data, command):
    os.system(command)

def set_time(test, source, data):
    source.write('setTime --year 2015 --month 10 --day 2 --hour ' + _global_dict['gnss_hour'] + ' --minute ' + _global_dict['gnss_minute'] + ' --sec ' + _global_dict['gnss_second'] + ' --usec ' + _global_dict['gnss_use']),
    _global_dict.pop('gnss_hour')
    _global_dict.pop('gnss_minute')
    _global_dict.pop('gnss_second')
    _global_dict.pop('gnss_use')

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
    elif 'print_pos 2015' in data:
            split_str = " |-|_|:|[|]"
            dataArray = re.split(split_str, data)
            _global_dict.clear()
            _global_dict['gnss_hour'] = dataArray[5]
            _global_dict['gnss_minute'] = dataArray[6]
            _global_dict['gnss_second'] = dataArray[7]
            _global_dict['gnss_use'] = dataArray[8]
    elif '=' in data:
            test.log.info(data)

class TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super().__init__(conf)

        self.gnss_test_device = GnssTestDevice(
            kwargs.get('dut_device', None), 'GNSS_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(self.config.projects[0].path, 'sdk/configs/examples/gnss_extension-defconfig')

    def get_devices_list(self):
        return [self.gnss_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.gnss_test_device)

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
        timeout = 10

        gnss_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WRITE, gnss_test_device, 'begin'),
            #Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
            Step(Action.EXECUTE, gnss_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.WRITE, gnss_test_device, 'end'),
            #Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
            Step(Action.EXECUTE, gnss_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        gnssOffline_tc_testing = Test(
            name='gnssOffline_tc_testing',
            timeout=TIMEOUT_GNSS,
            setup=setup,
            test=[
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: time.sleep(10)),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_start.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                     lambda test, source, data: time.sleep(10)),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --cycle 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --cold'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 90 --value 0.01'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),

                Step(Action.EXECUTE, gnss_test_device, None, lambda test, source, data: time.sleep(10)),
                Step(Action.WRITE, gnss_test_device, 'checkOffline --time 90 --value 0.00'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),

                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_start.sh")),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 90 --value 0.01'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),

                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
            ],
            teardown=teardown
        )

        gnssOffline_tc_testing1 = gnssOffline_tc_testing.clone()
        gnssOffline_tc_testing1.name = 'gnssOffline_tc_testing1'
        gnssOffline_tc_testing2 = gnssOffline_tc_testing.clone()
        gnssOffline_tc_testing2.name = 'gnssOffline_tc_testing2'
        gnssOffline_tc_testing3 = gnssOffline_tc_testing.clone()
        gnssOffline_tc_testing3.name = 'gnssOffline_tc_testing3'
        gnssOffline_tc_testing4 = gnssOffline_tc_testing.clone()
        gnssOffline_tc_testing4.name = 'gnssOffline_tc_testing4'
        gnssOffline_tc_testing5 = gnssOffline_tc_testing.clone()
        gnssOffline_tc_testing5.name = 'gnssOffline_tc_testing5'
        gnssOffline_tc_testing6 = gnssOffline_tc_testing.clone()
        gnssOffline_tc_testing6.name = 'gnssOffline_tc_testing6'
        gnssOffline_tc_testing7 = gnssOffline_tc_testing.clone()
        gnssOffline_tc_testing7.name = 'gnssOffline_tc_testing7'
        gnssOffline_tc_testing8 = gnssOffline_tc_testing.clone()
        gnssOffline_tc_testing8.name = 'gnssOffline_tc_testing8'
        gnssOffline_tc_testing9 = gnssOffline_tc_testing.clone()
        gnssOffline_tc_testing9.name = 'gnssOffline_tc_testing9'

        gnss_testing_group = TestGroup(
            name='gnss_testing_group',
            devices=[gnss_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                gnssOffline_tc_testing,
                gnssOffline_tc_testing1,
                gnssOffline_tc_testing2,
                gnssOffline_tc_testing3,
                gnssOffline_tc_testing4,
                gnssOffline_tc_testing5,
                gnssOffline_tc_testing6,
                gnssOffline_tc_testing7,
                gnssOffline_tc_testing8,
                gnssOffline_tc_testing9,
            ]
        )

        test_groups = [
            gnss_testing_group,
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
    tc_runner = TestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
