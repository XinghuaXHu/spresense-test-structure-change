#! /usr/bin/env python3
import sys
import os

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.adt7310_test.adt7310_test_device import Adt7310TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['adt7310']


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

# noinspection PyUnusedLocal
def dut_monitor(test, source, data):
    if 'ADT7310Test' in data:
        test.log.debug('init loop count')
        test.storage.count = 0
        return

    try:
        float(data)
    except:
        test.set_fail(' '.join(['cant convert to float. data = ', data]))
    else:
        test.storage.count += 1
        if not 32.0 >= float(data) >= 18.0:
            test.set_fail(' '.join(['Out of range. data = ', data]))
            return

        if test.storage.count >= 5:
            test.add_user_event(source, 'the task ADT7310 test ok!')

class Adt7310TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(Adt7310TestTcRunner, self).__init__(conf)

        self.adt7310_test_device = Adt7310TestDevice(
            kwargs.get('dut_device', None), 'ADT7310_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.getcwd(), 'defconfig/adt7310-defconfig')

    def get_devices_list(self):
        return [self.adt7310_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.adt7310_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.adt7310_test_device)

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

        adt7310_test_device = self.get_devices_list()[0]

        if 'lte' == adt7310_test_device.ext_type:
            steps=[
                Step(Action.WAIT_FOR, adt7310_test_device, 'Please Input SPI Type:'),
                Step(Action.WRITE, adt7310_test_device, 'SPI3', None, '\r'),
                Step(Action.EXECUTE, adt7310_test_device, ('dut_mon', dut_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, adt7310_test_device, 'the task ADT7310 test ok!'),
            ]
        else:
            steps=[

                Step(Action.WAIT_FOR, adt7310_test_device, 'Please Input SPI Type:'),
                Step(Action.WRITE, adt7310_test_device, 'SPI', None, '\r'),
                Step(Action.EXECUTE, adt7310_test_device, ('dut_mon', dut_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, adt7310_test_device, 'the task ADT7310 test ok!'),
            ]
        
        setup = [
        ]

        teardown = [
        ]

        ADN_Arduinoshield_15877 = Test(
            name='ADN_Arduinoshield_15877',
            timeout=timeout,
            setup=setup,
            test=steps,
            teardown=teardown
        )

        adt7310 = TestGroup(
            name='adt7310',
            devices=[adt7310_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ADN_Arduinoshield_15877,
            ]
        )

        test_groups = [
            adt7310,
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
    tc_runner = Adt7310TestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
