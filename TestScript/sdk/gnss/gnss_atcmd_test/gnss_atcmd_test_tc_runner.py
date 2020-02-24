#! /usr/bin/env python3
import sys
import os
import re
import time
import serial

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.gnss_atcmd_test.gnss15973_test_device import Gnss15973TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser
GPGSV_count = 0
QZGSV_count = 0
pos_count = 0
nema_gga_count = 0
nema_gll_count = 0

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['gnss_atcmd']

def wait_sec(test, source, data, sec, description):
    if description is not None:
        test.log.info("")
        test.log.info("#############################################")
        test.log.info("## [" + str(sec) + " sec] "+ description)
        test.log.info("#############################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    global pos_count
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
    elif 'GGA' in data:
        split_str = ","
        dataArray = re.split(split_str, data)
        lat = dataArray[2][0:2]
        lng = dataArray[4][0:3]
        if len(lat) != 0 and len(lng) != 0:
            if int(lat) == 35 and int(lng) == 139:
                pos_count = pos_count + 1
        if pos_count > 10:
            test.add_user_event(source, 'gnss test OK')

def gps_fix_quality_2_monitor(test, source, data):
    if 'GGA' in data:
        split_str = ","
        dataArray = re.split(split_str, data)
        fq = dataArray[6]
        if len(fq) != 0:
            if int(fq) != 2:
                test.set_fail('fix_quality is not 2')

def gps_fix_quality_not2_monitor(test, source, data):
    if 'GGA' in data:
        split_str = ","
        dataArray = re.split(split_str, data)
        fq = dataArray[6]
        if len(fq) != 0:
            if int(fq) == 2:
                test.set_fail('fix_quality is 2')

def gps_log_monitor(test, source, data):
    global QZGSV_count
    global GPGSV_count
    if 'GPGSV' in data:
        GPGSV_count = GPGSV_count +1
    elif 'QZGSV' in data:
        test.set_fail("GLGSV")
    elif 'GLGSV' in data:
        test.set_fail("GLGSV")
    if GPGSV_count > 10:
        GPGSV_count = 0
        test.add_user_event(source, 'gps OK')
    if QZGSV_count > 10:
        QZGSV_count = 0
        test.add_user_event(source, 'gz OK')

def gps_qz_log_monitor(test, source, data):
    global QZGSV_count
    global GPGSV_count
    if 'GPGSV' in data:
        GPGSV_count = GPGSV_count +1
    elif 'QZGSV' in data:
        QZGSV_count = QZGSV_count +1
    elif 'GLGSV' in data:
        test.set_fail("GLGSV")
    if GPGSV_count > 10:
        GPGSV_count = 0
        test.add_user_event(source, 'gps OK')
    if QZGSV_count > 10:
        QZGSV_count = 0
        test.add_user_event(source, 'qz OK')


def gps_gln_log_monitor(test, source, data):
    global QZGSV_count
    global GPGSV_count
    if 'GPGSV' in data:
        GPGSV_count = GPGSV_count +1
    elif 'QZGSV' in data:
        test.set_fail("GLGSV")
    if GPGSV_count > 10:
        GPGSV_count = 0
        test.add_user_event(source, 'gps OK')
    if QZGSV_count > 10:
        QZGSV_count = 0
        test.add_user_event(source, 'gz OK')

def gps_gln_qz_log_monitor(test, source, data):
    global QZGSV_count
    global GPGSV_count
    if 'GPGSV' in data:
        GPGSV_count = GPGSV_count +1
    elif 'QZGSV' in data:
        QZGSV_count = QZGSV_count +1
    if GPGSV_count > 10:
        GPGSV_count = 0
        test.add_user_event(source, 'gps OK')
    if QZGSV_count > 10:
        QZGSV_count = 0
        test.add_user_event(source, 'qz OK')


def resetCount(test, source, data):
    global QZGSV_count
    global GPGSV_count
    global pos_count
    global nema_gga_count
    global nema_gll_count
    QZGSV_count = 0
    GPGSV_count = 0
    pos_count = 0
    nema_gll_count = 0
    nema_gga_count = 0

def checkStep(test, source, data, value):
    test.log.info(value)

def call_shell(test, source, data, command):
    test.log.info("start")
    os.system(command)
    test.log.info("end")

class GnssTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(GnssTestTcRunner, self).__init__(conf)

        self.gnss15973_test2_device = Gnss15973TestDevice(
            kwargs.get('dut2_device', None), 'GNSS15973_TEST2_DEVICE') if kwargs.get('dut2_device', None)\
            else None
        self.gnss15973_test_device = Gnss15973TestDevice(
            kwargs.get('dut_device', None), 'GNSS15973_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')


        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/gnss_atcmd-defconfig')

    def get_devices_list(self):
        return [self.gnss15973_test2_device, self.gnss15973_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.gnss15973_test2_device, binaries['dut_bin'], log)
        usb0 = serial.Serial(self.gnss15973_test2_device.serial, timeout=1, baudrate=115200)
        time.sleep(1)
        data = "reboot"
        usb0.write(data.encode('utf-8') + b'\r\n')
        time.sleep(5)
        data = "gnss_atcmd"
        usb0.write(data.encode('utf-8') + b'\r\n')
        time.sleep(5)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        data = usb0.readline().decode("Cp1252", errors="ignore")
        log.info(data)
        if 'Successfully registered the serial driver' in data:
            log.info('open main tty OK')
            time.sleep(20)
        else:
            log.info('open main tty NG')

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.gnss15973_test2_device)

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
        timeout = 300

        gnss15973_test2_device, gnss15973_test_device = self.get_devices_list()

        setup = [
            Step(Action.EXECUTE, gnss15973_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
            Step(Action.EXECUTE, gnss15973_test_device, None,
                lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
            Step(Action.EXECUTE, gnss15973_test_device, None,
                lambda test, source, data: wait_sec(test, source, data, 5, "waiting")),
            Step(Action.EXECUTE, gnss15973_test_device, None,
                lambda test, source, data: call_shell(test, source, data, "./labsat_start.sh")),
        ]

        setup2 = [
            Step(Action.EXECUTE, gnss15973_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
            Step(Action.EXECUTE, gnss15973_test_device, None,
                lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
            Step(Action.EXECUTE, gnss15973_test_device, None,
                lambda test, source, data: wait_sec(test, source, data, 5, "waiting")),
            Step(Action.EXECUTE, gnss15973_test_device, None,
                lambda test, source, data: call_shell(test, source, data, "./labsat_start2.sh")),
        ]

        teardown = [
            Step(Action.EXECUTE, gnss15973_test_device, 'dut_reboot_mon', Test.remove_monitor),
            Step(Action.EXECUTE, gnss15973_test_device, None,
                lambda test, source, data: call_shell(test, source, data, "./labsat_end.sh")),
        ]

        SDK_Gnss_15851_nema = Test(
            name='SDK_Gnss_15851_nema',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x01'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WRITE, gnss15973_test_device, '@GCD'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GGA'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GLL'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GGA'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GLL'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GGA'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GLL'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GGA'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GLL'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GGA'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GLL'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GGA'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GLL'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GGA'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'GLL'),
                Step(Action.WRITE, gnss15973_test_device, '@GSTP'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
            ],
            teardown=teardown
        )

        SDK_Gnss_15973_gps = Test(
            name='SDK_Gnss_15973_gps',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x01'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WRITE, gnss15973_test_device, '@GCD'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.EXECUTE, gnss15973_test_device, ('gps_log_mon', gps_log_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gps OK'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gnss test OK'),
                Step(Action.EXECUTE, gnss15973_test_device, 'gps_log_mon', Test.remove_monitor),
                Step(Action.WRITE, gnss15973_test_device, '@GSTP'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
            ],
            teardown=teardown
        )

        SDK_Gnss_15973_qz = Test(
            name='SDK_Gnss_15973_qz',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x01'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WRITE, gnss15973_test_device, '@GCD'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 2, "waiting")),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x29'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.EXECUTE, gnss15973_test_device, ('gps_qz_log_mon', gps_qz_log_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gps OK'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'qz OK'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gnss test OK'),
                Step(Action.EXECUTE, gnss15973_test_device, 'gps_qz_log_mon', Test.remove_monitor),
                Step(Action.WRITE, gnss15973_test_device, '@GSTP'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
            ],
            teardown=teardown
        )

        SDK_Gnss_15974 = Test(
            name='SDK_Gnss_15974',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x01'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WRITE, gnss15973_test_device, '@GCD'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gnss test OK'),

                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.EXECUTE, gnss15973_test_device, ('gps_log_mon', gps_log_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gps OK'),
                Step(Action.EXECUTE, gnss15973_test_device, 'gps_log_mon', Test.remove_monitor),

                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x29'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.EXECUTE, gnss15973_test_device, ('gps_qz_log_mon', gps_qz_log_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gps OK'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'qz OK'),
                Step(Action.EXECUTE, gnss15973_test_device, 'gps_qz_log_mon', Test.remove_monitor),

                Step(Action.WRITE, gnss15973_test_device, '@GSTP'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
            ],
            teardown=teardown
        )


        SDK_Gnss_15975_gps_gln = Test(
            name='SDK_Gnss_15975_gps_gln',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x03'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WRITE, gnss15973_test_device, '@GCD'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.EXECUTE, gnss15973_test_device, ('gps_gln_log_mon', gps_gln_log_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gps OK'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gnss test OK'),
                Step(Action.EXECUTE, gnss15973_test_device, 'gps_gln_log_mon', Test.remove_monitor),
                Step(Action.WRITE, gnss15973_test_device, '@GSTP'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
            ],
            teardown=teardown
        )

        SDK_Gnss_15975_gps_gln_qz = Test(
            name='SDK_Gnss_15975_gps_gln_qz',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x03'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WRITE, gnss15973_test_device, '@GCD'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 2, "waiting")),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x2B'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.EXECUTE, gnss15973_test_device, ('gps_gln_qz_log_mon', gps_gln_qz_log_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gps OK'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'qz OK'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gnss test OK'),
                Step(Action.EXECUTE, gnss15973_test_device, 'gps_gln_qz_log_mon', Test.remove_monitor),
                Step(Action.WRITE, gnss15973_test_device, '@GSTP'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
            ],
            teardown=teardown
        )


        SDK_Gnss_15976 = Test(
            name='SDK_Gnss_15976',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x03'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WRITE, gnss15973_test_device, '@GCD'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gnss test OK'),

                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.EXECUTE, gnss15973_test_device, ('gps_gln_log_mon', gps_gln_log_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gps OK'),
                Step(Action.EXECUTE, gnss15973_test_device, 'gps_gln_log_mon', Test.remove_monitor),

                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x2B'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.EXECUTE, gnss15973_test_device, ('gps_gln_qz_log_mon', gps_gln_qz_log_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gps OK'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'qz OK'),
                Step(Action.EXECUTE, gnss15973_test_device, 'gps_gln_qz_log_mon', Test.remove_monitor),

                Step(Action.WRITE, gnss15973_test_device, '@GSTP'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
            ],
            teardown=teardown
        )

        SDK_Gnss_15977_gln = Test(
            name='SDK_Gnss_15977_gln',
            timeout=timeout,
            setup=setup2,
            test=[
                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x02'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WRITE, gnss15973_test_device, '@GCD'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.EXECUTE, gnss15973_test_device, ('gps_fix_quality_not2_mon', gps_fix_quality_not2_monitor),
                     Test.add_monitor),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gnss test OK'),
                Step(Action.EXECUTE, gnss15973_test_device, 'gps_fix_quality_not2_mon', Test.remove_monitor),
                Step(Action.WRITE, gnss15973_test_device, '@GSTP'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
            ],
            teardown=teardown
        )

        SDK_Gnss_15978_gln = Test(
            name='SDK_Gnss_15978_gln',
            timeout=timeout,
            setup=setup2,
            test=[
                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: resetCount(test, source, data)),
                Step(Action.WRITE, gnss15973_test_device, '@GNS 0x02'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WRITE, gnss15973_test_device, '@GCD'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'gnss test OK'),
                Step(Action.EXECUTE, gnss15973_test_device, ('gps_fix_quality_not2_mon', gps_fix_quality_not2_monitor),
                     Test.add_monitor),
                Step(Action.EXECUTE, gnss15973_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 20, "waiting")),
                Step(Action.EXECUTE, gnss15973_test_device, 'gps_fix_quality_not2_mon', Test.remove_monitor),
                Step(Action.WRITE, gnss15973_test_device, '@GSTP'),
                Step(Action.WAIT_FOR, gnss15973_test_device, 'Done'),
            ],
            teardown=teardown
        )

        gnss = TestGroup(
            name='gnss',
            devices=[gnss15973_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Gnss_15851_nema,
                SDK_Gnss_15973_gps,
                SDK_Gnss_15973_qz,
                SDK_Gnss_15974,
                SDK_Gnss_15975_gps_gln,
                SDK_Gnss_15975_gps_gln_qz,
                SDK_Gnss_15976,
                SDK_Gnss_15977_gln,
                SDK_Gnss_15978_gln,
            ]
        )

        test_groups = [
            gnss,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()

    parser.add_argument('--master', metavar='SERIAL_PORT', help='Set master')
    parser.add_argument('--slave', metavar='SERIAL_PORT', help='Set slave')

    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    dut2_device, dut_device = dev_manager.get_devices_by_serials(args.master, args.slave)

    # Create test runner instance
    tc_runner = GnssTestTcRunner(config, dut2_device=dut2_device, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)