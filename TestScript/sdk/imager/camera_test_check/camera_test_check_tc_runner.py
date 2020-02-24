#! /usr/bin/env python3
import sys
import os
import subprocess
import time
import datetime

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from api.nuttshell_device import *
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger, Timer
from api.config import Config
from api.runner_parser import RunnerParser
from utilities.xmllog import xmllog


PROJECT = 'spresense'

UPDATER = 'updater.py'
UPDATER_PATH = 'autotest/src/api'

TEST_CAMERA_DATA_PATH = '/WORK/Spresense/backup/autotest_sd_picture'
TEST_OUT_PATH = '/mnt/spresense_autotest_win'
LOG_XML_PATH = '/mnt/spresense_autotest_win'
CAMERA_TEST_BACKUP_PATH = '/WORK/Spresense/backup'

XML_LOG_CHECK_TIME_OUT = 10

global backup_folder

def check_xmllog(test, source, data, testno, class_xmllog, path):
    test.log.info('testno {}'.format(testno))
    class_xmllog.readXmlLogDir(path,XML_LOG_CHECK_TIME_OUT)
    test.log.info('{} test {}'.format(path,class_xmllog.getResult()))
    return True

def test_result(test, source, data, testno, class_xmllog):
    if 'fail' == class_xmllog.getResult():
        test.set_fail('TEST NO {} FAIL'.format(testno))
    else:
        test.log.info('TEST NO {} SUCCESS'.format(testno))
    return True
def compress_backup(test, source, data):
    global backup_folder
    
    test.log.info('tar -zcvf {}/{}.tar.gz {}/{}'.format(CAMERA_TEST_BACKUP_PATH,backup_folder, CAMERA_TEST_BACKUP_PATH,backup_folder))
    usbcommand(test, source, data, 'tar -zcvf {}/{}.tar.gz {}/{}'.format(CAMERA_TEST_BACKUP_PATH,backup_folder, CAMERA_TEST_BACKUP_PATH,backup_folder))
    usbcommand(test, source, data, 'sync')

    test.log.info('rm -rf {}/{}'.format(CAMERA_TEST_BACKUP_PATH,backup_folder))
    usbcommand(test, source, data, 'rm -rf {}/{}'.format(CAMERA_TEST_BACKUP_PATH,backup_folder))
    usbcommand(test, source, data, 'sync')

    test.add_user_event(source, 'compress_backup complete')
    
def mkdir_backup(test, source, data, testcase):
    global backup_folder
    
    test.storage.backup_folder = []
    test.storage.backup_folder = '{}'.format(datetime.datetime.now().strftime("%Y%m%d"))
    backup_folder = test.storage.backup_folder

    time.sleep(0)
    usbcommand(test, source, data, 'mkdir -p {}/{}'.format(CAMERA_TEST_BACKUP_PATH , test.storage.backup_folder))
    usbcommand(test, source, data, 'sync')
    time.sleep(0)
    usbcommand(test, source, data, 'mkdir -p {}/{}/{}'.format(CAMERA_TEST_BACKUP_PATH , test.storage.backup_folder , testcase))
    usbcommand(test, source, data, 'sync')
    time.sleep(0)    
    test.add_user_event(source, 'mkdir done')
    return True

def mv_backup(test, source, data, testcase, filepath, file):
    dest_path = CAMERA_TEST_BACKUP_PATH + '/' + test.storage.backup_folder + '/' + testcase
    usbcommand(test, source, data, 'mkdir -p {}'.format(dest_path))

    if 'log' == file:
        filelist = os.listdir(LOG_XML_PATH + filepath + '/' + file)
        if filelist.count == 0:
            test.set_fail('TEST NO {} NO CHECK LOG FILE'.format(testcase))
            return False

        file_path = LOG_XML_PATH + filepath + '/' + file + '/*'
    else:
        file_path = TEST_OUT_PATH + filepath + '/' + file
    
    test.log.info('mv {}->{}'.format(file_path, dest_path + '/'))
    usbcommand(test, source, data, 'mv {} {}'.format(file_path, dest_path + '/'))
    usbcommand(test, source, data, 'sync')
    time.sleep(0)
    test.add_user_event(source, 'File {} moved'.format(file))
    return True
