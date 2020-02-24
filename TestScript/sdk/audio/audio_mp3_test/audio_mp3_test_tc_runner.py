#! /usr/bin/env python3
import sys
import os
import subprocess
import time
import numpy
import wave
import io
import time


SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.audio_mp3_test.player_device import PlayerDevice
from tc.audio_mp3_test.recorder_device import RecorderDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger, Timer
from api.config import Config
from api.runner_parser import RunnerParser
from utilities.usbmsc_mount import mountCurrentUSBDisk,umountCurrentUSBDisk,runBashFunction

PROJECT = 'spresense'
PLAYER_APPS = ['audio_player', 'zmodem']
RECORDER_APPS = ['audio_recorder', 'zmodem']
PLAYER_APPS_TO_BUILTIN = ['examples/audio_player', 'zmodem']
RECORDER_APPS_TO_BUILTIN = ['examples/audio_recorder', 'zmodem']

UPDATER = 'updater.py'
UPDATER_PATH = 'autotest/src/api'
DSP_PATH = 'sdk/modules/audio/dsp'
DECODER = 'MP3DEC'
SRC = 'SRC'
#ENCODER = 'OPUSENC'

ZMODEM_DEFCONFIG = 'defconfig/zmodem-defconfig'
global PLAYLIST_FILE #= 'input_24k_stereo/TRACK_DB.CSV'
global AUDIO_FILE #= 'input_24k_stereo/test.mp3'

BIN = 'BIN'
PLAYLIST = 'PLAYLIST'
AUDIO = 'AUDIO'
REC = 'REC'

SD_CARD_MOUNT_VERIFY_TIME_OUT = 10

PLAYER_FOLDERS = ['BIN', 'AUDIO', 'PLAYLIST']
RECORDER_FOLDERS = ['BIN', 'REC']

AUDIO_EXAMPLE_PATH = '/mnt/sd0/'

TEST_AUDIO_FILE_FREQUENCY = 1000
FREQUENCY_VERIFY_ACCURACY = 2
DSP_CODERS = ['WAVDEC', 'MP3DEC', 'MP3ENC', 'SRC']

# noinspection PyUnusedLocal
def player_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.log.debug(' '.join(['Unexpected', str(source), 'reboot!']))


# noinspection PyUnusedLocal
def recorder_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.log.debug(' '.join(['Unexpected', str(source), 'reboot!']))


# noinspection PyUnusedLocal
def player_error_monitor(test, source, data):
    if source.APP_ERROR in data:
        test.set_fail('Player error: {}'.format(data.get_value(source.KEY_ERROR)))


# noinspection PyUnusedLocal
def recorder_error_monitor(test, source, data):
    if source.APP_ERROR in data:
        test.set_fail('Recorder error: {}'.format(data.get_value(source.KEY_ERROR)))


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

# noinspection PyUnusedLocal
def execute_command(test, source, data, command, confirm):
    source.write(command)
    if confirm:
        test.add_user_event(source, 'Command {} executed'.format(command))
    return True

def execute_record(test, source, data, codec, sampling, channel, confirm):
    test.storage.rec_codec = codec
    test.storage.rec_sampling = sampling
    test.storage.rec_channel = channel
    command = '{} {} {} {} 16BIT'.format('recorder_test', codec, sampling, channel)
    return execute_command(test, source, data, command, confirm)

def store_rec_file_path(test, source, data):
    test.storage.rec_file_path = data.get_value(source.KEY_FILE_PATH)
    return True

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
    ret = mountCurrentUSBDisk(25, '/tmp/sd0', source, True)
    if len(ret) == 0:
        test.set_fail('usbmsc mount failed')

def mountEnd(test, source, data):
    umountCurrentUSBDisk(5, '/tmp/sd0', source, True)

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
def mv_file(test, source, data, file, dest_path, newfile=None):
    file_path, file_name = os.path.split(test.storage.rec_file_path)
    file_path_name, ext = os.path.splitext(test.storage.rec_file_path)
    if newfile:
        newfile = '{}_{}_{}{}'.format(test.storage.rec_codec,
                                      test.storage.rec_sampling,
                                      test.storage.rec_channel,
                                      ext)
    else:
        newfile = file_name
    runBashFunction('mkdir -p {}'.format(dest_path))
    runBashFunction('rm -fr {}/{}'.format(dest_path, newfile))
    runBashFunction('sync')
    file_path = '/tmp/sd0/' + file + '/' + file_name
    test.log.debug('mv {}->{}/{}'.format(file_path, dest_path, newfile))
    runBashFunction('mv {} {}/{}'.format(file_path, dest_path, newfile))
    runBashFunction('sync')
    time.sleep(1)
    test.add_user_event(source, 'File {} moved'.format(file))
    return True

# noinspection PyUnusedLocal
def verify_recorded_file(test, source, data):
    test_file_path = os.path.join(os.getcwd(), 'output', test.storage.rec_file_path.split('/')[-1])
    test.log.debug('verify file : ' + test_file_path)
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


