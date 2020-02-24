#! /usr/bin/env python3
import sys
import os
import subprocess
import time
import re

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


PROJECT = 'spresense'

APPS = ['msconn', 'msdis']

APPS_TO_BUILTIN = ['usbmsc']

USBMSC_DEFCONFIG = 'defconfig/usbmsc-defconfig'

UPDATER = 'updater.py'
UPDATER_PATH = 'autotest/src/api'

TEST_OUT_PATH = '/mnt/spresense_autotest/backup/autotest_sd_picture'

SD_CARD_MOUNT_VERIFY_TIME_OUT = 10

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
            if None != err1 or output1 == b'':
                test.set_fail('usbmsc mount failed')
            else:
                str1 = output1.decode('UTF-8')
                str1Arr = str1.split()
                gmedia_path['usb'] = str1Arr[2]
                return True
            return False


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


def usbcommand(test, source, data, command):
    p3 = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
    (output3, err3) = p3.communicate()
    p3.wait()
    if None != err3 or output3 != b'':
        test.set_fail('cmd NG')

def func_mv(test, source, data, mv_type):
    print("####mvfd {} -> {}".format(mv_type.mvfdsrc,mv_type.mvfddst))
    print("####mvfl {}->{}".format(mv_type.mvflsrc,mv_type.mvfldst))
    if mv_type.mvfdsrc != None and mv_type.mvfddst != None:
        cp_folder(test, source, data, mv_type.mvfdsrc, mv_type.mvfddst)
        rm_sd_folder(test, source, data, mv_type.mvfdsrc)
    elif mv_type.mvflsrc != None and mv_type.mvfldst != None:
        cp_file(test, source, data, mv_type.mvflsrc, mv_type.mvfldst)
        rm_sd_file(test, source, data, mv_type.mvflsrc)
    else:
        test.set_fail('Parameter error')
def mv_jpg(test, source, data, file, dest_dir):
    dest_path = gmedia_path['usb'] + '/autotest/' + dest_dir
    dest_file = file.split('.')[0] + '.jpg'
    dest_path = dest_path + '/' + dest_file
    test.log.info('mv {}->{}'.format(file, dest_path))
    usbcommand(test, source, data, 'mv {}/{} {}'.format(gmedia_path['usb'], file, dest_path))
    usbcommand(test, source, data, 'sync')
    time.sleep(0)
    test.add_user_event(source, 'File {} copied'.format(file))
    return True
    

def func(test, source, data, func_type):
    func_parameter = False

    if func_type.testno == '16293':
        folderlist = os.listdir(gmedia_path['usb'] + '/autotest')
        folderlist.sort()
        testfolderlist = []

        for foldername in folderlist:
            if foldername.split("_")[1] != "16293":
                continue
            else:
                testfolderlist.append(foldername)

        i = 0
        for root, dirs, files in os.walk(gmedia_path['usb']):
            for filename in files:
                matchObj = re.match( r'PICT', filename, re.M|re.I)
                if matchObj and filename.split(".")[-1] == "JPG":
                   mv_jpg(test, source, data,filename, testfolderlist[i])
                   i=i+1

    if func_type.cpfdsrc != None and func_type.cpfddst != None:
        cp_folder(test, source, data, func_type.cpfdsrc, func_type.cpfddst)
        func_parameter = True
    
    if func_type.cpflsrc != None and func_type.cpfldst != None:
        cp_file(test, source, data, func_type.cpflsrc, func_type.cpfldst)
        func_parameter = True
    
    if func_type.mvfdsrc != None and func_type.mvfddst != None:
        cp_folder(test, source, data, func_type.mvfdsrc, func_type.mvfddst)
        rm_sd_folder(test, source, data, func_type.mvfdsrc)
        func_parameter = True
    
    if func_type.mvflsrc != None and func_type.mvfldst != None:
        cp_file(test, source, data, func_type.mvflsrc, func_type.mvfldst)
        rm_sd_file(test, source, data, func_type.mvflsrc)
        func_parameter = True
    
    if func_parameter == False:
        test.set_fail('Parameter error')

# noinspection PyUnusedLocal
def cp_file(test, source, data, file, dest_dir):
    dest_path = gmedia_path['usb'] + '/' + dest_dir
    test.log.info('cp {}->{}'.format(file, dest_path))
    usbcommand(test, source, data, 'mkdir -p {}'.format(dest_path))
    usbcommand(test, source, data, 'cp -r {} {}'.format(file, dest_path))
    usbcommand(test, source, data, 'sync')
    time.sleep(1)
    test.add_user_event(source, 'File {} copied'.format(file))
    return True

# noinspection PyUnusedLocal
def cp_folder(test, source, data, folder, dest_dir):
    SD_floder_path = gmedia_path['usb'] + '/' + folder
    test.log.info('cp -rf {}->{}'.format(SD_floder_path, dest_dir))
    #usbcommand(test, source, data, 'chmod -R 777 {}'.format(SD_floder_path))
    usbcommand(test, source, data, 'cp -rf {} {}'.format(SD_floder_path, dest_dir))
    usbcommand(test, source, data, 'sync')
    time.sleep(0)
    test.add_user_event(source, 'Folder {} copied'.format(folder))
    return True

# noinspection PyUnusedLocal
def rm_sd_folder(test, source, data, folder):
    SD_floder_path = gmedia_path['usb'] + '/' + folder
    test.log.info('rm -r {}'.format(SD_floder_path))
    usbcommand(test, source, data, 'rm -r {}'.format(SD_floder_path))
    usbcommand(test, source, data, 'sync')
    time.sleep(0)
    test.add_user_event(source, 'Folder {} deleted'.format(SD_floder_path))
    return True

