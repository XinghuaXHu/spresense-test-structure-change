#! /usr/bin/env python3
import sys
import os
import re
import time

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.camera_sdk_example_test.camera_sdk_example_test_device import CameraSdkExampleTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['camera']

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
def mkdir_autotest(test, source, data):
    test.storage.still_dirname = []
    #time.sleep(1)
    #source.write('mkdir /mnt/sd0/autotest')
    #time.sleep(1)
    return True

def mkdir_still(test, source, data, number=0):
    test.storage.still_file_path = []
    foldername = 'Test_{}_{}'.format(test.name.split("_")[-1].strip(" "), number)
    test.storage.still_dirname.append(foldername)

def store_still_file_path(test, source, data):
    test.storage.still_file_path.append(data.get_value(source.KEY_FILE_PATH))
    return True

def mv_still_file(test, source, data):
    for path in test.storage.still_file_path:
        if isinstance(test.storage.still_dirname,list):
           folder = test.storage.still_dirname[0]
           test.storage.still_dirname.pop(0)
        else:
            folder = test.storage.still_dirname

        target = '{}/autotest/{}'.format("/mnt/sd0", folder)
        filename = os.path.basename(path).split('.')[0] + '.yuv'
        source.write('cp {} {}/{}'.format(path, target, filename))
        print('mv {} {}/{}'.format(path, target, filename))
        time.sleep(2)
        source.write('rm {}'.format(path))
        time.sleep(1)

    test.storage.still_file_path.clear()

class CameraSdkExampleTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(CameraSdkExampleTestTcRunner, self).__init__(conf)

        self.camera_example_test_device = CameraSdkExampleTestDevice(
            kwargs.get('dut_device', None), 'CAMERA_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(self.config.projects[0].path, 'sdk/configs/examples/camera-defconfig')

    def get_devices_list(self):
        return [self.camera_example_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.camera_example_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.camera_example_test_device)

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

        camera_test_device = self.get_devices_list()[0]

        def setup_mkdir_still(count=0,offset=0):
            r = []
            r = r + [
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: mkdir_autotest(test, source, data)),
                #Step(Action.WAIT_FOR, camera_test_device, camera_test_device.NSH_PROMPT),
                #Step(Action.WAIT_FOR, camera_test_device, camera_test_device.NSH_PROMPT),
            ]
            if count == 0:
               r = r + [
                 Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: mkdir_still(test, source, data)),
                 #Step(Action.WAIT_FOR, camera_test_device, camera_test_device.NSH_PROMPT),
                 #Step(Action.WAIT_FOR, camera_test_device, camera_test_device.NSH_PROMPT),
               ]
            else:
               for number in range(count):
                    r = r + [
                        Step(Action.EXECUTE, camera_test_device, None,
                            lambda test, source, data, val=number+offset+1: mkdir_still(test, source, data, val)),
                        #Step(Action.WAIT_FOR, camera_test_device, camera_test_device.NSH_PROMPT),
                        #Step(Action.WAIT_FOR, camera_test_device, camera_test_device.NSH_PROMPT),
                    ]

            return r
        def run_video_capture():
            r = []

            for i in range(10):
                r.append(
                    Step(Action.WAIT_FOR, camera_test_device, camera_test_device.STILL_DATA,
                         lambda test, source, data: store_still_file_path(test, source, data)),
                )
            r = r + [
                #Step(Action.WAIT_FOR, camera_test_device, 'video capture finish'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: mv_still_file(test, source, data)),
            ]
            return r

        setup = [
            Step(Action.WRITE, camera_test_device, 'reboot'),
            Step(Action.WAIT_FOR, camera_test_device, camera_test_device.NUTTSHELL),
            Step(Action.EXECUTE, camera_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
            Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: mkdir_autotest(test, source, data)),
        ]

        teardown = [
            Step(Action.EXECUTE, camera_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        camera_sdk_15840 = Test(
            name='camera_sdk_15840',
            timeout=timeout,
            setup=setup + setup_mkdir_still(10),
            test=[
                Step(Action.WRITE, camera_test_device, 'camera'),
            ] +  run_video_capture(),
            teardown=teardown
        )

        camera = TestGroup(
            name='camera',
            devices=[camera_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                camera_sdk_15840,
            ]
        )

        test_groups = [
            camera,
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
    tc_runner = CameraSdkExampleTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
