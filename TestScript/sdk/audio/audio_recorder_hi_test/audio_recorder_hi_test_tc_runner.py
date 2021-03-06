#! /usr/bin/env python3
import sys
import os
import subprocess
import time
import numpy
import wave
import io


SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "../../../..","TestFramework", "src"))

sys.path.append(API_PATH)
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, Timer
from utilities.usbmsc_mount import mountCurrentUSBDisk, umountCurrentUSBDisk, runBashFunction
from api.spresense_test_runner import SpresenseTestRunner


UPDATER = 'updater.py'
UPDATER_PATH = 'autotest/src/api'
DSP_PATH = 'sdk/modules/audio/dsp'
DECODERS = ['MP3DEC', 'WAVDEC']
SRC = 'SRC'
ENCODERS = ['MP3ENC']

DATA_PATH = '../../../data/audio/audio_recorder_hi_test'
PLAYLIST_FILE = 'input/TRACK_DB.CSV'
AUDIO_FILE = 'input/LPCM_96K_STEREO_24BIT.wav'
AUDIO_FILE2 = 'input/LPCM_88.2K_STEREO_24BIT.wav'
AUDIO_FILE3 = 'input/LPCM_192K_STEREO_24BIT.wav'
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
FREQUENCY_VERIFY_ACCURACY = 100


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
            mv_name = '{}_{}_{}_{}{}'.format(test.storage.rec_codec,
                                             test.storage.rec_sampling,
                                             test.storage.rec_channel,
                                             test.storage.bitlen,
                                             ext)
            os.rename(file_name, mv_name)
            test.log.debug('renamed {} -> {}'.format(file_name, mv_name))
            test.storage.rec_file_path = mv_name
        test.add_user_event(source, 'File {} successfully transferred'.format(file_name))
        os.chdir(cwd)
        return True
    else:
        test.set_fail('Failed to transfer file')


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

    target = os.path.join(SCRIPT_PATH, DATA_PATH, CREATE_PLAYLIST_FILE)
    text = '{},Anyone,1stAlbum,{},{},{},{},0\n'.format(file_name, channel, bitlength, sampling, codec)
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


def execute_record(test, source, data, codec, sampling, channel, bitlen, confirm):
    test.storage.rec_codec = codec
    test.storage.rec_sampling = sampling
    test.storage.rec_channel = channel
    test.storage.bitlen = bitlen
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


# noinspection PyUnusedLocal
def verify_recorded_file(test, source, data):
    test_file_path = os.path.join(os.getcwd(), DATA_PATH, 'output', test.storage.rec_file_path.split('/')[-1])

    wave_read = wave.open(test_file_path)
    sample_rate = wave_read.getframerate()
    frames_number = wave_read.getnframes()

    k = numpy.arange(frames_number)
    T = frames_number / sample_rate
    frequencies = k / T
    frequencies = frequencies[range(int(frames_number / 2))]

    signal = wave_read.readframes(frames_number)

    if test.storage.rec_channel == 'MONO':
        ch = 1
    elif test.storage.rec_channel == 'STEREO':
        ch = 2
    else:
        ch = 4

    if test.storage.bitlen is '24BIT':
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


