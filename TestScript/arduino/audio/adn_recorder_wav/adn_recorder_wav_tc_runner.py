#! /usr/bin/env python3
import sys
import os
import time
import shutil
import getpass
import numpy
import wave
import subprocess
import io

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.adn_recorder_wav.player_device import PlayerDevice
from tc.adn_recorder_wav.recorder_device import RecorderDevice
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['']
DSP_PATH = 'sdk/modules/audio/dsp'
DECODERS = ['WAVDEC']
ENCODERS = ['SRC']
PLAY_FILES = ['16660']
RECORD_FILES = ['15947', '16645', '16646', '16647', '16648', '16649',
                '16651', '16652', '16653', '16654', '16655', '16656']
CHANNELS = [2, 1, 2, 4, 1, 4,
            1, 2, 1, 2, 1, 2]
BITLENGTH = [16, 16, 16, 16, 16, 16,
             24, 24, 16, 16, 24, 24]
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
        if ch is 1:
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


class AdnRecorderWavTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AdnRecorderWavTcRunner, self).__init__(conf)

        self.player = PlayerDevice(kwargs.get('player', None), 'PLAYER_DEVICE') \
            if kwargs.get('player', None) else None
        self.recorder = RecorderDevice(kwargs.get('recorder', None), 'RECORDER_DEVICE') \
            if kwargs.get('recorder', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least two device is needed!')

        self.audio_decoder = os.path.join(self.config.projects[0].path, DSP_PATH, DECODERS[0])
        self.audio_encoder = os.path.join(self.config.projects[0].path, DSP_PATH, ENCODERS[0])

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
                     lambda test, source, data: cp_file(test, source, data, self.audio_encoder, 'BIN')),
                Step(Action.WAIT_FOR, device, 'copied'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: cp_file(test, source, data, self.audio_decoder, 'BIN')),
                Step(Action.WAIT_FOR, device, 'copied'),
            ]
            return r

        def add_file_to_player():
            r = [ ]
            for file in PLAY_FILES:
                r += [
                    Step(Action.EXECUTE, player_device, None,
                         lambda test, source, data, filename=file: cp_file(test, source, data, os.path.join(os.getcwd(), 'input/' + filename + '.wav'), '')),
                    Step(Action.WAIT_FOR, player_device, 'copied'),
                ]
            return r

        setup = [
            Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
            Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
        ]

        teardown = [
        ]

        tc_15947_16346 = Test(
            name='tc_15947_16346',
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
                Step(Action.WRITE, player_device, '16660'),
                Step(Action.WRITE, recorder_device, '15947'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization Audio Library'),
                Step(Action.WAIT_FOR, recorder_device, 'Init Recorder!'),
                Step(Action.WAIT_FOR, recorder_device, 'Write Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '15947.wav', os.getcwd() + '/output', '15947.wav')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),

                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, '15947.wav')),

                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=teardown
        )

        tests = []
        for i in range(1, len(RECORD_FILES)):
            tests.append(Test(
                name='tc_' + RECORD_FILES[i],
                timeout=timeout,
                setup=setup,
                test=[
                    #Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                    Step(Action.WRITE, player_device, '16660'),
                    Step(Action.WRITE, recorder_device, RECORD_FILES[i]),
                    Step(Action.WAIT_FOR, recorder_device, 'initialization Audio Library'),
                    Step(Action.WAIT_FOR, recorder_device, 'Init Recorder!'),
                    Step(Action.WAIT_FOR, recorder_device, 'Write Header!'),
                    Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                    Step(Action.WAIT_FOR, recorder_device, 'End Recording'),

                    Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
                    ] + usb_s(recorder_device) + [
                    Step(Action.EXECUTE, recorder_device, None,
                         lambda test, source, data, index=i: mv_file(test, source, data, RECORD_FILES[index] + '.wav', os.getcwd() + '/output', RECORD_FILES[index] + '.wav')),
                    Step(Action.WAIT_FOR, recorder_device, 'moved'),
                    Step(Action.WRITE, recorder_device, 'end'),
                    Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                         lambda test, source, data, index=i: verify_recorded_file(test, source, data, RECORD_FILES[index] + '.wav', CHANNELS[index], dtype=BITLENGTH[index])),
                    Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
                ],
                teardown=teardown
            ))

        test_group = TestGroup(
            name='test_group',
            devices=[player_device, recorder_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_15947_16346,
            ] + tests
        )

        test_groups = [
            test_group,
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
    tc_runner = AdnRecorderWavTcRunner(config, player=player, recorder=recorder)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
