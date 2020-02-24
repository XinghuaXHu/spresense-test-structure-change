#! /usr/bin/env python3
import sys
import os
import time
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.gnss_extension_test.gnsstest_test_device import GnssTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser
from datetime import datetime

PROJECT = 'spresense'

TEST_DEVICE_APP_NAME = 'gnss_extension'
TIMEOUT_GNSS = 1800 
SUCCESS = 'SUCCESS'
FAILED = 'FAILED'
global _global_dict
_global_dict = {}

NEGATIVE = True
APPS_TO_BUILTIN = ['gnss_extension']

def call_shell(test, source, data, command):
    os.system(command)

def setTCXO(test, source, data):
    source.write('setTCXO --offset ' + _global_dict['offset']),
    _global_dict.pop('offset')

def set_time(test, source, data):
    source.write('setTime --year 2015 --month 10 --day 2 --hour ' + _global_dict['gnss_hour'] + ' --minute ' + _global_dict['gnss_minute'] + ' --sec ' + _global_dict['gnss_second'] + ' --usec ' + _global_dict['gnss_use']),
    _global_dict.pop('gnss_hour')
    _global_dict.pop('gnss_minute')
    _global_dict.pop('gnss_second')
    _global_dict.pop('gnss_use')

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
    elif 'DebugPrintfSatellites 2015' in data:
            split_str = " |-|_|:|[|]"
            dataArray = re.split(split_str, data)
            _global_dict.clear()
            _global_dict['gnss_hour'] = dataArray[4]
            _global_dict['gnss_minute'] = dataArray[5]
            _global_dict['gnss_second'] = dataArray[6]
            _global_dict['gnss_use'] = dataArray[7]
    elif '=' in data:
            test.log.info(data)
    elif 'FAILED' in data:
            test.log.info(data)

class TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super().__init__(conf)

        self.gnss_test_device = GnssTestDevice(
            kwargs.get('dut_device', None), 'GNSS_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/gnss_extension-defconfig')

    def get_devices_list(self):
        return [self.gnss_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.gnss_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.gnss_test_device)

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

        gnss_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WRITE, gnss_test_device, 'reboot'),
            Step(Action.WAIT_FOR, gnss_test_device, gnss_test_device.NUTTSHELL),
            Step(Action.WRITE, gnss_test_device, 'gnss_extension'),
            Step(Action.EXECUTE, gnss_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.WRITE, gnss_test_device, 'quit'),
            Step(Action.EXECUTE, gnss_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        gnssPvt_tc_testing = Test(
            name='gnssPvt_tc_testing',
            timeout=TIMEOUT_GNSS,
            setup=setup,
            test=[
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: time.sleep(10)),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_start.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                     lambda test, source, data: time.sleep(10)),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 1 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtDeleteLog'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStart'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --cold'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 68'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 0 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),

                Step(Action.WRITE, gnss_test_device, 'setEllipsoidal --latitude 35.42936720 --longitude 139.37787512 --altitude -7.50189438'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.EXECUTE, gnss_test_device, None, lambda test, source, data: set_time(test, source, data)),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setAlmanac --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setEphemeris --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTCXO --offset 700'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 1 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtDeleteLog'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStart'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --hot'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 1 --time2 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 0 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),

                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: time.sleep(10)),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_start.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                     lambda test, source, data: time.sleep(10)),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 1 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtDeleteLog'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStart'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --cold'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 0 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),

                Step(Action.WRITE, gnss_test_device, 'eraseBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setEllipsoidal --latitude 35.42936720 --longitude 139.37787512 --altitude -7.50189438'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.EXECUTE, gnss_test_device, None, lambda test, source, data: set_time(test, source, data)),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setAlmanac --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTCXO --offset 700'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 1 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtDeleteLog'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStart'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --warm'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 52'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 0 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),



                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: time.sleep(10)),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_start.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                     lambda test, source, data: time.sleep(10)),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 1 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --GLONASS '),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtDeleteLog'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStart'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --cold'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 68'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 0 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),


                Step(Action.WRITE, gnss_test_device, 'setEllipsoidal --latitude 35.42936720 --longitude 139.37787512 --altitude -7.50189438'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.EXECUTE, gnss_test_device, None, lambda test, source, data: set_time(test, source, data)),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setAlmanac --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setEphemeris --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTCXO --offset 700'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 1 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --GLONASS '),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtDeleteLog'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStart'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --hot'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 1 --time2 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 0 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),



                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: time.sleep(10)),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_start.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                     lambda test, source, data: time.sleep(10)),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 1 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --GLONASS '),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtDeleteLog'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStart'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --cold'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 0 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),

                Step(Action.WRITE, gnss_test_device, 'eraseBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setEllipsoidal --latitude 35.42936720 --longitude 139.37787512 --altitude -7.50189438'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.EXECUTE, gnss_test_device, None, lambda test, source, data: set_time(test, source, data)),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setAlmanac --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTCXO --offset 700'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 1 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --GLONASS'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtDeleteLog'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStart'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --warm'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 52'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 0 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),



                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: time.sleep(10)),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_start.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                     lambda test, source, data: time.sleep(10)),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 1 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --L1CA --L1S'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtDeleteLog'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStart'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --cold'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 68'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 0 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),


                Step(Action.WRITE, gnss_test_device, 'setEllipsoidal --latitude 35.42936720 --longitude 139.37787512 --altitude -7.50189438'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.EXECUTE, gnss_test_device, None, lambda test, source, data: set_time(test, source, data)),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setAlmanac --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setEphemeris --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTCXO --offset 700'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 1 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --L1CA --L1S'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtDeleteLog'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStart'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --hot'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 1 --time2 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 0 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),



                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: time.sleep(10)),
                Step(Action.EXECUTE, gnss_test_device, None,
                    lambda test, source, data: call_shell(test, source, data, "./labsat_start.sh")),
                Step(Action.EXECUTE, gnss_test_device, None,
                     lambda test, source, data: time.sleep(10)),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 1 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --L1CA --L1S'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtDeleteLog'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStart'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --cold'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 0 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),

                Step(Action.WRITE, gnss_test_device, 'eraseBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setEllipsoidal --latitude 35.42936720 --longitude 139.37787512 --altitude -7.50189438'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.EXECUTE, gnss_test_device, None, lambda test, source, data: set_time(test, source, data)),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setAlmanac --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTCXO --offset 700'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 1 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --L1CA --L1S'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtDeleteLog'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStart'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --warm'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 52'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'pvtLogStop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_PVTLOG --enable 0 --signo 19'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
            ],
            teardown=teardown
        )

        gnssPvt_tc_testing1 = gnssPvt_tc_testing.clone()
        gnssPvt_tc_testing1.name = 'gnssPvt_tc_testing1'
        gnssPvt_tc_testing2 = gnssPvt_tc_testing.clone()
        gnssPvt_tc_testing2.name = 'gnssPvt_tc_testing2'
        gnssPvt_tc_testing3 = gnssPvt_tc_testing.clone()
        gnssPvt_tc_testing3.name = 'gnssPvt_tc_testing3'
        gnssPvt_tc_testing4 = gnssPvt_tc_testing.clone()
        gnssPvt_tc_testing4.name = 'gnssPvt_tc_testing4'
        gnssPvt_tc_testing5 = gnssPvt_tc_testing.clone()
        gnssPvt_tc_testing5.name = 'gnssPvt_tc_testing5'
        gnssPvt_tc_testing6 = gnssPvt_tc_testing.clone()
        gnssPvt_tc_testing6.name = 'gnssPvt_tc_testing6'
        gnssPvt_tc_testing7 = gnssPvt_tc_testing.clone()
        gnssPvt_tc_testing7.name = 'gnssPvt_tc_testing7'
        gnssPvt_tc_testing8 = gnssPvt_tc_testing.clone()
        gnssPvt_tc_testing8.name = 'gnssPvt_tc_testing8'
        gnssPvt_tc_testing9 = gnssPvt_tc_testing.clone()
        gnssPvt_tc_testing9.name = 'gnssPvt_tc_testing9'

        gnss_testing_group = TestGroup(
            name='gnss_testing_group',
            devices=[gnss_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                gnssPvt_tc_testing,
                gnssPvt_tc_testing1,
                gnssPvt_tc_testing2,
                gnssPvt_tc_testing3,
                gnssPvt_tc_testing4,
                gnssPvt_tc_testing5,
                gnssPvt_tc_testing6,
                gnssPvt_tc_testing7,
                gnssPvt_tc_testing8,
                gnssPvt_tc_testing9,
            ]
        )

        test_groups = [
            gnss_testing_group,
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
    tc_runner = TestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
