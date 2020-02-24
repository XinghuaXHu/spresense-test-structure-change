#! /usr/bin/env python3
import sys
import os
import time
import datetime

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.camera_test.camera_test_device import CameraTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['camera']

global free_used_size_one
free_used_size_one = 0
global free_used_size_two
free_used_size_two = 0
global free_used_size_three
free_used_size_three = 0

def free_used_monitor(test, source, data):
    global free_used_size_one
    global free_used_size_two
    global free_used_size_three
    if 'Mem' in data:
        print("### used {}".format(data.split()[2]))
        if free_used_size_one == 0:
            free_used_size_one = data.split()[2]
        elif free_used_size_two == 0:
            free_used_size_two = data.split()[2]
            if free_used_size_two > free_used_size_one:
                test.log.debug('used add ok')
            else:
                test.set_fail(' used add fail')

        elif free_used_size_three == 0:
            free_used_size_three = data.split()[2]
            if free_used_size_two == free_used_size_three:
                test.log.debug('used no change ok')
            else:
                test.set_fail(' used no change fail')
            free_used_size_one = free_used_size_two = free_used_size_three = 0


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

def mkdir_autotest(test, source, data):
    test.storage.still_dirname = []
    #time.sleep(1)
    #source.write('mkdir /mnt/sd0/autotest')
    #time.sleep(1)
    return True

def mkdir_still(test, source, data, number=0):
    test.storage.still_file_path = []
#    test.storage.still_dirname = '{}-{}'.format(datetime.datetime.now().strftime("%Y%m%d%H%M%S"), test.name)
#    test.storage.still_dirname = '{}-{}'.format(datetime.datetime.now().strftime("%m%d%H%M%S"), test.name)

    if number == 0:
       test.storage.still_dirname = 'Test_{}'.format(test.name.split("_")[-1].strip(" "))
       time.sleep(0)
       #source.write('mkdir /mnt/sd0/autotest/{}'.format(test.storage.still_dirname))
       time.sleep(0)
    else:
        foldername = 'Test_{}_{}'.format(test.name.split("_")[-1].strip(" "), number)
        test.storage.still_dirname.append(foldername)
        time.sleep(0)
        #source.write('mkdir /mnt/sd0/autotest/{}'.format(foldername))
        time.sleep(0)


def mv_still_file(test, source, data):
    for path in test.storage.still_file_path:
        if isinstance(test.storage.still_dirname,list):
           folder = test.storage.still_dirname[0]
           test.storage.still_dirname.pop(0)
        else:
            folder = test.storage.still_dirname

        target = '{}/autotest/{}'.format(os.path.dirname(path), folder)
        source.write('mv {} {}/{}'.format(path, target, os.path.basename(path)))
        time.sleep(2)
        
        

    test.storage.still_file_path.clear()

class CameraTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(CameraTestTcRunner, self).__init__(conf)

        self.camera_test_device = CameraTestDevice(
            kwargs.get('dut_device', None), 'CAMERA_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.getcwd(), 'defconfig/camera-defconfig')

    def get_devices_list(self):
        return [self.camera_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.camera_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.camera_test_device)

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

        camera_test_device = self.get_devices_list()[0]

        def videostart_steps(width=320, height=240, fps=120):
            return [
                Step(Action.WRITE, camera_test_device, 'camera videostart {} {} {}'.format(width, height, fps)),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, 'Video start {}x{} fps={}'.format(width, height, fps))),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
            ]

        def videostart_defsteps():
            width = 320
            height = 240
            fps = 120
            return [
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, 'Video start {}x{} fps={}'.format(width, height, fps))),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
            ]

        def still_loop_steps(cmd, width=1920, height=1080, count=1, fps=15, mv=True):
            r = []
            r = r + [
                Step(Action.WRITE, camera_test_device, 'camera {} {} {} {} {}'.format(cmd, width, height, count, fps)),
            ]
            for i in range(count):
                r.append(
                    Step(Action.WAIT_FOR, camera_test_device, camera_test_device.STILL_DATA,
                         lambda test, source, data: store_still_file_path(test, source, data)),
                )
            r = r + [
                Step(Action.WAIT_FOR, camera_test_device, 'Confirmation'),
            ]
            
            if True == mv:
                r = r + [
                     Step(Action.EXECUTE, camera_test_device, None,
                         lambda test, source, data: mv_still_file(test, source, data)),
                ]
            return r

        def yuv_jpeg_loop_steps(cmd, jpeg_width, jpeg_height, yuv_width, yuv_height, count, fps):
            r = []
            r = r + [
                Step(Action.WRITE, camera_test_device, 'camera {} {} {} {} {} {} {}'.format(cmd, jpeg_width, jpeg_height, yuv_width, yuv_height, count, fps)),
            ]
            for i in range(count*2):
                r.append(
                    Step(Action.WAIT_FOR, camera_test_device, camera_test_device.STILL_DATA,
                         lambda test, source, data: store_still_file_path(test, source, data)),
                )
            r = r + [
                Step(Action.WAIT_FOR, camera_test_device, 'Still Confirmation'),
                Step(Action.EXECUTE, camera_test_device, None,
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
                    Step(Action.WRITE, camera_test_device, 'camera {} {}'.format(cmd, value)),
                ]
                if ok_str is not None:
                    r = r + [
                        Step(Action.WAIT_FOR, camera_test_device, ok_str),
                    ]
                if wait_time > 0:
                    r = r + [
                        Step(Action.EXECUTE, camera_test_device, None,
                            lambda test, source, data, val=value: wait_sec(test, source, data, wait_time, 'cmd : {} / value : {}'.format(cmd, val))),
                    ]
                if still == True:
                    #r = r + still_loop_steps("still")
                    r = r + still_loop_steps("still", 320, 240, 1, 60)
            return r

        setup = [
            #Step(Action.WAIT_FOR, camera_test_device, camera_test_device.NSH_PROMPT),
            Step(Action.WRITE, camera_test_device, 'reboot'),
            Step(Action.WAIT_FOR, camera_test_device, camera_test_device.NUTTSHELL),
            #Step(Action.EXECUTE, camera_test_device, None,
                 #lambda test, source, data: wait_sec(test, source, data, 3, '3s')),
        ]

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
            r = r + [
                Step(Action.WRITE, camera_test_device, 'camera video_capture_start'),
            ]

            for i in range(3):
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

        def camera_open_close_100_times():
            r = []
            for i in range(100):
                r = r + [
                   Step(Action.WRITE, camera_test_device, 'camera open'),
                   Step(Action.WRITE, camera_test_device, 'camera close'),
                   Step(Action.WAIT_FOR, camera_test_device, 'closed'),
                ]
            return r

        teardown = [
        ]

        SDK_Imager_16121  = Test(
            name='SDK_Imager_16121',
            timeout=timeout,
            setup=setup + setup_mkdir_still(3),
            test=videostart_defsteps()+run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16122 = Test(
            name='SDK_Imager_16122',
            timeout=timeout,
            setup=setup + setup_mkdir_still(3),
            test=videostart_steps(480, 360, 60)+run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16123 = Test(
            name='SDK_Imager_16123',
            timeout=timeout,
            setup=setup + setup_mkdir_still(3),
            test=videostart_steps(96, 64, 120)+run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16169_16168 = Test(
            name='SDK_Imager_16169_16168',
            timeout=timeout,
            setup=setup + setup_mkdir_still(3),
            test=videostart_steps()+\
                 [
                    Step(Action.WRITE, camera_test_device, "camera streamoff"),
                    Step(Action.WAIT_FOR, camera_test_device, 'stream off'),
                    Step(Action.WRITE, camera_test_device, "camera streamon"),
                    Step(Action.WAIT_FOR, camera_test_device, 'stream on'),
                    Step(Action.WAIT_FOR, camera_test_device, 'stream on'),
                 ] + run_video_capture(),
            teardown=teardown
        )
        SDK_Imager_16165 = Test(
            name='SDK_Imager_16165',
            timeout=timeout,
            setup=setup + setup_mkdir_still(3),
            test=videostart_steps()+run_video_capture()+\
                 [
                    Step(Action.WRITE, camera_test_device, "camera streamoff"),
                    Step(Action.WAIT_FOR, camera_test_device, 'stream off'),
                 ],
            teardown=teardown
        )
        SDK_Imager_16171_16197_16198_16199_16124 = Test(
            name='SDK_Imager_16171_16197_16198_16199_16124',
            timeout=timeout,
            setup=setup + setup_mkdir_still(12) + videostart_defsteps(),
            test=cam_cmd_steps("brightness", [ "-128", "0", "127" ], 0, "ioctl calling", True)+ 
                 [
                     Step(Action.WRITE, camera_test_device, "camera brightness -128"),
                     Step(Action.WRITE, camera_test_device, 'camera check 0'),
                     Step(Action.WAIT_FOR, camera_test_device, 'brightness: 128'),
                 ] + run_video_capture() +[
                     Step(Action.WRITE, camera_test_device, "camera brightness 0"),
                     Step(Action.WRITE, camera_test_device, 'camera check 0'),
                     Step(Action.WAIT_FOR, camera_test_device, 'brightness: 0'),
                 ] + run_video_capture() +[
                     Step(Action.WRITE, camera_test_device, "camera brightness 127"),
                     Step(Action.WRITE, camera_test_device, 'camera check 0'),
                     Step(Action.WAIT_FOR, camera_test_device, 'brightness: 127'),
                 ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16172_16197_16198_16199_16125 = Test(
            name='SDK_Imager_16172_16197_16198_16199_16125',
            timeout=timeout,
            setup=setup + setup_mkdir_still(12) + videostart_defsteps(),
            test=cam_cmd_steps("contrast", [ "0", "127", "255" ], 0, "ioctl calling", True)+             
                 [
                     Step(Action.WRITE, camera_test_device, "camera contrast 0"),
                     Step(Action.WRITE, camera_test_device, 'camera check 1'),
                     Step(Action.WAIT_FOR, camera_test_device, 'contrast: 0'),
                 ] + run_video_capture() +[
                     Step(Action.WRITE, camera_test_device, "camera contrast 127"),
                     Step(Action.WRITE, camera_test_device, 'camera check 1'),
                     Step(Action.WAIT_FOR, camera_test_device, 'contrast: 127'),
                 ] + run_video_capture() +[
                     Step(Action.WRITE, camera_test_device, "camera contrast 255"),
                     Step(Action.WRITE, camera_test_device, 'camera check 1'),
                     Step(Action.WAIT_FOR, camera_test_device, 'contrast: 255'),
                 ] + run_video_capture(),
             teardown=teardown

        )

        SDK_Imager_16173_16197_16151_16152_16199_16126 = Test(
            name='SDK_Imager_16197_16173_16151_16152_16199_16126',
            timeout=timeout,
            setup=setup + setup_mkdir_still(12) + videostart_defsteps(),
            test=cam_cmd_steps("saturation", [ "0", "127", "255" ], 0, "ioctl calling", True) +
                 [
                     Step(Action.WRITE, camera_test_device, "camera saturation 0"),
                     Step(Action.WRITE, camera_test_device, 'camera check 2'),
                     Step(Action.WAIT_FOR, camera_test_device, 'saturation: 0'),
                 ] + run_video_capture() +[
                     Step(Action.WRITE, camera_test_device, "camera saturation 127"),
                     Step(Action.WRITE, camera_test_device, 'camera check 2'),
                     Step(Action.WAIT_FOR, camera_test_device, 'saturation: 127'),
                 ] + run_video_capture() +[
                     Step(Action.WRITE, camera_test_device, "camera saturation 255"),
                     Step(Action.WRITE, camera_test_device, 'camera check 2'),
                     Step(Action.WAIT_FOR, camera_test_device, 'saturation: 255'),
                 ]+run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16174_16197_16151_16152_16199_16127 = Test(
            name='SDK_Imager_16174_16197_16151_16152_16199_16127',
            timeout=timeout,
            setup=setup + setup_mkdir_still(12) + videostart_defsteps(),
            test=cam_cmd_steps("hue", [ "0", "127", "255" ], 0, "ioctl calling", True) + 
            [
                     Step(Action.WRITE, camera_test_device, "camera hue 0"),
                     Step(Action.WRITE, camera_test_device, 'camera check 3'),
                     Step(Action.WAIT_FOR, camera_test_device, 'hue: 0'),
            ] + run_video_capture() +[
                     Step(Action.WRITE, camera_test_device, "camera hue 127"),
                     Step(Action.WRITE, camera_test_device, 'camera check 3'),
                     Step(Action.WAIT_FOR, camera_test_device, 'hue: 127'),
            ] + run_video_capture() +[
                     Step(Action.WRITE, camera_test_device, "camera hue 255"),
                     Step(Action.WRITE, camera_test_device, 'camera check 3'),
                     Step(Action.WAIT_FOR, camera_test_device, 'hue: 255'),
            ]+ run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16175_16197_16199_16128 = Test(
            name='SDK_Imager_16175_16197_16199_16128',
            timeout=timeout,
            setup=setup + setup_mkdir_still(8) + videostart_defsteps(),
            test=cam_cmd_steps("awb", [ "0", "1" ], 0, "ioctl calling", True) +
                 [
                     Step(Action.WRITE, camera_test_device, "camera awb 0"),
                     Step(Action.WRITE, camera_test_device, 'camera check 4'),
                     Step(Action.WAIT_FOR, camera_test_device, 'auto white balance: 0'),
                 ] + run_video_capture() +[
                     Step(Action.WRITE, camera_test_device, "camera awb 1"),
                     Step(Action.WRITE, camera_test_device, 'camera check 4'),
                     Step(Action.WAIT_FOR, camera_test_device, 'auto white balance: 2'),
                 ]+ run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16129 = Test(
            name='SDK_Imager_16129',
            timeout=timeout,
            setup=setup + setup_mkdir_still() + videostart_defsteps(),
            test=cam_cmd_steps("redbalance", [ "32768", "65535" ], 5, "ioctl calling", True) + [
                Step(Action.WRITE, camera_test_device, "camera redbalance 0"),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.WAIT_FOR, camera_test_device, 'Failed to ioctl s_ctrl.errno:22'),
            ],
            teardown=teardown
        )

        SDK_Imager_16130 = Test(
            name='SDK_Imager_16130',
            timeout=timeout,
            setup=setup + setup_mkdir_still() + videostart_defsteps(),
            test=cam_cmd_steps("bluebalance", [ "32768", "65535" ], 5, "ioctl calling", True) + [
                Step(Action.WRITE, camera_test_device, "camera bluebalance 0"),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.WAIT_FOR, camera_test_device, 'Failed to ioctl s_ctrl.errno:22'),
            ],
            teardown=teardown
        )

        SDK_Imager_16178_16197_16199_16131 = Test(
            name='SDK_Imager_16178_16197_16199_16131',
            timeout=timeout,
            setup=setup + setup_mkdir_still(8) + videostart_defsteps(),
            test=cam_cmd_steps("gamma", [ "0", "3" ], 0, "gamma change test", True) + [
                Step(Action.WRITE, camera_test_device, "camera gamma 3"),
                Step(Action.WRITE, camera_test_device, 'camera check 8'),
                Step(Action.WAIT_FOR, camera_test_device, 'data.controls->id'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera gamma 0"),
                Step(Action.WRITE, camera_test_device, 'camera check 8'),
                Step(Action.WAIT_FOR, camera_test_device, 'data.controls->id'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16179_16197_16199_16132 = Test(
            name='SDK_Imager_16179_16197_16199_16132',
            timeout=timeout,
            setup=setup + setup_mkdir_still(12) + videostart_defsteps(),
            test=cam_cmd_steps("exposure", [ "-6", "0", "6" ], 0, "ioctl calling", True) + [
                Step(Action.WRITE, camera_test_device, "camera exposure -6"),
                Step(Action.WRITE, camera_test_device, 'camera check 9'),
                Step(Action.WAIT_FOR, camera_test_device, 'exposure: 250'),
           ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera exposure 0"),
                Step(Action.WRITE, camera_test_device, 'camera check 9'),
                Step(Action.WAIT_FOR, camera_test_device, 'exposure: 0'),
           ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera exposure 6"),
                Step(Action.WRITE, camera_test_device, 'camera check 9'),
                Step(Action.WAIT_FOR, camera_test_device, 'exposure: 6'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16180_16197_16199_16133 = Test(
            name='SDK_Imager_16180_16197_16199_16133',
            timeout=timeout,
            setup=setup + setup_mkdir_still(8) + videostart_defsteps(),
            test=cam_cmd_steps("vflip", [ "1", "0" ], 0, "ioctl calling", True) + [
                Step(Action.WRITE, camera_test_device, "camera vflip 1"),
                Step(Action.WRITE, camera_test_device, 'camera check 11'),
                Step(Action.WAIT_FOR, camera_test_device, 'vflip: 1'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera vflip 0"),
                Step(Action.WRITE, camera_test_device, 'camera check 11'),
                Step(Action.WAIT_FOR, camera_test_device, 'vflip: 0'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16181_16197_16199_16134 = Test(
            name='SDK_Imager_16181_16197_16199_16134',
            timeout=timeout,
            setup=setup + setup_mkdir_still(8) + videostart_defsteps(),
            test=cam_cmd_steps("hflip", [ "1", "0" ], 0, "ioctl calling", True) + [
                Step(Action.WRITE, camera_test_device, "camera hflip 1"),
                Step(Action.WRITE, camera_test_device, 'camera check 10'),
                Step(Action.WAIT_FOR, camera_test_device, 'hflip: 1'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera hflip 0"),
                Step(Action.WRITE, camera_test_device, 'camera check 10'),
                Step(Action.WAIT_FOR, camera_test_device, 'hflip: 0'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16182_16197_16199_16135 = Test(
            name='SDK_Imager_16182_16197_16199_16135',
            timeout=timeout,
            setup=setup + setup_mkdir_still(12) + videostart_defsteps(),
            test=cam_cmd_steps("sharpness", [ "0", "128", "255" ], 0, "ioctl calling", True) + [
                Step(Action.WRITE, camera_test_device, "camera sharpness 127"),
                Step(Action.WRITE, camera_test_device, 'camera check 14'),
                Step(Action.WAIT_FOR, camera_test_device, 'sharpness: 127'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera sharpness 0"),
                Step(Action.WRITE, camera_test_device, 'camera check 14'),
                Step(Action.WAIT_FOR, camera_test_device, 'sharpness: 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera sharpness 255"),
                Step(Action.WRITE, camera_test_device, 'camera check 14'),
                Step(Action.WAIT_FOR, camera_test_device, 'sharpness: 255'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16183_16197_16199_16136 = Test(
            name='SDK_Imager_16183_16197_16199_16136',
            timeout=timeout,
            setup=setup + setup_mkdir_still(8) + videostart_defsteps(),
            test=cam_cmd_steps("colorkill", [ "1", "0" ], 5, "ioctl calling", True) + [
                Step(Action.WRITE, camera_test_device, "camera colorkill 1"),
                Step(Action.WRITE, camera_test_device, 'camera check 15'),
                Step(Action.WAIT_FOR, camera_test_device, 'color killer: 1'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera colorkill 0"),
                Step(Action.WRITE, camera_test_device, 'camera check 15'),
                Step(Action.WAIT_FOR, camera_test_device, 'color killer: 0'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16184_16197_16199_16137 = Test(
            name='SDK_Imager_16184_16197_16199_16137',
            timeout=timeout,
            setup=setup + setup_mkdir_still(28) + videostart_defsteps(),
            test=cam_cmd_steps("colorfx", [ "0", "1", "2", "3", "5", "13", "16" ], 5, "ioctl calling", True) + [
                Step(Action.WRITE, camera_test_device, "camera colorfx 0"),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera colorfx 1"),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 1'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera colorfx 2"),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 2'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera colorfx 3"),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 3'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera colorfx 5"),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 5'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera colorfx 13"),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 13'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera colorfx 16"),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 16'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16185_16197_16199_16138 = Test(
            name='SDK_Imager_16185_16197_16199_16138',
            timeout=timeout,
            setup=setup + setup_mkdir_still(8) + videostart_defsteps(),
            test=cam_cmd_steps("autoexpo", [ "0", "1" ], 0, "test_vidioc_s_ext_ctrl call", True) +
            [
                Step(Action.WRITE, camera_test_device, "camera autoexpo 1"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera autoexpo 0"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16186_16197_16139 = Test(
            name='SDK_Imager_16186_16197_16139',
            timeout=timeout,
            setup=setup + setup_mkdir_still(12) + videostart_defsteps(),
            test=cam_cmd_steps("expoabs", [ "1", "10", "100" ], 0, "test_vidioc_s_ext_ctrl call", True) +
            [
                Step(Action.WRITE, camera_test_device, "camera expoabs 1"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera expoabs 10"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera expoabs 100"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16187_16197_16140 = Test(
            name='SDK_Imager_16187_16197_16140',
            timeout=timeout,
            setup=setup + setup_mkdir_still(16) + videostart_defsteps(),
            test=cam_cmd_steps("expomete", [ "0", "1", "2", "3" ], 0, "test_vidioc_s_ext_ctrl call", True) +
            [
                Step(Action.WRITE, camera_test_device, "camera expomete 0"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera expomete 1"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera expomete 2"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera expomete 3"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16141 = Test(
            name='SDK_Imager_16141',
            timeout=timeout,
            setup=setup + setup_mkdir_still() + videostart_defsteps(),
            test=cam_cmd_steps("zoom", [ "0 1", "0 2", "0 4" ], 3, "test_vidioc_s_ext_ctrl call", True) + [
                Step(Action.WRITE, camera_test_device, 'camera zoom 1 10'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_ext_ctrl call'),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = -1'),
                Step(Action.WAIT_FOR, camera_test_device, 'Failed to ioctl s_ext_ctrl.errno'),
            ],
            teardown=teardown
        )

        SDK_Imager_16142 = Test(
            name='SDK_Imager_16142',
            timeout=timeout,
            setup=setup + setup_mkdir_still() + videostart_defsteps(),
            test=cam_cmd_steps("zoom", [ "1 1", "1 2", "1 4" ], 3, "test_vidioc_s_ext_ctrl call", True) + [
                Step(Action.WRITE, camera_test_device, 'camera zoom 0 10'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_ext_ctrl call'),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = -1'),
                Step(Action.WAIT_FOR, camera_test_device, 'Failed to ioctl s_ext_ctrl.errno'),
            ],
            teardown=teardown
        )

        SDK_Imager_16190_16197_16143 = Test(
            name='SDK_Imager_16190_16197_16143',
            timeout=timeout,
            setup=setup + setup_mkdir_still(24) + videostart_defsteps(),
            test=cam_cmd_steps("whitebala", [ "1", "2", "3", "6", "8", "9" ], 0, "test_vidioc_s_ext_ctrl call", True) +
            [
                Step(Action.WRITE, camera_test_device, "camera whitebala 1"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera whitebala 2"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera whitebala 3"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera whitebala 6"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera whitebala 8"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera whitebala 9"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16191_16197_16144 = Test(
            name='SDK_Imager_16191_16197_16144',
            timeout=timeout,
            setup=setup + setup_mkdir_still(8) + videostart_defsteps(),
            test=cam_cmd_steps("wide", [ "1", "0" ], 0, "test_vidioc_s_ext_ctrl call", True) +
            [
                Step(Action.WRITE, camera_test_device, "camera wide 0"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera wide 1"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16192_16197_16145 = Test(
            name='SDK_Imager_16192_16197_16145',
            timeout=timeout,
            setup=setup + setup_mkdir_still(12) + videostart_defsteps(),
            test=cam_cmd_steps("iso", [ "25", "800", "1600" ], 0, "test_vidioc_s_ext_ctrl call", True) +
            [
                Step(Action.WRITE, camera_test_device, "camera iso 25"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera iso 800"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera iso 1600"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16193_16197_16146 = Test(
            name='SDK_Imager_16193_16197_16146',
            timeout=timeout,
            setup=setup + setup_mkdir_still(8) + videostart_defsteps(),
            test=[
                Step(Action.WRITE, camera_test_device, "camera awb 0"),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
            ] + cam_cmd_steps("isoauto", [ "0", "1" ], 0, "test_vidioc_s_ext_ctrl call", True) +
            [
                Step(Action.WRITE, camera_test_device, "camera isoauto 1"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera isoauto 0"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16194_16197_16147 = Test(
            name='SDK_Imager_16194_16197_16147',
            timeout=timeout,
            setup=setup + setup_mkdir_still(8) + videostart_defsteps(),
            test=cam_cmd_steps("3a", [ "1", "0" ], 0, "test_vidioc_s_ext_ctrl call", True) +
            [
                Step(Action.WRITE, camera_test_device, "camera 3a 0"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture() +[
                Step(Action.WRITE, camera_test_device, "camera 3a 1"),
                Step(Action.WAIT_FOR, camera_test_device, 'ret = 0'),
            ] + run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16195_16197_16148 = Test(
            name='SDK_Imager_16195_16197_16148',
            timeout=timeout,
            setup=setup + setup_mkdir_still(2),
            test=cam_cmd_steps("jpg", [ "100" ], 5, "test_vidioc_s_ext_ctrl call", False) + \
                 still_loop_steps("still", 1280, 960, 1, 30) + \
                 cam_cmd_steps("jpg", [ "90" ], 5, "test_vidioc_s_ext_ctrl call", False) + \
                 still_loop_steps("still", 1280, 960, 1, 30),
            teardown=teardown
        )

        SDK_Imager_16197_16196 = Test(
            name='SDK_Imager_16197_16196',
            timeout=timeout,
            setup=setup,
            test=[
                 Step(Action.WRITE, camera_test_device, "camera check 0"),
                 Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                 Step(Action.WAIT_FOR, camera_test_device, 'brightness: 0'),
                 Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                 Step(Action.WRITE, camera_test_device, "camera brightness 100"),
                 Step(Action.WAIT_FOR, camera_test_device, 'brightness change test'),
                 Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),

                 Step(Action.WRITE, camera_test_device, "echo $?"),
                 Step(Action.WAIT_FOR, camera_test_device, '0'),

                 Step(Action.WRITE, camera_test_device, "camera check 0"),
                 Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                 Step(Action.WAIT_FOR, camera_test_device, 'brightness: 0'),
                 Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                 Step(Action.WRITE, camera_test_device, "camera check 16"),
                 Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                 Step(Action.WAIT_FOR, camera_test_device, 'color fx: 0'),
                 Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                 Step(Action.WRITE, camera_test_device, "camera colorfx 5"),
                 Step(Action.WAIT_FOR, camera_test_device, 'colorfx scale change test'),
                 Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),

                 Step(Action.WRITE, camera_test_device, "echo $?"),
                 Step(Action.WAIT_FOR, camera_test_device, '0'),

                 Step(Action.WRITE, camera_test_device, "camera check 16"),
                 Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                 Step(Action.WAIT_FOR, camera_test_device, 'color fx: 0'),
                 Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

            ],
            teardown=teardown
        )

        SDK_Imager_16149 = Test(
            name='SDK_Imager_16149',
            timeout=timeout,
            setup=setup + setup_mkdir_still(6) + videostart_steps(),
            test=cam_cmd_steps("af", [ "" ], 5, "ret = 0", False) + \
                 run_video_capture() + \
                 cam_cmd_steps("afnot", [ "" ], 5, "ret = 0", False) + \
                 run_video_capture(),
            teardown=teardown
        )

        SDK_Imager_16150 = Test(
            name='SDK_Imager_16150',
            timeout=timeout,
            setup=setup,
            test=cam_cmd_steps("af", [ "" ], 0, "ret = -1", False) + \
                 videostart_steps() + \
                 cam_cmd_steps("streamoff", [ "" ], 2, "stream off", False) + \
                 cam_cmd_steps("af", [ "" ], 0, "ret = 0", False),
            teardown=teardown
        )

        SDK_Imager_16153 = Test(
            name='SDK_Imager_16153',
            timeout=timeout,
            setup=setup+setup_mkdir_still()+videostart_steps(320, 240, 120),
            test=still_loop_steps("still", 320, 240, 1, 120),
            teardown=teardown
        )

        SDK_Imager_16154 = Test(
            name='SDK_Imager_16154',
            timeout=timeout,
            setup=setup+setup_mkdir_still()+videostart_steps(320, 240, 120),
            test=still_loop_steps("still", 640, 480, 1, 60),
            teardown=teardown
        )

        SDK_Imager_16155 = Test(
            name='SDK_Imager_16155',
            timeout=timeout,
            setup=setup+setup_mkdir_still()+videostart_steps(320, 240, 120),
            test=still_loop_steps("still", 1280, 960, 1, 30),
            teardown=teardown
        )

        SDK_Imager_16156 = Test(
            name='SDK_Imager_16156',
            timeout=timeout,
            setup=setup+setup_mkdir_still()+videostart_steps(320, 240, 120),
            test=still_loop_steps("still", 1920, 1080, 1, 15),
            teardown=teardown
        )

        SDK_Imager_16157 = Test(
            name='SDK_Imager_16157',
            timeout=timeout,
            setup=setup+setup_mkdir_still()+videostart_steps(320, 240, 120),
            test=still_loop_steps("still", 2592, 1944, 1, 15),
            teardown=teardown
        )

        SDK_Imager_16163 = Test(
            name='SDK_Imager_16163',
            timeout=timeout,
            setup=setup+setup_mkdir_still(),
            test=[         
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, 'Still 320x240 fps=120')),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
            ]+still_loop_steps("still", 320, 240, 1, 120),
            teardown=teardown
        )

        SDK_Imager_16220 = Test(
            name='SDK_Imager_16220',
            timeout=timeout,
            setup=setup+setup_mkdir_still(),
            test=[         
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, 'Still 320x240 fps=120')),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
            ]+still_loop_steps("yuv", 320, 240, 1, 120),
            teardown=teardown
        )

        SDK_Imager_16221 = Test(
            name='SDK_Imager_16221',
            timeout=timeout,
            setup=setup+setup_mkdir_still(2),
            test=yuv_jpeg_loop_steps("sub", 1280, 960, 400, 240, 1, 30),
            teardown=teardown
        )

        SDK_Imager_16158 = Test(
            name='SDK_Imager_16158',
            timeout=timeout,
            setup=setup+setup_mkdir_still()+videostart_steps(320, 240, 120),
            test=still_loop_steps("yuv", 320, 240, 1, 120),
            teardown=teardown
        )

        SDK_Imager_16159 = Test(
            name='SDK_Imager_16159',
            timeout=timeout,
            setup=setup+setup_mkdir_still()+videostart_steps(320, 240, 120),
            test=still_loop_steps("yuv", 480, 360, 1, 60),
            teardown=teardown
        )

        SDK_Imager_16160 = Test(
            name='SDK_Imager_16160',
            timeout=timeout,
            setup=setup+setup_mkdir_still()+videostart_steps(320, 240, 120),
            test=still_loop_steps("yuv", 96, 64, 1, 120),
            teardown=teardown
        )

        SDK_Imager_16161 = Test(
            name='SDK_Imager_16161',
            timeout=timeout,
            setup=setup+setup_mkdir_still(2)+videostart_steps(320, 240, 120),
            test=yuv_jpeg_loop_steps("sub", 320, 240, 320, 240, 1, 15),
            teardown=teardown
        )

        SDK_Imager_16162 = Test(
            name='SDK_Imager_16162',
            timeout=timeout,
            setup=setup+setup_mkdir_still(2)+videostart_steps(320, 240, 120),
            test=yuv_jpeg_loop_steps("sub", 1280, 960, 400, 240, 1, 30),
            teardown=teardown
        )

        SDK_Imager_16197 = Test(
            name='SDK_Imager_16197',
            timeout=timeout,
            setup=setup+setup_mkdir_still()+videostart_steps(320, 240, 120),
            test=loop_steps(5,
                cam_cmd_steps("brightness", [ "-128", "0", "127" ], 3, "ioctl calling", True)
                ),
            teardown=teardown
        )

        SDK_Imager_16164 = Test(
            name='SDK_Imager_16164',
            timeout=timeout,
            setup=setup+setup_mkdir_still(),
            test=[
                Step(Action.WRITE, camera_test_device, 'camera streamon'),
                Step(Action.WAIT_FOR, camera_test_device, 'stream on'),
                Step(Action.WAIT_FOR, camera_test_device, 'stream on'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, 'camera streamon')),
            ] + still_loop_steps("still", 320, 240, 1, 60),
            teardown=teardown
        )

        SDK_Imager_16166_16170  = Test(
            name='SDK_Imager_16166_16170',
            timeout=720,
            setup=setup+setup_mkdir_still(3),
            test=loop_steps(100, [
                Step(Action.WRITE, camera_test_device, 'camera streamon'),
                Step(Action.WAIT_FOR, camera_test_device, 'stream on'),
                Step(Action.WAIT_FOR, camera_test_device, 'stream on'),
                #Step(Action.EXECUTE, camera_test_device, None,
                     #lambda test, source, data: wait_sec(test, source, data, 3, "Stream ON")),
                Step(Action.WRITE, camera_test_device, 'camera streamoff'),
                Step(Action.WAIT_FOR, camera_test_device, 'stream off'),
                #Step(Action.EXECUTE, camera_test_device, None,
                     #lambda test, source, data: wait_sec(test, source, data, 3, "Stream OFF")),
            ]) + videostart_steps() + run_video_capture(),
            teardown=teardown
        )
        SDK_Imager_16351  = Test(
            name='SDK_Imager_16351',
            timeout=720,
            setup=setup+setup_mkdir_still(3),
            test=videostart_steps() + run_video_capture(),
            teardown=teardown
        )
        SDK_Imager_16352  = Test(
            name='SDK_Imager_16352',
            timeout=720,
            setup=setup+setup_mkdir_still(),
            test=[
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, '1s')),
            ] + still_loop_steps("still", 320, 240, 1, 120),
            teardown=teardown
        )
        SDK_Imager_16167  = Test(
            name='SDK_Imager_16167',
            timeout=timeout,
            setup=setup+setup_mkdir_still(),
            test=[
                Step(Action.WRITE, camera_test_device, 'camera streamoff'),
                Step(Action.WAIT_FOR, camera_test_device, 'stream off'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, "Stream OFF")),
            ] + still_loop_steps("stillc"),
            teardown=teardown
        )

        SDK_Imager_16285  = Test(
            name='SDK_Imager_16285',
            timeout=5800,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'nximage_initialize: Initializing LCD'),
                Step(Action.WAIT_FOR, camera_test_device, 'nximage_initialize: Open NX'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_fmt'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WRITE, camera_test_device, 'camera testno 16285'),
            ] + setup_mkdir_still(100,0)+ still_loop_steps("still", 320, 240, 100, 60, False)\
              + setup_mkdir_still(100,100)+ still_loop_steps("still", 640, 480, 100, 60, False)\
              + setup_mkdir_still(100,200)+ still_loop_steps("still", 1280, 720, 100, 30, False)\
              + setup_mkdir_still(100,300)+ still_loop_steps("still", 1280, 960, 100, 30, False)+\
            [
               Step(Action.WRITE, camera_test_device, 'camera testno 0'),
            ],   
            teardown=teardown
        )
        SDK_Imager_16200  = Test(
            name='SDK_Imager_16200',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera check 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'brightness: 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 1'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'contrast: 128'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 2'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'saturation: 128'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 3'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'hue: 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 4'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'auto white balance: 2'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 5'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'red balance: Failed! (ret = -1)'),

                Step(Action.WRITE, camera_test_device, 'camera check 6'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'blue balance: Failed! (ret = -1)'),

                Step(Action.WRITE, camera_test_device, 'camera check 7'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'gamma: Failed! (ret = -1)'),


                Step(Action.WRITE, camera_test_device, 'camera check 8'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'data.controls->id = 8'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 9'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'exposure: 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 10'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'hflip: 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 11'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'vflip: 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 12'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'hflip_still: 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 13'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'vflip_still: 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 14'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'sharpness: 32'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 15'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'color killer: 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),
            ],
            teardown=teardown
        )
        SDK_Imager_16201  = Test(
            name='SDK_Imager_16201',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera device'),
                Step(Action.WAIT_FOR, camera_test_device, 'device setting check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 1, name:Brightness'),
                Step(Action.WAIT_FOR, camera_test_device, 'type:10, name:Contrast'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 1, name:Saturation'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 1, name:Hue'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 2, name:Automatic white balance'),
                Step(Action.WAIT_FOR, camera_test_device, 'type:12, name:Exposure value'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 2, name:Mirror horizontally(VIDEO)'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 2, name:Mirror vertically(VIDEO)'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 2, name:Mirror horizontally(STILL)'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 2, name:Mirror vertically(STILL)'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 1, name:Sharpness'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 2, name:Color killer'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 9, name:Color effect'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 1, name:Auto Exposure'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 1, name:Exposure time(usec)'),
                Step(Action.WAIT_FOR, camera_test_device, 'type:11, name:Zoom'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 9, name:Preset white balance'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 2, name:Wide dynamic range'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 9, name:ISO sensitivity'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 9, name:Automatic ISO sensitivity'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 9, name:Photometry'),
                Step(Action.WAIT_FOR, camera_test_device, 'type: 1, name:JPEG compression quality'),
            ],
            teardown=teardown
        )

        SDK_Imager_16199_01 =Test(
            name='SDK_Imager_16199_01',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'brightness: 0'),
                Step(Action.WRITE, camera_test_device, 'camera brightness -128'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "Brightness = -128")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'brightness: -128'),
                Step(Action.WRITE, camera_test_device, 'camera brightness 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "Brightness = 0")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'brightness: 0'),
                Step(Action.WRITE, camera_test_device, 'camera brightness 127'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "Brightness = 127")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'brightness: 127'),
            ]
        )

        SDK_Imager_16199_02 =Test(
            name='SDK_Imager_16199_02',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 1'),
                Step(Action.WAIT_FOR, camera_test_device, 'contrast: 128'),
                Step(Action.WRITE, camera_test_device, 'camera contrast 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "contrast = 0")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 1'),
                Step(Action.WAIT_FOR, camera_test_device, 'contrast: 0'),
                Step(Action.WRITE, camera_test_device, 'camera contrast 127'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "contrast = 127")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 1'),
                Step(Action.WAIT_FOR, camera_test_device, 'contrast: 127'),
                Step(Action.WRITE, camera_test_device, 'camera contrast 255'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "contrast = 255")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 1'),
                Step(Action.WAIT_FOR, camera_test_device, 'contrast: 255'),
            ]
        )

        SDK_Imager_16199_03 =Test(
            name='SDK_Imager_16199_03',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 2'),
                Step(Action.WAIT_FOR, camera_test_device, 'saturation: 128'),
                Step(Action.WRITE, camera_test_device, 'camera saturation 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "contrast = 0")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 2'),
                Step(Action.WAIT_FOR, camera_test_device, 'saturation: 0'),
                Step(Action.WRITE, camera_test_device, 'camera saturation 127'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "contrast = 127")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 2'),
                Step(Action.WAIT_FOR, camera_test_device, 'saturation: 127'),
                Step(Action.WRITE, camera_test_device, 'camera saturation 255'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "contrast = 255")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 2'),
                Step(Action.WAIT_FOR, camera_test_device, 'saturation: 255'),
            ]
        )

        SDK_Imager_16199_04 =Test(
            name='SDK_Imager_16199_04',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 3'),
                Step(Action.WAIT_FOR, camera_test_device, 'hue: 0'),
                Step(Action.WRITE, camera_test_device, 'camera hue 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, "HUE = 0")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera hue 32'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, "HUE = 32")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera hue 64'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, "HUE = 64")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera hue 96'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, "HUE = 96")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera hue 128'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, "HUE = 128")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera hue 160'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, "HUE = 160")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera hue 192'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, "HUE = 192")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera hue 224'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, "HUE = 224")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera hue 255'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, "HUE = 255")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 3'),
                Step(Action.WAIT_FOR, camera_test_device, 'hue: 255'),
            ]
        )

        SDK_Imager_16199_05 =Test(
            name='SDK_Imager_16199_05',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 4'),
                Step(Action.WAIT_FOR, camera_test_device, 'auto white balance: 2'),
                Step(Action.WRITE, camera_test_device, 'camera awb 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "awb = 0")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 4'),
                Step(Action.WAIT_FOR, camera_test_device, 'auto white balance: 0'),
                Step(Action.WRITE, camera_test_device, 'camera awb 1'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "awb = 1")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 4'),
                Step(Action.WAIT_FOR, camera_test_device, 'auto white balance: 2'),
            ]
        )

        SDK_Imager_16199_06 =Test(
            name='SDK_Imager_16199_06',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 5'),
                Step(Action.WAIT_FOR, camera_test_device, 'red balance: Failed! (ret = -1)'),

#if red balance support, un-comment this and test it!
#                Step(Action.WAIT_FOR, camera_test_device, 'redbalance: 0'),
#                Step(Action.WRITE, camera_test_device, 'camera redbalance: 32768'),
#                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
#                Step(Action.EXECUTE, camera_test_device, None,
#                    lambda test, source, data: wait_sec(test, source, data, 3, "redbalance = 0")),
#                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
#                Step(Action.WRITE, camera_test_device, 'camera check 5'),
#                Step(Action.WAIT_FOR, camera_test_device, 'redbalace: 32768'),
#                Step(Action.WRITE, camera_test_device, 'camera redbalance 65565'),
#                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
#                Step(Action.EXECUTE, camera_test_device, None,
#                    lambda test, source, data: wait_sec(test, source, data, 3, "redbalance = 127")),
#                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
#                Step(Action.WRITE, camera_test_device, 'camera check 5'),
#                Step(Action.WAIT_FOR, camera_test_device, 'redbalance: 65535'),
            ]
        )

        SDK_Imager_16199_07 =Test(
            name='SDK_Imager_16199_07',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 6'),
                Step(Action.WAIT_FOR, camera_test_device, 'blue balance: Failed! (ret = -1)'),

#if blue balance support, un-comment this and test it!
#                 Step(Action.WAIT_FOR, camera_test_device, 'bluebalance: 0'),
#                 Step(Action.WRITE, camera_test_device, 'camera bluebalance: 32768'),
#                 Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
#                 Step(Action.EXECUTE, camera_test_device, None,
#                     lambda test, source, data: wait_sec(test, source, data, 3, "bluebalance = 0")),
#                 Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
#                 Step(Action.WRITE, camera_test_device, 'camera check 6'),
#                 Step(Action.WAIT_FOR, camera_test_device, 'bluebalace: 32768'),
#                 Step(Action.WRITE, camera_test_device, 'camera bluebalance 65565'),
#                 Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
#                 Step(Action.EXECUTE, camera_test_device, None,
#                     lambda test, source, data: wait_sec(test, source, data, 3, "bluebalance = 127")),
#                 Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
#                 Step(Action.WRITE, camera_test_device, 'camera check 6'),
#                 Step(Action.WAIT_FOR, camera_test_device, 'bluebalance: 65535'),
            ]
        )

        SDK_Imager_16199_08 =Test(
            name='SDK_Imager_16199_08',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 8'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'gamma curve:'),
                Step(Action.WAIT_FOR, camera_test_device, 'data.controls->id = 8'),
                Step(Action.WAIT_FOR, camera_test_device, '0'),
                Step(Action.WAIT_FOR, camera_test_device, '7'),
                Step(Action.WAIT_FOR, camera_test_device, '29'),
                Step(Action.WAIT_FOR, camera_test_device, '47'),
                Step(Action.WAIT_FOR, camera_test_device, '61'),
                Step(Action.WAIT_FOR, camera_test_device, '74'),
                Step(Action.WAIT_FOR, camera_test_device, '81'),
                Step(Action.WAIT_FOR, camera_test_device, '90'),
                Step(Action.WAIT_FOR, camera_test_device, '97'),
                Step(Action.WAIT_FOR, camera_test_device, '106'),
                Step(Action.WAIT_FOR, camera_test_device, '73'),
                Step(Action.WAIT_FOR, camera_test_device, '130'),
                Step(Action.WAIT_FOR, camera_test_device, '173'),
                Step(Action.WAIT_FOR, camera_test_device, '204'),
                Step(Action.WAIT_FOR, camera_test_device, '225'),
                Step(Action.WAIT_FOR, camera_test_device, '237'),
                Step(Action.WAIT_FOR, camera_test_device, '246'),
                Step(Action.WAIT_FOR, camera_test_device, '262'),
                Step(Action.WAIT_FOR, camera_test_device, '268'),
                Step(Action.WAIT_FOR, camera_test_device, 'returned check value'),

                Step(Action.WRITE, camera_test_device, 'camera gamma 3'),
                Step(Action.WAIT_FOR, camera_test_device, '0'),
                Step(Action.WAIT_FOR, camera_test_device, '111'),
                Step(Action.WAIT_FOR, camera_test_device, '128'),
                Step(Action.WAIT_FOR, camera_test_device, '138'),
                Step(Action.WAIT_FOR, camera_test_device, '146'),
                Step(Action.WAIT_FOR, camera_test_device, '153'),
                Step(Action.WAIT_FOR, camera_test_device, '159'),
                Step(Action.WAIT_FOR, camera_test_device, '164'),
                Step(Action.WAIT_FOR, camera_test_device, '168'),
                Step(Action.WAIT_FOR, camera_test_device, '172'),
                Step(Action.WAIT_FOR, camera_test_device, '157'),
                Step(Action.WAIT_FOR, camera_test_device, '183'),
                Step(Action.WAIT_FOR, camera_test_device, '206'),
                Step(Action.WAIT_FOR, camera_test_device, '221'),
                Step(Action.WAIT_FOR, camera_test_device, '232'),
                Step(Action.WAIT_FOR, camera_test_device, '241'),
                Step(Action.WAIT_FOR, camera_test_device, '250'),
                Step(Action.WAIT_FOR, camera_test_device, '259'),
                Step(Action.WAIT_FOR, camera_test_device, '258'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Gamma changed Check Display")),


                Step(Action.WRITE, camera_test_device, 'camera check 8'),
                Step(Action.WAIT_FOR, camera_test_device, 'check test'),
                Step(Action.WAIT_FOR, camera_test_device, 'gamma curve:'),
                Step(Action.WAIT_FOR, camera_test_device, 'data.controls->id = 8'),
                Step(Action.WAIT_FOR, camera_test_device, '0'),
                Step(Action.WAIT_FOR, camera_test_device, '111'),
                Step(Action.WAIT_FOR, camera_test_device, '128'),
                Step(Action.WAIT_FOR, camera_test_device, '138'),
                Step(Action.WAIT_FOR, camera_test_device, '146'),
                Step(Action.WAIT_FOR, camera_test_device, '153'),
                Step(Action.WAIT_FOR, camera_test_device, '159'),
                Step(Action.WAIT_FOR, camera_test_device, '164'),
                Step(Action.WAIT_FOR, camera_test_device, '168'),
                Step(Action.WAIT_FOR, camera_test_device, '172'),
                Step(Action.WAIT_FOR, camera_test_device, '157'),
                Step(Action.WAIT_FOR, camera_test_device, '183'),
                Step(Action.WAIT_FOR, camera_test_device, '206'),
                Step(Action.WAIT_FOR, camera_test_device, '221'),
                Step(Action.WAIT_FOR, camera_test_device, '232'),
                Step(Action.WAIT_FOR, camera_test_device, '241'),
                Step(Action.WAIT_FOR, camera_test_device, '250'),
                Step(Action.WAIT_FOR, camera_test_device, '259'),
                Step(Action.WAIT_FOR, camera_test_device, '258'),

            ]
        )

        SDK_Imager_16199_09 = Test(
            name='SDK_Imager_16199_09',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 9'),
                Step(Action.WAIT_FOR, camera_test_device, 'exposure: 0'),

                Step(Action.WRITE, camera_test_device, 'camera exposure -6'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Exposure = -6")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 9'),
                Step(Action.WAIT_FOR, camera_test_device, 'exposure: -6'),

                Step(Action.WRITE, camera_test_device, 'camera exposure 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Exposure = 0")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 9'),
                Step(Action.WAIT_FOR, camera_test_device, 'exposure: 0'),

                Step(Action.WRITE, camera_test_device, 'camera exposure 6'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Exposure = 6")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 9'),
                Step(Action.WAIT_FOR, camera_test_device, 'exposure: 6'),
            ],
            teardown=teardown
        )

        SDK_Imager_16199_10 = Test(
            name='SDK_Imager_16199_10',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, camera_test_device, 'camera videostart'),
                    Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                    Step(Action.EXECUTE, camera_test_device, None,
                         lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                    Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                    Step(Action.WRITE, camera_test_device, 'camera check 10'),
                    Step(Action.WAIT_FOR, camera_test_device, 'hflip: 0'),
                    Step(Action.WRITE, camera_test_device, 'camera check 12'),
                    Step(Action.WAIT_FOR, camera_test_device, 'hflip_still: 0'),
                    Step(Action.WRITE, camera_test_device, 'camera hflip 1'),
                    Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                    Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                    Step(Action.EXECUTE, camera_test_device, None,
                         lambda test, source, data: wait_sec(test, source, data, 5, "Hflip ON")),
                    Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                    Step(Action.WRITE, camera_test_device, 'camera check 10'),
                    Step(Action.WAIT_FOR, camera_test_device, 'hflip: 1'),
                    Step(Action.WRITE, camera_test_device, 'camera check 12'),
                    Step(Action.WAIT_FOR, camera_test_device, 'hflip_still: 1'),

                    Step(Action.WRITE, camera_test_device, 'camera hflip 0'),
                    Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                    Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                    Step(Action.EXECUTE, camera_test_device, None,
                         lambda test, source, data: wait_sec(test, source, data, 5, "Hflip OFF")),
                    Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                    Step(Action.WRITE, camera_test_device, 'camera check 10'),
                    Step(Action.WAIT_FOR, camera_test_device, 'hflip: 0'),
                    Step(Action.WRITE, camera_test_device, 'camera check 12'),
                    Step(Action.WAIT_FOR, camera_test_device, 'hflip_still: 0'),

            ],
            teardown=teardown
        )


        SDK_Imager_16199_11 = Test(
            name='SDK_Imager_16199_11',
            timeout=timeout,
            setup=setup,
            test=[
                    Step(Action.WRITE, camera_test_device, 'camera videostart'),
                    Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                    Step(Action.EXECUTE, camera_test_device, None,
                         lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                    Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                    Step(Action.WRITE, camera_test_device, 'camera check 11'),
                    Step(Action.WAIT_FOR, camera_test_device, 'vflip: 0'),
                    Step(Action.WRITE, camera_test_device, 'camera check 13'),
                    Step(Action.WAIT_FOR, camera_test_device, 'vflip_still: 0'),
                    Step(Action.WRITE, camera_test_device, 'camera vflip 1'),
                    Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                    Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                    Step(Action.EXECUTE, camera_test_device, None,
                         lambda test, source, data: wait_sec(test, source, data, 5, "Vflip ON")),
                    Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                    Step(Action.WRITE, camera_test_device, 'camera check 11'),
                    Step(Action.WAIT_FOR, camera_test_device, 'vflip: 1'),
                    Step(Action.WRITE, camera_test_device, 'camera check 13'),
                    Step(Action.WAIT_FOR, camera_test_device, 'vflip_still: 1'),

                    Step(Action.WRITE, camera_test_device, 'camera vflip 0'),
                    Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                    Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                    Step(Action.EXECUTE, camera_test_device, None,
                         lambda test, source, data: wait_sec(test, source, data, 5, "Vflip OFF")),
                    Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                    Step(Action.WRITE, camera_test_device, 'camera check 11'),
                    Step(Action.WAIT_FOR, camera_test_device, 'vflip: 0'),
                    Step(Action.WRITE, camera_test_device, 'camera check 13'),
                    Step(Action.WAIT_FOR, camera_test_device, 'vflip_still: 0'),

            ],
            teardown=teardown
        )



        SDK_Imager_16199_12 = Test(
            name='SDK_Imager_16199_12',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 14'),
                Step(Action.WAIT_FOR, camera_test_device, 'sharpness: 32'),

                Step(Action.WRITE, camera_test_device, 'camera sharpness 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Sharpness = 0")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 14'),
                Step(Action.WAIT_FOR, camera_test_device, 'sharpness: 0'),

                Step(Action.WRITE, camera_test_device, 'camera sharpness 128'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Sharpness = 128")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 14'),
                Step(Action.WAIT_FOR, camera_test_device, 'sharpness: 128'),

                Step(Action.WRITE, camera_test_device, 'camera sharpness 255'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Sharpness = 255")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 14'),
                Step(Action.WAIT_FOR, camera_test_device, 'sharpness: 255'),

            ],
            teardown=teardown
        )

        SDK_Imager_16199_13 = Test(
            name='SDK_Imager_16199_13',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 15'),
                Step(Action.WAIT_FOR, camera_test_device, 'color killer: 0'),
                Step(Action.WRITE, camera_test_device, 'camera colorkill 1'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Colorkill ON")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                    Step(Action.WRITE, camera_test_device, 'camera check 15'),
                    Step(Action.WAIT_FOR, camera_test_device, 'color killer: 1'),
                Step(Action.WRITE, camera_test_device, 'camera colorkill 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Colorkill OFF")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 15'),
                Step(Action.WAIT_FOR, camera_test_device, 'color killer: 0'),



            ],
            teardown=teardown
        )


        SDK_Imager_16199_14 = Test(
            name='SDK_Imager_16199_14',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera videostart'),
                Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_s_param'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "Video start (Default setting)")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 0'),


                Step(Action.WRITE, camera_test_device, 'camera colorfx 0'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Colorfx = 0")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 0'),

                Step(Action.WRITE, camera_test_device, 'camera colorfx 1'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Colorfx = 1")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 1'),

                Step(Action.WRITE, camera_test_device, 'camera colorfx 2'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Colorfx = 2")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 2'),

                Step(Action.WRITE, camera_test_device, 'camera colorfx 3'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Colorfx = 3")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 3'),

                Step(Action.WRITE, camera_test_device, 'camera colorfx 5'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Colorfx = 5")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 5'),

                Step(Action.WRITE, camera_test_device, 'camera colorfx 13'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Colorfx = 13")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 13'),

                Step(Action.WRITE, camera_test_device, 'camera colorfx 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'ioctl calling'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "Colorfx = 16")),
                Step(Action.WAIT_FOR, camera_test_device, 'wait sec completed'),
                Step(Action.WRITE, camera_test_device, 'camera check 16'),
                Step(Action.WAIT_FOR, camera_test_device, 'color fx: 16'),


            ],
            teardown=teardown
        )

        SDK_Imager_16203_16202 = Test(
            name='SDK_Imager_16203_16202',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera format 200 200'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can on videocapture, jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can on videocapture, yuv'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can on stillcapture, jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can on stillcapture, yuv'),

                Step(Action.WRITE, camera_test_device, 'camera format 400 400'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can on videocapture, jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not videocapture, yuv'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can on stillcapture, jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not stillcapture, yuv'),

                Step(Action.WRITE, camera_test_device, 'camera format 1920 1020'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can on videocapture, jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not videocapture, yuv'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can on stillcapture, jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not stillcapture, yuv'),

                Step(Action.WRITE, camera_test_device, 'camera format 1280 960 320 240'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not videocapture, jpeg+yuv in jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can on videocapture, jpeg+yuv in yuv'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not stillcaputure jepg+yuv in jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can on stillcapture, jpeg+yuv in yuv'),

                Step(Action.WRITE, camera_test_device, 'camera format 12800 960 320 300'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not videocapture, jpeg+yuv in jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not videocapture, jpeg+yuv in yuv'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not stillcaputure jepg+yuv in jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not stillcaputure jepg+yuv in yuv'),
            ],
            teardown=teardown
        )

        SDK_Imager_16203 = Test(
            name='SDK_Imager_16203',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera format 500 300'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not videocapture, jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not videocapture, yuv'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can on stillcapture, jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not stillcapture, yuv'),
                Step(Action.WRITE, camera_test_device, 'camera format 3000 1000'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not videocapture, jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not videocapture, yuv'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not stillcapture, jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not stillcapture, yuv'),
                Step(Action.WRITE, camera_test_device, 'camera format 300 300 666 200'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not videocapture, jpeg+yuv in jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not videocapture, jpeg+yuv in yuv'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not stillcaputure jepg+yuv in jpeg'),
                Step(Action.WAIT_FOR, camera_test_device, 'it can not stillcaputure jepg+yuv in yuv'),
            ],
            teardown=teardown
        )

        SDK_Imager_16204= Test(
            name='SDK_Imager_16204',
            timeout=timeout,
            setup=setup,
            test=[
            Step(Action.WRITE, camera_test_device, 'camera buftest 0'),
            Step(Action.WAIT_FOR, camera_test_device, 'buffer test'),
            Step(Action.WAIT_FOR, camera_test_device, 'buftest 0'),
            Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_reqbufs'),
            Step(Action.WAIT_FOR, camera_test_device, 'ok video, fifo'),
            Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_reqbufs'),
            Step(Action.WAIT_FOR, camera_test_device, 'ok, video ring'),
            Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_reqbufs'),
            Step(Action.WAIT_FOR, camera_test_device, 'ok still fifo'),
            Step(Action.WAIT_FOR, camera_test_device, 'test_vidioc_reqbufs'),
            Step(Action.WAIT_FOR, camera_test_device, 'ok still ring'),
            ],
            teardown=teardown
        )

        SDK_Imager_16208= Test(
            name='SDK_Imager_16208',
            timeout=timeout,
            setup=setup,
            test=[
            Step(Action.WRITE, camera_test_device, 'camera buftest 1'),
            Step(Action.WAIT_FOR, camera_test_device, 'Fail QBUF 12'),
            ],
            teardown=teardown
        )

        SDK_Imager_16209= Test(
            name='SDK_Imager_16209',
            timeout=timeout,
            setup=setup,
            test=[
            Step(Action.WRITE, camera_test_device, 'camera buftest 1'),
            Step(Action.WAIT_FOR, camera_test_device, 'Fail de ok'),
            Step(Action.WRITE, camera_test_device, 'camera buftest 2'),
            Step(Action.WAIT_FOR, camera_test_device, 'deque with No que return error test'),
            Step(Action.WAIT_FOR, camera_test_device, 'Out of memory'),
            ],
            teardown=teardown
        )

        SDK_Imager_16214 = Test(
            name='SDK_Imager_16214',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.EXECUTE, camera_test_device, ('free_used_mon', free_used_monitor),
                    Test.add_monitor),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, "free")),
                Step(Action.WRITE, camera_test_device, 'free'),
                Step(Action.WAIT_FOR, camera_test_device, 'Mem'),
                Step(Action.WRITE, camera_test_device, 'camera init'),
                Step(Action.WAIT_FOR, camera_test_device, 'Do free'),
                Step(Action.WRITE, camera_test_device, 'free'),
                Step(Action.WAIT_FOR, camera_test_device, 'Mem'),
               ] + camera_open_close_100_times() +[ 

                Step(Action.WRITE, camera_test_device, 'free'),
                Step(Action.WAIT_FOR, camera_test_device, 'Mem'),

                Step(Action.EXECUTE, camera_test_device, 'free_used_mon', Test.remove_monitor),
            ],
            teardown=teardown
        )

        SDK_Imager_16215 = Test(
            name='SDK_Imager_16215',
            timeout=timeout,
            setup=setup+setup_mkdir_still(),
            test=[
                Step(Action.EXECUTE, camera_test_device, ('free_used_mon', free_used_monitor),
                 Test.add_monitor),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, "free")),
                Step(Action.WRITE, camera_test_device, 'free'),
                Step(Action.WAIT_FOR, camera_test_device, 'Mem'),
                Step(Action.WRITE, camera_test_device, 'camera init'),
                Step(Action.WAIT_FOR, camera_test_device, 'Do free'),
                Step(Action.WRITE, camera_test_device, 'free'),
                Step(Action.WAIT_FOR, camera_test_device, 'Mem'),
                Step(Action.WRITE, camera_test_device, 'camera stillc'),
                Step(Action.WAIT_FOR, camera_test_device, camera_test_device.STILL_DATA,
                     lambda test, source, data: store_still_file_path(test, source, data)),
                Step(Action.WAIT_FOR, camera_test_device, 'Stillc Confirmation'),
                Step(Action.EXECUTE, camera_test_device, None,
                     lambda test, source, data: mv_still_file(test, source, data)),
                Step(Action.WRITE, camera_test_device, 'camera close'),
                Step(Action.WAIT_FOR, camera_test_device, 'closed'),
                Step(Action.WRITE, camera_test_device, 'free'),
                Step(Action.WAIT_FOR, camera_test_device, 'Mem'),
                Step(Action.EXECUTE, camera_test_device, 'free_used_mon', Test.remove_monitor),

            ],
            teardown=teardown
        )

        SDK_Imager_16217_16216 = Test(
            name='SDK_Imager_16217_16216',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, camera_test_device, 'camera streamon'),
                Step(Action.WAIT_FOR, camera_test_device, 'video stream on test'),
                Step(Action.WAIT_FOR, camera_test_device, 'stream on'),
                Step(Action.WAIT_FOR, camera_test_device, 'stream on'),
                Step(Action.WRITE, camera_test_device, 'camera close'),
                Step(Action.WAIT_FOR, camera_test_device, 'closed'),
                Step(Action.WRITE, camera_test_device, 'camera streamon'),
                Step(Action.WAIT_FOR, camera_test_device, 'video stream on test'),
                Step(Action.WAIT_FOR, camera_test_device, 'stream on'),
                Step(Action.WAIT_FOR, camera_test_device, 'stream on'),                
            ],
            teardown=teardown
        )


        still_capture = TestGroup(
            name='still_capture',
            devices=[camera_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                
                ##SDK_Imager_16129,
                ##SDK_Imager_16130,
                ##SDK_Imager_16141,
                ##SDK_Imager_16142,
                ##SDK_Imager_16149,
                ##SDK_Imager_16150,
                SDK_Imager_16153,
                SDK_Imager_16154,
                SDK_Imager_16155,
                SDK_Imager_16156,
                SDK_Imager_16157,
                SDK_Imager_16158,
                SDK_Imager_16159,
                SDK_Imager_16160,
                SDK_Imager_16161,
                SDK_Imager_16162,
                SDK_Imager_16163,
                SDK_Imager_16220,
                SDK_Imager_16221,
                SDK_Imager_16164,
#                capture_stream_ctrl_02,
                SDK_Imager_16167,
                SDK_Imager_16197_16196,
                SDK_Imager_16285,
                SDK_Imager_16352,
            ]
        )
        video_capture = TestGroup(
            name='video_capture',
            devices=[camera_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Imager_16169_16168,
                SDK_Imager_16121,
                SDK_Imager_16122,
                SDK_Imager_16123,
                SDK_Imager_16171_16197_16198_16199_16124,
                SDK_Imager_16172_16197_16198_16199_16125,
                SDK_Imager_16173_16197_16151_16152_16199_16126,
                SDK_Imager_16174_16197_16151_16152_16199_16127,
                SDK_Imager_16175_16197_16199_16128,
                SDK_Imager_16178_16197_16199_16131,
                SDK_Imager_16179_16197_16199_16132,
                SDK_Imager_16180_16197_16199_16133,
                SDK_Imager_16181_16197_16199_16134,
                SDK_Imager_16182_16197_16199_16135,
                SDK_Imager_16183_16197_16199_16136,
                SDK_Imager_16184_16197_16199_16137,
                SDK_Imager_16185_16197_16199_16138,
                SDK_Imager_16186_16197_16139,
                SDK_Imager_16187_16197_16140,
                SDK_Imager_16190_16197_16143,
                SDK_Imager_16191_16197_16144,
                SDK_Imager_16192_16197_16145,
                SDK_Imager_16193_16197_16146,
                SDK_Imager_16194_16197_16147,
                SDK_Imager_16195_16197_16148,
                SDK_Imager_16165,
                SDK_Imager_16166_16170,
                SDK_Imager_16351,
            ]
        )

        camera_develop = TestGroup(
            name='camera_develop',
            devices=[camera_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Imager_16197,
#This group is to test test-script work
            ]
        )

        camera_gr = TestGroup(
            name='camera_develop',
            devices=[camera_test_device],
            tag=[Tag.POSITIVE],
            tests=[
               ##SDK_Imager_16199_01,
               ##SDK_Imager_16199_02,
               ##SDK_Imager_16199_03,
               ##SDK_Imager_16199_04,
               ##SDK_Imager_16199_05,
               ##SDK_Imager_16199_06,
               ##SDK_Imager_16199_07,
               ##SDK_Imager_16199_08,
               ##SDK_Imager_16199_09,
               ##SDK_Imager_16199_10,
               ##SDK_Imager_16199_11,
               ##SDK_Imager_16199_12,
               ##SDK_Imager_16199_13,
               ##SDK_Imager_16199_14,
               ##SDK_Imager_16203,
               SDK_Imager_16200,
               SDK_Imager_16201,
               SDK_Imager_16203_16202,
               SDK_Imager_16204,
               SDK_Imager_16208,
               SDK_Imager_16209,
               SDK_Imager_16214,
               SDK_Imager_16215,
               SDK_Imager_16217_16216,
            ]
        )


        test_groups = [
            still_capture,
            video_capture,
            camera_gr,
#            camera_develop,
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
    tc_runner = CameraTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
