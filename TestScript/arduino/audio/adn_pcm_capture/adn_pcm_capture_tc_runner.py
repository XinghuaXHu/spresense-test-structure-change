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
from tc.adn_pcm_capture.player_device import PlayerDevice
from tc.adn_pcm_capture.recorder_device import RecorderDevice
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger, Timer
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
DSP_PATH = 'sdk/modules/audio/dsp'
DECODERS = ['MP3DEC', 'WAVDEC']
SRC = 'SRC'
ENCODERS = ['MP3ENC']

AUDIO_FILE = 'input/16660.wav'

BIN = 'BIN'

AUDIO_EXAMPLE_PATH = '/mnt/sd0/'

TEST_AUDIO_FILE_FREQUENCY = 1000
FREQUENCY_VERIFY_ACCURACY = 2
global gmedia_dict
gmedia_dict = {}

global gmedia_dict2
gmedia_dict2 = {}

global gmedia_path
gmedia_path = {}

# noinspection PyUnusedLocal
def reset_data(test, source, event):
    gmedia_path = {}
    gmedia_dict2 = {}
    gmedia_dict = {}

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
            test.log.info(str1Arr[2])
            gmedia_path['usb'] = str1Arr[2]
            return True

def wait_sec(test, source, data, sec, description):
    if description is not None:
        test.log.info("")
        test.log.info("#############################################")
        test.log.info("## [" + str(sec) + " sec] "+ description)
        test.log.info("#############################################")
        test.log.info("")
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')
    return True

# noinspection PyUnusedLocal
def player_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


# noinspection PyUnusedLocal
def recorder_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


# noinspection PyUnusedLocal
def player_error_monitor(test, source, data):
    if source.APP_ERROR in data:
        test.set_fail('Player error: {}'.format(data.get_value(source.KEY_ERROR)))


# noinspection PyUnusedLocal
def recorder_error_monitor(test, source, data):
    if source.APP_ERROR in data:
        test.set_fail('Recorder error: {}'.format(data.get_value(source.KEY_ERROR)))


def end_test(test, source, data):
    test.set_fail('Failed to transfer file')

def usbcommand(test, source, data, command):
    p3 = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
    (output3, err3) = p3.communicate()
    p3.wait()
    if None != err3 or output3 != b'':
        test.set_fail('cmd NG')

# noinspection PyUnusedLocal
def cp_file(test, source, data, file, dest_dir):
    dest_path = gmedia_path['usb'] + '/' + dest_dir
    test.log.info('{}->{}'.format(file, dest_path))
    usbcommand(test, source, data, 'mkdir -p {}'.format(dest_path))
    usbcommand(test, source, data, 'cp -r {} {}'.format(file, dest_path))
    usbcommand(test, source, data, 'sync')
    time.sleep(1)
    test.add_user_event(source, 'File {} copied'.format(file))
    return True


# noinspection PyUnusedLocal
def verify_recorded_file(test, source, data):
    test_file_path = os.path.join(os.getcwd(), 'output', test.storage.rec_file_path.split('/')[-1])

    wave_read = wave.open(test_file_path)
    sample_rate = wave_read.getframerate()
    frames_number = wave_read.getnframes()

    k = numpy.arange(frames_number)
    T = frames_number / sample_rate
    frequencies = k / T
    frequencies = frequencies[range(int(frames_number / 2))]

    signal = wave_read.readframes(frames_number)
    signal = numpy.frombuffer(signal, numpy.int16)

    ft = numpy.fft.fft(signal) / frames_number
    ft = ft[range(int(frames_number / 2))]
    ft = abs(ft)

    frequency = frequencies[numpy.argmax(ft)]

    test.log.debug('Found frequency {} Hz'.format(frequency))

    test.assert_true(int(frequency) in range(TEST_AUDIO_FILE_FREQUENCY - FREQUENCY_VERIFY_ACCURACY,
                                             TEST_AUDIO_FILE_FREQUENCY + FREQUENCY_VERIFY_ACCURACY),
                     'Recorded file verification')
    test.add_user_event(source, 'Recorded file verification success')
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

        self.audio_decoder = os.path.join(self.config.projects[0].path, DSP_PATH)
        self.audio_encoder = os.path.join(self.config.projects[0].path, DSP_PATH)
        self.src = os.path.join(self.config.projects[0].path, DSP_PATH, SRC)
        self.audio_file = os.path.join(os.getcwd(), AUDIO_FILE)

    def get_devices_list(self):
        return [self.player, self.recorder]

    def setup(self, arguments, log=None):
        debug = not arguments.release

    def generate_test_groups(self, arguments, log=None):
        timeout = 300

        player_device, recorder_device = self.get_devices_list()

        def prepare_player_fs_steps():
            return [
                Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.EXECUTE, player_device, None, lambda test, source, data: reset_data(test, source, data)),
                Step(Action.EXECUTE, player_device, None, lambda test, source, data: wait_sec(test, source, data, 5, 'wait sd')),
                Step(Action.EXECUTE, player_device, None, lambda test, source, data: checkDefaultDisk(test, source, data)),

                Step(Action.WRITE, player_device, 'usbmsc'),
                Step(Action.WAIT_FOR, player_device, 'Begin USB Mass Storage Operation',
                     lambda test, source, data: wait_sec(test, source, data, 15, 'usb mount to PC')),

                Step(Action.EXECUTE, player_device, None,
                     lambda test, source, data: checkNewDisk(test, source, data)),

                Step(Action.EXECUTE, player_device, None,
                     lambda test, source, data: cp_file(test, source, data, os.path.join(os.getcwd(), 'input/16660.wav'), '')),
                Step(Action.WAIT_FOR, player_device, 'copied',
                     lambda test, source, data: cp_file(test, source, data, os.path.join(self.config.projects[0].path, 'sdk/modules/audio/dsp/WAVDEC'), 'BIN')),
                Step(Action.WRITE, player_device, 'end'),
                Step(Action.WAIT_FOR, player_device, 'Finish USB Mass Storage Operation'),
            ]

        tc_16338 = Test(
            name='tc_16338',
            timeout=timeout,
            setup=prepare_player_fs_steps(),
            test=[
                Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.WRITE, player_device, '16660'),
                Step(Action.WAIT_FOR, player_device, 'initialization Audio Library'),
                Step(Action.WRITE, recorder_device, '16336'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization Audio Library'),
                Step(Action.WAIT_FOR, player_device, 'Open!'),
                Step(Action.WAIT_FOR, player_device, 'Play!'),
                Step(Action.WAIT_FOR, recorder_device, 'Rec!'),
                Step(Action.WAIT_FOR, player_device, 'Main player File End!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording',
                     lambda test, source, data: test.stop()),
            ],
            teardown=[],
        )

        audio = TestGroup(
            name='audio',
            devices=[player_device, recorder_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_16338,
            ]
        )

        test_groups = [
            audio,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()

    parser.add_argument('--player', metavar='SERIAL_PORT', help='Set player')
    parser.add_argument('--recorder', metavar='SERIAL_PORT', help='Set recorder')
    parser.add_argument('--player_fs_ready', action='store_true',
                        help='Player folders and files already created')
    parser.add_argument('--recorder_fs_ready', action='store_true',
                        help='Recorder folders and files already created')
    parser.add_argument('--preserve_player_fs', action='store_true',
                        help='Do not remove player folders and files after test')
    parser.add_argument('--preserve_recorder_fs', action='store_true',
                        help='Do not remove recorder folders and files after test')

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
