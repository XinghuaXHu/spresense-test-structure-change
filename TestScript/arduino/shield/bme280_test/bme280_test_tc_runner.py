#! /usr/bin/env python3
import sys
import os

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.bme280_test.bme280_test_device import Bme280TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['bme280']


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

# noinspection PyUnusedLocal
def dut_monitor(test, source, data):
    if not hasattr(test.storage, 'count'):
        test.log.debug('init loop count')
        test.storage.count = 0

    test.storage.count += 1
    if test.storage.count > 15 or test.storage.count < 5:
        return

    split_colon = data.split(' : ')

    temp = split_colon[1].split(' ')[0]
    press = split_colon[2].split(' ')[0]
    hum = split_colon[3].split(' ')[0]
    test.log.debug('[' + str(test.storage.count) + '] temp:' + temp + ' press:' + press + ' hum:' + hum)

    if not 32.0 >= float(temp) >= 18.0:
        test.set_fail(' '.join(['Out of range. temp = ', temp]))
        return

    if not 1040.0 >= float(press) >= 980.0:
        test.set_fail(' '.join(['Out of range. press = ', press]))
        return

    if not 90.0 >= float(hum) >= 0.0:
        test.set_fail(' '.join(['Out of range. hum = ', hum]))
        return

    if test.storage.count >= 15:
        test.add_user_event(source, 'the task BME280 test ok!')

class Bme280TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(Bme280TestTcRunner, self).__init__(conf)

        self.bme280_test_device = Bme280TestDevice(
            kwargs.get('dut_device', None), 'BME280_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.getcwd(), 'defconfig/bme280-defconfig')

    def get_devices_list(self):
        return [self.bme280_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.bme280_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.bme280_test_device)

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
        timeout = 30

        bme280_test_device = self.get_devices_list()[0]

        setup = [
        ]

        teardown = [
        ]

        ADN_Arduinoshield_15878 = Test(
            name='ADN_Arduinoshield_15878',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, bme280_test_device, ('dut_mon', dut_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, bme280_test_device, 'the task BME280 test ok!'),
            ],
            teardown=teardown
        )

        bme280 = TestGroup(
            name='bme280',
            devices=[bme280_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ADN_Arduinoshield_15878,
            ]
        )

        test_groups = [
            bme280,
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
    tc_runner = Bme280TestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
