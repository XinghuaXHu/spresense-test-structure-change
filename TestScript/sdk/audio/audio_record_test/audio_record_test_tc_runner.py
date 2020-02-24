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
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.audio_record_test.player_device import PlayerDevice
from tc.audio_record_test.recorder_device import RecorderDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger, Timer
from api.config import Config
from api.runner_parser import RunnerParser
from utilities.usbmsc_mount import mountCurrentUSBDisk,umountCurrentUSBDisk,runBashFunction

PROJECT = 'spresense'
PLAYER_APPS = ['audio_player', 'zmodem']
RECORDER_APPS = ['recorder_test', 'zmodem']
PLAYER_APPS_TO_BUILTIN = ['examples/audio_player', 'zmodem']
RECORDER_APPS_TO_BUILTIN = ['recorder_test']

UPDATER = 'updater.py'
UPDATER_PATH = 'autotest/src/api'
DSP_PATH = 'sdk/modules/audio/dsp'
DECODER = 'MP3DEC'
#DECODERS = ['MP3DEC', 'AACDEC', 'OPUSDEC', 'WAVDEC']
DECODERS = ['MP3DEC', 'WAVDEC']
SRC = 'SRC'
ENCODER = 'OPUSENC'
#ENCODERS = ['OPUSENC', 'MP3ENC']
ENCODERS = ['MP3ENC']

ZMODEM_DEFCONFIG = 'defconfig/zmodem-defconfig'
RECORDER_TEST_DEFCONFIG = 'defconfig/recorder_test-defconfig'
PLAYLIST_FILE = 'input/TRACK_DB.CSV'
AUDIO_FILE = 'input/test.mp3'
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
AUDIO_TEST_FILES = ['test', ]
AUDIO_FILES = ['LPCM_16K_STEREO.wav', 'LPCM_48K_STEREO.wav', 'MP3_16K_STEREO.mp3', 'MP3_48K_MONO.mp3',
               'MP3_48K_STEREO.mp3', 'LPCM_16K_MONO.wav']
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
    target = os.path.join(SCRIPT_PATH, CREATE_PLAYLIST_FILE)
    text = '{},Anyone,1stAlbum,{},16,{},{},0\n'.format(file_name, channel, sampling, codec)
    test.log.debug(target)
    test.log.debug(text)
    f = open(target, 'w')
    f.write(text)
    f.close()
    test.add_user_event(source, 'Playlist created')
    return True

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
def mv_file(test, source, data, file, dest_path, newfile=True):
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

def trans_mp3_to_wav(file_path,file):
    wav_file = file.split('.')[0] + '.wav'
    dest_file_path = os.path.join(os.getcwd(), 'output', wav_file)
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
    
    return codec,hzvalue,chnum,ext

# noinspection PyUnusedLocal
def verify_recorded_file(test, source, data, file):
    print("verify_recorded_file")
    test_file_path = os.path.join(os.getcwd(), 'output', file)
    print("verify_recorded_file1")
    codec,hzvalue,chnum,ext = test_file_info(test,file)
    print('ext {}'.format(ext))
    if ext == '.mp3':
        test_file_path = trans_mp3_to_wav(test_file_path,file)

    wave_read = wave.open(test_file_path)
    sample_rate = wave_read.getframerate()
    frames_number = wave_read.getnframes()
    channel_number = wave_read.getnchannels()

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
    test.log.debug('Found file ext {} '.format(ext))

    test.assert_true(int(frequency) in range(TEST_AUDIO_FILE_FREQUENCY - FREQUENCY_VERIFY_ACCURACY,
                                             TEST_AUDIO_FILE_FREQUENCY + FREQUENCY_VERIFY_ACCURACY)
                    and channel_number == chnum
                    and sample_rate == hzvalue
                    and codec == ext,
                     'Recorded file verification')
    
    test.add_user_event(source, 'Recorded file verification success')
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

        self.updater = os.path.join(self.config.projects[0].path, UPDATER_PATH, UPDATER)
#        self.audio_decoder = os.path.join(self.config.projects[0].path, DSP_PATH, DECODER)
        self.audio_decoder = os.path.join(self.config.projects[0].path, DSP_PATH)
