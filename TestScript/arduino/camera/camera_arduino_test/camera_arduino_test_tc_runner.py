#! /usr/bin/env python3
import sys
import os
import time
import datetime
import subprocess

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.camera_arduino_test.camera_arduino_test_device import CameraArduinoTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser
from utilities.xmllog import xmllog


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['camera']

TEST_OUT_PATH = '/home/spr/WORK/autotest'
LOG_XML_PATH = '/home/spr/WORK/autotest'
CAMERA_TEST_BACKUP_PATH = '/home/spr/WORK/autotest/backup'
XML_LOG_CHECK_TIME_OUT = 5

global gmedia_dict
gmedia_dict = {}
global gmedia_dict2
gmedia_dict2 = {}
global gmedia_path
gmedia_path = {}
global awb_mode_file_path
awb_mode_file_path = []
global still_capture_file_path
still_capture_file_path = []
global iso_file_path
iso_file_path = []
global color_mode_file_path
color_mode_file_path = []

def check_xmllog(test, source, data, testno, class_xmllog, path):
    class_xmllog.readXmlLogDir(path,XML_LOG_CHECK_TIME_OUT)
    test.log.info('{} test {}'.format(path,class_xmllog.getResult()))
    return True

def test_result(test, source, data, testno, class_xmllog):
    if 'fail' == class_xmllog.getResult():
        test.set_fail('TEST NO {} FAIL'.format(testno))
    else:
        test.log.info('TEST NO {} SUCCESS'.format(testno))
    return True

def awb_mode_filename(test, source, data):
    global awb_mode_file_path
    awb_mode_file_path.append( data.split("=")[-1].strip(" ").replace('\r\n',''))
    return True

def still_capture_filename(test, source, data):
    global still_capture_file_path
    still_capture_file_path.append( data.split("=")[-1].strip(" ").replace('\r\n',''))
    return True

def iso_filename(test, source, data):
    global iso_file_path
    iso_file_path.append( data.split("=")[-1].strip(" ").replace('\r\n',''))
    return True

def color_mode_filename(test, source, data):
    global color_mode_file_path
    color_mode_file_path.append( data.split("=")[-1].strip(" ").replace('\r\n',''))
    return True

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
    print("checkNewDisk#####")
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
                print("######gmedia_path {}".format(gmedia_path['usb']))
                return True
            return False

def usbcommand(test, source, data, command):
    p3 = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
    (output3, err3) = p3.communicate()
    p3.wait()
    if None != err3 or output3 != b'':
        test.set_fail('cmd NG')

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
def mv_file(test, source, data, file_path, dest_path, newfile):
    usbcommand(test, source, data, 'mkdir -p {}'.format(dest_path))
    usbcommand(test, source, data, 'rm -fr {}/{}'.format(dest_path, newfile))
    usbcommand(test, source, data, 'sync')

    folder_path = gmedia_path['usb'] + '/' + file_path.split("/")[0].strip(" ")
    file_path = gmedia_path['usb'] + '/' + file_path
    print("##### {} ### {}".format(folder_path, file_path))
    
    test.log.info('mv {}->{}/{}'.format(file_path, dest_path, newfile))

    usbcommand(test, source, data, 'cp {} {}/{}'.format(file_path, dest_path, newfile))
    usbcommand(test, source, data, 'sync')
    time.sleep(0)


    usbcommand(test, source, data, 'rm -r {}'.format(folder_path))
    usbcommand(test, source, data, 'sync')
    time.sleep(0)

    #usbcommand(test, source, data, 'mv {} {}/{}'.format(file_path, dest_path, newfile))
    #usbcommand(test, source, data, 'sync')

    test.add_user_event(source, 'File {} moved'.format(file_path))
    return True

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

def dut_img_monitor(test, source, data):
    global strdata
    if 'Mem' in data:
        if strdata == "":
            strdata = data
        else:
            if data == strdata:
                test.log.debug('OK!')
            else:
                test.set_fail(' '.join(['error']))


