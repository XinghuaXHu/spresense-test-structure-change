#! /usr/bin/env python3
import sys
import os
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.gnss15434_arduino_example_test.gnss15434_arduino_example_test_device import Gnss15434TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

from datetime import datetime
import time
now = time.time()
utc = datetime.utcfromtimestamp(now)


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['gnss']
pos_count = 0

def call_shell(test, source, data, command):
    test.log.info("start")
    os.system(command)
    test.log.info("end")

def dut_gnss_monitor(test, source, data):
    global pos_count
    if 'No-Fix' in data:
        pos_count = pos_count
    elif 'Fix' in data:
        split_str = ",|="
        dataArray = re.split(split_str, data)
        lat = dataArray[4][0:2]
        lng = dataArray[6][0:3]
        if int(lat) == 35 and int(lng) == 139:
            pos_count = pos_count + 1
        else:
            test.set_fail(' '.join(['error']))
        if pos_count > 100:
            test.add_user_event(source, 'gnss test OK')


class GnssTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(GnssTestTcRunner, self).__init__(conf)

        self.gnss15434_test_device = Gnss15434TestDevice(
            kwargs.get('dut_device', None), 'GNSS15434_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/gnss-defconfig')

    def get_devices_list(self):
        return [self.gnss15434_test_device]

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.gnss15434_test_device)

        return dict(dut_bin=dut_bin)

    @staticmethod
    def __build_binary(toolbox, defconfig, device):
        bin_file = toolbox.builder.build_project(PROJECT, APPS_TO_BUILTIN, device, defconfig)
        return os.path.normpath(bin_file)

    # noinspection PyMethodMayBeStatic
    def flash_n_check(self, device, dut_bin, log=None):
        device.check_device_config(str(device), APPS_TO_BUILTIN, log)

        if os.path.basename(dut_bin).startswith(str(device)):
            try:
                os.remove(dut_bin)
            except OSError as e:
                log.error(e)
                raise

    def generate_test_groups(self, arguments, log=None):
        timeout = 600

        gnss15434_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.EXECUTE, gnss15434_test_device, None,
                lambda test, source, data: call_shell(test, source, data, "./labsat_start.sh")),
        ]

        teardown = [
            Step(Action.EXECUTE, gnss15434_test_device, None,
                lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
            Step(Action.EXECUTE, gnss15434_test_device, 'dut_gnss_mon', Test.remove_monitor),
        ]

        SDK_Gnss_15434 = Test(
            name='SDK_Gnss_15434',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, gnss15434_test_device, 'mode = HOT_START'),
                Step(Action.EXECUTE, gnss15434_test_device, ('dut_gnss_mon', dut_gnss_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, gnss15434_test_device, "gnss test OK"),
            ],
            teardown=teardown
        )

        gnss = TestGroup(
            name='gnss',
            devices=[gnss15434_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Gnss_15434,
            ]
        )

        test_groups = [
            gnss,
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
    tc_runner = GnssTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
