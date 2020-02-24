#! /usr/bin/env python3
import sys
import os
import subprocess
import time
import numpy
import wave
import io
import pydub


SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "../../../..","TestFramework", "src"))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger, Timer
from api.config import Config
from api.runner_parser import RunnerParser
from utilities.usbmsc_mount import mountCurrentUSBDisk,umountCurrentUSBDisk,runBashFunction

sys.path.append(SCRIPT_PATH)
from player_device import PlayerDevice
from recorder_device import RecorderDevice

PROJECT = 'spresense'
PLAYER_APPS = ['player', 'zmodem']
RECORDER_APPS = ['recorder', 'zmodem']
PLAYER_APPS_TO_BUILTIN = ['player', 'zmodem']
RECORDER_APPS_TO_BUILTIN = ['recorder']

UPDATER = 'updater.py'
UPDATER_PATH = 'autotest/src/api'
DSP_PATH = 'sdk/modules/audio/dsp'
DECODER = 'MP3DEC'
DECODERS = ['MP3DEC', 'WAVDEC']
SRC = 'SRC'
ENCODERS = ['MP3ENC']

ZMODEM_DEFCONFIG = 'defconfig/zmodem-defconfig'
DATA_PATH = '../../../data/audio/adn_player_recorder_example_test'
PLAYLIST_FILE = 'input/TRACK_DB.CSV'
AUDIO_FILE = 'input/LPCM_96K_STEREO_24BIT.wav'
AUDIO_FILE2 = 'input/LPCM_88.2K_STEREO_24BIT.wav'
CREATE_PLAYLIST_FILE = 'output/TRACK_DB.CSV'

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
AUDIO_FILES = ['16344', '16666', '16667', '16668']

