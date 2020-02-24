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
from tc.adn_player_mp3.player_device import PlayerDevice
from tc.adn_player_mp3.recorder_device import RecorderDevice
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser
from utilities.usbmsc_mount import mountCurrentUSBDisk,umountCurrentUSBDisk,runBashFunction

PROJECT = 'spresense'
APPS_TO_BUILTIN = ['']
DSP_PATH = 'sdk/modules/audio/dsp'
DSP_CODERS = ['WAVDEC', 'MP3DEC', 'MP3ENC', 'SRC']
AUDIO_FILES = ['16340_15432', '16666', '16667', '16668']
AUDIO_EXAMPLE_PATH = '/mnt/sd0/'

TEST_AUDIO_FILE_FREQUENCY = 1000
FREQUENCY_VERIFY_ACCURACY = 2

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

def mountStart(test, source, data):
    ret = mountCurrentUSBDisk(25, '/tmp/sd0', source)
    if len(ret) == 0:
        test.set_fail('usbmsc mount failed')

def mountEnd(test, source, data):
    umountCurrentUSBDisk(5, '/tmp/sd0', source)

# noinspection PyUnusedLocal
def cp_file(test, source, data, file, dest_dir):
    dest_path = '/tmp/sd0/' + dest_dir
    test.log.debug('cp {}->{}'.format(file, dest_path))
    runBashFunction('mkdir -p {}'.format(dest_path))
    runBashFunction('cp -r {} {}'.format(file, dest_path))
    runBashFunction('sync')
    time.sleep(1)
    test.add_user_event(source, 'File {} copied'.format(file))
    return True

# noinspection PyUnusedLocal
def mv_file(test, source, data, file, dest_path, newfile):
    runBashFunction('mkdir -p {}'.format(dest_path))
    runBashFunction('rm -fr {}/{}'.format(dest_path, newfile))
    runBashFunction('sync')
    file_path = '/tmp/sd0/' + file
    test.log.debug('mv {}->{}/{}'.format(file_path, dest_path, newfile))
    runBashFunction('mv {} {}/{}'.format(file_path, dest_path, newfile))
    runBashFunction('sync')
    time.sleep(1)
    test.add_user_event(source, 'File {} moved'.format(file))
    return True

# noinspection PyUnusedLocal
def verify_recorded_file(test, source, data, file):
    test_file_path = os.path.join(os.getcwd(), 'output', file)

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


class AdnPlayerMp3TcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AdnPlayerMp3TcRunner, self).__init__(conf)

        self.player = PlayerDevice(kwargs.get('player', None), 'PLAYER_DEVICE') \
            if kwargs.get('player', None) else None
        self.recorder = RecorderDevice(kwargs.get('recorder', None), 'RECORDER_DEVICE') \
            if kwargs.get('recorder', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least two device is needed!')

        self.audio_dsp0 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[0])
        self.audio_dsp1 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[1])
        self.audio_dsp2 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[2])
        self.audio_dsp3 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[3])

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
                Step(Action.EXECUTE, device, None, lambda test, source, data: wait_sec(test, source, data, 5, 'wait sd')),
                Step(Action.EXECUTE, device, None, lambda test, source, data: mountStart(test, source, data)),
            ]
            return r

        def usb_e(device):
            r = [
                Step(Action.EXECUTE, device, None, lambda test, source, data: mountEnd(test, source, data)),
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
            r = [ ]
            for file in AUDIO_FILES:
                r += [
                    Step(Action.EXECUTE, player_device, None,
                         lambda test, source, data, filename=file: cp_file(test, source, data, os.path.join(os.getcwd(), 'input/' + filename + '.mp3'), '')),
                    Step(Action.WAIT_FOR, player_device, 'copied'),
                ]
            return r

        setup = [
            Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
            Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
        ]

        teardown = [
        ]

        tc_16340_15432 = Test(
            name='tc_16340_15432',
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
                Step(Action.WRITE, player_device, '16340_15432'),
                #Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
                Step(Action.WRITE, recorder_device, '15947'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization Audio Library'),
                Step(Action.WAIT_FOR, recorder_device, 'Init Recorder!'),
                Step(Action.WAIT_FOR, recorder_device, 'Write Header!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '15947.wav', os.getcwd() + '/output', '16340_15432.wav')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),

                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, '16340_15432.wav')),

                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=teardown
        )

        tests = []
        for i in range(1, len(AUDIO_FILES)):
            tests.append(Test(
                name='tc_' + AUDIO_FILES[i],
                timeout=timeout,
                setup=setup,
                test=[
                    #Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                    Step(Action.WRITE, player_device, AUDIO_FILES[i]),
                    #Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
                    Step(Action.WRITE, recorder_device, '15947'),
                    Step(Action.WAIT_FOR, recorder_device, 'initialization Audio Library'),
                    Step(Action.WAIT_FOR, recorder_device, 'Init Recorder!'),
                    Step(Action.WAIT_FOR, recorder_device, 'Write Header!'),
                    Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                    Step(Action.WAIT_FOR, recorder_device, 'End Recording'),

                    Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
                    ] + usb_s(recorder_device) + [
                    Step(Action.EXECUTE, recorder_device, None,
                          lambda test, source, data, index=i: mv_file(test, source, data, '15947.wav', os.getcwd() + '/output', AUDIO_FILES[index] + '.wav')),
                    Step(Action.WAIT_FOR, recorder_device, 'moved'),
                    Step(Action.WRITE, recorder_device, 'end'),
                    Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                         lambda test, source, data, index=i: verify_recorded_file(test, source, data, AUDIO_FILES[index] + '.wav')),
                    Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
                ],
                teardown=teardown
            ))

        test_group = TestGroup(
            name='test_group',
            devices=[player_device, recorder_device],
            tag=[Tag.POSITIVE],
            tests=[
                 tc_16340_15432,
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
    tc_runner = AdnPlayerMp3TcRunner(config, player=player, recorder=recorder)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
