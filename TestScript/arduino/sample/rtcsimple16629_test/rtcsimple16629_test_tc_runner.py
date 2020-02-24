#! /usr/bin/env python3
import sys
import os
import re
import datetime
import time

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.rtcsimple16629_test.rtcsimple16629_test_device import RctSimple16629TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

t1 = datetime.datetime.now()
t2 = t1
rtc_count = 0

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['rct']

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    global t1
    global t2
    global t3
    global t4
    global rtc_count
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
    elif '1970' in data:
        timedata = data[0:19]
        if t1 == t2:
            t1 = datetime.datetime.strptime(str(timedata), "%Y/%m/%d %H:%M:%S")
            t2 = t1 + datetime.timedelta(seconds=1)
        else:
            t1 = datetime.datetime.strptime(str(timedata), "%Y/%m/%d %H:%M:%S")
            if t1 != t2:
                test.set_fail('rtc NG')
            else:
                rtc_count = rtc_count + 1
            t2 = t1 + datetime.timedelta(seconds=1)
            if rtc_count > 15:
                test.add_user_event(source, 'Rtc Simple Done.')
    elif '2018' in data:
        t2 = t1

def call_shell(test, source, data, command):
    test.log.info("start")
    os.system(command)
    test.log.info("end")

class RctSimpleTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(RctSimpleTestTcRunner, self).__init__(conf)

        self.rctsimple16629_test_device = RctSimple16629TestDevice(
            kwargs.get('dut_device', None), 'RCTGNSS16629_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/RctSimple-defconfig')
    def get_devices_list(self):
        return [self.rctsimple16629_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.rctsimple16629_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.rctsimple16629_test_device)

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
        timeout = 60

        rctsimple16629_test_device = self.get_devices_list()[0]


        setup = [
            Step(Action.EXECUTE, rctsimple16629_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
    ]

        teardown = [
            Step(Action.EXECUTE, rctsimple16629_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        Arduino_RtcSimple_16629 = Test(
            name='Arduino_RtcSimple_16629',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, rctsimple16629_test_device, 'Example for RTC simple clock'),
                Step(Action.WRITE, rctsimple16629_test_device, 'T1545264000'),
                Step(Action.WAIT_FOR, rctsimple16629_test_device, '2018/12/20 00:00:00'),
                Step(Action.WAIT_FOR, rctsimple16629_test_device, '2018/12/20 00:00:01'),
                Step(Action.WRITE, rctsimple16629_test_device, 'T0000000100'),
                Step(Action.WAIT_FOR, rctsimple16629_test_device, '1970/01/01 00:01:40'),
                Step(Action.WAIT_FOR, rctsimple16629_test_device, '1970/01/01 00:01:41'),
                Step(Action.WAIT_FOR, rctsimple16629_test_device, 'Rtc Simple Done.'),
            ],
            teardown=teardown
        )

        RctSimple = TestGroup(
            name='RctSimple',
            devices=[rctsimple16629_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                Arduino_RtcSimple_16629,
            ]
        )

        test_groups = [
            RctSimple,
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
    tc_runner = RctSimpleTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
