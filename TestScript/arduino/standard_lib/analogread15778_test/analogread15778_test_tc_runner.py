#! /usr/bin/env python3
import sys
import os
import re
import subprocess
import time

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)


from api.device_manager import DeviceManager
from tc.analogread15778_test.analogread15778_test_device import AnalogReadTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['analogread']
value_count = 0

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

def call_shell(test, source, data, command):
    global value_count
    value_count = 0
    test.log.info("start")
    p = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
    p.communicate()
    p.wait()
    test.log.info("end")
    time.sleep(5)
    test.add_user_event(source, 'wait sec completed')

def dut_analogread_monitor_0V(test, source, data):
    global value_count
    if 'voltage' in data and 'A2' in data or 'voltage' in data and 'A3' in data:
        read_value = int(data.split("=")[1].strip(" "))
        if 0 <= read_value <= 20:
            value_count += 1
            test.log.debug('{} 0V ok'.format(data[0:2]))
        else:
            test.set_fail(' '.join(['Out of range. data = ', data]))
    if value_count > 100:
        test.add_user_event(source, 'Test Done')

def dut_analogread_monitor_0p9V(test, source, data):
    global value_count
    if 'voltage' in data and 'A2' in data or 'voltage' in data and 'A3' in data:
        read_value = int(data.split("=")[1].strip(" "))
        if 511 <= read_value <= 513:
            test.log.debug('{} 0.0V ok'.format(data[0:2]))
        else:
            test.set_fail(' '.join(['Out of range. data = ', data]))
    if value_count > 100:
        test.add_user_event(source, 'Test Done')

def dut_analogread_monitor_1p8V(test, source, data):
    global value_count
    if 'voltage' in data and 'A2' in data or 'voltage' in data and 'A3' in data:
        read_value = int(data.split("=")[1].strip(" "))
        if 1023 == read_value:
            test.log.debug('{} 1.8V ok'.format(data[0:2]))
        else:
            test.set_fail(' '.join(['Out of range. data = ', data]))
    if value_count > 100:
        test.add_user_event(source, 'Test Done')

class AnalogReadTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(AnalogReadTestTcRunner, self).__init__(conf)

        self.analogread15778_test_device = AnalogReadTestDevice(
            kwargs.get('dut_device', None), 'ANALOGREAD_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.getcwd(), 'defconfig/analogread-defconfig')

    def get_devices_list(self):
        return [self.analogread15778_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.analogread15778_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.analogread15778_test_device)

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

        analogread15778_test_device = self.get_devices_list()[0]

        setup = []

        teardown = [
            Step(Action.EXECUTE, analogread15778_test_device, None,
                 lambda test, source, data: call_shell(test, source, data, "./kikusui_0V.sh")),
        ]

        ADN_Analog_io_15778 = Test(
            name='ADN_Analog_io_15778',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, analogread15778_test_device, None,
                     lambda test, source, data: call_shell(test, source, data, "./kikusui_0V.sh")),
                Step(Action.WAIT_FOR, analogread15778_test_device, 'wait sec completed'),
                Step(Action.EXECUTE, analogread15778_test_device,
                     ('dut_analogread_monitor0', dut_analogread_monitor_0V), Test.add_monitor),
                Step(Action.WAIT_FOR, analogread15778_test_device, 'Test Done'),
                Step(Action.EXECUTE, analogread15778_test_device, 'dut_analogread_monitor0', Test.remove_monitor),

                Step(Action.EXECUTE, analogread15778_test_device, None,
                     lambda test, source, data: call_shell(test, source, data, "./kikusui_0.9V.sh")),
                Step(Action.WAIT_FOR, analogread15778_test_device, 'wait sec completed'),
                Step(Action.EXECUTE, analogread15778_test_device,
                     ('dut_analogread_monitor0p9', dut_analogread_monitor_0p9V), Test.add_monitor),
                Step(Action.WAIT_FOR, analogread15778_test_device, 'Test Done'),
                Step(Action.EXECUTE, analogread15778_test_device, 'dut_analogread_monitor0p9', Test.remove_monitor),

                Step(Action.EXECUTE, analogread15778_test_device, None,
                     lambda test, source, data: call_shell(test, source, data, "./kikusui_1.8V.sh")),
                Step(Action.WAIT_FOR, analogread15778_test_device, 'wait sec completed'),
                Step(Action.EXECUTE, analogread15778_test_device,
                     ('dut_analogread_monitor1p8', dut_analogread_monitor_1p8V), Test.add_monitor),
                Step(Action.WAIT_FOR, analogread15778_test_device, 'Test Done'),
                Step(Action.EXECUTE, analogread15778_test_device, 'dut_analogread_monitor1p8', Test.remove_monitor),
            ],
            teardown=teardown
        )

        analogread = TestGroup(
            name='analogread',
            devices=[analogread15778_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ADN_Analog_io_15778,
            ]
        )

        test_groups = [
            analogread,
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
    tc_runner = AnalogReadTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
