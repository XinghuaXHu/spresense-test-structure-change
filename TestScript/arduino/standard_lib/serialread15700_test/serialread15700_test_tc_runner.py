#! /usr/bin/env python3
import sys
import os
import time

sys.path.append('../../')
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.serialread15700_test.serialread15700_test_device import Serialread15700TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['serialread15700']


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

def wait_sec(test, source, data, sec, description):
    if description is not None:
        test.log.info("")
        test.log.info("#############################################")
        test.log.info("## [" + str(sec) + " sec] "+ description)
        test.log.info("#############################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')



class Serialread15700TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(Serialread15700TestTcRunner, self).__init__(conf)

        self.serialread15700_test_device = Serialread15700TestDevice(
            kwargs.get('dut_device', None), 'SERIALREAD15700_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.readr_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(self.config.projects[0].path,
                                           'test/autotest/src/tc/serialread15700_test/defconfig/serialread-defconfig')

    def get_devices_list(self):
        return [self.serialread15700_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.serialread15700_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.serialread15700_test_device)

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

        serialread15700_test_device = self.get_devices_list()[0]

        setup = [
        ]

        teardown = [
        ]

        ADN_Serial_15700 = Test(
            name='ADN_Serial_15700',
            timeout=timeout,
            setup=setup,
            #serial_read_15700.ino in Serial.print
            # to Serial.println
            test=[
                Step(Action.WAIT_FOR, serialread15700_test_device, 'test start'),
                Step(Action.WRITE, serialread15700_test_device, 'a4 Gk$'),
                Step(Action.WAIT_FOR, serialread15700_test_device, 'readBytesUntil len: 3,buffer: a4 '),
                #Step(Action.WAIT_FOR, serialread15700_test_device, 'readBytesUntil len: 3,buffer: a4 '),
                #Step(Action.WAIT_FOR, serialread15700_test_device, 'readBytesUntil len: 6,buffer: a4 Gk$'),
                # use by testcase, do comment out
            ],
            teardown=teardown
        )

        serialread15700 = TestGroup(
            name='serialread15700',
            devices=[serialread15700_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ADN_Serial_15700,
            ]
        )

        test_groups = [
            serialread15700,
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
    tc_runner = Serialread15700TestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
