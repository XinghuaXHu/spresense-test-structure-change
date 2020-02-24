#! /usr/bin/env python3
import sys
import os
import re
import time

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.prime16616_test.prime16616_test_device import Prime16616TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser
t1 = 0
t2 = 0

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['prime']

def wait_sec(test, source, data, sec):
    test.log.info("")
    test.log.info("#############################################")
    test.log.info("## [" + str(sec) + " sec] ")
    test.log.info("#############################################")
    test.log.info("")
    time.sleep(sec)

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    global t1
    global t2
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
    elif 'Multicore (5CPUs) execution time' in data:
        split_str = "m|s"
        arr = re.split(split_str, data[34:])
        t1 = arr[0]
    elif 'Singlecore execution time' in data:
        split_str = "m|s"
        arr = re.split(split_str, data[28:])
        t2 = arr[0]
    elif 'Ratio' in data:
        split_str = "%"
        arr = re.split(split_str, data[11:])
        ratio = arr[0]
        if int(t2) > 10000 and int(t2)< 30000 and int(t1) < 10000 and int(t1) < int(t2) and int(ratio) > 100 and int(ratio) < 500:
            test.add_user_event(source, 'Prime Test Done')


class PrimeTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(PrimeTestTcRunner, self).__init__(conf)

        self.prime16616_test_device = Prime16616TestDevice(
            kwargs.get('dut_device', None), 'PRIME16616_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/prime-defconfig')
    def get_devices_list(self):
        return [self.prime16616_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.prime16616_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.prime16616_test_device)

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
        timeout = 120

        prime16616_test_device = self.get_devices_list()[0]


        setup = [
            Step(Action.WAIT_FOR, prime16616_test_device, prime16616_test_device.NSH_PROMPT),
            Step(Action.WRITE, prime16616_test_device, 'reboot'),
            Step(Action.WAIT_FOR, prime16616_test_device, prime16616_test_device.NUTTSHELL),
            Step(Action.EXECUTE, prime16616_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, prime16616_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        SDK_Prime_16616 = Test(
            name='SDK_Prime_16616',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, prime16616_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5)),
                Step(Action.WRITE, prime16616_test_device, 'prime'),
                Step(Action.WAIT_FOR, prime16616_test_device, 'Starting multicore'),
                Step(Action.WAIT_FOR, prime16616_test_device, '[0] Init task, CPU id'),
                Step(Action.WAIT_FOR, prime16616_test_device, '[1] Init task, CPU id'),
                Step(Action.WAIT_FOR, prime16616_test_device, '[2] Init task, CPU id'),
                Step(Action.WAIT_FOR, prime16616_test_device, '[3] Init task, CPU id'),
                Step(Action.WAIT_FOR, prime16616_test_device, '[4] Init task, CPU id'),
                Step(Action.WAIT_FOR, prime16616_test_device, 'Sending messages'),
                Step(Action.WAIT_FOR, prime16616_test_device, '[0] prime calc range start      0 lenght 15000'),
                Step(Action.WAIT_FOR, prime16616_test_device, '[1] prime calc range start  15000 lenght 15000'),
                Step(Action.WAIT_FOR, prime16616_test_device, '[2] prime calc range start  30000 lenght 15000'),
                Step(Action.WAIT_FOR, prime16616_test_device, '[3] prime calc range start  45000 lenght 15000'),
                Step(Action.WAIT_FOR, prime16616_test_device, '[4] prime calc range start  60000 lenght 15000'),
                Step(Action.WAIT_FOR, prime16616_test_device, 'Receiving response'),
                Step(Action.WAIT_FOR, prime16616_test_device, '[0] found '),
                Step(Action.WAIT_FOR, prime16616_test_device, '[1] found '),
                Step(Action.WAIT_FOR, prime16616_test_device, '[2] found '),
                Step(Action.WAIT_FOR, prime16616_test_device, '[3] found '),
                Step(Action.WAIT_FOR, prime16616_test_device, '[4] found '),
                Step(Action.WAIT_FOR, prime16616_test_device, 'Time consumed:'),
                Step(Action.WAIT_FOR, prime16616_test_device, 'Running same exercise on a single core'),
                Step(Action.WAIT_FOR, prime16616_test_device, 'Time consumed:'),
                Step(Action.WAIT_FOR, prime16616_test_device, 'Prime Test Done'),
            ],
            teardown=teardown
        )

        prime = TestGroup(
            name='prime',
            devices=[prime16616_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Prime_16616,
            ]
        )

        test_groups = [
            prime,
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
    tc_runner = PrimeTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
