#! /usr/bin/env python3
import sys
import os
import subprocess
import time
import numpy
import wave


SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.audio_player_wav_mp3.player_device import PlayerDevice
from tc.audio_player_wav_mp3.recorder_device import RecorderDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger, Timer
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
PLAYER_APPS = ['player_test', 'msconn', 'msdis']
RECORDER_APPS = ['audio_recorder', 'msconn', 'msdis']
PLAYER_APPS_TO_BUILTIN = ['player_wav_mp3']
RECORDER_APPS_TO_BUILTIN = ['examples/audio_recorder', 'usbmsc']

UPDATER = 'updater.py'
UPDATER_PATH = 'autotest/src/api'
DSP_PATH = 'sdk/modules/audio/dsp'
DECODERS = ['MP3DEC', 'WAVDEC']
SRC = 'SRC'

PLAYLIST_FILE = 'input/TRACK_DB.CSV'
AUDIO_FILES = ['input/test.mp3', 'input/test2.wav']

BIN = 'BIN'
PLAYLIST = 'PLAYLIST'
AUDIO = 'AUDIO'
REC = 'REC'

SD_CARD_MOUNT_VERIFY_TIME_OUT = 10

PLAYER_FOLDERS = ['BIN', 'AUDIO', 'PLAYLIST']
RECORDER_FOLDERS = ['BIN', 'REC']

AUDIO_EXAMPLE_PATH = '/mnt/sd0/'

TEST_AUDIO_FILE_FREQUENCY = 1000
TEST_AUDIO_FILE_FREQUENCY2 = 500
FREQUENCY_VERIFY_ACCURACY = 2


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
    file_path = gmedia_path['usb'] + '/REC/' + file
    test.log.info('mv {}->{}/{}'.format(file_path, dest_path, newfile))
    usbcommand(test, source, data, 'mv {} {}/{}'.format(file_path, dest_path, newfile))
    usbcommand(test, source, data, 'sync')
    time.sleep(1)
    test.add_user_event(source, 'File {} moved'.format(file))
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


def store_rec_file_path(test, source, data):
    test.storage.rec_file_path = data.get_value(source.KEY_FILE_PATH).split('/')[-1]
    return True


# noinspection PyUnusedLocal
def remove_recorded_file(test, source, data):
    if hasattr(test.storage, 'rec_file_path'):
        source.write('rm {}'.format(test.storage.rec_file_path))


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

    test_file_path = os.path.join(os.getcwd(), 'output', '15451.wav')

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


