#! /usr/bin/env python3
import sys
import os
import subprocess
import time
import numpy
import wave
import io


SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.audio_recorder_preproc.player_device import PlayerDevice
from tc.audio_recorder_preproc.recorder_device import RecorderDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger, Timer
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
PLAYER_APPS = ['player']  # , 'msconn', 'msdis']
RECORDER_APPS = ['recorder']  # , 'msconn', 'msdis']
PLAYER_APPS_TO_BUILTIN = ['usbmsc']
RECORDER_APPS_TO_BUILTIN = ['recorder_preproc']

UPDATER = 'updater.py'
UPDATER_PATH = 'autotest/src/api'
DSP_PATH = 'sdk/modules/audio/dsp'
PREPROC_PATH = 'test/sqa/singlefunction/audio_recorder_preproc/worker/src'
DECODERS = ['MP3DEC', 'WAVDEC']
SRC = 'SRC'
ENCODERS = ['MP3ENC', 'PREPROC']

USBMSC_DEFCONFIG = 'defconfig/usbmsc-defconfig'
RECORDER_TEST_DEFCONFIG = 'defconfig/recorder_preproc-defconfig'

PLAYLIST_FILE = 'input/TRACK_DB.CSV'
AUDIO_FILE = 'input/test.mp3'

BIN = 'BIN'
PLAYLIST = 'PLAYLIST'
AUDIO = 'AUDIO'
REC = 'REC'

REC_FORMAT = [
    'LPCM 16K MONO 16BIT',
    'LPCM 16K STEREO 16BIT',
    'LPCM 16K 4CH 16BIT',
    'LPCM 48K MONO 16BIT',
    'LPCM 48K STEREO 16BIT',
    'LPCM 48K 4CH 16BIT',
    'LPCM 48K MONO 24BIT',
    'LPCM 48K STEREO 24BIT',
    'LPCM 192K MONO 16BIT',
    'LPCM 192K STEREO 16BIT',
    'LPCM 192K MONO 24BIT',
    'LPCM 192K STEREO 24BIT',
    'MP3 16K MONO 16BIT',
    'MP3 16K STEREO 16BIT',
    'MP3 48K MONO 16BIT',
    'MP3 48K STEREO 16BIT',
]

SD_CARD_MOUNT_VERIFY_TIME_OUT = 10

PLAYER_FOLDERS = ['BIN', 'AUDIO', 'PLAYLIST']
RECORDER_FOLDERS = ['BIN', 'REC']

AUDIO_EXAMPLE_PATH = '/mnt/sd0/'

TEST_AUDIO_FILE_FREQUENCY = 1000
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
        if key2_ount == 0 and len(key2) >= 9 and key2[-1] == '1':
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


def end_test(test, source, data):
    test.set_fail('Failed to transfer file')


def gen_filename(test, case_num, index):
    return str(case_num) + '_' + str(index) + '.' + ('wav' if index < 12 else 'mp3')


def get_channel(index):
    if REC_FORMAT[index].split()[-2] == 'MONO':
        return 1
    elif REC_FORMAT[index].split()[-2] == 'STEREO':
        return 2
    else:
        return 4


def get_dtype(index):
    return int(REC_FORMAT[index].split()[-1][:2])


# noinspection PyUnusedLocal
def execute_command(test, source, data, command, confirm):
    source.write(command)
    if confirm:
        test.add_user_event(source, 'Command {} executed'.format(command))
    return True


def execute_record(test, source, data, rec_format, confirm):
    command = '{} {}'.format('recorder', rec_format)
    return execute_command(test, source, data, command, confirm)


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
def verify_recorded_file(test, source, data, filename, is_in_range=True, ch=1, dtype=16):
    test_file_path = os.path.join(os.getcwd(), 'output', filename)

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

    test.log.debug('max ft {}'.format(numpy.max(ft)))

    frequency = frequencies[numpy.argmax(ft)]

    test.log.debug('Found frequency {} Hz'.format(frequency))

    if is_in_range:
        test.assert_true(int(frequency) in range(TEST_AUDIO_FILE_FREQUENCY - FREQUENCY_VERIFY_ACCURACY,
                                                 TEST_AUDIO_FILE_FREQUENCY + FREQUENCY_VERIFY_ACCURACY),
                         'Recorded file verification')
    else:
        test.assert_true(int(frequency) not in range(TEST_AUDIO_FILE_FREQUENCY - FREQUENCY_VERIFY_ACCURACY,
                                                     TEST_AUDIO_FILE_FREQUENCY + FREQUENCY_VERIFY_ACCURACY)
                         or numpy.max(ft) < 1,
                         'Recorded file verification')
    test.add_user_event(source, 'Recorded file verification finished')
    return True


class AudioRecordTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AudioRecordTestTcRunner, self).__init__(conf)

        self.player = PlayerDevice(kwargs.get('player', None), 'PLAYER_DEVICE') \
            if kwargs.get('player', None) else None
        self.recorder = RecorderDevice(kwargs.get('recorder', None), 'RECORDER_DEVICE') \
            if kwargs.get('recorder', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least two devices are needed!')

        self.player.name = 'player'
        self.recorder.name = 'recorder'

        self.updater = os.path.join(self.config.projects[0].path, UPDATER_PATH, UPDATER)
        self.audio_decoder = os.path.join(self.config.projects[0].path, DSP_PATH)
        self.audio_encoder = os.path.join(self.config.projects[0].path, DSP_PATH)
        self.src = os.path.join(self.config.projects[0].path, DSP_PATH, SRC)
        self.preproc_path = os.path.join(self.config.projects[0].path, PREPROC_PATH)
        self.playlist = os.path.join(os.getcwd(), PLAYLIST_FILE)
        self.audio_file = os.path.join(os.getcwd(), AUDIO_FILE)
        self.usbmsc_defconfig = os.path.join(os.path.dirname(__file__), USBMSC_DEFCONFIG)
        self.recorder_test_defconfig = os.path.join(os.path.dirname(__file__), RECORDER_TEST_DEFCONFIG)

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
                                             self.usbmsc_defconfig)

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
        timeout = 240
        play_record_timeout = 600

        player_device, recorder_device = self.get_devices_list()

        def usb_s(device, isbuiltin=False):
            r = umount(device) + [
                Step(Action.EXECUTE, device, None, lambda test, source, data: reset_data(test, source, data)),
                Step(Action.EXECUTE, device, None, lambda test, source, data: wait_sec(test, source, data, 5, 'wait sd')),
                Step(Action.EXECUTE, device, None, lambda test, source, data: checkDefaultDisk(test, source, data)),

                Step(Action.WRITE, device, 'msconn' if isbuiltin else (device.name + ' MSCONN'), terminator='\n'),
                Step(Action.WAIT_FOR, device, 'Connected',
                     lambda test, source, data: wait_sec(test, source, data, 25, 'usb mount to PC')),

                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: checkNewDisk(test, source, data)),
            ]
            return r

        def usb_e(device, isbuiltin=False):
            r = [
                Step(Action.WRITE, device, 'msdis' if isbuiltin else (device.name + ' MSDIS')),
                Step(Action.WAIT_FOR, device, 'Disconnected'),
            ] + mount(device)
            return r

        def mount(device):
            r = [
                Step(Action.WRITE, device, 'mount -t vfat /dev/mmcsd0 /mnt/sd0'),
                Step(Action.EXECUTE, device, '',
                     lambda test, source, data: wait_sec(test, source, data, 5, 'mount')),
            ]
            return r

        def umount(device):
            r = [
                Step(Action.WRITE, device, 'umount /mnt/sd0'),
                Step(Action.EXECUTE, device, '',
                     lambda test, source, data: wait_sec(test, source, data, 1, 'umount')),
            ]
            return r

        def setup_steps(device, reboot_monitor, error_monitor):
            return [
                Step(Action.WRITE, device, 'reboot'),
                Step(Action.WAIT_FOR, device, device.NUTTSHELL),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: wait_sec(test, source, data, 3, 'wait nuttshell')),
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
                     lambda test, source, data: cp_file(test, source, data, self.playlist, 'PLAYLIST')),
                Step(Action.WAIT_FOR, device, 'copied',
                     lambda test, source, data: cp_file(test, source, data,
                                                        os.path.join(self.audio_decoder, DECODERS[0]), 'BIN')),
                Step(Action.WAIT_FOR, device, 'copied',
                     lambda test, source, data: cp_file(test, source, data,
                                                        os.path.join(self.audio_decoder, DECODERS[1]), 'BIN')),
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
                Step(Action.WAIT_FOR, device, 'copied',
                     lambda test, source, data: cp_file(test, source, data,
                                                        os.path.join(self.audio_encoder, ENCODERS[0]), 'BIN')),
                Step(Action.WAIT_FOR, device, 'copied',
                     lambda test, source, data: cp_file(test, source, data,
                                                        os.path.join(self.preproc_path, ENCODERS[1]), 'BIN')),
                Step(Action.WAIT_FOR, device, 'copied'),
            ] + usb_e(device)

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

        tc_recorder_preproc_16728_0 = Test(
            name='tc_recorder_preproc_16728_0',
            timeout=play_record_timeout,
            setup=player_setup(player_device, player_reboot_monitor, player_error_monitor) +
                 recorder_setup(recorder_device, recorder_reboot_monitor, recorder_error_monitor),
            test=[
                Step(Action.WRITE, player_device, 'player'),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               REC_FORMAT[0] + ' EN', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),
            ]
            + teardown(recorder_device)
            + setup_steps(recorder_device, recorder_reboot_monitor, recorder_error_monitor)
            + verify_sd_card_mount(recorder_device)
            + usb_s(recorder_device, False) +
            [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, test.storage.rec_file_path, os.getcwd() + '/output', '16728_0.wav')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
            ] + usb_e(recorder_device, False) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: verify_recorded_file(test, source, data, '16728_0.wav', False)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification finished'),
            ],
            teardown=teardown(player_device) + teardown(recorder_device)
        )

        tests = [tc_recorder_preproc_16728_0]
        for i in range(1, len(REC_FORMAT)):
            tests.append(Test(
                name='tc_recorder_preproc_16728_' + str(i),
                timeout=timeout,
                setup=setup_steps(player_device, player_reboot_monitor, player_error_monitor)
                      + setup_steps(recorder_device, recorder_reboot_monitor, recorder_error_monitor)
                      + verify_sd_card_mount(player_device)
                      + verify_sd_card_mount(recorder_device),
                test=[
                    Step(Action.WRITE, player_device, 'player'),
                    Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                         lambda test, source, data, index=i: execute_record(test, recorder_device, data,
                                                                            REC_FORMAT[index] + ' EN', False)),
                    Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                         lambda test, source, data: store_rec_file_path(test, source, data)),
                    Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                    Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                    Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                    Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                    Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                    Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),
                ]
                + teardown(recorder_device)
                + setup_steps(recorder_device, recorder_reboot_monitor, recorder_error_monitor)
                + verify_sd_card_mount(recorder_device)
                + usb_s(recorder_device, False) +
                [
                    Step(Action.EXECUTE, recorder_device, None,
                         lambda test, source, data, index=i: mv_file(test, source, data, test.storage.rec_file_path,
                                                                     os.getcwd() + '/output',
                                                                     gen_filename(test, 16728, index))),
                    Step(Action.WAIT_FOR, recorder_device, 'moved'),
                ] + usb_e(recorder_device, False) + ([
                    Step(Action.EXECUTE, recorder_device, None,
                         lambda test, source, data, index=i:
                         verify_recorded_file(test, source, data, gen_filename(test, 16728, index), False,
                                              ch=get_channel(index),
                                              dtype=get_dtype(index))),
                    Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification finished'),
                ] if i < 12 else []),
                teardown=teardown(player_device) + teardown(recorder_device)
            ))

        for i in range(0, len(REC_FORMAT)):
            tests.append(Test(
                name='tc_recorder_preproc_16730_' + str(i),
                timeout=timeout,
                setup=setup_steps(player_device, player_reboot_monitor, player_error_monitor)
                      + setup_steps(recorder_device, recorder_reboot_monitor, recorder_error_monitor)
                      + verify_sd_card_mount(player_device)
                      + verify_sd_card_mount(recorder_device),
                test=[
                    Step(Action.WRITE, player_device, 'player'),
                    Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                         lambda test, source, data, index=i: execute_record(test, recorder_device, data,
                                                                   REC_FORMAT[index] + ' DIS', False)),
                    Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                         lambda test, source, data: store_rec_file_path(test, source, data)),
                    Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                    Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                    Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                    Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                    Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                    Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),
                ]
                + teardown(recorder_device)
                + setup_steps(recorder_device, recorder_reboot_monitor, recorder_error_monitor)
                + verify_sd_card_mount(recorder_device)
                + usb_s(recorder_device, False) +
                [
                    Step(Action.EXECUTE, recorder_device, None,
                              lambda test, source, data, index=i: mv_file(test, source, data,
                                                                     test.storage.rec_file_path,
                                                                     os.getcwd() + '/output',
                                                                     gen_filename(test, 16730, index))),
                    Step(Action.WAIT_FOR, recorder_device, 'moved'),
                ] + usb_e(recorder_device, False) + ([
                    Step(Action.EXECUTE, recorder_device, None,
                              lambda test, source, data, index=i:
                         verify_recorded_file(test, source, data,
                                              gen_filename(test, 16730, index),
                                              ch=get_channel(index),
                                              dtype=get_dtype(index))),
                    Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification finished'),
                ] if i < 12 else []),
                teardown=teardown(player_device) + teardown(recorder_device)
            ))

        audio = TestGroup(
            name='audio',
            devices=[player_device, recorder_device],
            tag=[Tag.POSITIVE],
            tests=tests,
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
    parser.add_argument('--use_preproc', action='store_true',
                        help='use PreProc')

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
    tc_runner = AudioRecordTestTcRunner(config, player=player, recorder=recorder)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
