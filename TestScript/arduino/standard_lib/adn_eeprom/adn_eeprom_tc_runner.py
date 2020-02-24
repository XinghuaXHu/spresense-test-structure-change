#! /usr/bin/env python3
import sys
import os
import time
import pexpect

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.adn_eeprom.adn_eeprom_device import AdnEepromDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['']
PERF_MS_READ   = 7
PERF_MS_WRITE  = 60
PERF_MS_PUT1   = 60
PERF_MS_PUT100 = 117
PERF_MS_GET1   = 7
PERF_MS_GET100 = 7
PERF_MS_MARGIN = 2

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

def wait_sec(test, source, data, sec, description=None):
    if description is not None:
        test.log.info("")
        test.log.info("############################################################")
        test.log.info("## [{}sec] {}".format(str(sec), description))
        test.log.info("############################################################")
        test.log.info("")

    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')

def check_performance(test, source, data, max_millisec):
    value = data.split(':')[1].split('ms')[0]
    test.log.debug('data:{} max_millisec:{} value:{}'.format(data.strip(), max_millisec, value))
    test.assert_true(int(value) <= max_millisec + PERF_MS_MARGIN, 'value:{} <= max_millisec:{}'.format(value, max_millisec + PERF_MS_MARGIN))
    test.add_user_event(source, 'check_performance ok!')
    return True

class AdnEepromTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AdnEepromTcRunner, self).__init__(conf)

        self.adn_eeprom_device = AdnEepromDevice(
            kwargs.get('dut_device', None), 'ADN_EEPROM_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')

    def get_devices_list(self):
        return [self.adn_eeprom_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.adn_eeprom_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.adn_eeprom_device)

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
        timeout = 20

        adn_eeprom_device = self.get_devices_list()[0]

        setup = [
        ]

        teardown = [
        ]

        def wait_sec_steps(device, sec, description=None):
            return [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: wait_sec(test, source, data, sec, description)),
                Step(Action.WAIT_FOR, device, 'wait sec completed'),
            ]

        def check_performance_steps(device, cmd, wait_string, max_millisec):
            r = [
                Step(Action.WAIT_FOR, device, 'input \"[command];\"'),
                Step(Action.WRITE, device, '{};'.format(cmd)),
            ]
            for (stri, max_m) in zip(wait_string, max_millisec):
                r = r + [
                    Step(Action.WAIT_FOR, device, stri,
                         lambda test, source, data, max_m_buf=max_m: check_performance(test, source, data, max_m_buf)),
                    Step(Action.WAIT_FOR, device, 'check_performance ok!'),
                ]
            return r

        def check_api_steps(device, cmd):
            r = [
                Step(Action.WAIT_FOR, device, 'input \"[command];\"'),
                Step(Action.WRITE, device, '{};'.format(cmd)),
                Step(Action.WAIT_FOR, device, '{} ok'.format(cmd)),
            ]
            return r

        def test_16312_steps(device):
            return check_performance_steps(device, 'perf_write_ave', ['write_ave:'], [PERF_MS_WRITE])

        def test_16311_steps(device):
            return check_performance_steps(device, 'perf_read_ave', ['read_ave:'], [PERF_MS_READ])

        def test_16314_steps(device):
            return check_performance_steps(device, 'perf_put_ave', ['put(1byte)_ave:', 'put(100byte)_ave:'], [PERF_MS_PUT1, PERF_MS_PUT100])

        def test_16313_steps(device):
            return check_performance_steps(device, 'perf_get_ave', ['get(1byte)_ave:', 'get(100byte)_ave:'], [PERF_MS_GET1, PERF_MS_GET100])

        def test_15936_steps(device):
            return check_api_steps(device, 'read_write')

        def test_15937_steps(device):
            return check_api_steps(device, 'read_write')

        def test_15938_steps(device):
            return check_api_steps(device, 'read_update')

        def test_15939_steps(device):
            return check_api_steps(device, 'get_put')

        def test_15940_steps(device):
            return check_api_steps(device, 'get_put')

        def test_15941_steps(device):
            return check_api_steps(device, 'iterator')

        def test_15944_steps(device):
            return check_api_steps(device, 'clear')

        adn_eeprom_15936 = Test(
            name='ADN_Eeprom_15936',
            timeout=timeout,
            setup=setup,
            test=test_15936_steps(adn_eeprom_device),
            teardown=teardown
        )

        adn_eeprom_15937 = Test(
            name='ADN_Eeprom_15937',
            timeout=timeout,
            setup=setup,
            test=test_15937_steps(adn_eeprom_device),
            teardown=teardown
        )

        adn_eeprom_15938 = Test(
            name='ADN_Eeprom_15938',
            timeout=timeout,
            setup=setup,
            test=test_15938_steps(adn_eeprom_device),
            teardown=teardown
        )

        adn_eeprom_15939 = Test(
            name='ADN_Eeprom_15939',
            timeout=timeout,
            setup=setup,
            test=test_15939_steps(adn_eeprom_device),
            teardown=teardown
        )

        adn_eeprom_15940 = Test(
            name='ADN_Eeprom_15940',
            timeout=timeout,
            setup=setup,
            test=test_15940_steps(adn_eeprom_device),
            teardown=teardown
        )

        adn_eeprom_15941 = Test(
            name='ADN_Eeprom_15941',
            timeout=timeout,
            setup=setup,
            test=test_15941_steps(adn_eeprom_device),
            teardown=teardown
        )

        adn_eeprom_15944 = Test(
            name='ADN_Eeprom_15944',
            timeout=360,
            setup=setup,
            test=test_15944_steps(adn_eeprom_device),
            teardown=teardown
        )

        adn_eeprom_16312 = Test(
            name='ADN_Eeprom_16312',
            timeout=timeout,
            setup=setup,
            test=test_16312_steps(adn_eeprom_device),
            teardown=teardown
        )

        adn_eeprom_16311 = Test(
            name='ADN_Eeprom_16311',
            timeout=timeout,
            setup=setup,
            test=test_16311_steps(adn_eeprom_device),
            teardown=teardown
        )

        adn_eeprom_16314 = Test(
            name='ADN_Eeprom_16314',
            timeout=timeout,
            setup=setup,
            test=test_16314_steps(adn_eeprom_device),
            teardown=teardown
        )

        adn_eeprom_16313 = Test(
            name='ADN_Eeprom_16313',
            timeout=timeout,
            setup=setup,
            test=test_16313_steps(adn_eeprom_device),
            teardown=teardown
        )

        adn_eeprom_performance_group = TestGroup(
            name='adn_eeprom_performance_group',
            devices=[adn_eeprom_device],
            tag=[Tag.POSITIVE],
            tests=[
                adn_eeprom_16312,
                adn_eeprom_16311,
                adn_eeprom_16314,
                adn_eeprom_16313,
            ]
        )

        adn_eeprom_api_group = TestGroup(
            name='adn_eeprom_api_group',
            devices=[adn_eeprom_device],
            tag=[Tag.POSITIVE],
            tests=[
                adn_eeprom_15936,
                adn_eeprom_15937,
                adn_eeprom_15938,
                adn_eeprom_15939,
                adn_eeprom_15940,
                adn_eeprom_15941,
                adn_eeprom_15944,
            ]
        )

        test_groups = [
            adn_eeprom_performance_group,
            adn_eeprom_api_group,
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
    tc_runner = AdnEepromTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
