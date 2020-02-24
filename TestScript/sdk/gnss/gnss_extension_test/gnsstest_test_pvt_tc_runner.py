#! /usr/bin/env python3
import sys
import os
import time

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


PROJECT = 'spresense'

TEST_DEVICE_APP_NAME = 'gnss_extension'
TIMEOUT_GNSS = 1200 
TIMEOUT_240 = 240 
TIMEOUT_780 = 780 
SUCCESS = 'SUCCESS'
FAILED = 'FAILED'

NEGATIVE = True
APPS_TO_BUILTIN = ['gnss_extension']


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
#    if 'NuttShell (NSH)' in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


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
            Step(Action.WAIT_FOR, gnss_test_device, gnss_test_device.NSH_PROMPT),
            Step(Action.WRITE, gnss_test_device, 'reboot'),
            Step(Action.WAIT_FOR, gnss_test_device, gnss_test_device.NUTTSHELL),
            Step(Action.WRITE, gnss_test_device, 'gnss_extension'),
            Step(Action.WAIT_FOR, gnss_test_device, "gnss app start"),
            Step(Action.EXECUTE, gnss_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.WRITE, gnss_test_device, 'quit'),
            Step(Action.WAIT_FOR, gnss_test_device, "gnss app end"),
            Step(Action.EXECUTE, gnss_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        gnss_tc_gps_testing = Test(
            name='gnss_tc_gps_testing',
            timeout=TIMEOUT_240,
            setup=setup,
            test=[
                # GPS test
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --cold'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 90'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),


                Step(Action.WRITE, gnss_test_device, 'setEllipsoidal --latitude 35.42936720 --longitude 139.37787512 --altitude -7.50189438'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTime --year 2015 --month 10 --day 2 --hour 8 --minute 39 --sec 11 --usec 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setAlmanac --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setEphemeris --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTCXO --offset 700'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
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
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),


                Step(Action.WRITE, gnss_test_device, 'setEllipsoidal --latitude 35.42936720 --longitude 139.37787512 --altitude -7.50189438'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTime --year 2015 --month 10 --day 2 --hour 8 --minute 39 --sec 11 --usec 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setAlmanac --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setEphemeris --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTCXO --offset 700'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --warm'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 60'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
            ],
            teardown=teardown
        )


        gnss_tc_gps_glonass_testing = Test(
            name='gnss_tc_gps_glonass_testing',
            timeout=TIMEOUT_240,
            setup=setup,
            test=[
                # GPS GLONASS test
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --GLONASS '),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --cold'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 90'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),


                Step(Action.WRITE, gnss_test_device, 'setEllipsoidal --latitude 35.42936720 --longitude 139.37787512 --altitude -7.50189438'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTime --year 2015 --month 10 --day 2 --hour 8 --minute 39 --sec 11 --usec 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setAlmanac --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setEphemeris --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTCXO --offset 700'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --GLONASS '),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
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
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),

                Step(Action.WRITE, gnss_test_device, 'setEllipsoidal --latitude 35.42936720 --longitude 139.37787512 --altitude -7.50189438'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTime --year 2015 --month 10 --day 2 --hour 8 --minute 39 --sec 11 --usec 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setAlmanac --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setEphemeris --GLONASS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTCXO --offset 700'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --GLONASS'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --warm'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 60'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
            ],
            teardown=teardown
        )


        gnss_tc_gps_l1_testing = Test(
            name='gnss_tc_gps_l1_testing',
            timeout=TIMEOUT_240,
            setup=setup,
            test=[
                # GPS L1CA L1S test
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --L1CA --L1S'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --cold'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 90'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),


                Step(Action.WRITE, gnss_test_device, 'setEllipsoidal --latitude 35.42936720 --longitude 139.37787512 --altitude -7.50189438'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTime --year 2015 --month 10 --day 2 --hour 8 --minute 39 --sec 11 --usec 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setAlmanac --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setEphemeris --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTCXO --offset 700'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --L1CA --L1S'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
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
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'saveBackupData'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getAlmanac --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'getEphemeris --GPS --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),

                Step(Action.WRITE, gnss_test_device, 'setEllipsoidal --latitude 35.42936720 --longitude 139.37787512 --altitude -7.50189438'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTime --year 2015 --month 10 --day 2 --hour 8 --minute 39 --sec 11 --usec 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setAlmanac --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setEphemeris --QZSSL1CA --num 11'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setTCXO --offset 700'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 1'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSatellite --GPS --L1CA --L1S'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setOPE --mode 1 --cycle 1000'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'start --warm'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 60'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'stop'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'setSignal --SIG_GNSS --signo 18 --enable 0'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
            ],
            teardown=teardown
        )

        gnss_tc_pvt_gps_testing = Test(
            name='gnss_tc_pvt_gps_testing',
            timeout=TIMEOUT_780,
            setup=setup,
            test=[
                # GPS test
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
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 90'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
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
                Step(Action.WRITE, gnss_test_device, 'setTime --year 2015 --month 10 --day 2 --hour 8 --minute 39 --sec 11 --usec 11'),
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
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 1 --time2 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
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
                Step(Action.WRITE, gnss_test_device, 'setTime --year 2015 --month 10 --day 2 --hour 8 --minute 39 --sec 11 --usec 11'),
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
                Step(Action.WRITE, gnss_test_device, 'start --warm'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 60'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
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

        gnss_tc_pvt_gps_glonass_testing = Test(
            name='gnss_tc_pvt_gps_glonass_testing',
            timeout=TIMEOUT_780,
            setup=setup,
            test=[
                # GPS GLONASS test
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
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 90'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
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
                Step(Action.WRITE, gnss_test_device, 'setTime --year 2015 --month 10 --day 2 --hour 8 --minute 39 --sec 11 --usec 11'),
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
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 1 --time2 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
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
                Step(Action.WRITE, gnss_test_device, 'setTime --year 2015 --month 10 --day 2 --hour 8 --minute 39 --sec 11 --usec 11'),
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
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 60'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
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

        gnss_tc_pvt_gps_l1_testing = Test(
            name='gnss_tc_pvt_gps_l1_testing',
            timeout=TIMEOUT_780,
            setup=setup,
            test=[
                # GPS L1CA L1S test
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
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 90'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
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
                Step(Action.WRITE, gnss_test_device, 'setTime --year 2015 --month 10 --day 2 --hour 8 --minute 39 --sec 11 --usec 11'),
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
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 1 --time2 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
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
                Step(Action.WRITE, gnss_test_device, 'setTime --year 2015 --month 10 --day 2 --hour 8 --minute 39 --sec 11 --usec 11'),
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
                Step(Action.WRITE, gnss_test_device, 'start --warm'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "LAT 35.25."),
                Step(Action.WAIT_FOR, gnss_test_device, "LNG 139.22."),
                Step(Action.WRITE, gnss_test_device, 'checkGnssNMEA --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosition --time 10 --value 0.9'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssPosAccuracy --time 10  --value1 0 --value2 30'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkPvtLog --time 10'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WRITE, gnss_test_device, 'checkGnssTTFF --time1 20 --time2 60'),
                Step(Action.WAIT_FOR, gnss_test_device, "PASS"),
                Step(Action.WAIT_FOR, gnss_test_device, "ttff time= "),
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


        gnss_testing_static_group = TestGroup(
            name='gnss_testing_static_group',
            devices=[gnss_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                #every test function 4min, all function need 12min
                gnss_tc_gps_testing,
                gnss_tc_gps_glonass_testing,
                gnss_tc_gps_l1_testing,
            ]
        )

        gnss_testing_dynamic_group = TestGroup(
            name='gnss_testing_dynamic_group',
            devices=[gnss_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                #every test function 4min, all function need 12min
                gnss_tc_gps_testing,
                gnss_tc_gps_glonass_testing,
                gnss_tc_gps_l1_testing,
            ]
        )

        gnss_testing_pvt_group = TestGroup(
            name='gnss_testing_pvt_group',
            devices=[gnss_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                #every test function 10min, all function need 30min
                gnss_tc_pvt_gps_testing,
                gnss_tc_pvt_gps_glonass_testing,
                gnss_tc_pvt_gps_l1_testing,
            ]
        )

        test_groups = [
            gnss_testing_pvt_group,
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
