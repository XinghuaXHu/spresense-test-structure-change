#! /usr/bin/env python3
import sys
import os
import re
import datetime
import time
import subprocess

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.sdhci16295_arduino_example_test.sdhci16295_arduino_example_test_device import SDHCI16295TestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['SDHCI']
global gmedia_dict
gmedia_dict = {}
global gmedia_dict2
gmedia_dict2 = {}
global gmedia_path
gmedia_path = {}

def reset_data(test, source, event):
    gmedia_path.clear()
    gmedia_dict2.clear()
    gmedia_dict.clear()

def wait_sec(test, source, data, sec, description=None):
    if description is not None:
        test.log.info("")
        test.log.info("############################################################")
        test.log.info("## [{}sec] {}".format(str(sec), description))
        test.log.info("############################################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')
    return True

def checkDefaultDisk(test, source, data):
    p = subprocess.Popen("ls /dev/sd*", stdout=subprocess.PIPE, shell=True)
    (output, err) = p.communicate()
    p.wait()
    outputstr = output.decode('UTF-8')
    dataArr = outputstr.split()
    test.log.info(dataArr)
    for i in dataArr:
        gmedia_dict[i] = i

def checkNewDisk(test, source, data):
    p0 = subprocess.Popen("ls /dev/sd*", stdout=subprocess.PIPE, shell=True)
    (output0, err0) = p0.communicate()
    p0.wait()
    str0 = output0.decode('UTF-8')
    str0Arr = str0.split()
    test.log.info(str0Arr)
    for i in str0Arr:
        gmedia_dict2[i] = i

    for key2 in gmedia_dict2.keys():
        key2_ount = 0
        for key in gmedia_dict.keys():
            if key == key2:
                key2_ount += 1
        if key2_ount == 0 and len(key2) == 9:
            mountcmd = './mount.bash {} /tmp/sd0'.format(key2)
            p3 = subprocess.Popen(mountcmd, stdout=subprocess.PIPE, shell=True)
            (output3, err3) = p3.communicate()
            p3.wait()
            mountcmd = 'mount | grep ' + key2
            p1 = subprocess.Popen(mountcmd, stdout=subprocess.PIPE, shell=True)
            (output1, err1) = p1.communicate()
            p1.wait()
            str1 = output1.decode('UTF-8')
            str1Arr = str1.split()
            gmedia_path['usb'] = str1Arr[2]
            return True

def usbcommand(test, source, data, command):
    p3 = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
    (output3, err3) = p3.communicate()
    p3.wait()
    if None != err3 or output3 != b'':
        test.set_fail('cmd NG')

def usb_test(test, source, data):
    filechangecmd = "echo test12355 >> " + gmedia_path['usb'] + '/test123.txt'
    test.log.info(filechangecmd)
    p2 = subprocess.Popen(filechangecmd, stdout=subprocess.PIPE, shell=True)
    (output2, err2) = p2.communicate()
    p2.wait()
    if None == err2 and b'' == output2:
        test.log.info("file change ok")
    else:
        test.set_fail('file change NG')
    lscmd = 'ls ' + gmedia_path['usb']
    p3 = subprocess.Popen(lscmd, stdout=subprocess.PIPE, shell=True)
    (output3, err3) = p3.communicate()
    p3.wait()
    str3 = output3.decode('UTF-8')
    #test.log.info(str3)
    fileslist = str3.split()
    for file in fileslist:
        if 'test123.txt' == file:
            test.add_user_event(source, 'test done')
            break

class SDHCITestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(SDHCITestTcRunner, self).__init__(conf)

        self.sdhci16295_test_device = SDHCI16295TestDevice(
            kwargs.get('dut_device', None), 'SDHCI16295_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/sdhci-defconfig')

    def get_devices_list(self):
        return [self.sdhci16295_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.sdhci16295_test_device)

        return dict(dut_bin=dut_bin)

    @staticmethod
    def __build_binary(toolbox, defconfig, device):
        bin_file = toolbox.builder.build_project(PROJECT, APPS_TO_BUILTIN, device, defconfig)
        return os.path.normpath(bin_file)

    # noinspection PyMethodMayBeStatic
    def flash_n_check(self, device, dut_bin, log=None):
        device.check_device_config(str(device), APPS_TO_BUILTIN, log)

        if os.path.basename(dut_bin).startswith(str(device)):
            try:
                os.remove(dut_bin)
            except OSError as e:
                log.error(e)
                raise

    def generate_test_groups(self, arguments, log=None):
        timeout = 30

        sdhci16295_test_device = self.get_devices_list()[0]

        setup = [
        ]

        teardown = [
        ]

        Arduino_SDHCI_16295 = Test(
            name='Arduino_SDHCI_16295',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, sdhci16295_test_device, None,
                     lambda test, source, data: reset_data(test, source, data)),
                Step(Action.EXECUTE, sdhci16295_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, 'wait sd0')),
                Step(Action.EXECUTE, sdhci16295_test_device, None,
                     lambda test, source, data: checkDefaultDisk(test, source, data)),
                Step(Action.WAIT_FOR, sdhci16295_test_device, '*** USB MSC Prepared! ***'),
                Step(Action.WAIT_FOR, sdhci16295_test_device, 'Insert SD and Connect Extension Board USB to PC.'),
                Step(Action.EXECUTE, sdhci16295_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 15, 'wait usb mount')),

                Step(Action.EXECUTE, sdhci16295_test_device, None,
                     lambda test, source, data: checkNewDisk(test, source, data)),

                Step(Action.EXECUTE, sdhci16295_test_device, None,
                     lambda test, source, data: usb_test(test, source, data)),

                Step(Action.WAIT_FOR, sdhci16295_test_device, 'test done'),
            ],
            teardown=teardown
        )

        sdhci_gp = TestGroup(
            name='sdhci_gp',
            devices=[sdhci16295_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                Arduino_SDHCI_16295,
            ]
        )

        test_groups = [
            sdhci_gp,
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
    tc_runner = SDHCITestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
