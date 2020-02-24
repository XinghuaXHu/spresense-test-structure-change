#! /usr/bin/env python3
import sys
import os

sys.path.append('../../')
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.advancedIO15861_test.advancedIO15861_test_device import AdvancedIO15861TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['advancedIO15861']
LTE_PINS = [2,3,5,6,7,9]
EXT_PINS = [0,14]
# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


class AdvancedIO15861TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AdvancedIO15861TestTcRunner, self).__init__(conf)

        self.advancedIO15861_test_device = AdvancedIO15861TestDevice(
            kwargs.get('dut_device', None), 'ADVANCEDIO15861_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.readr_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(self.config.projects[0].path,
                                           'test/autotest/src/tc/advancedIO15861_test/defconfig/advancedIO15861-defconfig')

    def get_devices_list(self):
        return [self.advancedIO15861_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.advancedIO15861_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.advancedIO15861_test_device)

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
        timeout = 270

        advancedIO15861_test_device = self.get_devices_list()[0]

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

                if 'lte' == device.ext_type:
                    PIN_OUT = PINS[o_pin]
                else:
                    PIN_OUT = PINS[i_pin] + 1

                step += [
                    Step(Action.WAIT_FOR, device, 'Please Input Test PIN ID:'),
                    Step(Action.WRITE, device, str(PINS[i_pin]), None, '\r'),
                    Step(Action.WAIT_FOR, device, 'Please Input Out PIN ID:'),
                    Step(Action.WRITE, device, str(PIN_OUT), None, '\r'),                
                    Step(Action.WAIT_FOR, advancedIO15861_test_device, 'test case start'),
                    Step(Action.WAIT_FOR, advancedIO15861_test_device, '= 1'),
                    Step(Action.WAIT_FOR, advancedIO15861_test_device, '= 0'),
                    Step(Action.WAIT_FOR, advancedIO15861_test_device, '= 1'),
                    Step(Action.WAIT_FOR, advancedIO15861_test_device, '1st pulseIn value:'),
                    Step(Action.WAIT_FOR, advancedIO15861_test_device, '= 1'),
                    Step(Action.WAIT_FOR, advancedIO15861_test_device, '= 0'),
                    Step(Action.WAIT_FOR, advancedIO15861_test_device, '2st pulseIn value:'),
                    Step(Action.WAIT_FOR, advancedIO15861_test_device, 'test case end'),
                ]
            
            step += [
                Step(Action.WAIT_FOR, device, 'Test Done'),
            ]

            return step

        setup = [
        ]

        teardown = [
        ]

        ADN_Advanced_io_15861= Test(
            name='ADN_Advanced_io_15861',
            timeout=timeout,
            setup=setup,
            #serial_read_15702.ino in Serial.print
            # to Serial.println
            test=verify_pin(advancedIO15861_test_device),
            teardown=teardown
        )

        advancedIO15861 = TestGroup(
            name='advancedIO15861',
            devices=[advancedIO15861_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ADN_Advanced_io_15861,
            ]
        )

        test_groups = [
            advancedIO15861,
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
    tc_runner = AdvancedIO15861TestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
