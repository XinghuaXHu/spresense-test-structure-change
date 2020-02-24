#! /usr/bin/env python3
import sys
import os
import time
import numpy
import wave
import subprocess

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.audio_dual_players.player_device import PlayerDevice
from tc.audio_dual_players.recorder_device import RecorderDevice
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser
from api.utils import Toolbox
from utilities.usbmsc_mount import mountCurrentUSBDisk,umountCurrentUSBDisk,runBashFunction
from api.test import Test, Step, Action, TestGroup, TestLogger, Timer

PROJECT = 'spresense'
PLAYER_APPS = ['audio_dual_players', 'zmodem']
RECORDER_APPS = ['audio_recorder_objif', 'zmodem']
PLAYER_APPS_TO_BUILTIN = ['examples/audio_dual_players', 'zmodem']
RECORDER_APPS_TO_BUILTIN = ['examples/audio_recorder_objif', 'zmodem']

UPDATER = 'updater.py'
UPDATER_PATH = 'autotest/src/api'
DSP_PATH = 'sdk/modules/audio/dsp'
DECODER = 'MP3DEC'
SRC = 'SRC'

ZMODEM_DEFCONFIG = 'defconfig/zmodem-defconfig'

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

AUDIO_FILES = ['Sound0.mp3', 'Sound1.mp3']

global gmedia_dict
gmedia_dict = {}
global gmedia_dict2
gmedia_dict2 = {}
global gmedia_path
gmedia_path = {}

TEST_AUDIO_FILE_FREQUENCY = 1000
TEST_AUDIO_FILE_FREQUENCY2 = 500
FREQUENCY_VERIFY_ACCURACY = 2

# noinspection PyUnusedLocal
def player_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.log.info(' '.join(['Unexpected', str(source), 'reboot!']))


# noinspection PyUnusedLocal
def recorder_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.log.info(' '.join(['Unexpected', str(source), 'reboot!']))


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

def reset_data(test, source, event):
    gmedia_path.clear()
    gmedia_dict2.clear()
    gmedia_dict.clear()


def usbcommand(test, source, data, command):
    p3 = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
    (output3, err3) = p3.communicate()
    p3.wait()
    if None != err3 or output3 != b'':
        test.set_fail('cmd NG')


# noinspection PyUnusedLocal
def cp_file(test, source, data, file, dest_dir):
    dest_path = '/tmp/sd0/' + dest_dir
    test.log.info('cp {}->{}'.format(file, dest_path))
    usbcommand(test, source, data, 'mkdir -p {}'.format(dest_path))
    usbcommand(test, source, data, 'cp -r {} {}'.format(file, dest_path))
    usbcommand(test, source, data, 'sync')
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
def findpeaks(data, spacing=1, limit=None):
    """Finds peaks in `data` which are of `spacing` width and >=`limit`.
    :param data: values
    :param spacing: minimum spacing to the next peak (should be 1 or more)
    :param limit: peaks should have value greater or equal
    :return:
    """
    len = data.size
    x = numpy.zeros(len + 2 * spacing)
    x[:spacing] = data[0] - 1.e-6
    x[-spacing:] = data[-1] - 1.e-6
    x[spacing:spacing + len] = data
    peak_candidate = numpy.zeros(len)
    peak_candidate[:] = True
    for s in range(spacing):
        start = spacing - s - 1
        h_b = x[start: start + len]  # before
        start = spacing
        h_c = x[start: start + len]  # central
        start = spacing + s + 1
        h_a = x[start: start + len]  # after
        peak_candidate = numpy.logical_and(peak_candidate, numpy.logical_and(h_c > h_b, h_c > h_a))

    ind = numpy.argwhere(peak_candidate)
    ind = ind.reshape(ind.size)
    if limit is not None:
        ind = ind[data[ind] > limit]

    return ind


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

    indexes = findpeaks(ft, spacing=3, limit=max(ft)/8)
    frequency = frequencies[indexes]

    freq_range_1000 = [i for i in range(TEST_AUDIO_FILE_FREQUENCY - FREQUENCY_VERIFY_ACCURACY,
                                   TEST_AUDIO_FILE_FREQUENCY + FREQUENCY_VERIFY_ACCURACY)]
    freq_range_500 = [i for i in range(TEST_AUDIO_FILE_FREQUENCY2 - FREQUENCY_VERIFY_ACCURACY,
                                        TEST_AUDIO_FILE_FREQUENCY2 + FREQUENCY_VERIFY_ACCURACY)]

    for f in frequency:
        test.log.debug('Found frequency {} Hz'.format(f))
        if int(f) in freq_range_1000:
            test.log.debug('Found right frequency {} Hz'.format(f))
            freq_1000_pass = True
        elif int(f) in freq_range_500:
            test.log.debug('Found right frequency {} Hz'.format(f))
            freq_500_pass = True    
               
    test.assert_true(freq_1000_pass and freq_500_pass, 'Recorded file verification')
   
    test.add_user_event(source, 'Recorded file verification finished')

    return True


class AudioDualPlayerTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AudioDualPlayerTcRunner, self).__init__(conf)

        self.player = PlayerDevice(kwargs.get('player', None), 'PLAYER_DEVICE') \
            if kwargs.get('player', None) else None
        self.recorder = RecorderDevice(kwargs.get('recorder', None), 'RECORDER_DEVICE') \
            if kwargs.get('recorder', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least two devices are needed!')

        self.updater = os.path.join(self.config.projects[0].path, UPDATER_PATH, UPDATER)
        self.audio_decoder = os.path.join(self.config.projects[0].path, DSP_PATH, DECODER)
        self.src = os.path.join(self.config.projects[0].path, DSP_PATH, SRC)
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
            print(not all)
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
        timeout = 480

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
            r = []
            for file in AUDIO_FILES:
                r += [
                    Step(Action.EXECUTE, player_device, None,
                         lambda test, source, data, filename=file: cp_file(test, source, data, os.path.join(os.getcwd(), 'input/' + filename), 'AUDIO')),
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
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification finished'),
            ]
            return r

        teardown = [
        ]

        tc_dual_players_17160_17163 = Test(
            name='tc_dual_players_17160_17163',
            timeout=timeout,
            setup=setup_steps(player_device, player_reboot_monitor, player_error_monitor) + \
                  setup_steps(recorder_device, recorder_reboot_monitor, recorder_error_monitor) + \
                  verify_sd_card_mount(player_device) + verify_sd_card_mount(recorder_device),
            test=usb_s(recorder_device) +
                 dsp_steps(recorder_device) +
                 usb_e(recorder_device) +
                 usb_s(player_device) +
                 dsp_steps(player_device) +
                 add_file_to_player() +
                 usb_e(player_device) + [
                Step(Action.WRITE, player_device, 'audio_dual_players'),
                Step(Action.WRITE, recorder_device, 'audio_recorder_objif'),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),
            ] + usb_s(recorder_device) + verify_rec_file() + usb_e(recorder_device),
            teardown=teardown
        )

        test_group = TestGroup(
            name='test_group',
            devices=[player_device, recorder_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_dual_players_17160_17163,
            ]
        )

        test_groups = [
            test_group,
        ]

        return test_groups


if __name__ == "__main__":
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

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    player, recorder = dev_manager.get_devices_by_serials(args.player, args.recorder)

    # Create test runner instance
    tc_runner = AudioDualPlayerTcRunner(config, player=player, recorder=recorder)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)