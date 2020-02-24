#! /usr/bin/env python3
import sys
import os
import datetime

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.ard_dnnrt_16333.ard_dnnrt_16333_device import SpresenseTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['ddnrt']

t1 = datetime.datetime.now()
t2 = t1

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    global t1
    global t2
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
    elif 'value' in data:
        if t1 == t2:
            t1 = datetime.datetime.now()
            t2 = t1 + datetime.timedelta(seconds=600)
        else:
            t = datetime.datetime.now()
            if t > t2:
                test.add_user_event(source, 'Test Done')

        dataArr = data.split()
        if dataArr[1] != '1.00':
            test.set_fail('dnrrt loop test NG')
    elif 'nnb not found' in data or 'pgmfile not found' in data:
        test.set_fail(' '.join(['ERROR', data]))



class ArdDNRRT_TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(ArdDNRRT_TestTcRunner, self).__init__(conf)

        self.test_device = SpresenseTestDevice(
            kwargs.get('dut_device', None), 'ARD_DNRRT16333_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/hello-defconfig')

    def get_devices_list(self):
        return [self.test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.test_device, binaries['dut_bin'], log)

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
        timeout = 660

        test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.EXECUTE, test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        def loop_steps(count=10):
            r = []
            for i in range(count):
                r += [
                    Step(Action.WAIT_FOR, test_device, '0.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image is 0'),
                    Step(Action.WAIT_FOR, test_device, 'value 1.00'),
                    Step(Action.WAIT_FOR, test_device, '1.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image is 1'),
                    Step(Action.WAIT_FOR, test_device, 'value 1.00'),
                    Step(Action.WAIT_FOR, test_device, '2.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image is 2'),
                    Step(Action.WAIT_FOR, test_device, 'value 1.00'),
                    Step(Action.WAIT_FOR, test_device, '3.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image is 3'),
                    Step(Action.WAIT_FOR, test_device, 'value 1.00'),
                    Step(Action.WAIT_FOR, test_device, '4.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image is 4'),
                    Step(Action.WAIT_FOR, test_device, 'value 1.00'),
                    Step(Action.WAIT_FOR, test_device, '5.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image is 5'),
                    Step(Action.WAIT_FOR, test_device, 'value 1.00'),
                    Step(Action.WAIT_FOR, test_device, '6.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image is 6'),
                    Step(Action.WAIT_FOR, test_device, 'value 1.00'),
                    Step(Action.WAIT_FOR, test_device, '7.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image is 7'),
                    Step(Action.WAIT_FOR, test_device, 'value 1.00'),
                    Step(Action.WAIT_FOR, test_device, '8.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image is 8'),
                    Step(Action.WAIT_FOR, test_device, 'value 1.00'),
                    Step(Action.WAIT_FOR, test_device, '9.pgm'),
                    Step(Action.WAIT_FOR, test_device, 'Image is 9'),
                    Step(Action.WAIT_FOR, test_device, 'value 1.00'),
                ]
            return r
            



        ARD_DNNRT_test_16333 = Test(
            name='ARD_DNNRT_test_16333',
            timeout=timeout,
            setup=setup,
            test= loop_steps(10) + [
                Step(Action.WAIT_FOR, test_device, 'Test Done'),
            ],
            teardown=teardown
        )

        testGP = TestGroup(
            name='testGP',
            devices=[test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ARD_DNNRT_test_16333,
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
    tc_runner = ArdDNRRT_TestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
