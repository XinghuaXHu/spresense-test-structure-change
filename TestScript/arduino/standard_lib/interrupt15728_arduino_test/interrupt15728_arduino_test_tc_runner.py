#! /usr/bin/env python3
import sys
import os

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)


from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.interrupt15728_arduino_test.interrupt15728_arduino_test_device import Interrupt15728TestDevice
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['Interrupt']

LTE_PINS = [2,3,5,6,7,9]
EXT_PINS = [0,1,2,3,4,5,6,7]

def check_step(test, source, data, value):
    test.log.info(value)


class InterruptTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(InterruptTestTcRunner, self).__init__(conf)

        self.test_device = Interrupt15728TestDevice(
            kwargs.get('dut_device', None), 'INTERRUPT15728_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

    def get_devices_list(self):
        return [self.test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

    # noinspection PyUnresolvedReferences
    def build(self, debug=True, log=None, **kwargs):
        dut_bin = kwargs.get('dut_bin', None)

        if not all([dut_bin]):
            if log:
                log.info('Building project sources')

        return dict(dut_bin=dut_bin)

    def generate_test_groups(self, arguments, log=None):
        timeout = 150

        test_device = self.get_devices_list()[0]

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
                    Step(Action.WAIT_FOR, device, '------Start Test pin[' + str(PINS[i_pin]) + '] Mode LOW------'),
                    Step(Action.WAIT_FOR, device, 'to LOW'),
                    Step(Action.WAIT_FOR, device, 'Interrupt occur! Pin[' + str(PINS[i_pin]) + '], count 1'),
                    Step(Action.WAIT_FOR, device, 'Interrupt occur! Pin[' + str(PINS[i_pin]) + '], count 2'),
                    Step(Action.WAIT_FOR, device, 'Interrupt occur! Pin[' + str(PINS[i_pin]) + '], count 3'),
                    Step(Action.WAIT_FOR, device, 'Interrupt occur! Pin[' + str(PINS[i_pin]) + '], count 4'),
                    Step(Action.WAIT_FOR, device, 'Interrupt occur! Pin[' + str(PINS[i_pin]) + '], count 5'),
                    Step(Action.WAIT_FOR, device, 'Interrupt occur! Pin[' + str(PINS[i_pin]) + '], count 6'),
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

        arduino_interrupt_15728 = Test(
            name='arduino_interrupt_15728',
            timeout=timeout,
            setup=setup,
            test=verify_pin(test_device),
            teardown=teardown
        )

        interrupt_gp = TestGroup(
            name='interrupt_gp',
            devices=[test_device],
            tag=[Tag.POSITIVE],
            tests=[
                arduino_interrupt_15728,
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