#        self.audio_encoder = os.path.join(self.config.projects[0].path, DSP_PATH, ENCODER)
        self.audio_encoder = os.path.join(self.config.projects[0].path, DSP_PATH)
        self.src = os.path.join(self.config.projects[0].path, DSP_PATH, SRC)
        self.playlist = os.path.join(os.getcwd(), PLAYLIST_FILE)
        self.audio_file = os.path.join(os.getcwd(), AUDIO_FILE)
        self.zmodem_defconfig = os.path.join(os.getcwd(), ZMODEM_DEFCONFIG)
        self.recorder_test_defconfig = os.path.join(os.path.dirname(__file__), RECORDER_TEST_DEFCONFIG)
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
        timeout = 400
        play_record_timeout = 1500

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
            for file in AUDIO_TEST_FILES:
                r += [
                    Step(Action.EXECUTE, player_device, None,
                         lambda test, source, data, filename=file: cp_file(test, source, data, os.path.join(os.getcwd(), 'input/' + filename + '.mp3'), 'AUDIO')),
                    Step(Action.WAIT_FOR, player_device, 'copied'),

                ]
            r += [
                Step(Action.EXECUTE, player_device, None,
                     lambda test, source, data: cp_file(test, source, data, self.playlist, 'PLAYLIST')),
                Step(Action.WAIT_FOR, player_device, 'copied'),
            ]
            return r

        def add_playfile_to_player(file_name):
            r = [ ]
            for file in AUDIO_FILES:
                r += [
                    Step(Action.EXECUTE, player_device, None,
                         lambda test, source, data, filename=file: cp_file(test, source, data, os.path.join(os.getcwd(), 'output/' + filename), 'AUDIO')),
                    Step(Action.WAIT_FOR, player_device, 'copied'),
                ]
            r += [
                Step(Action.EXECUTE, player_device, None,
                     lambda test, source, data: create_playlist(test, player_device, data, file_name)),
                Step(Action.WAIT_FOR, player_device, 'Playlist created',
                     lambda test, source, data: cp_file(test, source, data, os.path.join(os.getcwd(), 'output/TRACK_DB.CSV'), 'PLAYLIST')),
                Step(Action.WAIT_FOR, player_device, 'copied'),
            ]
            return r
        
        def verify_rec_file(file):
            r = [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, 'REC', os.getcwd() + '/output')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: verify_recorded_file(test, source, data, file)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ]
            return r
        
        """
        SDK_Audio_14930_04 = Test(
            name='SDK_Audio_14930_04',
            timeout=play_record_timeout,
            setup=setup_steps(player_device, player_reboot_monitor, player_error_monitor) + \
                  setup_steps(recorder_device, recorder_reboot_monitor, recorder_error_monitor) + \
                  verify_sd_card_mount(player_device) + verify_sd_card_mount(recorder_device),
            test=usb_s(player_device) + \
                 add_playfile_to_player('LPCM_16K_STEREO.wav') + \
                 usb_e(player_device) + [
                Step(Action.WRITE, player_device, 'audio_player'),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_command(test, recorder_device, data,
                                                                'recorder_test', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + verify_rec_file() + usb_e(recorder_device) + \
                 usb_s(player_device) + add_playfile_to_player('LPCM_48K_STEREO.wav') + usb_e(player_device) + [

                Step(Action.WRITE, player_device, 'audio_player'),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_command(test, recorder_device, data,
                                                                'recorder_test', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + verify_rec_file() + usb_e(recorder_device) + \
                 usb_s(player_device) + add_playfile_to_player('MP3_16K_STEREO.mp3') + usb_e(player_device) + [

                Step(Action.WRITE, player_device, 'audio_player'),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_command(test, recorder_device, data,
                                                                'recorder_test', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + verify_rec_file() + usb_e(recorder_device) + \
                 usb_s(player_device) + add_playfile_to_player('MP3_48K_MONO.mp3') + usb_e(player_device) + [

                Step(Action.WRITE, player_device, 'audio_player'),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_command(test, recorder_device, data,
                                                                'recorder_test', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT,),

                 ] + usb_s(recorder_device) + verify_rec_file() + usb_e(recorder_device) + \
                 usb_s(player_device) + add_playfile_to_player('MP3_48K_STEREO.mp3') + usb_e(player_device) + [

                Step(Action.WRITE, player_device, 'audio_player'),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_command(test, recorder_device, data,
                                                                'recorder_test', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + verify_rec_file() + usb_e(recorder_device) + \
                 usb_s(player_device) + add_playfile_to_player('LPCM_16K_MONO.wav') + usb_e(player_device) + [

                Step(Action.WRITE, player_device, 'audio_player'),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_command(test, recorder_device, data,
                                                                'recorder_test', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + verify_rec_file() + usb_e(recorder_device) + \
                 usb_s(player_device) + add_playfile_to_player('LPCM_16K_MONO.wav') + usb_e(player_device),
            teardown=teardown(player_device) + teardown(recorder_device),
        )

        SDK_Audio_14930_03 = Test(
            name='SDK_Audio_14930_03',
            timeout=timeout,
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
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'LPCM', '16K', 'MONO', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                          lambda test, source, data: mv_file(test, source, data, 'REC', os.getcwd() + '/output', 'NULL')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                 ] + usb_e(recorder_device) + [

                Step(Action.WRITE, player_device, 'audio_player'),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'MP3', '48K', 'MONO', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                          lambda test, source, data: mv_file(test, source, data, 'REC', os.getcwd() + '/output', 'NULL')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                 ] + usb_e(recorder_device) + [

                Step(Action.WRITE, player_device, 'audio_player'),

                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'LPCM', '16K', 'STEREO', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                          lambda test, source, data: mv_file(test, source, data, 'REC', os.getcwd() + '/output', 'NULL')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                 ] + usb_e(recorder_device) + [

                Step(Action.WRITE, player_device, 'audio_player'),

                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'LPCM', '48K', 'STEREO', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                    lambda test, source, data: mv_file(test, source, data, 'REC', os.getcwd() + '/output', 'NULL')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                 ] + usb_e(recorder_device) + [

                Step(Action.WRITE, player_device, 'audio_player'),

                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'MP3', '16K', 'STEREO', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, 'REC', os.getcwd() + '/output', 'NULL')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                 ] + usb_e(recorder_device) + [

                Step(Action.WRITE, player_device, 'audio_player'),

                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'MP3', '48K', 'STEREO', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data: mv_file(test, source, data, 'REC', os.getcwd() + '/output', 'NULL')),
                Step(Action.WAIT_FOR, recorder_device, 'moved'),
                 ] + usb_e(recorder_device),
            teardown=teardown(player_device) + teardown(recorder_device),
        )
        """
        SDK_Audio_15558 = Test(
            name='SDK_Audio_15558',
            timeout=timeout,
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
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'LPCM', '16K', 'MONO', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + verify_rec_file('LPCM_16K_MONO.wav') + usb_e(recorder_device),
            teardown=teardown(player_device) + teardown(recorder_device),
        )

        SDK_Audio_15559 = Test(
            name='SDK_Audio_15559',
            timeout=timeout,
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
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'MP3', '48K', 'MONO', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + verify_rec_file('MP3_48K_MONO.mp3') + usb_e(recorder_device),
            teardown=teardown(player_device) + teardown(recorder_device),
        )

        SDK_Audio_14930 = Test(
            name='SDK_Audio_14930',
            timeout=timeout,
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
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'LPCM', '16K', 'STEREO', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + verify_rec_file('LPCM_16K_STEREO.wav') + usb_e(recorder_device),
            teardown=teardown(player_device) + teardown(recorder_device),
        )

        SDK_Audio_14931 = Test(
            name='SDK_Audio_14931',
            timeout=timeout,
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
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'LPCM', '48K', 'STEREO', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + verify_rec_file('LPCM_48K_STEREO.wav') + usb_e(recorder_device),
            teardown=teardown(player_device) + teardown(recorder_device),
        )

        SDK_Audio_14932 = Test(
            name='SDK_Audio_14932',
            timeout=timeout,
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
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'MP3', '16K', 'STEREO', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + verify_rec_file('MP3_16K_STEREO.mp3') + usb_e(recorder_device),
            teardown=teardown(player_device) + teardown(recorder_device),
        )

        SDK_Audio_14933 = Test(
            name='SDK_Audio_14933',
            timeout=timeout,
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
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'MP3', '48K', 'STEREO', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORDER_EXIT),

                 ] + usb_s(recorder_device) + verify_rec_file('MP3_48K_STEREO.mp3') + usb_e(recorder_device),
            teardown=teardown(player_device) + teardown(recorder_device),
        )

        audio = TestGroup(
            name='audio',
            devices=[player_device, recorder_device],
            tag=[Tag.POSITIVE],
            tests=[
                #SDK_Audio_14930_03,
                SDK_Audio_15558,
                SDK_Audio_15559,
                SDK_Audio_14930,
                SDK_Audio_14931,
                SDK_Audio_14932,
                SDK_Audio_14933,
                #SDK_Audio_14930_04,
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
    tc_runner = AudioRecordTestTcRunner(config, player=player, recorder=recorder)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