def usbcommand(test, source, data, command):
    p3 = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
    (output3, err3) = p3.communicate()
    p3.wait()
    if None != err3 or output3 != b'':
        test.log.info('cmd err {}'.format(err3))

def timer_callback(test):
    test.storage.sdcard = False
    test.set_fail('No SD card mounted!')


# noinspection PyUnusedLocal
def start_timer(test, source, timeout):
    test.storage.timer = Timer(timeout, timer_callback, [test])
    test.storage.timer.start()


# noinspection PyUnusedLocal
def cancel_timer(test):
    if test.storage.timer:
        test.storage.timer.cancel()


def end_test(test, source, data):
    test.set_fail('Failed to transfer file')


# noinspection PyUnusedLocal
def execute_command(test, source, data, command, confirm):
    source.write(command)
    if confirm:
        test.add_user_event(source, 'Command {} executed'.format(command))
    return True


# noinspection PyUnusedLocal
def create_dirs(test, source, data, dirs, path):
    ls = data.get_value('ls')
    if ls['path'] == path:
        for directory in dirs:
            if directory not in ls['dirs']:
                source.write('mkdir {}'.format(directory))
        test.add_user_event(source, 'Directories created')
        return True


# noinspection PyUnusedLocal
def clean_dir(test, source, data):
    ls = data.get_value('ls')

    for file in ls['files']:
        source.write('rm {}'.format(file))
    test.add_user_event(source, 'Files removed from {}'.format(ls['path'].split('/')[-2]))
    return True


# noinspection PyUnusedLocal
def rm_dir(test, source, data):
    ls = data.get_value('ls')

    if not ls['files']:
        source.write('cd ..')
        source.write('rmdir {}'.format(ls['path'].split('/')[-2]))
        test.add_user_event(source, '{} folder removed'.format(ls['path'].split('/')[-2]))
    return True


# noinspection PyUnusedLocal
def remove_transferred_file(test, source, data):
    if hasattr(test.storage, 'rec_file_path'):
        file_name = test.storage.rec_file_path
        try:
            os.remove(os.path.join(os.getcwd(), 'output', file_name))
        except:
            test.log.debug('Failed to remove {}'.format(file_name))


# noinspection PyUnusedLocal
def trigger_event(test, source, event):
    test.add_user_event(source, event)


# noinspection PyUnusedLocal
def trigger_cleaning(test, source, event):
    if test.storage.sdcard:
        trigger_event(test, source, event)
    else:
        test.set_fail('No SD card mounted!')

def cp_folder(test, source, data, foldername):
    God_floder_path = TEST_CAMERA_DATA_PATH + '/' + foldername
    Dest_floder_path = TEST_OUT_PATH + '/'
    test.log.info('cp -rf {}->{}'.format(God_floder_path, Dest_floder_path))
    #usbcommand(test, source, data, 'chmod -R 777 {}'.format(SD_floder_path))
    usbcommand(test, source, data, 'cp -rf {} {}'.format(God_floder_path, Dest_floder_path))
    usbcommand(test, source, data, 'sync')
    time.sleep(0)
    test.add_user_event(source, 'Folder {} copied'.format(foldername))
    return True

class CameraTestCheckTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(CameraTestCheckTcRunner, self).__init__(conf)

        self.device = NuttshellDevice(kwargs.get('device', None), 'DUT_DEVICE') \
            if kwargs.get('device', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one devices are needed!')

        self.updater = os.path.join(self.config.projects[0].path, UPDATER_PATH, UPDATER)
        self.read_xmllog = xmllog()

    def get_devices_list(self):
        return [self.device]

    def get_read_xmllog_class(self):
        return self.read_xmllog

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin)\
            if arguments.dut_bin else None

        #binaries = self.build(debug, log, **binaries)

        #self.flash_n_check(self.device, binaries['dut_bin'], log, APPS)

    # noinspection PyUnresolvedReferences
    def build(self, debug=True, log=None, **kwargs):
        dut_bin = kwargs.get('dut_bin', None)
        toolbox = None

        return dict(dut_bin=dut_bin)

    @staticmethod
    def __build_binary(toolbox, apps, device, defconfig):
        bin_file = toolbox.builder.build_project(PROJECT, apps, device, defconfig)
        return os.path.normpath(bin_file)

    # noinspection PyMethodMayBeStatic
    def flash_n_check(self, device, bin_path, log=None, apps=None):
        if log:
            log.info('Flashing ' + str(device))

        device.flash(str(device), log, bin_path)
        device.check_device_config(str(device), apps, log)

        if os.path.basename(bin_path).startswith(str(device)):
            try:
                os.remove(bin_path)
            except OSError as e:
                log.error(e)
                raise

    def generate_test_groups(self, arguments, log=None):
        timeout = 1200

        device = self.get_devices_list()[0]
        func_xmllog = self.get_read_xmllog_class()


        def camera_picture_check(device, testcase, testcase_folder, filename):
            r =[
                   Step(Action.EXECUTE, device, None, 
                      lambda test, source, data: cp_folder(test, source, data, testcase_folder)),

                   Step(Action.EXECUTE, device, None, 
                      lambda test, source, data: check_xmllog(test, source, data, testcase, func_xmllog, LOG_XML_PATH + '/' + testcase_folder + '/log')),

                   Step(Action.EXECUTE, device, None, 
                      lambda test, source, data: mkdir_backup(test, source, data, testcase_folder)),
                   Step(Action.WAIT_FOR, device, 'mkdir done'),
                   Step(Action.EXECUTE, device, None, 
                      lambda test, source, data: mv_backup(test, source, data, testcase_folder, '/' + testcase_folder, filename)),
                   Step(Action.WAIT_FOR, device, 'moved'),
                   Step(Action.EXECUTE, device, None, 
                      lambda test, source, data: mv_backup(test, source, data, testcase_folder, '/' + testcase_folder, 'log')),
                   Step(Action.WAIT_FOR, device, 'moved'),

                   Step(Action.EXECUTE, device, None, 
                      lambda test, source, data: test_result(test, source, data, testcase, func_xmllog)),
            ]
            return r

        setup = [
        ]

        teardown = [
        ]

        folderlist = os.listdir(TEST_CAMERA_DATA_PATH)
        folderlist.sort()

        tests = []
        for foldername in folderlist:
            if foldername.split("_")[0] != "Test":
                continue

            testcase = foldername.split("_", 1)[-1]
            folderpath = TEST_CAMERA_DATA_PATH + '/' + foldername
            test_filename = ""
            print("1# {} # {} # {}".format(testcase, foldername, test_filename))    
            for root, dirs, files in os.walk(folderpath):
                for filename in files:
                    if filename.split(".")[0] != "god" and filename.split(".",1)[-1] != "yuv.jpg":
                       test_filename = filename
                       break
                break

            print("2# {} # {} # {}".format(testcase, foldername, test_filename))    

            tests.append(Test(
                name='ADN_Camera_' + testcase + '_check',
                timeout=300,
                setup=setup,
                test=camera_picture_check(device, testcase, foldername, test_filename),
                teardown= teardown
            ))

        Camera_Test_Backup_Compress =Test(
            name='Camera_Test_Backup_Compress',
            timeout=300,
            setup=setup,
            test=[
                Step(Action.EXECUTE, device, None, 
                     lambda test, source, data: compress_backup(test, source, data)),
                Step(Action.WAIT_FOR, device, 'compress_backup complete'),
            ],
            teardown= teardown
        )

        test_group_check_backup = TestGroup(
            name='test_group_check_backup',
            devices=[device],
            tag=[Tag.POSITIVE],
            tests=tests
        )
        test_group_compress = TestGroup(
            name='test_group_compress',
            devices=[device],
            tag=[Tag.POSITIVE],
            tests=[
                    Camera_Test_Backup_Compress, 
            ]
        )

        test_groups = [
            test_group_check_backup,
            test_group_compress
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
    device = dev_manager.get_devices_by_serials(args.dut_device)[0]

    # Create test runner instance
    tc_runner = CameraTestCheckTcRunner(config, device=device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
