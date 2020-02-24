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
from tc.dnnrt16424_test.dnnrt16424_test_device import SpresenseTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['dnnrt_paramtest']

def wait_sec(test, source, data, sec, description):
    if description is not None:
        test.log.info("")
        test.log.info("#############################################")
        test.log.info("## [" + str(sec) + " sec] " + description)
        test.log.info("#############################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')

def check_file_size(test, source, data):
    size_value = re.search(r'( [0-9]+ )', data)
    if size_value:
        size_str = size_value.group().strip()
        if int(size_str) >= 0:
            return True
        else:
            test.set_fail(' '.join(['file not find. data=', data]))
    return False

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
    elif 'NG' in data or 'failed' in data:
        test.set_fail(' '.join(['Test Failed = ', data]))

class SdkDNRRT_TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(SdkDNRRT_TestTcRunner, self).__init__(conf)

        self.test_device = SpresenseTestDevice(
            kwargs.get('dut_device', None), 'ARD_DNRRT16424_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/dnnrt_paramtest-defconfig')

    def get_devices_list(self):
        return [self.test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.test_device)

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

        test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, test_device, test_device.NSH_PROMPT),
            Step(Action.WRITE, test_device, 'reboot'),
            Step(Action.WAIT_FOR, test_device, test_device.NUTTSHELL),
            Step(Action.EXECUTE, test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
            Step(Action.EXECUTE, test_device, None,
                 lambda test, source, data: wait_sec(test, source, data, 5, 'wait for sd0')),
        ]

        teardown = [
            Step(Action.EXECUTE, test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        SDK_DNNRT_test_16424_16425_to_16434 = Test(
            name='SDK_DNNRT_test_16424_16425_to_16434',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, test_device, 'ls -l /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, '/mnt/sd0/lenet-5.nnb',
                         lambda test, source, data: check_file_size(test, source, data)),
                    Step(Action.WRITE, test_device, 'ls -l /mnt/sd0/0.pgm'),
                    Step(Action.WAIT_FOR, test_device, '/mnt/sd0/0.pgm',
                         lambda test, source, data: check_file_size(test, source, data)),
                    Step(Action.WRITE, test_device, 'dnnrt_paramtest'),
                    Step(Action.WAIT_FOR, test_device, 'Load nnb file: /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, 'Load pgm image: /mnt/sd0/0.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image Normalization (1.0/255.0): enabled'),
                    Step(Action.WAIT_FOR, test_device, 'start checking inputval!'),
                    Step(Action.WAIT_FOR, test_device, 'dnn_runtime_input_num():1 check OK'),
                    Step(Action.WAIT_FOR, test_device, 'dnn_runtime_input_size():784 check OK'),
                    Step(Action.WAIT_FOR, test_device, 'dnn_runtime_input_ndim():4 check OK'),
                    Step(Action.WAIT_FOR, test_device, 'm:0 dnn_runtime_input_shape():1 check OK'),
                    Step(Action.WAIT_FOR, test_device, 'm:1 dnn_runtime_input_shape():1 check OK'),
                    Step(Action.WAIT_FOR, test_device, 'm:2 dnn_runtime_input_shape():28 check OK'),
                    Step(Action.WAIT_FOR, test_device, 'm:3 dnn_runtime_input_shape():28 check OK'),
                    Step(Action.WAIT_FOR, test_device, 'start checking outputval!'),
                    Step(Action.WAIT_FOR, test_device, 'dnn_runtime_output_num():1 check OK'),
                    Step(Action.WAIT_FOR, test_device, 'dnn_runtime_output_size():10 check OK'),
                    Step(Action.WAIT_FOR, test_device, 'dnn_runtime_output_ndim():2 check OK'),
                    Step(Action.WAIT_FOR, test_device, 'm:0 dnn_runtime_output_shape():1 check OK'),
                    Step(Action.WAIT_FOR, test_device, 'm:1 dnn_runtime_output_shape():10 check OK'),
                    Step(Action.WAIT_FOR, test_device, 'SUCCEDED check_parm finished!!'),
            ],
            teardown=teardown
        )

        testGP = TestGroup(
            name='testGP',
            devices=[test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_DNNRT_test_16424_16425_to_16434,
            ]
        )

        test_groups = [
            testGP,
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
    tc_runner = SdkDNRRT_TestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