class AudioPlayerWavMp3TcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AudioPlayerWavMp3TcRunner, self).__init__(conf)

        self.player = PlayerDevice(kwargs.get('player', None), 'PLAYER_DEVICE') \
            if kwargs.get('player', None) else None
        self.recorder = RecorderDevice(kwargs.get('recorder', None), 'RECORDER_DEVICE') \
            if kwargs.get('recorder', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least two devices are needed!')

        self.updater = os.path.join(self.config.projects[0].path, UPDATER_PATH, UPDATER)
        self.src = os.path.join(self.config.projects[0].path, DSP_PATH, SRC)
        self.playlist = os.path.join(os.getcwd(), PLAYLIST_FILE)
        self.audio_file = os.path.join(os.getcwd(), AUDIO_FILES[0])
        self.audio_file2 = os.path.join(os.getcwd(), AUDIO_FILES[1])

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
                                             'defconfig/player_wav_mp3-defconfig')

        if not recorder_bin:
            print('not recorder_bin')
            log.info('Building {}'.format(self.recorder))
            recorder_bin = self.__build_binary(toolbox, RECORDER_APPS_TO_BUILTIN, self.recorder,
                                               'defconfig/usbmsc-defconfig')

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
        play_record_timeout = 10 * 60

        player_device, recorder_device = self.get_devices_list()

        def usb_s(device):
            r = [
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

        def setup_steps(device, reboot_monitor, error_monitor):
            return [
                Step(Action.WRITE, device, 'reboot'),
                Step(Action.WAIT_FOR, device, device.NUTTSHELL),
                Step(Action.EXECUTE, device, ('{}_reboot_mon'.format(str(device)), reboot_monitor),
                     Test.add_monitor),
                Step(Action.EXECUTE, device, ('{}_error_mon'.format(str(device)), error_monitor),
                     Test.add_monitor),
            ]

        def player_setup(device, reboot_monitor, error_monitor):
            if arguments.player_fs_ready:
                return setup_steps(device, reboot_monitor, error_monitor) +\
                       verify_sd_card_mount(device)
            else:
                return setup_steps(device, reboot_monitor, error_monitor) +\
                       verify_sd_card_mount(device) + prepare_player_fs_steps(device)

        def recorder_setup(device, reboot_monitor, error_monitor):
            if arguments.recorder_fs_ready:
                return setup_steps(device, reboot_monitor, error_monitor) +\
                       verify_sd_card_mount(device)
            else:
                return setup_steps(device, reboot_monitor, error_monitor) +\
                       verify_sd_card_mount(device) + prepare_recorder_fs_steps(device)

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

        def prepare_player_fs_steps(device):
            return [
                Step(Action.WAIT_FOR, device, device.NSH_PROMPT),
            ] + usb_s(device) + [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: cp_file(test, source, data, self.audio_file, 'AUDIO')),
                Step(Action.WAIT_FOR, device, 'copied',
                     lambda test, source, data: cp_file(test, source, data, self.audio_file2, 'AUDIO')),
                Step(Action.WAIT_FOR, device, 'copied',
                     lambda test, source, data: cp_file(test, source, data, self.playlist, 'PLAYLIST')),
                Step(Action.WAIT_FOR, device, 'copied',
                     lambda test, source, data: cp_file(test, source, data,
                                                        os.path.join(self.config.projects[0].path, DSP_PATH, DECODERS[0]), 'BIN')),
                Step(Action.WAIT_FOR, device, 'copied',
                     lambda test, source, data: cp_file(test, source, data,
                                                        os.path.join(self.config.projects[0].path, DSP_PATH, DECODERS[1]), 'BIN')),
                Step(Action.WAIT_FOR, device, 'copied'),
            ] + usb_e(device)

        def clean_player_fs(device):
            return [
                Step(Action.EXECUTE, device, 'Start cleaning', trigger_cleaning),
                Step(Action.WAIT_FOR, device, 'Start cleaning',
                     lambda test, source, data: execute_command(test, source, data, 'cd /', True)),
                Step(Action.WAIT_FOR, device, 'Command cd / executed',
                     lambda test, source, data:
                     execute_command(test, source, data, 'cd {}{}'.format(AUDIO_EXAMPLE_PATH, BIN),
                                     True)),
                Step(Action.WAIT_FOR, device, 'cd {}{}'.format(AUDIO_EXAMPLE_PATH, BIN),
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS,
                     lambda test, source, data: clean_dir(test, source, data)),
                Step(Action.WAIT_FOR, device, 'Files removed from {}'.format(BIN),
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS,
                     lambda test, source, data: rm_dir(test, source, data)),
                Step(Action.WAIT_FOR, device, '{} folder removed'.format(BIN),
                     lambda test, source, data: execute_command(test, source, data,
                                                                'cd {}'.format(AUDIO), True)),
                Step(Action.WAIT_FOR, device, 'Command cd {} executed'.format(AUDIO),
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS,
                     lambda test, source, data: clean_dir(test, source, data)),
                Step(Action.WAIT_FOR, device, 'Files removed from {}'.format(AUDIO),
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS,
                     lambda test, source, data: rm_dir(test, source, data)),
                Step(Action.WAIT_FOR, device, '{} folder removed'.format(AUDIO),
                     lambda test, source, data: execute_command(test, source, data,
                                                                'cd {}'.format(PLAYLIST), True)),
                Step(Action.WAIT_FOR, device, 'Command cd {} executed'.format(PLAYLIST),
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS,
                     lambda test, source, data: clean_dir(test, source, data)),
                Step(Action.WAIT_FOR, device, 'Files removed from {}'.format(PLAYLIST),
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS,
                     lambda test, source, data: rm_dir(test, source, data)),
                Step(Action.WAIT_FOR, device, '{} folder removed'.format(PLAYLIST)),
            ]

        def prepare_recorder_fs_steps(device):
            return [
                Step(Action.WAIT_FOR, device, device.NSH_PROMPT),
                ] + usb_s(device) + [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: cp_file(test, source, data, self.src, 'BIN')),
                Step(Action.WAIT_FOR, device, 'copied'),
            ] + usb_e(device)

        def remove_recorded_files_steps(device):
            return [
                Step(Action.EXECUTE, device, None, remove_recorded_file),
            ]

        def clean_recorder_fs(device):
            return [
                Step(Action.EXECUTE, device, 'Start cleaning', trigger_cleaning),
                Step(Action.WAIT_FOR, device, 'Start cleaning',
                     lambda test, source, data: execute_command(test, source, data, 'cd /', True)),
                Step(Action.WAIT_FOR, device, 'Command cd / executed',
                     lambda test, source, data:
                     execute_command(test, source, data, 'cd {}{}'.format(AUDIO_EXAMPLE_PATH, BIN),
                                     True)),
                Step(Action.WAIT_FOR, device, 'cd {}{}'.format(AUDIO_EXAMPLE_PATH, BIN),
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS,
                     lambda test, source, data: clean_dir(test, source, data)),
                Step(Action.WAIT_FOR, device, 'Files removed from {}'.format(BIN),
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS,
                     lambda test, source, data: rm_dir(test, source, data)),
                Step(Action.WAIT_FOR, device, '{} folder removed'.format(BIN),
                     lambda test, source, data: execute_command(test, source, data,
                                                                'cd {}'.format(REC), True)),
                Step(Action.WAIT_FOR, device, 'Command cd {} executed'.format(REC),
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS,
                     lambda test, source, data: clean_dir(test, source, data)),
                Step(Action.WAIT_FOR, device, 'Files removed from {}'.format(REC),
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS,
                     lambda test, source, data: rm_dir(test, source, data)),
                Step(Action.WAIT_FOR, device, '{} folder removed'.format(REC)),
            ]

        def teardown(device):
            return [
                Step(Action.EXECUTE, device, '{}_reboot_mon'.format(str(device)),
                     Test.remove_monitor),
                Step(Action.EXECUTE, device, '{}_error_mon'.format(str(device)),
                     Test.remove_monitor),
            ]

        def player_teardown(device):
            if arguments.preserve_player_fs:
                return teardown(device)
            else:
                return clean_player_fs(device) + teardown(device)

        def recorder_teardown(device):
            if arguments.preserve_recorder_fs:
                return teardown(device) + remove_recorded_files_steps(device)
            else:
                return clean_recorder_fs(device) + teardown(device)

        tc_player_wav_mp3_15451 = Test(
            name='tc_player_wav_mp3_15451',
            timeout=play_record_timeout,
            setup=player_setup(player_device, player_reboot_monitor, player_error_monitor) +
            recorder_setup(recorder_device, recorder_reboot_monitor, recorder_error_monitor),
            test=[
                Step(Action.WRITE, player_device, 'player_test', terminator='\n'),
                Step(Action.WRITE, player_device, '8', terminator='\n'),
                Step(Action.WRITE, recorder_device, 'audio_recorder'),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WRITE, player_device, 'quit', terminator='\n'),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),
            ]
            + teardown(recorder_device)
            + setup_steps(recorder_device, recorder_reboot_monitor, recorder_error_monitor)
            + verify_sd_card_mount(recorder_device)
            + usb_s(recorder_device) +
            [
                     Step(Action.EXECUTE, recorder_device, None,
                          lambda test, source, data: mv_file(test, source, data, test.storage.rec_file_path, os.getcwd() + '/output', '15451.wav')),
                     Step(Action.WAIT_FOR, recorder_device, 'moved'),
                     Step(Action.WRITE, recorder_device, 'msdis'),
                     Step(Action.WAIT_FOR, recorder_device, 'Disconnected',
                          lambda test, source, data: verify_recorded_file(test, source, data)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification finished'),
            ],
            teardown=player_teardown(player_device) + recorder_teardown(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None, remove_transferred_file),
            ],
        )

        audio = TestGroup(
            name='audio',
            devices=[player_device, recorder_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_player_wav_mp3_15451,
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
    tc_runner = AudioPlayerWavMp3TcRunner(config, player=player, recorder=recorder)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