"""
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


# noinspection PyUnusedLocal
def rz_receive_file(test, source, data, rename):
    command = '{} {} {}'.format('rz', '<' + source.serial, '>' + source.serial)

    cwd = os.getcwd()
    os.chdir(cwd + '/output')

    pr = subprocess.Popen(command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)

    (out, error) = pr.communicate()

    transfer_completed = False
    file_name = 'unknown'

    if error:
        error = error.decode()
        for line in error.split('\r'):
            if 'Receiving:' in line:
                file_name = line.split(':')[1].strip()
            if 'Transfer complete' in line:
                transfer_completed = True
            if 'rz: not found' in line:
                test.set_fail('The program \'rz\' is currently not installed. '
                              'Install lrzsz package.')
                return
            if line.strip(' \t\n\r') != '':
                test.log.debug('[HOST PC] {}'.format(line.strip()))

    if transfer_completed:
        if rename:
            root, ext = os.path.splitext(file_name)
            mv_name = '{}_{}_{}_24BIT{}'.format(test.storage.rec_codec,
                                                test.storage.rec_sampling,
                                                test.storage.rec_channel,
                                                ext)
            os.rename(file_name, mv_name)
            test.log.debug('renamed {} -> {}'.format(file_name, mv_name))
        test.add_user_event(source, 'File {} successfully transferred'.format(file_name))
        os.chdir(cwd)
        return True
    else:
        test.set_fail('Failed to transfer file')


# noinspection PyUnusedLocal
def sz_send_file(test, source, data, path):
    test.log.debug('[HOST PC] {}{}'.format('path = ', path))
    command = '{} {} {} {}'.format('sz', path, '<' + source.serial, '>' + source.serial)

    while True:
        test.log.debug('start popen')
        pr = subprocess.Popen(command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE, bufsize=io.DEFAULT_BUFFER_SIZE * 2)

        test.log.debug('start communicate')
        (out, error) = pr.communicate()
        test.log.debug('end communicate')
        if 'Transfer complete' not in error.decode():
            test.log.debug('sz not completed. retry.')
            continue
        else:
            break

    transfer_completed = False
    file_name = 'unknown'

    if error:
        error = error.decode()
        for line in error.split('\r'):
            if 'Sending:' in line:
                file_name = line.split(':')[1].strip()
            if 'Transfer complete' in line:
                transfer_completed = True
            if 'TIMEOUT' in line:
                test.set_fail('Failed to transfer {} file'.format(file_name))
                return
            if 'sz: not found' in line:
                test.set_fail('The program \'sz\' is currently not installed. '
                              'Install lrzsz package.')
                return
            if line.strip(' \t\n\r') != '':
                test.log.debug('[HOST PC] {}'.format(line.strip()))

    if transfer_completed:
        # Wait for target
        time.sleep(1)
        test.add_user_event(source, 'File {} successfully transferred'.format(file_name))
        return True
    else:
        test.set_fail('Failed to transfer file')


# noinspection PyUnusedLocal
def sz_send_file_from_target(test, source, data):
    if not hasattr(test.storage, 'rec_file_path'):
        test.set_fail('Recorded file path not found')
    else:
        source.write('sz {}'.format(test.storage.rec_file_path))
        test.add_user_event(source, 'Sending file from {}'.format(source))
    return True


def sz_send_output_file(test, source, data, file_name):
    test.log.debug('sz_send_output_file ' + file_name)
    cwd = os.getcwd()
    os.chdir(cwd + '/output')
    command = '{} {} {} {}'.format('sz', file_name, '<' + source.serial, '>' + source.serial)
    source.write('rz')
    while True:
        test.log.debug('start popen')
        pr = subprocess.Popen(command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE)
        test.log.debug('start communicate')
        (out, error) = pr.communicate()
        test.log.debug('end communicate')
        if error:
            test.log.debug('error exist')
        if 'Transfer complete' not in error.decode():
            test.log.debug('sz not completed. retry.')
            continue
        else:
            break

    error = error.decode()

    for line in error.split('\r'):
        if line.strip(' \t\n\r') != '':
            test.log.debug('[HOST PC] {}'.format(line.strip()))
    test.add_user_event(source, 'File {} successfully transferred'.format(file_name))
    os.chdir(cwd)
    return True


def create_playlist(test, source, data, file_name):
    name, ext = os.path.splitext(file_name)
    opt = name.split('_')

    if opt[0] == 'MP3':
        codec = 'mp3'
    elif opt[0] == 'LPCM':
        codec = 'wav'
    elif opt[0] == 'OPUS':
        codec = 'opus'
    else:
        test.set_fail('Invalid codec {}'.format(opt[0]))
        return False

    if opt[1] == '8K':
        sampling = '8000'
    elif opt[1] == '16K':
        sampling = '16000'
    elif opt[1] == '48K':
        sampling = '48000'
    elif opt[1] == '88.2K':
        sampling = '88200'
    elif opt[1] == '96K':
        sampling = '96000'
    elif opt[1] == '192K':
        sampling = '192000'
    else:
        test.set_fail('Invalid sampling {}'.format(opt[1]))
        return False

    if opt[2] == 'MONO':
        channel = '1'
    elif opt[2] == 'STEREO':
        channel = '2'
    else:
        test.set_fail('Invalid channel {}'.format(opt[2]))
        return False
    if opt[3] == '16BIT':
        bitlength = '16'
    elif opt[3] == '24BIT':
        bitlength = '24'
    else:
        test.set_fail('Invalid channel {}'.format(opt[3]))
        return False

    target = os.path.join(SCRIPT_PATH, CREATE_PLAYLIST_FILE)
    text = '{},Anyone,1stAlbum,{},{},{},{},0\n'.format(file_name, channel, bitlength, sampling, codec)
    test.log.debug(target)
    test.log.debug(text)
    f = open(target, 'w')
    f.write(text)
    f.close()
    test.add_user_event(source, 'Playlist created')
    return True


# noinspection PyUnusedLocal
def mv_file(test, source, data, file, dest_path):
    source.write('mv {} {}'.format(file, dest_path))
    time.sleep(2)
    test.add_user_event(source, 'File {} moved'.format(file))
    return True

# noinspection PyUnusedLocal
# def rename_file(test, source, data, file, codec, sampling, channel)


# noinspection PyUnusedLocal
def execute_command(test, source, data, command, confirm):
    source.write(command)
    if confirm:
        test.add_user_event(source, 'Command {} executed'.format(command))
    return True


def execute_record(test, source, data, codec, sampling, channel, bitlen, confirm):
    test.storage.rec_codec = codec
    test.storage.rec_sampling = sampling
    test.storage.rec_channel = channel
    command = '{} {} {} {} {}'.format('recorder_hi', codec, sampling, channel, bitlen)
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
    test.storage.rec_file_path = data.get_value(source.KEY_FILE_PATH)
    return True


# noinspection PyUnusedLocal
def remove_recorded_file(test, source, data):
    if hasattr(test.storage, 'rec_file_path'):
        source.write('rm {}'.format(test.storage.rec_file_path))


# noinspection PyUnusedLocal
def remove_transferred_file(test, source, data):
    if hasattr(test.storage, 'rec_file_path'):
        file_name = test.storage.rec_file_path.split('/')[-1]
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
"""