class AudioMp3TestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AudioMp3TestTcRunner, self).__init__(conf)

        self.player = PlayerDevice(kwargs.get('player', None), 'PLAYER_DEVICE') \
            if kwargs.get('player', None) else None
        self.recorder = RecorderDevice(kwargs.get('recorder', None), 'RECORDER_DEVICE') \
            if kwargs.get('recorder', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least two devices are needed!')

        self.updater = os.path.join(self.config.projects[0].path, UPDATER_PATH, UPDATER)
        self.audio_decoder = os.path.join(self.config.projects[0].path, DSP_PATH, DECODER)
#        self.audio_encoder = os.path.join(self.config.projects[0].path, DSP_PATH, ENCODER)
        self.src = os.path.join(self.config.projects[0].path, DSP_PATH, SRC)
        self.playlist = os.path.join(os.getcwd(), PLAYLIST_FILE)
        self.audio_file = os.path.join(os.getcwd(), AUDIO_FILE)
        self.zmodem_defconfig = os.path.join(os.getcwd(), ZMODEM_DEFCONFIG)

        self.defconfig_path = os.path.join(self.config.projects[0].path,
                                           'sdk/configs/examples')
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

        binaries = self.build(debug, log, **binaries)

#        self.flash_n_check(self.player, binaries['player_bin'], log, PLAYER_APPS_TO_BUILTIN)
#        self.flash_n_check(self.recorder, binaries['recorder_bin'], log, RECORDER_APPS_TO_BUILTIN)
        self.flash_n_check(self.player, binaries['player_bin'], log, PLAYER_APPS)
        self.flash_n_check(self.recorder, binaries['recorder_bin'], log, RECORDER_APPS)

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
            print ('not player_bin')
            log.info('Building {}'.format(self.player))
            player_bin = self.__build_binary(toolbox, PLAYER_APPS_TO_BUILTIN, self.player,
                                             self.zmodem_defconfig)

        if not recorder_bin:
            print ('not recorder_bin')
            log.info('Building {}'.format(self.recorder))
            recorder_bin = self.__build_binary(toolbox, RECORDER_APPS_TO_BUILTIN, self.recorder,
                                               self.zmodem_defconfig)

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
        timeout = 60
        play_record_timeout = 300

        player_device, recorder_device = self.get_devices_list()

        def setup_steps(device, reboot_monitor, error_monitor):
            return [
                Step(Action.WRITE, device, 'reboot'),
                Step(Action.WAIT_FOR, device, device.NUTTSHELL),
                Step(Action.EXECUTE, device, ('{}_reboot_mon'.format(str(device)), reboot_monitor),
                     Test.add_monitor),
                Step(Action.EXECUTE, device, ('{}_error_mon'.format(str(device)), error_monitor),
                     Test.add_monitor),
            ]

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
                Step(Action.EXECUTE, device, '{}_error_mon'.format(str(device)),
                     Test.remove_monitor),
            ]

        def usb_s(device):
            r = [
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
            r += [
                Step(Action.EXECUTE, player_device, None,
                     lambda test, source, data : cp_file(test, source, data, os.path.join(os.getcwd(), AUDIO_FILE ), 'AUDIO')),
                Step(Action.WAIT_FOR, player_device, 'copied'),
                Step(Action.EXECUTE, player_device, None,
                     lambda test, source, data: cp_file(test, source, data, os.path.join(os.getcwd(), PLAYLIST_FILE), 'PLAYLIST')),
                Step(Action.WAIT_FOR, player_device, 'copied'),
            ]
            return r

        def verify_rec_file():
            r = [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, 'REC', os.getcwd() + '/output')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: verify_recorded_file(test, source, data)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ]
            return r

        SDK_Audio_14915 = Test(
            name='SDK_Audio_14915',
            timeout=play_record_timeout,
            setup=setup_steps(player_device, player_reboot_monitor, player_error_monitor) + \
                  setup_steps(recorder_device, recorder_reboot_monitor, recorder_error_monitor) + \
                  verify_sd_card_mount(player_device) + verify_sd_card_mount(recorder_device),
            test=usb_s(recorder_device) + \
                 dsp_steps(recorder_device) + \
                 usb_e(recorder_device) + \
                 usb_s(player_device) + \
                 dsp_steps(player_device) + \
                 add_file_to_player() + \
                 usb_e(player_device) + [
                Step(Action.WRITE, player_device, 'audio_player'),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_command(test, recorder_device, data,
                                                                'audio_recorder', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_START),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),
                 ] + usb_s(recorder_device) + verify_rec_file() + usb_e(recorder_device),
            teardown=teardown(player_device) + teardown(recorder_device),
        )

        audio = TestGroup(
            name='audio',
            devices=[player_device, recorder_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Audio_14915,
            ]
        )

        test_groups = [
            audio,
        ]

        return test_groups


if __name__ == "__main__":
    global PLAYLIST_FILE
    global AUDIO_FILE

    parser = RunnerParser()

    parser.add_argument('--player_bin', help='Player binary file path')
    parser.add_argument('--recorder_bin', help='Recorder binary file path')
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
    parser.add_argument('--input_dirname', help='Play format ex:input_24k_stereo')

    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    if args.input_dirname:
        PLAYLIST_FILE = args.input_dirname + '/TRACK_DB.CSV'
        AUDIO_FILE = args.input_dirname + '/test.mp3'
    else:
        PLAYLIST_FILE = 'input_24k_stereo/TRACK_DB.CSV'
        AUDIO_FILE = 'input_24k_stereo/test.mp3'
    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    player, recorder = dev_manager.get_devices_by_serials(args.player, args.recorder)

    # Create test runner instance
    tc_runner = AudioMp3TestTcRunner(config, player=player, recorder=recorder)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
