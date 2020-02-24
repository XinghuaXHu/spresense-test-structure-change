#! /usr/bin/env python3
import sys
import os
import subprocess
import time
import shutil
import getpass
import numpy
import wave

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.adn_recorder_wav_objif_test.player_device import PlayerDevice
from tc.adn_recorder_wav_objif_test.recorder_device import AdnRecorderWavObjifTestDevice
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['']
DSP_PATH = 'sdk/modules/audio/dsp'
DSP_CODERS = ['WAVDEC', 'MP3DEC', 'MP3ENC', 'SRC']
AUDIO_FILE = 'input_48k_stereo/Sound.mp3'

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


# noinspection PyUnusedLocal
def verify_recorded_file(test, source, data, file, ch=1, dtype=16):
    test_file_path = os.path.join(os.getcwd(), 'output', file)

    wave_read = wave.open(test_file_path)
    sample_rate = wave_read.getframerate()
    frames_number = wave_read.getnframes()

    k = numpy.arange(frames_number)
    T = frames_number / sample_rate
    frequencies = k / T
    frequencies = frequencies[range(int(frames_number / 2))]

    signal = wave_read.readframes(frames_number)

    if dtype is not 16:
        tmp = numpy.frombuffer(signal, dtype=numpy.uint8)
        signal = numpy.zeros(tmp.shape[0] // 3 // ch, dtype=numpy.int32)
        for i in range(len(signal)):
            signal[i] = int.from_bytes([tmp[3*ch*i], tmp[3*ch*i+1], tmp[3*ch*i+2]], byteorder='little', signed=True)
        ft = numpy.fft.fft(signal) / frames_number
    else:
        if ch == 1:
            signal = numpy.frombuffer(signal, dtype=numpy.int16)
            ft = numpy.fft.fft(signal) / frames_number
        else:
            signal = numpy.frombuffer(signal, dtype=numpy.int16)
            signal = signal.reshape((-1, ch))
            signal = signal.T
            ft = numpy.fft.fft(signal[0]) / frames_number

    ft = ft[range(int(frames_number / 2))]
    ft = abs(ft)

    frequency = frequencies[numpy.argmax(ft)]

    test.log.debug('Found frequency {} Hz'.format(frequency))

    test.assert_true(int(frequency) in range(TEST_AUDIO_FILE_FREQUENCY - FREQUENCY_VERIFY_ACCURACY,
                                             TEST_AUDIO_FILE_FREQUENCY + FREQUENCY_VERIFY_ACCURACY),
                     'Recorded file verification')
    test.add_user_event(source, 'Recorded file verification success')
    return True


class AdnRecorderWavObjifTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AdnRecorderWavObjifTestTcRunner, self).__init__(conf)

        self.player = PlayerDevice(kwargs.get('player', None), 'PLAYER_DEVICE') \
            if kwargs.get('player', None) else None
        self.recorder = AdnRecorderWavObjifTestDevice(kwargs.get('recorder', None), 'RECORDER_DEVICE') \
            if kwargs.get('recorder', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least two device is needed!')

        self.audio_dsp0 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[0])
        self.audio_dsp1 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[1])
        self.audio_dsp2 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[2])
        self.audio_dsp3 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[3])
        self.audio_file = os.path.join(os.getcwd(), AUDIO_FILE)

    def get_devices_list(self):
        return [self.player, self.recorder]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None
        time.sleep(5)

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
                     lambda test, source, data: cp_file(test, source, data, os.path.join(os.getcwd(), self.audio_file), '')),
                Step(Action.WAIT_FOR, player_device, 'copied'),
            ]
            return r

        setup = [
            Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
            Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
        ]

        teardown = [
        ]

        adn_recorder_wav_objif_tc_16263 = Test(
            name='adn_recorder_wav_objif_tc_16263',
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
                Step(Action.WRITE, player_device, 'player'),
                # Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
                Step(Action.WRITE, recorder_device, '16263'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, recorder_device, 'Open!'),
                Step(Action.WAIT_FOR, recorder_device, 'Write Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'Update Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '16263.wav', os.getcwd() + '/output', '16263.wav')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, '16263.wav')),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=teardown
        )

        adn_recorder_wav_objif_tc_16264 = Test(
            name='adn_recorder_wav_objif_tc_16264',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, player_device, 'player'),
                Step(Action.WRITE, recorder_device, '16264'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, recorder_device, 'Open!'),
                Step(Action.WAIT_FOR, recorder_device, 'Write Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'Update Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '16264.wav', os.getcwd() + '/output', '16264.wav')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, '16264.wav', 2)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=teardown
        )

        adn_recorder_wav_objif_tc_16265 = Test(
            name='adn_recorder_wav_objif_tc_16265',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, player_device, 'player'),
                Step(Action.WRITE, recorder_device, '16265'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, recorder_device, 'Open!'),
                Step(Action.WAIT_FOR, recorder_device, 'Write Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'Update Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '16265.wav', os.getcwd() + '/output', '16265.wav')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, '16265.wav', 4)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=teardown
        )

        adn_recorder_wav_objif_tc_16266 = Test(
            name='adn_recorder_wav_objif_tc_16266',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, player_device, 'player'),
                Step(Action.WRITE, recorder_device, '16266'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, recorder_device, 'Open!'),
                Step(Action.WAIT_FOR, recorder_device, 'Write Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'Update Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '16266.wav', os.getcwd() + '/output', '16266.wav')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, '16266.wav', 1)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=teardown
        )

        adn_recorder_wav_objif_tc_16267 = Test(
            name='adn_recorder_wav_objif_tc_16267',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, player_device, 'player'),
                Step(Action.WRITE, recorder_device, '16267'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, recorder_device, 'Open!'),
                Step(Action.WAIT_FOR, recorder_device, 'Write Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'Update Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '16267.wav', os.getcwd() + '/output', '16267.wav')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, '16267.wav', 2)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=teardown
        )

        adn_recorder_wav_objif_tc_16304 = Test(
            name='adn_recorder_wav_objif_tc_16304',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, player_device, 'player'),
                Step(Action.WRITE, recorder_device, '16304'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, recorder_device, 'Open!'),
                Step(Action.WAIT_FOR, recorder_device, 'Write Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'Update Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '16304.wav', os.getcwd() + '/output', '16304.wav')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, '16304.wav', 1, 24)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=teardown
        )

        adn_recorder_wav_objif_tc_16305 = Test(
            name='adn_recorder_wav_objif_tc_16305',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, player_device, 'player'),
                Step(Action.WAIT_FOR, player_device, 'Play!'),
                Step(Action.WRITE, recorder_device, '16305'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, recorder_device, 'Open!'),
                Step(Action.WAIT_FOR, recorder_device, 'Write Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'Update Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '16305.wav', os.getcwd() + '/output', '16305.wav')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, '16305.wav', 2, 24)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=teardown
        )

        adn_recorder_wav_objif_tc_16308 = Test(
            name='adn_recorder_wav_objif_tc_16308',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, player_device, 'player'),
                Step(Action.WRITE, recorder_device, '16308'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization MediaRecorder'),
                Step(Action.WAIT_FOR, recorder_device, 'Open!'),
                Step(Action.WAIT_FOR, recorder_device, 'Write Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'Update Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input case number'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '16308.wav', os.getcwd() + '/output', '16308.wav')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, '16308.wav', 1, 24)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=teardown
        )

        adn_recorder_wav_objif_test_group = TestGroup(
            name='adn_recorder_wav_objif_test_group',
            devices=[player_device, recorder_device],
            tag=[Tag.POSITIVE],
            tests=[
                adn_recorder_wav_objif_tc_16263,
                adn_recorder_wav_objif_tc_16264,
                adn_recorder_wav_objif_tc_16265,
                adn_recorder_wav_objif_tc_16266,
                adn_recorder_wav_objif_tc_16267,
                adn_recorder_wav_objif_tc_16304,
                adn_recorder_wav_objif_tc_16305,
                adn_recorder_wav_objif_tc_16308,
            ]
        )

        test_groups = [
            adn_recorder_wav_objif_test_group,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()
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
    tc_runner = AdnRecorderWavObjifTestTcRunner(config, player=player, recorder=recorder)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
