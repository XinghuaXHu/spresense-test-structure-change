#! /usr/bin/env python3
import sys
import os
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.spiloopback16298_arduino_example_test.spiloopback16298_arduino_example_test_device import SpiLoopback16298TestDevice
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
APPS_TO_BUILTIN = ['spi']
pos_count = 0



class SpiLoopbackTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(SpiLoopbackTestTcRunner, self).__init__(conf)

        self.spiloopback16298_test_device = SpiLoopback16298TestDevice(
            kwargs.get('dut_device', None), 'SpiLoopBack6298_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/spiloopback-defconfig')

    def get_devices_list(self):
        return [self.spiloopback16298_test_device]

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.spiloopback16298_test_device)

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
        timeout = 10

        spiloopback16298_test_device = self.get_devices_list()[0]

        setup = [
        ]

        teardown = [
        ]

        Arduino_SpiLoopback_16298 = Test(
            name='Arduino_SpiLoopback_16298',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, spiloopback16298_test_device, 'SPI loopback test'),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, '8 bit loopback test passed'),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, '16 bit loopback test passed'),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "0	0	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "1	1	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "2	2	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "3	3	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "4	4	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "5	5	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "6	6	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "7	7	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "8	8	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "9	9	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "10	10	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "11	11	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "12	12	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "13	13	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "14	14	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "15	15	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "16	16	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "17	17	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "18	18	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "19	19	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "20	20	pass"),
                Step(Action.WAIT_FOR, spiloopback16298_test_device, "21	21	pass"),
            ],
            teardown=teardown
        )

        spiloopback = TestGroup(
            name='spiloopback',
            devices=[spiloopback16298_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                Arduino_SpiLoopback_16298,
            ]
        )

        test_groups = [
            spiloopback,
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
    tc_runner = SpiLoopbackTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
