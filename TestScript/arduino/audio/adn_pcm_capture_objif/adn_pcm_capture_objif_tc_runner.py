#! /usr/bin/env python3
import sys
import os
import subprocess
import time
import shutil
import getpass
import numpy
import wave
import io


SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.adn_pcm_capture_objif.player_device import PlayerDevice
from tc.adn_pcm_capture_objif.recorder_device import RecorderDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger, Timer
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['']
DSP_PATH = 'sdk/modules/audio/dsp'
DSP_CODERS = ['WAVDEC', 'MP3DEC', 'MP3ENC', 'SRC']

AUDIO_EXAMPLE_PATH = '/mnt/sd0/'
global gmedia_dict
gmedia_dict = {}
global gmedia_dict2
gmedia_dict2 = {}
global gmedia_path
gmedia_path = {}

TEST_AUDIO_FILE_FREQUENCY = 1000
FREQUENCY_VERIFY_ACCURACY = 2


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
                test.set_fail('usbmsc muount failed')
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
def mv_file(test, source, data, file, dest_path, newfile):
    usbcommand(test, source, data, 'mkdir -p {}'.format(dest_path))
    usbcommand(test, source, data, 'rm -fr {}/{}'.format(dest_path, newfile))
    usbcommand(test, source, data, 'sync')
    file_path = gmedia_path['usb'] + '/' + file
    test.log.info('mv {}->{}/{}'.format(file_path, dest_path, newfile))
    usbcommand(test, source, data, 'mv {} {}/{}'.format(file_path, dest_path, newfile))
    usbcommand(test, source, data, 'sync')
    time.sleep(1)
    test.add_user_event(source, 'File {} moved'.format(file))
    return True

class AdnPlayerRecorderExampleTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AdnPlayerRecorderExampleTestTcRunner, self).__init__(conf)

        self.player = PlayerDevice(kwargs.get('player', None), 'PLAYER_DEVICE') \
            if kwargs.get('player', None) else None
        self.recorder = RecorderDevice(kwargs.get('recorder', None), 'RECORDER_DEVICE') \
            if kwargs.get('recorder', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least two devices are needed!')

        self.audio_dsp0 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[0])
        self.audio_dsp1 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[1])
        self.audio_dsp2 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[2])
        self.audio_dsp3 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[3])

    def get_devices_list(self):
        return [self.player, self.recorder]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(player_bin=None, recorder_bin=None)

        binaries['player_bin'] = os.path.normpath(arguments.player_bin)\
            if arguments.player_bin else None
        binaries['recorder_bin'] = os.path.normpath(arguments.recorder_bin)\
            if arguments.recorder_bin else None

        # binaries = self.build(debug, log, **binaries)

        # self.flash_n_check(self.player, binaries['player_bin'], log, PLAYER_APPS)
        # self.flash_n_check(self.recorder, binaries['recorder_bin'], log, RECORDER_APPS)

    # noinspection PyUnresolvedReferences
    def build(self, debug=True, log=None, **kwargs):
        player_bin = kwargs.get('player_bin', None)
        recorder_bin = kwargs.get('recorder_bin', None)
        toolbox = None

        if not all([player_bin, recorder_bin]):
            print (not all)
            if log:
                log.info('Building project sources')

            toolbox = Toolbox(self.config, log)
            toolbox.init_builder_module()

        if not player_bin:
            print('not player_bin')
            log.info('Building {}'.format(self.player))
            player_bin = self.__build_binary(toolbox, PLAYER_APPS_TO_BUILTIN, self.player,
                                             self.zmodem_defconfig)

        if not recorder_bin:
            print('not recorder_bin')
            log.info('Building {}'.format(self.recorder))
            recorder_bin = self.__build_binary(toolbox, RECORDER_APPS_TO_BUILTIN, self.recorder,
                                               self.recorder_test_defconfig)

        return dict(player_bin=player_bin, recorder_bin=recorder_bin)

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
        timeout = 300

        player_device, recorder_device = self.get_devices_list()

        def usb_s(device):
            r = [
                Step(Action.EXECUTE, device, None, lambda test, source, data: reset_data(test, source, data)),
                Step(Action.EXECUTE, device, None, lambda test, source, data: wait_sec(test, source, data, 5, 'wait sd')),
                Step(Action.EXECUTE, device, None, lambda test, source, data: checkDefaultDisk(test, source, data)),

                Step(Action.WRITE, device, 'usbmsc'),
                Step(Action.WAIT_FOR, device, 'Begin USB Mass Storage Operation',
                     lambda test, source, data: wait_sec(test, source, data, 25, 'usb mount to PC')),

                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: checkNewDisk(test, source, data)),
            ]
            return r

        def usb_e(device):
            r = [
                Step(Action.WRITE, device, 'end'),
                Step(Action.WAIT_FOR, device, 'Finish USB Mass Storage Operation'),
            ]
            return r

        def dsp_steps(device):
            r = [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: cp_file(test, source, data, self.audio_dsp0, 'BIN')),
                Step(Action.WAIT_FOR, device, 'copied'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: cp_file(test, source, data, self.audio_dsp1, 'BIN')),
                Step(Action.WAIT_FOR, device, 'copied'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: cp_file(test, source, data, self.audio_dsp2, 'BIN')),
                Step(Action.WAIT_FOR, device, 'copied'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: cp_file(test, source, data, self.audio_dsp3, 'BIN')),
                Step(Action.WAIT_FOR, device, 'copied'),
            ]
            return r

        def add_file_to_player():
            r = [
                Step(Action.EXECUTE, player_device, None,
                     lambda test, source, data, filename='HiResSound': cp_file(test, source, data, os.path.join(os.getcwd(), 'input_192k_stereo_wav/' + filename + '.wav'), '')),
                Step(Action.WAIT_FOR, player_device, 'copied'),
            ]
            return r

        setup = [
            Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
            Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
        ]

        teardown = [
        ]


        tc_player_15946_recorder_16339_16262 = Test(
            name='tc_player_15946_recorder_16339_16262',
            timeout=timeout,
            setup=setup,
            test=usb_s(recorder_device) + \
                 dsp_steps(recorder_device) + \
                 usb_e(recorder_device) + \
                 usb_s(player_device) + \
                 dsp_steps(player_device) + \
                 add_file_to_player() + \
                 usb_e(player_device) + [
                Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.WRITE, player_device, '15946'),
                # Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
                Step(Action.WAIT_FOR, player_device, 'initialization Audio Library'),
                Step(Action.WRITE, recorder_device, '16339'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, player_device, 'Open!'),
                Step(Action.WAIT_FOR, player_device, 'Play!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, player_device, 'Main player File End!'),
                Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.WRITE, player_device, '15946'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording',
                     lambda test, source, data: test.stop()),
            ],
            teardown=teardown,
        )

        tc_player_15946_recorder_16260 = Test(
            name='tc_player_15946_recorder_16260',
            timeout=timeout,
            setup=setup,
            test=[
                #Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.WRITE, player_device, '15946'),
                # Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
                Step(Action.WAIT_FOR, player_device, 'initialization Audio Library'),
                Step(Action.WRITE, recorder_device, '16260'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, player_device, 'Open!'),
                Step(Action.WAIT_FOR, player_device, 'Play!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, player_device, 'Main player File End!'),
                Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.WRITE, player_device, '15946'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording',
                     lambda test, source, data: test.stop()),
            ],
            teardown=teardown,
        )

        tc_player_15946_recorder_16261 = Test(
            name='tc_player_15946_recorder_16261',
            timeout=timeout,
            setup=setup,
            test=[
                #Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.WRITE, player_device, '15946'),
                # Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
                Step(Action.WAIT_FOR, player_device, 'initialization Audio Library'),
                Step(Action.WRITE, recorder_device, '16261'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, player_device, 'Open!'),
                Step(Action.WAIT_FOR, player_device, 'Play!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, player_device, 'Main player File End!'),
                Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.WRITE, player_device, '15946'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording',
                     lambda test, source, data: test.stop()),
            ],
            teardown=teardown,
        )

        tc_player_15946_recorder_16288 = Test(
            name='tc_player_15946_recorder_16288',
            timeout=timeout,
            setup=setup,
            test=[
                #Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.WRITE, player_device, '15946'),
                # Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
                Step(Action.WAIT_FOR, player_device, 'initialization Audio Library'),
                Step(Action.WRITE, recorder_device, '16288'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, player_device, 'Open!'),
                Step(Action.WAIT_FOR, player_device, 'Play!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, player_device, 'Main player File End!'),
                Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.WRITE, player_device, '15946'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording',
                     lambda test, source, data: test.stop()),
            ],
            teardown=teardown,
        )

        tc_player_15946_recorder_16289 = Test(
            name='tc_player_15946_recorder_16289',
            timeout=timeout,
            setup=setup,
            test=[
                #Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.WRITE, player_device, '15946'),
                # Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
                Step(Action.WAIT_FOR, player_device, 'initialization Audio Library'),
                Step(Action.WRITE, recorder_device, '16289'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, player_device, 'Open!'),
                Step(Action.WAIT_FOR, player_device, 'Play!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, player_device, 'Main player File End!'),
                Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.WRITE, player_device, '15946'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording',
                     lambda test, source, data: test.stop()),
            ],
            teardown=teardown,
        )

        tc_player_15946_recorder_16290 = Test(
            name='tc_player_15946_recorder_16290',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, player_device, '15946'),
                Step(Action.WAIT_FOR, player_device, 'initialization Audio Library'),
                Step(Action.WRITE, recorder_device, '16290'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, player_device, 'Open!'),
                Step(Action.WAIT_FOR, player_device, 'Play!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, player_device, 'Main player File End!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording',
                     lambda test, source, data: test.stop()),
            ],
            teardown=teardown,
        )

        audio = TestGroup(
            name='audio',
            devices=[player_device, recorder_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_player_15946_recorder_16339_16262,
                tc_player_15946_recorder_16260,
                tc_player_15946_recorder_16261,
                tc_player_15946_recorder_16288,
                tc_player_15946_recorder_16289,
                tc_player_15946_recorder_16290,
            ]
        )

        test_groups = [
            audio,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()
    parser.add_argument('--player_bin', help='Player binary file path')
    parser.add_argument('--recorder_bin', help='Recorder binary file path')
    parser.add_argument('--player', metavar='SERIAL_PORT', help='Set player')
    parser.add_argument('--recorder', metavar='SERIAL_PORT', help='Set recorder')
    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    player, recorder = dev_manager.get_devices_by_serials(args.player, args.recorder)

    # Create test runner instance
    tc_runner = AdnPlayerRecorderExampleTestTcRunner(config, player=player, recorder=recorder)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
