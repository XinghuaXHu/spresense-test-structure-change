#! /usr/bin/env python3
import sys
import os
import re
import datetime

sys.path.append('../../')
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.advancedIO15860_test.advancedIO15860_test_device import AdvancedIO15860TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['advancedIO15860']
startTime = datetime.datetime.now()
LTE_PINS = [2,3,5,6,7,9]
EXT_PINS = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28]

def check_time(test, source, data, min, max):
    dataArr = data.split()
    if len(dataArr) > 3:
        size_value = dataArr[3]
        if max >= int(size_value) >= min:
            return True
        else:
            test.set_fail(' '.join(['Out of range. time = ', size_value]))
    return False


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    global startTime
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
    elif '2st pulseIn value:' in data:
        currentTime = datetime.datetime.now()
        endTime = startTime + datetime.timedelta(seconds=175)
        if endTime >= currentTime:
            test.set_fail(' '.join(['time is out of range. data = ', data]))
    elif '2st pulseIn value' in data:
        startTime = datetime.datetime.now()

class AdvancedIO15860TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AdvancedIO15860TestTcRunner, self).__init__(conf)

        self.advancedIO15860_test_device = AdvancedIO15860TestDevice(
            kwargs.get('dut_device', None), 'ADVANCEDIO15860_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.readr_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(self.config.projects[0].path,
                                           'test/autotest/src/tc/advancedIO15860_test/defconfig/advancedIO15860-defconfig')

    def get_devices_list(self):
        return [self.advancedIO15860_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.advancedIO15860_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.advancedIO15860_test_device)

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
        timeout = 200

        advancedIO15860_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.EXECUTE, advancedIO15860_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, advancedIO15860_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        ADN_Advanced_io_15860 = Test(
            name='ADN_Advanced_io_15860',
            timeout=timeout,
            setup=setup,
            test=[],
            teardown=teardown
        )


        def testLoop(pin_test='28', pin_out='27'):
            r = [
                Step(Action.WAIT_FOR, advancedIO15860_test_device, 'Please Input PIN ID'),
                Step(Action.WRITE, advancedIO15860_test_device, pin_test, None, '\r'),
                Step(Action.WAIT_FOR, advancedIO15860_test_device, 'Please Input PIN ID'),
                Step(Action.WRITE, advancedIO15860_test_device, pin_out, None, '\r'),

                Step(Action.WAIT_FOR, advancedIO15860_test_device, 'test case start'),
                Step(Action.WAIT_FOR, advancedIO15860_test_device, '= 0'),
                Step(Action.WAIT_FOR, advancedIO15860_test_device, '1st pulseIn value'),
                Step(Action.WAIT_FOR, advancedIO15860_test_device, '= 1'),
                Step(Action.WAIT_FOR, advancedIO15860_test_device, '= 0'),
                Step(Action.WAIT_FOR, advancedIO15860_test_device, '1st pulseIn value:',
                     lambda test, source, data: check_time(test, source, data, 10000, 20000)),

                Step(Action.WAIT_FOR, advancedIO15860_test_device, '2nd pulseIn value'),
                Step(Action.WAIT_FOR, advancedIO15860_test_device, '= 1'),
                Step(Action.WAIT_FOR, advancedIO15860_test_device, '= 0'),
                Step(Action.WAIT_FOR, advancedIO15860_test_device, '2nd pulseIn value:',
                     lambda test, source, data: check_time(test, source, data, 179900000,  180000000)),
                Step(Action.WAIT_FOR, advancedIO15860_test_device, 'test case end'),
            ]
            return r

        def getTest():
            g = []

            if 'lte' == advancedIO15860_test_device.ext_type:
                PINS = LTE_PINS
            else:
                PINS = EXT_PINS
            
            for i in range(len(PINS)):
                temp = ADN_Advanced_io_15860.clone()
                temp.name = temp.name + '_' + str(i)
                if (i % 2) == 0:
                    if i == (len(PINS) - 1):
                        temp.test = testLoop(str(PINS[i]), str(PINS[i - 1]))
                    else:
                        temp.test = testLoop(str(PINS[i]), str(PINS[i + 1]))
                else:
                    temp.test = testLoop(str(PINS[i]), str(PINS[i - 1]))
                g = g + [temp]

            return g

        advancedIO15860 = TestGroup(
            name='advancedIO15860',
            devices=[advancedIO15860_test_device],
            tag=[Tag.POSITIVE],
            tests=getTest(),
        )

        test_groups = [
            advancedIO15860,
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
    tc_runner = AdvancedIO15860TestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
