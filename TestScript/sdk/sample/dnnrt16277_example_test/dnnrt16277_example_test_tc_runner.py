#! /usr/bin/env python3
import sys
import os
import time

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.dnnrt16277_example_test.dnnrt16277_example_test_device import SpresenseTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['dnnrt_lenet']

def wait_sec(test, source, data, sec, description):
    if description is not None:
        test.log.info("")
        test.log.info("#############################################")
        test.log.info("## [" + str(sec) + " sec] " + description)
        test.log.info("#############################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')

def check_data_size(test, source, data):
    if '1.00' in data or '0.99' in data:
        return True
    else:
        test.set_fail(' '.join(['Out of range. value = ', data]))
    return False

def check_time(test, source, data):
    t = data[15:]
    if float(t) >= 0.09:
        return True
    else:
        test.set_fail(' '.join(['Out of range. value = ', data]))
    return False

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


class SdkDNRRT_TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(SdkDNRRT_TestTcRunner, self).__init__(conf)

        self.test_device = SpresenseTestDevice(
            kwargs.get('dut_device', None), 'ARD_DNRRT16277_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/dnnrt_lenet-defconfig')

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

        SDK_DNNRT_test_16277_0 = Test(
            name='SDK_DNNRT_test_16277_0',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, test_device, 'dnnrt_lenet /mnt/sd0/lenet-5.nnb /mnt/sd0/0.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Load nnb file: /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, 'Load pgm image: /mnt/sd0/0.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image Normalization (1.0/255.0): enabled'),
                    Step(Action.WAIT_FOR, test_device, 'start dnn_runtime_forward()'),
                    Step(Action.WAIT_FOR, test_device, 'output[0]=',
                         lambda test, source, data: check_data_size(test, source, data)),
                    Step(Action.WAIT_FOR, test_device, 'output[1]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[2]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[3]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[4]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[5]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[6]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[7]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[8]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[9]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'inference time=',
                         lambda test, source, data: check_time(test, source, data)),
            ],
            teardown=teardown
        )

        SDK_DNNRT_test_16277_1 = Test(
            name='SDK_DNNRT_test_16277_1',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, test_device, 'dnnrt_lenet /mnt/sd0/lenet-5.nnb /mnt/sd0/1.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Load nnb file: /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, 'Load pgm image: /mnt/sd0/1.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image Normalization (1.0/255.0): enabled'),
                    Step(Action.WAIT_FOR, test_device, 'start dnn_runtime_forward()'),
                    Step(Action.WAIT_FOR, test_device, 'output[0]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[1]=',
                         lambda test, source, data: check_data_size(test, source, data)),
                    Step(Action.WAIT_FOR, test_device, 'output[2]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[3]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[4]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[5]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[6]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[7]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[8]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[9]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'inference time=',
                         lambda test, source, data: check_time(test, source, data)),
            ],
            teardown=teardown
        )

        SDK_DNNRT_test_16277_2 = Test(
            name='SDK_DNNRT_test_16277_2',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, test_device, 'dnnrt_lenet /mnt/sd0/lenet-5.nnb /mnt/sd0/2.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Load nnb file: /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, 'Load pgm image: /mnt/sd0/2.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image Normalization (1.0/255.0): enabled'),
                    Step(Action.WAIT_FOR, test_device, 'start dnn_runtime_forward()'),
                    Step(Action.WAIT_FOR, test_device, 'output[0]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[1]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[2]=',
                         lambda test, source, data: check_data_size(test, source, data)),
                    Step(Action.WAIT_FOR, test_device, 'output[3]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[4]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[5]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[6]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[7]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[8]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[9]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'inference time=',
                         lambda test, source, data: check_time(test, source, data)),
            ],
            teardown=teardown
        )

        SDK_DNNRT_test_16277_3 = Test(
            name='SDK_DNNRT_test_16277_3',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, test_device, 'dnnrt_lenet /mnt/sd0/lenet-5.nnb /mnt/sd0/3.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Load nnb file: /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, 'Load pgm image: /mnt/sd0/3.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image Normalization (1.0/255.0): enabled'),
                    Step(Action.WAIT_FOR, test_device, 'start dnn_runtime_forward()'),
                    Step(Action.WAIT_FOR, test_device, 'output[0]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[1]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[2]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[3]=',
                         lambda test, source, data: check_data_size(test, source, data)),
                    Step(Action.WAIT_FOR, test_device, 'output[4]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[5]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[6]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[7]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[8]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[9]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'inference time=',
                         lambda test, source, data: check_time(test, source, data)),
            ],
            teardown=teardown
        )

        SDK_DNNRT_test_16277_4 = Test(
            name='SDK_DNNRT_test_16277_4',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, test_device, 'dnnrt_lenet /mnt/sd0/lenet-5.nnb /mnt/sd0/4.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Load nnb file: /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, 'Load pgm image: /mnt/sd0/4.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image Normalization (1.0/255.0): enabled'),
                    Step(Action.WAIT_FOR, test_device, 'start dnn_runtime_forward()'),
                    Step(Action.WAIT_FOR, test_device, 'output[0]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[1]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[2]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[3]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[4]=',
                         lambda test, source, data: check_data_size(test, source, data)),
                    Step(Action.WAIT_FOR, test_device, 'output[5]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[6]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[7]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[8]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[9]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'inference time=',
                         lambda test, source, data: check_time(test, source, data)),
            ],
            teardown=teardown
        )

        SDK_DNNRT_test_16277_5 = Test(
            name='SDK_DNNRT_test_16277_5',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, test_device, 'dnnrt_lenet /mnt/sd0/lenet-5.nnb /mnt/sd0/5.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Load nnb file: /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, 'Load pgm image: /mnt/sd0/5.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image Normalization (1.0/255.0): enabled'),
                    Step(Action.WAIT_FOR, test_device, 'start dnn_runtime_forward()'),
                    Step(Action.WAIT_FOR, test_device, 'output[0]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[1]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[2]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[3]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[4]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[5]=',
                         lambda test, source, data: check_data_size(test, source, data)),
                    Step(Action.WAIT_FOR, test_device, 'output[6]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[7]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[8]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[9]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'inference time=',
                         lambda test, source, data: check_time(test, source, data)),
            ],
            teardown=teardown
        )

        SDK_DNNRT_test_16277_6 = Test(
            name='SDK_DNNRT_test_16277_6',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, test_device, 'dnnrt_lenet /mnt/sd0/lenet-5.nnb /mnt/sd0/6.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Load nnb file: /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, 'Load pgm image: /mnt/sd0/6.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image Normalization (1.0/255.0): enabled'),
                    Step(Action.WAIT_FOR, test_device, 'start dnn_runtime_forward()'),
                    Step(Action.WAIT_FOR, test_device, 'output[0]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[1]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[2]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[3]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[4]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[5]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[6]=',
                         lambda test, source, data: check_data_size(test, source, data)),
                    Step(Action.WAIT_FOR, test_device, 'output[7]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[8]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[9]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'inference time=',
                         lambda test, source, data: check_time(test, source, data)),
            ],
            teardown=teardown
        )

        SDK_DNNRT_test_16277_7 = Test(
            name='SDK_DNNRT_test_16277_7',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, test_device, 'dnnrt_lenet /mnt/sd0/lenet-5.nnb /mnt/sd0/7.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Load nnb file: /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, 'Load pgm image: /mnt/sd0/7.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image Normalization (1.0/255.0): enabled'),
                    Step(Action.WAIT_FOR, test_device, 'start dnn_runtime_forward()'),
                    Step(Action.WAIT_FOR, test_device, 'output[0]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[1]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[2]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[3]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[4]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[5]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[6]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[7]=',
                         lambda test, source, data: check_data_size(test, source, data)),
                    Step(Action.WAIT_FOR, test_device, 'output[8]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[9]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'inference time=',
                         lambda test, source, data: check_time(test, source, data)),
            ],
            teardown=teardown
        )

        SDK_DNNRT_test_16277_8 = Test(
            name='SDK_DNNRT_test_16277_8',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, test_device, 'dnnrt_lenet /mnt/sd0/lenet-5.nnb /mnt/sd0/8.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Load nnb file: /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, 'Load pgm image: /mnt/sd0/8.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image Normalization (1.0/255.0): enabled'),
                    Step(Action.WAIT_FOR, test_device, 'start dnn_runtime_forward()'),
                    Step(Action.WAIT_FOR, test_device, 'output[0]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[1]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[2]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[3]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[4]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[5]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[6]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[7]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[8]=',
                         lambda test, source, data: check_data_size(test, source, data)),
                    Step(Action.WAIT_FOR, test_device, 'output[9]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'inference time=',
                         lambda test, source, data: check_time(test, source, data)),
            ],
            teardown=teardown
        )

        SDK_DNNRT_test_16277_9 = Test(
            name='SDK_DNNRT_test_16277_9',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, test_device, 'dnnrt_lenet /mnt/sd0/lenet-5.nnb /mnt/sd0/9.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Load nnb file: /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, 'Load pgm image: /mnt/sd0/9.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image Normalization (1.0/255.0): enabled'),
                    Step(Action.WAIT_FOR, test_device, 'start dnn_runtime_forward()'),
                    Step(Action.WAIT_FOR, test_device, 'output[0]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[1]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[2]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[3]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[4]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[5]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[6]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[7]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[8]=0.00'),
                    Step(Action.WAIT_FOR, test_device, 'output[9]=',
                         lambda test, source, data: check_data_size(test, source, data)),
                    Step(Action.WAIT_FOR, test_device, 'inference time=',
                         lambda test, source, data: check_time(test, source, data)),
            ],
            teardown=teardown
        )

        SDK_DNNRT_test_16278 = Test(
            name='SDK_DNNRT_test_16278',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, test_device, 'dnnrt_lenet /mnt/sd0/lenet-5.nnb /mnt/sd0/test2.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Load nnb file: /mnt/sd0/lenet-5.nnb'),
                    Step(Action.WAIT_FOR, test_device, 'Load pgm image: /mnt/sd0/test2.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image Normalization (1.0/255.0): enabled'),
                    Step(Action.WAIT_FOR, test_device, 'load pgm image failed due to -79'),
            ],
            teardown=teardown
        )

        SDK_DNNRT_test_16218 = SDK_DNNRT_test_16277_7.clone()
        SDK_DNNRT_test_16218.name = "SDK_DNNRT_test_16218"


        testGP = TestGroup(
            name='testGP',
            devices=[test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_DNNRT_test_16277_0,
                SDK_DNNRT_test_16277_1,
                SDK_DNNRT_test_16277_2,
                SDK_DNNRT_test_16277_3,
                SDK_DNNRT_test_16277_4,
                SDK_DNNRT_test_16277_5,
                SDK_DNNRT_test_16277_6,
                SDK_DNNRT_test_16277_7,
                SDK_DNNRT_test_16277_8,
                SDK_DNNRT_test_16277_9,
                SDK_DNNRT_test_16218,
                SDK_DNNRT_test_16278,
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