def trans_mp3_to_wav(file_path,file):
    wav_file = file.split('.')[0] + '.wav'
    dest_file_path = os.path.join(os.getcwd(), DATA_PATH, 'output', wav_file)
    wav_song = pydub.AudioSegment.from_mp3(file_path)
    wav_song.export(dest_file_path, format="wav")
    return dest_file_path

def test_file_info(test,file):
    name, ext = os.path.splitext(file)
    opt = name.split('_')

    if opt[0] == 'MP3':
        codec = '.mp3'
    elif opt[0] == 'LPCM':
        codec = '.wav'
    elif opt[0] == 'OPUS':
        codec = '.opus'
    else:
        test.set_fail('Invalid codec {}'.format(opt[0]))
    
    if opt[1] == '16K':
        hzvalue = 16000
    elif opt[1] == '48K':
        hzvalue = 48000
    else:
        test.set_fail('Invalid hz value {}'.format(opt[1]))

    if opt[2] == 'MONO':
        chnum = 1
    elif opt[2] == 'STEREO':
        chnum = 2
    else:
        test.set_fail('Invalid CH NO {}'.format(opt[0]))
    
    if opt[3] == '16BIT':
        bitvalue = 16/8
    else:
        test.set_fail('Invalid bit value {}'.format(opt[2]))
    
    return codec,hzvalue,chnum,bitvalue,ext

# noinspection PyUnusedLocal
def verify_recorded_file(test, source, data, file):
    test_file_path = os.path.join(os.getcwd(), DATA_PATH, 'output', file)
    codec,hzvalue,chnum,bitvalue,ext = test_file_info(test,file)

    if ext == '.mp3':
        test_file_path = trans_mp3_to_wav(test_file_path,file)

    wave_read = wave.open(test_file_path)
    sample_rate = wave_read.getframerate()
    frames_number = wave_read.getnframes()
    channel_number = wave_read.getnchannels()
    samp_width = wave_read.getsampwidth()

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
    test.log.debug('Found Channel Num {} '.format(channel_number))
    test.log.debug('Found sample rate {} Hz'.format(sample_rate))
    test.log.debug('Found sample width {} Bit '.format(samp_width*8))

    test.assert_true(int(frequency) in range(TEST_AUDIO_FILE_FREQUENCY - FREQUENCY_VERIFY_ACCURACY,
                                             TEST_AUDIO_FILE_FREQUENCY + FREQUENCY_VERIFY_ACCURACY)
                    and channel_number == chnum
                    and sample_rate == hzvalue
                    and codec == ext
                    and samp_width == bitvalue,
                     'Recorded file verification')
    
    test.add_user_event(source, 'Recorded file verification success')
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
    ret = mountCurrentUSBDisk(25, '/tmp/sd0', source)
    if len(ret) == 0:
        test.set_fail('usbmsc mount failed')

def mountEnd(test, source, data):
    umountCurrentUSBDisk(5, '/tmp/sd0', source)

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

class AdnPlayerRecorderExampleTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AdnPlayerRecorderExampleTestTcRunner, self).__init__(conf)

        self.player = PlayerDevice(kwargs.get('player', None), 'PLAYER_DEVICE') \
            if kwargs.get('player', None) else None
        self.recorder = RecorderDevice(kwargs.get('recorder', None), 'RECORDER_DEVICE') \
            if kwargs.get('recorder', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least two devices are needed!')

        self.updater = os.path.join(self.config.projects[0].path, UPDATER_PATH, UPDATER)
        self.audio_decoder = os.path.join(self.config.projects[0].path, DSP_PATH)
        self.audio_encoder = os.path.join(self.config.projects[0].path, DSP_PATH)
        self.src = os.path.join(self.config.projects[0].path, DSP_PATH, SRC)
        self.playlist = os.path.join(os.getcwd(), DATA_PATH, PLAYLIST_FILE)
        self.audio_file = os.path.join(os.getcwd(), DATA_PATH, AUDIO_FILE)
        self.audio_file2 = os.path.join(os.getcwd(), DATA_PATH, AUDIO_FILE2)
        self.zmodem_defconfig = os.path.join(os.getcwd(), ZMODEM_DEFCONFIG)

        self.audio_dsp0 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[0])
        self.audio_dsp1 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[1])
        self.audio_dsp2 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[2])
        self.audio_dsp3 = os.path.join(self.config.projects[0].path, DSP_PATH, DSP_CODERS[3])
        # self.recorder_test_defconfig = os.path.join(os.path.dirname(__file__), RECORDER_TEST_DEFCONFIG)

    def get_devices_list(self):
        return [self.player, self.recorder]

    def generate_test_groups(self, arguments, log=None):
        timeout = 600
        #play_record_timeout = 1800

        player_device, recorder_device = self.get_devices_list()

        """
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

        def player_verify_setup(device, reboot_monitor, error_monitor):
            return setup_steps(device, reboot_monitor, error_monitor) +\
                   verify_sd_card_mount(device) +\
                   prepare_player_verify_steps(device)

        def recorder_verify_setup(device, reboot_monitor, error_monitor):
            return setup_steps(device, reboot_monitor, error_monitor) + verify_sd_card_mount(device)

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

        def prepare_player_verify_steps(device):
            return [
                Step(Action.WAIT_FOR, device, device.NSH_PROMPT,
                     lambda test, source, data: execute_command(test, source, data, 'cd sd0',
                                                                False)),
                Step(Action.WAIT_FOR, device, 'cd sd0',
                     lambda test, source, data: sz_send_output_file(test, source, data, 'LPCM_192K_STEREO_24BIT.wav')),
                Step(Action.WAIT_FOR, device,
                     'File {} successfully transferred'.format('LPCM_192K_STEREO_24BIT.wav'),
                     lambda test, source, data: mv_file(test, source, data, 'LPCM_192K_STEREO_24BIT.wav', AUDIO)),
                Step(Action.WAIT_FOR, device, 'File {} moved'.format('LPCM_192K_STEREO_24BIT.wav')),
            ]

        def prepare_player_fs_steps(device):
            return [
                Step(Action.WAIT_FOR, device, device.NSH_PROMPT,
                     lambda test, source, data: execute_command(test, source, data, 'cd sd0',
                                                                False)),
                Step(Action.WAIT_FOR, device, 'cd sd0',
                     lambda test, source, data: sz_send_file(test, source, data, self.audio_file)),
                Step(Action.WAIT_FOR, device,
                     'File {} successfully transferred'.format(AUDIO_FILE.split('/')[1]),
                     lambda test, source, data: sz_send_file(test, source, data, self.audio_file2)),
                Step(Action.WAIT_FOR, device,
                     'File {} successfully transferred'.format(AUDIO_FILE2.split('/')[1]),
                     lambda test, source, data: sz_send_file(test, source, data, self.playlist)),
                Step(Action.WAIT_FOR, device,
                     'File {} successfully transferred'.format(PLAYLIST_FILE.split('/')[1]),
                     lambda test, source, data: sz_send_file(test, source, data,
                                                             os.path.join(self.audio_decoder, DECODERS[0]))),
                Step(Action.WAIT_FOR, device, 'File {} successfully transferred'.format(DECODERS[0]),
                     lambda test, source, data: sz_send_file(test, source, data,
                                                             os.path.join(self.audio_decoder, DECODERS[1]))),
                Step(Action.WAIT_FOR, device, 'File {} successfully transferred'.format(DECODERS[1]),
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS,
                     lambda test, source, data: create_dirs(test, source, data, PLAYER_FOLDERS,
                                                            AUDIO_EXAMPLE_PATH)),
                Step(Action.WAIT_FOR, device, 'Directories created',
                     lambda test, source, data: mv_file(test, source, data, DECODERS[0], BIN)),
                Step(Action.WAIT_FOR, device, 'File {} moved'.format(DECODERS[0]),
                     lambda test, source, data: mv_file(test, source, data, DECODERS[1], BIN)),
                Step(Action.WAIT_FOR, device, 'File {} moved'.format(DECODERS[1]),
                     lambda test, source, data: mv_file(test, source, data,
                                                        PLAYLIST_FILE.split('/')[1], PLAYLIST)),
                Step(Action.WAIT_FOR, device, 'File {} moved'.format(PLAYLIST_FILE.split('/')[1]),
                     lambda test, source, data: mv_file(test, source, data,
                                                        AUDIO_FILE.split('/')[1], AUDIO)),
                Step(Action.WAIT_FOR, device, 'File {} moved'.format(AUDIO_FILE.split('/')[1]),
                     lambda test, source, data: mv_file(test, source, data,
                                                        AUDIO_FILE2.split('/')[1], AUDIO)),
            ]

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
                Step(Action.WAIT_FOR, device, device.NSH_PROMPT,
                     lambda test, source, data: execute_command(test, source, data, 'cd sd0',
                                                                False)),
                Step(Action.WAIT_FOR, device, 'cd sd0',
                     lambda test, source, data: sz_send_file(test, source, data, self.src)),
                Step(Action.WAIT_FOR, device,
                     'File {} successfully transferred'.format(SRC),
                     lambda test, source, data: sz_send_file(test, source, data,
                                                             os.path.join(self.audio_encoder, ENCODERS[0]))),
                Step(Action.WAIT_FOR, device, 'File {} successfully transferred'.format(ENCODERS[0]),
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS,
                     lambda test, source, data: create_dirs(test, source, data, RECORDER_FOLDERS,
                                                            AUDIO_EXAMPLE_PATH)),
                Step(Action.WAIT_FOR, device, 'Directories created',
                     lambda test, source, data: mv_file(test, source, data, ENCODERS[0], BIN)),
                Step(Action.WAIT_FOR, device, 'File {} moved'.format(ENCODERS[0]),
                     lambda test, source, data: mv_file(test, source, data, SRC, BIN)),
            ]

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
        """
        
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
            return [
                Step(Action.EXECUTE, player_device, None,
                        lambda test, source, data, : cp_file(test, source, data, os.path.join(os.getcwd(), DATA_PATH, 'input/' + '16340.mp3'), '')),
                Step(Action.WAIT_FOR, player_device, 'copied'),
            ]


        setup = [
            Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
            Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
        ]


        tc_player_16340_recorder_16344 = Test(
            name='tc_player_16340_recorder_16344',
            timeout=timeout,
            setup=setup,  
                 # player_verify_setup(player_device, player_reboot_monitor, player_error_monitor) +
                 # recorder_verify_setup(recorder_device, recorder_reboot_monitor, recorder_error_monitor),
            test=usb_s(recorder_device) + \
                 dsp_steps(recorder_device) + \
                 usb_e(recorder_device) + \
                 usb_s(player_device) + \
                 dsp_steps(player_device) + \
                 add_file_to_player() + \
                 usb_e(player_device) + [
                Step(Action.WAIT_FOR, player_device, 'Please input cmd'),
                Step(Action.WRITE, player_device, '16340'),
                #Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
                Step(Action.WRITE, recorder_device, '16344'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization Audio Library'),
                Step(Action.WAIT_FOR, recorder_device, 'Init Recorder!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),
                Step(Action.WAIT_FOR, player_device, 'End.'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '16344.mp3', os.path.join(os.getcwd(), DATA_PATH, 'output'),'MP3_48K_STEREO_16BIT.mp3')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),

                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, 'MP3_48K_STEREO_16BIT.mp3')),

                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            #       lambda test, source, data: sz_send_file_from_target(test, source, data)),
            #   Step(Action.WAIT_FOR, recorder_device,
            #        'Sending file from {}'.format(recorder_device),
            #         lambda test, source, data: rz_receive_file(test, source, data, False)),
            #   Step(Action.WAIT_FOR, recorder_device, 'successfully transferred',
            #         lambda test, source, data: verify_recorded_file(test, source, data)),
            #   Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success',
            #         lambda test, source, data: test.stop()),
            teardown=[],  
            # player_teardown(player_device) + recorder_teardown(recorder_device) + [
            # Step(Action.EXECUTE, recorder_device, None, remove_transferred_file),
            # ],
        )

        tc_player_16340_recorder_16642 = Test(
            name='tc_player_16340_recorder_16642',
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
                Step(Action.WRITE, player_device, '16340'),
                Step(Action.WRITE, recorder_device, '16642'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization Audio Library'),
                Step(Action.WAIT_FOR, recorder_device, 'Init Recorder!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),
                Step(Action.WAIT_FOR, player_device, 'End.'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '16642.mp3', os.path.join(os.getcwd(), DATA_PATH, 'output'), 'MP3_16K_MONO_16BIT.mp3')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),

                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, 'MP3_16K_MONO_16BIT.mp3')),

                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=[],  
        )

        tc_player_16340_recorder_16643 = Test(
            name='tc_player_16340_recorder_16643',
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
                Step(Action.WRITE, player_device, '16340'),
                Step(Action.WRITE, recorder_device, '16643'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization Audio Library'),
                Step(Action.WAIT_FOR, recorder_device, 'Init Recorder!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),
                Step(Action.WAIT_FOR, player_device, 'End.'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '16643.mp3', os.path.join(os.getcwd(), DATA_PATH, 'output'), 'MP3_16K_STEREO_16BIT.mp3')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),

                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, 'MP3_16K_STEREO_16BIT.mp3')),

                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=[],  
        )

        tc_player_16340_recorder_16644 = Test(
            name='tc_player_16340_recorder_16644',
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
                Step(Action.WRITE, player_device, '16340'),
                Step(Action.WRITE, recorder_device, '16644'),
                Step(Action.WAIT_FOR, recorder_device, 'initialization Audio Library'),
                Step(Action.WAIT_FOR, recorder_device, 'Init Recorder!'),
                Step(Action.WAIT_FOR, recorder_device, 'Recording Start!'),
                Step(Action.WAIT_FOR, recorder_device, 'End Recording'),
                Step(Action.WAIT_FOR, player_device, 'End.'),

                Step(Action.WAIT_FOR, recorder_device, 'Please input cmd'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, '16644.mp3', os.path.join(os.getcwd(), DATA_PATH, 'output'), 'MP3_48K_MONO_16BIT.mp3')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),

                Step(Action.WRITE, recorder_device, 'end'),
                Step(Action.WAIT_FOR, recorder_device, 'Finish USB Mass Storage Operation',
                     lambda test, source, data: verify_recorded_file(test, source, data, 'MP3_48K_MONO_16BIT.mp3')),

                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ],
            teardown=[],  
        )

        audio = TestGroup(
            name='audio',
            devices=[player_device, recorder_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_player_16340_recorder_16344,
                tc_player_16340_recorder_16642,
                tc_player_16340_recorder_16643,
                tc_player_16340_recorder_16644,
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
        config = Config(os.path.join('../../../../TestFramework/', Config.DEFAULT_CONFIG_FILE))

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
