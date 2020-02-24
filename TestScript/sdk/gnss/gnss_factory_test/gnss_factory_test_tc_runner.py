#! /usr/bin/env python3
import sys
import os
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.gnss_factory_test.gnss_factory_test_device import GnssFactoryTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

from datetime import datetime
import time
now = time.time()
utc = datetime.utcfromtimestamp(now)

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['gnss']

# noinspection PyUnusedLocal
counter = 0
g_number = 0
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

def dut_factory_monitor(test, source, data):
    global counter
    global g_number
    if 'cn' in data:
        cn_value = (re.search(r"[0-9]+", data))
        cn_value = int(cn_value.group())
        if cn_value < 20000000:
            test.log.info(data)
            counter+=1
            if counter < 3:
                test.log.info("test_factory retry counter:"+str(counter+1))
                test_factory(test, source, data, g_number)
            else:
                counter = 0
                test.add_user_event(source, 'test ERROR')
        else:
            counter = 0
            test.add_user_event(source, 'verify OK')

def test_factory(test, source, data, number):
    global counter
    global g_number
    g_number = number
    source.write('gnss_test factory '+ str(number)) 
    return True

def call_shell(test, source, data, command):
    test.log.info("start")
    os.system(command)
    test.log.info("end")

class GnssFactoryTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(GnssFactoryTestTcRunner, self).__init__(conf)

        self.gnss_factory_test_device = GnssFactoryTestDevice(
            kwargs.get('dut_device', None), 'GNSSFACTORY_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/gnss-defconfig')

    def get_devices_list(self):
        return [self.gnss_factory_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.gnss_factory_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.gnss_factory_test_device)

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
        timeout = 60*15

        gnss_factory_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, gnss_factory_test_device, gnss_factory_test_device.NSH_PROMPT),
            Step(Action.WRITE, gnss_factory_test_device, 'reboot'),
            Step(Action.WAIT_FOR, gnss_factory_test_device, gnss_factory_test_device.NUTTSHELL),
            Step(Action.EXECUTE, gnss_factory_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
            Step(Action.EXECUTE, gnss_factory_test_device, ('dut_factory_mon', dut_factory_monitor),
                 Test.add_monitor),
            Step(Action.EXECUTE, gnss_factory_test_device, None,
                lambda test, source, data: call_shell(test, source, data, "./labsat_start.sh")),
        ]

        teardown = [
            Step(Action.EXECUTE, gnss_factory_test_device, 'dut_reboot_mon', Test.remove_monitor),
            Step(Action.EXECUTE, gnss_factory_test_device, 'dut_factory_mon', Test.remove_monitor),
            Step(Action.EXECUTE, gnss_factory_test_device, None,
                lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
        ]

        SDK_Example_test_15852 = Test(
            name='SDK_Example_test_15852',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, gnss_factory_test_device, 'gnss_factory'),
                Step(Action.WAIT_FOR, gnss_factory_test_device, 'test OK'),
            ],
            teardown=teardown
        )

        SDK_Gnss_15313 = Test(
            name='SDK_Gnss_15313',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, gnss_factory_test_device, None,
                     lambda test, source, data: test_factory(test, source, data,1)),
                Step(Action.WAIT_FOR, gnss_factory_test_device, 'verify OK'),
                Step(Action.EXECUTE, gnss_factory_test_device, None,
                     lambda test, source, data: test_factory(test, source, data,4)),
                Step(Action.WAIT_FOR, gnss_factory_test_device, 'test ERROR'),
                Step(Action.EXECUTE, gnss_factory_test_device, None,
                     lambda test, source, data: test_factory(test, source, data,2)),
                Step(Action.WAIT_FOR, gnss_factory_test_device, 'test ERROR'),

            ],
            teardown=teardown
        )


        gnss = TestGroup(
            name='gnss',
            devices=[gnss_factory_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Example_test_15852,
                SDK_Gnss_15313,
            ]
        )

        test_groups = [
            gnss,
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
    tc_runner = GnssFactoryTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