# noinspection PyUnusedLocal
def rm_sd_file(test, source, data, file):
    SD_file_path = gmedia_path['usb'] + '/' + file
    test.log.info('rm {}'.format(SD_file_path))
    usbcommand(test, source, data, 'rm {}'.format(SD_file_path))
    usbcommand(test, source, data, 'sync')
    time.sleep(0)
    test.add_user_event(source, 'Folder {} deleted'.format(SD_file_path))
    return True

# noinspection PyUnusedLocal
def reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


# noinspection PyUnusedLocal
def sd_mount_monitor(test, source, data):
    mnts = None
    if source.NSH_LS in data:
        # Wait for SD card mount
        time.sleep(2)
        source.write('ls')
        mnts = data.get_value('ls')
        if 'sd0' in mnts['dirs']:
            test.storage.sdcard = True
            test.add_user_event(source, 'SD card mounted')
            cancel_timer(test)


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


class UsbMscTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(UsbMscTcRunner, self).__init__(conf)

        self.device = NuttshellDevice(kwargs.get('device', None), 'DUT_DEVICE') \
            if kwargs.get('device', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one devices are needed!')

        self.updater = os.path.join(self.config.projects[0].path, UPDATER_PATH, UPDATER)
        self.test_defconfig = os.path.join(os.path.dirname(__file__), USBMSC_DEFCONFIG)

    def get_devices_list(self):
        return [self.device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin)\
            if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.device, binaries['dut_bin'], log, APPS)

    # noinspection PyUnresolvedReferences
    def build(self, debug=True, log=None, **kwargs):
        dut_bin = kwargs.get('dut_bin', None)
        toolbox = None

        if not all([dut_bin]):
            print(not all)
            if log:
                log.info('Building project sources')

            toolbox = Toolbox(self.config, log)
            toolbox.init_builder_module()

        if not dut_bin:
            print('not dut_bin')
            log.info('Building {}'.format(self.device))
            dut_bin = self.__build_binary(toolbox, APPS_TO_BUILTIN, self.device,
                                               self.test_defconfig)

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

        def usb_s(device):
            r = umount(device) + [
                Step(Action.EXECUTE, device, None, lambda test, source, data: reset_data(test, source, data)),
                Step(Action.EXECUTE, device, None, lambda test, source, data: wait_sec(test, source, data, 5, 'wait sd')),
                Step(Action.EXECUTE, device, None, lambda test, source, data: checkDefaultDisk(test, source, data)),

                Step(Action.WRITE, device, 'msconn', terminator='\n'),
                Step(Action.WAIT_FOR, device, 'Connected',
                     lambda test, source, data: wait_sec(test, source, data, 25, 'usb mount to PC')),

                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: checkNewDisk(test, source, data)),
            ]
            return r

        def usb_e(device):
            r = [
                Step(Action.WRITE, device, 'msdis'),
                Step(Action.WAIT_FOR, device, 'Disconnected'),
            ]
            return r

        def umount(device):
            r = [
                Step(Action.WRITE, device, 'umount /mnt/sd0'),
                Step(Action.EXECUTE, device, '',
                     lambda test, source, data: wait_sec(test, source, data, 1, 'umount')),
            ]
            return r

        def setup_steps(device, reboot_monitor):
            return [
                Step(Action.WRITE, device, 'reboot'),
                Step(Action.WAIT_FOR, device, device.NUTTSHELL),
                Step(Action.EXECUTE, device, ('{}_reboot_mon'.format(str(device)), reboot_monitor),
                     Test.add_monitor),
            ]

        def setup(device, reboot_monitor):
            return setup_steps(device, reboot_monitor) +\
                       verify_sd_card_mount(device)

        def verify_sd_card_mount(device):
            return [
                Step(Action.EXECUTE, device, ('{}_sd_mount_monitor'.format(str(device)),
                                              sd_mount_monitor), Test.add_monitor),
                Step(Action.WRITE, device, 'cd /mnt'),
                Step(Action.WRITE, device, 'ls'),
                Step(Action.EXECUTE, device, SD_CARD_MOUNT_VERIFY_TIME_OUT, start_timer),
                Step(Action.WAIT_FOR, device, 'SD card mounted'),
                Step(Action.EXECUTE, device, '{}_sd_mount_monitor'.format(str(device)),
                     Test.remove_monitor),
            ]

        def teardown(device):
            return [
                Step(Action.EXECUTE, device, '{}_reboot_mon'.format(str(device)),
                     Test.remove_monitor),
            ]

        tc_usbmsc = Test(
            name='tc_usbmsc',
            timeout=timeout,
            setup=setup(device, reboot_monitor) + usb_s(device),
            test=[
                  Step(Action.EXECUTE, device, None,
                        lambda test, source, data: func(test, source, data, arguments)),
            ],
            teardown=teardown(device), 
        )
        func_move = TestGroup(
            name='tg',
            devices=[device],
            tag=[Tag.POSITIVE],
            tests=[tc_usbmsc],
        )

        test_groups = [
            func_move,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()

    parser.add_argument('--cpfdsrc', help='copy folder source')
    parser.add_argument('--cpfddst', help='copy folder dest')
    parser.add_argument('--cpflsrc', help='copy file source')
    parser.add_argument('--cpfldst', help='copy file dest')
    parser.add_argument('--mvfdsrc', help='move folder source')
    parser.add_argument('--mvfddst', help='move folder dest')
    parser.add_argument('--mvflsrc', help='move file source')
    parser.add_argument('--mvfldst', help='move file dest')
    parser.add_argument('--testno', help='test link no')

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
    tc_runner = UsbMscTcRunner(config, device=device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