class AudioRecorderHiTestTcRunner(SpresenseTestRunner):

    def __init__(self):
        super(AudioRecorderHiTestTcRunner, self).__init__()

        self.updater = os.path.join(self.config.projects[0].path, UPDATER_PATH, UPDATER)
        self.audio_decoder = os.path.join(self.config.projects[0].path, DSP_PATH, DECODERS[1])
        self.audio_encoder = os.path.join(self.config.projects[0].path, DSP_PATH, ENCODERS[0])
        self.src = os.path.join(self.config.projects[0].path, DSP_PATH, SRC)
        self.playlist = os.path.join(os.getcwd(), DATA_PATH, PLAYLIST_FILE)
        self.audio_file = os.path.join(os.getcwd(), DATA_PATH, AUDIO_FILE)
        self.audio_file2 = os.path.join(os.getcwd(), DATA_PATH, AUDIO_FILE2)
        self.audio_file3 = os.path.join(os.getcwd(), DATA_PATH, AUDIO_FILE3)

    def generate_test_groups(self, arguments, log=None):
        play_record_timeout = 1800

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
            if arguments.dut_fs_ready:
                return setup_steps(device, reboot_monitor, error_monitor) +\
                       verify_sd_card_mount(device)
            else:
                return setup_steps(device, reboot_monitor, error_monitor) +\
                       verify_sd_card_mount(device) + prepare_player_fs_steps(device)

        def player_verify_setup(device, reboot_monitor, error_monitor):
            return setup_steps(device, reboot_monitor, error_monitor) + verify_sd_card_mount(device)

        def recorder_verify_setup(device, reboot_monitor, error_monitor):
            return setup_steps(device, reboot_monitor, error_monitor) + verify_sd_card_mount(device)

        def recorder_setup(device, reboot_monitor, error_monitor):
            if arguments.peer_fs_ready:
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
            return usb_s(device) + [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: cp_file(test, source, data, self.audio_file, 'AUDIO')),
                Step(Action.WAIT_FOR, device, 'copied'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: cp_file(test, source, data, self.audio_file2, 'AUDIO')),
                Step(Action.WAIT_FOR, device, 'copied'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: cp_file(test, source, data, self.audio_file3, 'AUDIO')),
                Step(Action.WAIT_FOR, device, 'copied'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: cp_file(test, source, data, self.playlist, 'PLAYLIST')),
                Step(Action.WAIT_FOR, device, 'copied'),
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: cp_file(test, source, data, self.audio_decoder, 'BIN')),
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
            return usb_s(device) + [
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
            if arguments.preserve_dut_fs:
                return teardown(device)
            else:
                return clean_player_fs(device) + teardown(device)

        def recorder_teardown(device):
            if arguments.preserve_peer_fs:
                return teardown(device) + remove_recorded_files_steps(device)
            else:
                return clean_recorder_fs(device) + teardown(device)

        tc_player_16258_recorder_16306 = Test(
            name='tc_player_16258_recorder_16306',
            timeout=play_record_timeout,
            setup=player_setup(player_device, player_reboot_monitor, player_error_monitor) +
                 recorder_setup(recorder_device, recorder_reboot_monitor, recorder_error_monitor),
            test=[
                Step(Action.WRITE, player_device, 'audio_player'),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'LPCM', '192K', 'STEREO', '24BIT', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, 'Exit AudioPlayer example'),
                Step(Action.WAIT_FOR, recorder_device, 'Exit AudioRecorder example'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data:
                     mv_file(test, source, data,
                             'REC/' + test.storage.rec_file_path.split('/')[-1],
                             os.path.join(os.getcwd(), DATA_PATH, 'output'),
                             test.storage.rec_file_path.split('/')[-1])),
                Step(Action.WAIT_FOR, recorder_device, 'moved',
                      lambda test, source, data: verify_recorded_file(test, source, data)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ] + usb_e(recorder_device),
            teardown=teardown(player_device) + teardown(recorder_device)
        )

        tc_player_15942_recorder_15959 = Test(
            name='tc_player_15942_recorder_15959',
            timeout=play_record_timeout,
            setup=player_verify_setup(player_device, player_reboot_monitor, player_error_monitor) +
                 recorder_verify_setup(recorder_device, recorder_reboot_monitor, recorder_error_monitor),
            test=[
                Step(Action.EXECUTE, player_device, None,
                     lambda test, source, data: create_playlist(test, player_device, data, 'LPCM_192K_STEREO_24BIT.wav')),
                Step(Action.WAIT_FOR, player_device, 'Playlist created'),
            ] + usb_s(player_device) + [
                Step(Action.EXECUTE, player_device, None,
                     lambda test, source, data: cp_file(test, source, data,
                                                        os.path.join(os.getcwd(), DATA_PATH, CREATE_PLAYLIST_FILE),
                                                        'PLAYLIST')),
                Step(Action.WAIT_FOR, player_device, 'copied'),
            ] + usb_e(player_device) + [
                Step(Action.WRITE, player_device, 'audio_player'),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'LPCM', '192K', 'STEREO', '16BIT', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, 'Exit AudioPlayer example'),
                Step(Action.WAIT_FOR, recorder_device, 'Exit AudioRecorder example'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data:
                     mv_file(test, source, data,
                             'REC/' + test.storage.rec_file_path.split('/')[-1],
                             os.path.join(os.getcwd(), DATA_PATH, 'output'),
                             test.storage.rec_file_path.split('/')[-1])),
                Step(Action.WAIT_FOR, recorder_device, 'moved',
                     lambda test, source, data: verify_recorded_file(test, source, data)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ] + usb_e(recorder_device),
            teardown=teardown(player_device) + teardown(recorder_device)
        )

        tc_player_16280_recorder_16307 = Test(
            name='tc_player_16280_recorder_16307',
            timeout=play_record_timeout,
            setup=player_verify_setup(player_device, player_reboot_monitor, player_error_monitor) +
                 recorder_verify_setup(recorder_device, recorder_reboot_monitor, recorder_error_monitor),
            test=[
                Step(Action.EXECUTE, player_device, None,
                     lambda test, source, data: create_playlist(test, player_device, data, 'LPCM_88.2K_STEREO_24BIT.wav')),
                Step(Action.WAIT_FOR, player_device, 'Playlist created'),
            ] + usb_s(player_device) + [
                Step(Action.EXECUTE, player_device, None,
                     lambda test, source, data: cp_file(test, source, data,
                                                        os.path.join(os.getcwd(), DATA_PATH, CREATE_PLAYLIST_FILE),
                                                        'PLAYLIST')),
                Step(Action.WAIT_FOR, player_device, 'copied'),
            ] + usb_e(player_device) + [
                Step(Action.WRITE, player_device, 'audio_player'),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME,
                     lambda test, source, data: execute_record(test, recorder_device, data,
                                                               'LPCM', '48K', 'STEREO', '24BIT', False)),
                Step(Action.WAIT_FOR, recorder_device, recorder_device.RECORD_DATA,
                     lambda test, source, data: store_rec_file_path(test, source, data)),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, player_device.RUNNING_TIME),
                Step(Action.WAIT_FOR, player_device, 'Exit AudioPlayer example'),
                Step(Action.WAIT_FOR, recorder_device, 'Exit AudioRecorder example'),
            ] + usb_s(recorder_device) + [
                Step(Action.EXECUTE, recorder_device, None,
                     lambda test, source, data:
                     mv_file(test, source, data,
                             'REC/' + test.storage.rec_file_path.split('/')[-1],
                             os.path.join(os.getcwd(), DATA_PATH, 'output'),
                             test.storage.rec_file_path.split('/')[-1])),
                Step(Action.WAIT_FOR, recorder_device, 'moved',
                     lambda test, source, data: verify_recorded_file(test, source, data)),
                Step(Action.WAIT_FOR, recorder_device, 'Recorded file verification success'),
            ] + usb_e(recorder_device),
            teardown=player_teardown(player_device) + recorder_teardown(recorder_device),
        )

        audio = TestGroup(
            name='audio',
            devices=[player_device, recorder_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_player_16258_recorder_16306,
                tc_player_15942_recorder_15959,
                tc_player_16280_recorder_16307,
            ]
        )

        test_groups = [
            audio,
        ]

        return test_groups


if __name__ == "__main__":
    # Create test runner instance
    tc_runner = AudioRecorderHiTestTcRunner()
