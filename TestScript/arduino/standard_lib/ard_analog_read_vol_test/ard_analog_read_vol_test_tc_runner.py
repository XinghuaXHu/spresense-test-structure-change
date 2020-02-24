#! /usr/bin/env python3
import sys
import os

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.ard_analog_read_vol_test.ard_analog_read_vol_test_device import ArdAnalogReadVolTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['ard_analog_read_vol']


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

# noinspection PyUnusedLocal
def dut_monitor(test, source, data):
    try:
        float(data)
    except:
        test.log.debug('cant convert to float')
    else:
        if 3.6 >= float(data) >= 3.06:
            test.add_user_event(source, 'the task ArdAnalogReadVol test ok!')
        else:
            test.set_fail(' '.join(['Out of range. data = ', data]))

class ArdAnalogReadVolTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(ArdAnalogReadVolTestTcRunner, self).__init__(conf)

        self.ard_analog_read_vol_test_device = ArdAnalogReadVolTestDevice(
            kwargs.get('dut_device', None), 'ARD_ANALOG_READ_VOL_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if self.ard_analog_read_vol_test_device:
            self.ard_analog_read_vol_test_device.set_baudrate(9600)

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.getcwd(), 'defconfig/ard_analog_read_vol-defconfig')

    def get_devices_list(self):
        return [self.ard_analog_read_vol_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.ard_analog_read_vol_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.ard_analog_read_vol_test_device)

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
        timeout = 15

        ard_analog_read_vol_test_device = self.get_devices_list()[0]

        setup = [
        ]

        teardown = [
        ]

        ADN_Sample_sketch_15882 = Test(
            name='ADN_Sample_sketch_15882',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, ard_analog_read_vol_test_device, ('dut_mon', dut_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, ard_analog_read_vol_test_device, 'the task ArdAnalogReadVol test ok!'),
            ],
            teardown=[
                Step(Action.EXECUTE, ard_analog_read_vol_test_device, 'dut_mon', Test.remove_monitor),
            ]
        )

        ard_analog_read_vol = TestGroup(
            name='ard_analog_read_vol',
            devices=[ard_analog_read_vol_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ADN_Sample_sketch_15882,
            ]
        )

        test_groups = [
            ard_analog_read_vol,
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
    tc_runner = ArdAnalogReadVolTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