def wait_sec(test, source, data, sec, description):
    if description is not None:
        test.log.info("")
        test.log.info("#############################################")
        test.log.info("## [" + str(sec) + " sec] "+ description)
        test.log.info("#############################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')

def store_still_file_path(test, source, data):
    test.storage.still_file_path.append(data.get_value(source.KEY_FILE_PATH))
    return True

def mkdir_backup(test, source, data, testcase):
    test.storage.backup_folder = []
    test.storage.backup_folder = '{}'.format(datetime.datetime.now().strftime("%Y%m%d"))
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

        file_path = LOG_XML_PATH + filepath + '/' + file + '/' + filelist[0]
    else:
        file_path = TEST_OUT_PATH + filepath + '/' + file
    
    test.log.info('mv {}->{}'.format(file_path, dest_path + '/'))
    usbcommand(test, source, data, 'mv {} {}'.format(file_path, dest_path + '/'))
    usbcommand(test, source, data, 'sync')
    time.sleep(0)
    test.add_user_event(source, 'File {} moved'.format(file))
    return True

def mkdir_still(test, source, data):
    test.storage.still_file_path = []
#    test.storage.still_dirname = '{}-{}'.format(datetime.datetime.now().strftime("%Y%m%d%H%M%S"), test.name)
    test.storage.still_dirname = '{}-{}'.format(datetime.datetime.now().strftime("%m%d%H%M%S"), test.name)
    time.sleep(5)
    source.write('mkdir /mnt/sd0/{}'.format(test.storage.still_dirname))
    time.sleep(2)

def mv_still_file(test, source, data):
    for path in test.storage.still_file_path:
        target = '{}/{}'.format(os.path.dirname(path), test.storage.still_dirname)
        source.write('mv {} {}/{}'.format(path, target, os.path.basename(path)))
        time.sleep(2)
    test.storage.still_file_path.clear()

class CameraArduinoTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(CameraArduinoTestTcRunner, self).__init__(conf)

        self.camera_arduino_test_device = CameraArduinoTestDevice(
            kwargs.get('dut_device', None), 'CAMERA_ARDUINO_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.getcwd(), 'defconfig/camera-defconfig')
        self.read_xmllog = xmllog()

    def get_devices_list(self):
        return [self.camera_arduino_test_device]

    def get_read_xmllog_class(self):
        return self.read_xmllog

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.camera_arduino_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.camera_arduino_test_device)

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
        timeout = 180

        camera_arduino_test_device = self.get_devices_list()[0]
        func_xmllog = self.get_read_xmllog_class()

        def usb_s(device):
            r = [
                Step(Action.EXECUTE, device, None, lambda test, source, data: reset_data(test, source, data)),
                Step(Action.EXECUTE, device, None, lambda test, source, data: wait_sec(test, source, data, 5, 'wait sd')),
                Step(Action.EXECUTE, device, None, lambda test, source, data: checkDefaultDisk(test, source, data)),

                Step(Action.WRITE, device, 'usbmsc'),
                Step(Action.WAIT_FOR, device, 'Connect Extension Board USB to PC.'),
                Step(Action.EXECUTE, device, None, lambda test, source, data: wait_sec(test, source, data, 25, 'usb mount to PC')),

                Step(Action.EXECUTE, device, None, lambda test, source, data: checkNewDisk(test, source, data)),
            ]
            return r

        def usb_e(device):
            r = [
                Step(Action.WRITE, device, 'end'),
                Step(Action.WAIT_FOR, device, 'Finish USB Mass Storage Operation'),
            ]
            return r


        def videostart_steps(width=320, height=240, fps=120):
            return [
                Step(Action.WRITE, camera_arduino_test_device, 'camera videostart {} {} {}'.format(width, height, fps)),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_arduino_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, 'Video start {}x{} fps={}'.format(width, height, fps))),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'wait sec completed'),
            ]

        def still_loop_steps(cmd, width=1920, height=1080, count=1, fps=15):
            r = []
            r = r + [
                Step(Action.WRITE, camera_arduino_test_device, 'camera {} {} {} {} {}'.format(cmd, width, height, count, fps)),
            ]
            for i in range(count):
                r.append(
                    Step(Action.WAIT_FOR, camera_arduino_test_device, camera_arduino_test_device.STILL_DATA,
                         lambda test, source, data: store_still_file_path(test, source, data)),
                )
            r = r + [
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Confirmation'),
                Step(Action.EXECUTE, camera_arduino_test_device, None,
                     lambda test, source, data: mv_still_file(test, source, data)),
            ]
            return r

        def yuv_jpeg_loop_steps(cmd, jpeg_width, jpeg_height, yuv_width, yuv_height, count, fps):
            r = []
            r = r + [
                Step(Action.WRITE, camera_arduino_test_device, 'camera {} {} {} {} {} {} {}'.format(cmd, jpeg_width, jpeg_height, yuv_width, yuv_height, count, fps)),
            ]
            for i in range(count):
                r.append(
                    Step(Action.WAIT_FOR, camera_arduino_test_device, camera_arduino_test_device.STILL_DATA,
                         lambda test, source, data: store_still_file_path(test, source, data)),
                )
            r = r + [
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Still Confirmation'),
                Step(Action.EXECUTE, camera_arduino_test_device, None,
                     lambda test, source, data: mv_still_file(test, source, data)),
            ]
            return r

        def loop_steps(count, steps):
            r = []
            for i in range(count):
                r = r + steps
            return r

        def cam_cmd_steps(cmd, value_list, wait_time=0, ok_str=None, still=False):
            r = []
            for value in value_list:
                r = r + [
                    Step(Action.WRITE, camera_arduino_test_device, 'camera {} {}'.format(cmd, value)),
                ]
                if ok_str is not None:
                    r = r + [
                        Step(Action.WAIT_FOR, camera_arduino_test_device, ok_str),
                    ]
                if wait_time > 0:
                    r = r + [
                        Step(Action.EXECUTE, camera_arduino_test_device, None,
                            lambda test, source, data, val=value: wait_sec(test, source, data, wait_time, 'cmd : {} / value : {}'.format(cmd, val))),
                    ]
                if still == True:
                    r = r + still_loop_steps("still")
            return r

        setup = [
        ]

        setup_mkdir_still = [
            Step(Action.EXECUTE, camera_arduino_test_device, None,
                 lambda test, source, data: mkdir_still(test, source, data)),
        ]

        teardown = [
        ]

        ADN_Camera_16349_16225_16226_16227_16222 =Test(
            name='ADN_Camera_16349_16225_16226_16227_16222',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),
                Step(Action.WRITE, camera_arduino_test_device, 'video_capture_test'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Run: video_capture_test'),
            ],
            teardown=teardown
        )

        ADN_Camera_16228_16239 =Test(
            name='ADN_Camera_16228_16239',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),
                Step(Action.WRITE, camera_arduino_test_device, 'awb_test'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setAutoWhiteBalance'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'capture success'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Enable AWB'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setAutoWhiteBalance'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'capture success'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Disable AWB'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success stopStreaming'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Finish awb test'),
            ],
            teardown=teardown

        )

        ADN_Camera_16230_16241 =Test(
            name='ADN_Camera_16230_16241',
            timeout=300,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),
                Step(Action.WRITE, camera_arduino_test_device, 'awb_mode_test'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setAutoWhiteBalanceMode'),
                Step(Action.EXECUTE, camera_arduino_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, 'awb_mode  = CAM_WHITE_BALANCE_INCANDESCENT')),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setAutoWhiteBalanceMode'),
                Step(Action.EXECUTE, camera_arduino_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, 'awb_mode  = CAM_WHITE_BALANCE_FLUORESCENT')),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setAutoWhiteBalanceMode'),
                Step(Action.EXECUTE, camera_arduino_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, 'awb_mode  = CAM_WHITE_BALANCE_DAYLIGHT')),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Error setAutoWhiteBalanceMode, Err = -7'),
                Step(Action.EXECUTE, camera_arduino_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, 'awb_mode  = CAM_WHITE_BALANCE_FLASH')),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setAutoWhiteBalanceMode'),
                Step(Action.EXECUTE, camera_arduino_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, 'awb_mode  = CAM_WHITE_BALANCE_CLOUDY')),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setAutoWhiteBalanceMode'),
                Step(Action.EXECUTE, camera_arduino_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, 'awb_mode  = CAM_WHITE_BALANCE_INCANDESCENT')),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Finish awb mode test'),

            ],
            teardown=teardown
        )

        ADN_Camera_16231_16242 =Test(
            name='ADN_Camera_16231_16242',
            timeout=720,
            setup=setup,
            test=[
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),
            Step(Action.WRITE, camera_arduino_test_device, 'iso_test'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=25'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=25'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=64'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=64'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=80'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=80'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=100'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=100'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=125'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=125'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=160'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=160'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=200'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=200'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=250'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=250'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=320'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=320'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=500'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=500'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=640'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=640'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=800'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=800'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=1000'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=1000'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO=1600'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test ISO sensitivity, ISO=1600'),
            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setISOSensitivity'),

            Step(Action.WAIT_FOR, camera_arduino_test_device, 'Finish iso sensitivity test'),
            ],
            teardown=teardown

        )


        ADN_Camera_16232_16243 =Test(
            name='ADN_Camera_16232_16243',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),
                Step(Action.WRITE, camera_arduino_test_device, 'aiso_test'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setAutoISOSensitive'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'capture success'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Enable AISO'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setAutoISOSensitive'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'capture success'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Disable AISO'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success stopStreaming'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Finish ISO test'),
            ],
            teardown=teardown

        )
        ADN_Camera_16233_16244 =Test(
            name='ADN_Camera_16233_16244',
            timeout=300,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),
                Step(Action.WRITE, camera_arduino_test_device, 'color_mode_test'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start color mode test'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_NONE'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setColorEffect'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_BW'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setColorEffect'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_SEPIA'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setColorEffect'),


                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_NEGATIVE '),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setColorEffect'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_EMBOSS'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -7'),


                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_SKETCH'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setColorEffect'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_SKY_BLUE'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -7'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_GRASS_GREEN'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -7'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_SKIN_WHITEN'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -7'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_VIVID'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -7'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_AQUA'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -7'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_ART_FREEZE'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -7'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_SILHOUETTE'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -7'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_SOLARIZATION'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setColorEffect'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_ANTIQUE'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -7'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_SET_CBCR'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -7'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Test CAM_COLOR_FX_PASTEL'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setColorEffect'),


                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Finish color mode test'),


            ],
            teardown=teardown

        )

        ADN_Camera_16350_16286_16234 =Test(
            name='ADN_Camera_16350_16286_16234',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),

                Step(Action.WRITE, camera_arduino_test_device, 'still_capture_test qvga'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start QVGA still capture test'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setStillPictiureImageFormat'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Width = 320'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Height = 240'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Format = JPG'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'still_filename', lambda test, source, data: still_capture_filename(test, source, data)),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'capture success'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Still capture test end'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Run: still_capture_test'),
            ],
            teardown=teardown
        )

        ADN_Camera_16235 =Test(
            name='ADN_Camera_16235',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),

                Step(Action.WRITE, camera_arduino_test_device, 'still_capture_test vga'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start VGA still capture test'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setStillPictiureImageFormat'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Width = 640'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Height = 480'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Format = JPG'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'still_filename', lambda test, source, data: still_capture_filename(test, source, data)),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'capture success'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Still capture test end'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Run: still_capture_test'),
            ],
            teardown=teardown
        )

        ADN_Camera_16237 =Test(
            name='ADN_Camera_16237',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),

                Step(Action.WRITE, camera_arduino_test_device, 'still_capture_test hd'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start HD still capture test'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setStillPictiureImageFormat'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Width = 1280'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Height = 720'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Format = JPG'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'still_filename', lambda test, source, data: still_capture_filename(test, source, data)),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'capture success'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Still capture test end'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Run: still_capture_test'),
            ],
            teardown=teardown
        )

        ADN_Camera_16236 =Test(
            name='ADN_Camera_16236',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),

                Step(Action.WRITE, camera_arduino_test_device, 'still_capture_test quadvga'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start QuADVGA still capture test'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Success setStillPictiureImageFormat'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Width = 1280'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Height = 960'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Format = JPG'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'still_filename', lambda test, source, data: still_capture_filename(test, source, data)),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'capture success'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Still capture test end'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Run: still_capture_test'),
            ],
            teardown=teardown
        )

        ADN_Camera_16238 =Test(
            name='ADN_Camera_16238',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),

                Step(Action.WRITE, camera_arduino_test_device, 'still_capture_test fullhd'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start FullHD still capture test'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -8'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Still capture test end'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Run: still_capture_test'),
            ],
            teardown=teardown
        )

        ADN_Camera_16252 =Test(
            name='ADN_Camera_16252',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),

                Step(Action.WRITE, camera_arduino_test_device, 'still_capture_test 5m'),


                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start 5M still capture test'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -8'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Still capture test end'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Run: still_capture_test'),
            ],
            teardown=teardown
        )

        ADN_Camera_16253 =Test(
            name='ADN_Camera_16253',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start Camera test'),

                Step(Action.WRITE, camera_arduino_test_device, 'still_capture_test 3m'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Start 3M still capture test'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Err = -8'),

                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Still capture test end'),
                Step(Action.WAIT_FOR, camera_arduino_test_device, 'Run: still_capture_test'),
            ],
            teardown=teardown
        )

        video_capture = TestGroup(
            name='video_capture',
            devices=[camera_arduino_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                ADN_Camera_16349_16225_16226_16227_16222,
                ADN_Camera_16228_16239,
                ADN_Camera_16230_16241,
                ADN_Camera_16231_16242,
                ADN_Camera_16232_16243,
                ADN_Camera_16233_16244,
                ADN_Camera_16350_16286_16234,
                ADN_Camera_16235,
                ADN_Camera_16236,
                ADN_Camera_16237,
                ADN_Camera_16238,
                ADN_Camera_16252,
                ADN_Camera_16253,

            ],
        )

        test_groups = [
            video_capture,
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
    tc_runner = CameraArduinoTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
