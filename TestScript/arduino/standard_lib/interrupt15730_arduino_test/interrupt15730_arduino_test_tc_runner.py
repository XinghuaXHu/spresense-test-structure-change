#! /usr/bin/env python3
import sys
import os
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.interrupt15730_arduino_test.interrupt15730_arduino_test_device import Interrupt15730TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser
import datetime
import time


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['Interrupt']
LTE_PINS = [2,3,5,6,7,9]
EXT_PINS = [0,1,2,3,4,5,6,7]

def check_step(test, source, data, value):
    test.log.info(value)

class InterruptTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(InterruptTestTcRunner, self).__init__(conf)

        self.interrupt15730_test_device = Interrupt15730TestDevice(
            kwargs.get('dut_device', None), 'INTERRUPT15730_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/interrupt-defconfig')

    def get_devices_list(self):
        return [self.interrupt15730_test_device]

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.interrupt15730_test_device)

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
        timeout = 150

        interrupt15730_test_device = self.get_devices_list()[0]

        def verify_pin(device):
            step = []
            if 'lte' == device.ext_type:
                PINS = LTE_PINS
            else:
                PINS = EXT_PINS

            step += [
                Step(Action.WAIT_FOR, device, 'Please Input Test PIN Count:'),
                Step(Action.WRITE, device, str(len(PINS)), None, '\r'),
            ]

            for i_pin in range(len(PINS)):
                if 0 == i_pin % 2:
                    o_pin = i_pin + 1
                else:
                    o_pin = i_pin - 1

                step += [
                    Step(Action.WAIT_FOR, device, 'Please Input Test PIN ID:'),
                    Step(Action.WRITE, device, str(PINS[i_pin]), None, '\r'),
                    Step(Action.WAIT_FOR, device, 'Please Input Out PIN ID:'),
                    Step(Action.WRITE, device, str(PINS[o_pin]), None, '\r'),                
                    Step(Action.WAIT_FOR, device, '------Start Test pin[' + str(PINS[i_pin]) + '] Mode FALLING------'),
                    Step(Action.WAIT_FOR, device, 'to LOW'),
                    Step(Action.WAIT_FOR, device, 'Interrupt occur! Pin[' + str(PINS[i_pin]) + '], count 1'),
                    Step(Action.WAIT_FOR, device, 'to HIGH'),
                    Step(Action.WAIT_FOR, device, 'to LOW'),
                    Step(Action.WAIT_FOR, device, 'to HIGH'),
                    Step(Action.WAIT_FOR, device, 'Test pin[' + str(PINS[i_pin]) + '] = OK'),
                ]
            
            step += [
                Step(Action.WAIT_FOR, device, 'Test Done'),
            ]

            return step

        setup = [
        ]

        teardown = [
        ]

        Arduino_Interrupt_15730 = Test(
            name='Arduino_Interrupt_15730',
            timeout=timeout,
            setup=setup,
            test=verify_pin(interrupt15730_test_device),
            teardown=teardown
        )

        interrupt_gp = TestGroup(
            name='interrupt_gp',
            devices=[interrupt15730_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                Arduino_Interrupt_15730,
            ]
        )

        test_groups = [
            interrupt_gp,
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
    tc_runner = InterruptTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
