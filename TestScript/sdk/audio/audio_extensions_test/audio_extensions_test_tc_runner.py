#! /usr/bin/env python3
import sys
import os
import subprocess
import time
import random


SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.audio_extensions_test.player_device import PlayerDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger, Timer
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
PLAYER_APPS = ['audio_extensions_test', 'zmodem']
PLAYER_APPS_TO_BUILTIN = ['audio_extensions_test', 'zmodem']

UPDATER = 'updater.py'
UPDATER_PATH = 'autotest/src/api'
DSP_PATH = 'sdk/modules/audio/dsp'
DECODER = 'MP3DEC'
DECODER_WAV = 'WAVDEC'
ENCODER = 'MP3ENC'
SRC = 'SRC'

ZMODEM_DEFCONFIG = 'defconfig/audio_extensions_test-defconfig'
AUDIO_FILE = ['input/Sound.mp3', 'input/m2.mp3', 'input/w2.wav', 'input/a2.mp3', 'input/a3.mp3', ]

BIN = 'BIN'
AUDIO = 'AUDIO'

SD_CARD_MOUNT_VERIFY_TIME_OUT = 10

PLAYER_FOLDERS = ['BIN', 'AUDIO']

AUDIO_EXAMPLE_PATH = '/mnt/sd0/'


# noinspection PyUnusedLocal
def player_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


# noinspection PyUnusedLocal
def player_error_monitor(test, source, data):
    if source.APP_ERROR in data:
        test.set_fail('Player error: {}'.format(data.get_value(source.KEY_ERROR)))


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
def rz_receive_file(test, source, data):
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
        test.add_user_event(source, 'File {} successfully transferred'.format(file_name))
        os.chdir(cwd)
        return True
    else:
        test.set_fail('Failed to transfer file')


# noinspection PyUnusedLocal
def sz_send_file(source, path):
    command = '{} {} {} {}'.format('sz', path, '<' + source.serial, '>' + source.serial)

    pr = subprocess.Popen(command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)

    (out, error) = pr.communicate()

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
                # test.set_fail('Failed to transfer {} file'.format(file_name))
                return
            if 'sz: not found' in line:
                # test.set_fail('The program \'sz\' is currently not installed. '
                #               'Install lrzsz package.')
                return
            # if line.strip(' \t\n\r') != '':
                # test.log.debug('[HOST PC] {}'.format(line.strip()))

    if transfer_completed:
        # Wait for target
        time.sleep(1)
        # test.add_user_event(source, 'File {} successfully transferred'.format(file_name))
        return True
    # else:
        # test.set_fail('Failed to transfer file')


# noinspection PyUnusedLocal
def mv_file(source, file, dest_path, log):
    log.info('Moving ' + file)
    source.write('mv {} {}'.format(file, dest_path))
    print(source.read())
    print(source.read())
    print(source.read())
    time.sleep(2)
    # test.add_user_event(source, 'File {} moved'.format(file))
    return True


# noinspection PyUnusedLocal
def execute_command(test, source, data, command, confirm):
    source.write(command)
    if confirm:
        test.add_user_event(source, 'Command {} executed'.format(command))
    return True


# noinspection PyUnusedLocal
def test_exec_sleep(test, source, data):
    time.sleep(data)
    return True


# noinspection PyUnusedLocal
def test_sleep(test, source, data, sec):
    time.sleep(sec)
    return True


# noinspection PyUnusedLocal
def test_print_log(test, source, data):
    test.add_user_event(source, data)
    print(data)
    return True


# noinspection PyUnusedLocal
def create_dirs(source, dirs, path):
    for directory in dirs:
        source.write('mkdir {}'.format(path + directory))
        print(source.read())
        print(source.read())
        print(source.read())
        time.sleep(1)
    # test.add_user_event(source, 'Directories created')
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


# noinspection PyUnusedLocal
def trigger_event(test, source, event):
    test.add_user_event(source, event)


# noinspection PyUnusedLocal
def trigger_cleaning(test, source, event):
    if test.storage.sdcard:
        trigger_event(test, source, event)
    else:
        test.set_fail('No SD card mounted!')


class PlayerTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(PlayerTestTcRunner, self).__init__(conf)

        self.player = PlayerDevice(kwargs.get('player', None), 'PLAYER_DEVICE') \
            if kwargs.get('player', None) else None

        if not all(self.get_devices_list()):
            raise TesterException('At least 1 devices are needed!')

        self.updater = os.path.join(self.config.projects[0].path, UPDATER_PATH, UPDATER)
        self.audio_decoder = os.path.join(self.config.projects[0].path, DSP_PATH, DECODER)
        self.audio_decoder_wav = os.path.join(self.config.projects[0].path, DSP_PATH, DECODER_WAV)
        self.audio_encoder = os.path.join(self.config.projects[0].path, DSP_PATH, ENCODER)
        self.src = os.path.join(self.config.projects[0].path, DSP_PATH, SRC)
        self.audio_file = []
        for filename in AUDIO_FILE:
            self.audio_file.append(os.path.join(os.getcwd(), filename))
        self.zmodem_defconfig = os.path.join(os.getcwd(), ZMODEM_DEFCONFIG)

        self.defconfig_path = os.path.join(self.config.projects[0].path,
                                           'sdk/configs/examples')

    def get_devices_list(self):
        return [self.player]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        if arguments.flash_bin:
            binaries = dict(player_bin=None)

            binaries['player_bin'] = os.path.normpath(arguments.player_bin)\
                if arguments.player_bin else None

            binaries = self.build(debug, log, **binaries)

            self.flash_n_check(self.player, binaries['player_bin'], log, PLAYER_APPS)

            time.sleep(5)
        time.sleep(2)
        if arguments.player_fs_ready:
            return

        time.sleep(3)
        device = self.player

        for index in range(len(AUDIO_FILE)):
            log.info('Sending ' + self.audio_file[index])
            sz_send_file(device, self.audio_file[index])

        log.info('Sending ' + self.audio_decoder)
        sz_send_file(device, self.audio_decoder)
        log.info('Sending ' + self.audio_decoder_wav)
        sz_send_file(device, self.audio_decoder_wav)
        log.info('Sending ' + self.audio_encoder)
        sz_send_file(device, self.audio_encoder)
        log.info('Sending ' + self.src)
        sz_send_file(device, self.src)
        time.sleep(3)

        device.open()
        time.sleep(3)
        device.write('cd /mnt/sd0/')
        print(device.read())
        print(device.read())
        print(device.read())
        device.write('ls')
        print(device.read())
        print(device.read())
        print(device.read())
        log.info('Creating Dirs')
        create_dirs(device, PLAYER_FOLDERS, AUDIO_EXAMPLE_PATH)

        for i in range(len(AUDIO_FILE)):
            mv_file(device, AUDIO_FILE[i].split('/')[1], AUDIO_EXAMPLE_PATH + AUDIO, log)

        mv_file(device, DECODER, AUDIO_EXAMPLE_PATH + BIN, log)
        mv_file(device, DECODER_WAV, AUDIO_EXAMPLE_PATH + BIN, log)
        mv_file(device, ENCODER, AUDIO_EXAMPLE_PATH + BIN, log)
        mv_file(device, SRC, AUDIO_EXAMPLE_PATH + BIN, log)

        device.close()

    # noinspection PyUnresolvedReferences
    def build(self, debug=True, log=None, **kwargs):
        player_bin = kwargs.get('player_bin', None)
        toolbox = None

        if not all([player_bin]):
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

        return dict(player_bin=player_bin)

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

        player_device = self.get_devices_list()[0]

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
            if not arguments.reboot_each_tc:
                return None
            return setup_steps(device, reboot_monitor, error_monitor) +\
                verify_sd_card_mount(device)

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
            if not arguments.reboot_each_tc:
                return [
                    Step(Action.EXECUTE, device, 1, test_exec_sleep),
                ]
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
                return teardown(device)
                # return clean_player_fs(device) + teardown(device)

        tc_01_player0_mp3_mute = Test(
            name='tc_01_player0_mp3_mute',
            timeout=timeout,
            setup=player_setup(player_device, player_reboot_monitor, player_error_monitor),
            test=[
                Step(Action.WRITE, player_device, 'audio_extensions_test'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_START),
                Step(Action.WRITE, player_device, 'INIT_MEM player'),
                Step(Action.WRITE, player_device, 'CREATE'),
                Step(Action.WAIT_FOR, player_device, 'Create audio sub system succeed.'),
                Step(Action.WRITE, player_device, 'DIR open'),
                Step(Action.WRITE, player_device, 'FREQ_LOCK true'),
                Step(Action.WRITE, player_device, 'POWER on'),
                Step(Action.WRITE, player_device, 'INIT_FIFO'),
                Step(Action.WRITE, player_device, 'OUTSEL'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player0 m2.mp3'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player1 a2.mp3'),
                Step(Action.WRITE, player_device, 'CLK normal'),
                Step(Action.WRITE, player_device, 'STATUS player'),
                Step(Action.WAIT_FOR, player_device, 'Set PLAYER STATUS END'),
                Step(Action.WRITE, player_device, 'VOL -200 -200 -200'),
                Step(Action.WRITE, player_device, 'AMP_MUTE false'),
                Step(Action.WRITE, player_device, 'INITP 0 24000 2 16'),
                Step(Action.WRITE, player_device, 'INITP 1 AUTO 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Player0 Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY true true'),
                Step(Action.WAIT_FOR, player_device, 'Player0 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 3 sec'),
                Step(Action.WRITE, player_device, 'MUTE 1 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 6 sec'),
                Step(Action.WRITE, player_device, 'MUTE 0 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 10 sec'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 1 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Stopped'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 0 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Player0 Stopped'),
                Step(Action.WRITE, player_device, 'AMP_MUTE true'),
                Step(Action.WRITE, player_device, 'STATUS ready'),
                Step(Action.WRITE, player_device, 'POWER off'),
                Step(Action.WRITE, player_device, 'FREQ_LOCK false'),
                Step(Action.WRITE, player_device, 'DIR close'),
                Step(Action.WRITE, player_device, 'DELETE'),
                Step(Action.WRITE, player_device, 'FIN_MEM'),
                Step(Action.WRITE, player_device, 'QUIT'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
            ],
            teardown=player_teardown(player_device),
        )

        tc_01_player0_wav_mute = Test(
            name='tc_01_player0_wav_mute',
            timeout=timeout,
            setup=player_setup(player_device, player_reboot_monitor, player_error_monitor),
            test=[
                Step(Action.WRITE, player_device, 'audio_extensions_test'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_START),
                Step(Action.WRITE, player_device, 'INIT_MEM player'),
                Step(Action.WRITE, player_device, 'CREATE'),
                Step(Action.WAIT_FOR, player_device, 'Create audio sub system succeed.'),
                Step(Action.WRITE, player_device, 'DIR open'),
                Step(Action.WRITE, player_device, 'FREQ_LOCK true'),
                Step(Action.WRITE, player_device, 'POWER on'),
                Step(Action.WRITE, player_device, 'INIT_FIFO'),
                Step(Action.WRITE, player_device, 'OUTSEL'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player0 w2.wav'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player1 a2.mp3'),
                Step(Action.WRITE, player_device, 'CLK normal'),
                Step(Action.WRITE, player_device, 'STATUS player'),
                Step(Action.WAIT_FOR, player_device, 'Set PLAYER STATUS END'),
                Step(Action.WRITE, player_device, 'VOL -200 -200 -200'),
                Step(Action.WRITE, player_device, 'AMP_MUTE false'),
                Step(Action.WRITE, player_device, 'INITP 0 24000 2 16'),
                Step(Action.WRITE, player_device, 'INITP 1 AUTO 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Player0 Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY true true'),
                Step(Action.WAIT_FOR, player_device, 'Player0 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 3 sec'),
                Step(Action.WRITE, player_device, 'MUTE 1 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 6 sec'),
                Step(Action.WRITE, player_device, 'MUTE 0 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 10 sec'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 0 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Player0 Stopped'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 1 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Stopped'),
                Step(Action.WRITE, player_device, 'AMP_MUTE true'),
                Step(Action.WRITE, player_device, 'STATUS ready'),
                Step(Action.WRITE, player_device, 'POWER off'),
                Step(Action.WRITE, player_device, 'FREQ_LOCK false'),
                Step(Action.WRITE, player_device, 'DIR close'),
                Step(Action.WRITE, player_device, 'DELETE'),
                Step(Action.WRITE, player_device, 'FIN_MEM'),
                Step(Action.WRITE, player_device, 'QUIT'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
            ],
            teardown=player_teardown(player_device),
        )

        tc_02_player1_mp3_mute = Test(
            name='tc_02_player1_mp3_mute',
            timeout=timeout,
            setup=player_setup(player_device, player_reboot_monitor, player_error_monitor),
            test=[
                Step(Action.WRITE, player_device, 'audio_extensions_test'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_START),
                Step(Action.WRITE, player_device, 'INIT_MEM player'),
                Step(Action.WRITE, player_device, 'CREATE'),
                Step(Action.WRITE, player_device, 'DIR open'),
                Step(Action.WRITE, player_device, 'FREQ_LOCK true'),
                Step(Action.WRITE, player_device, 'POWER on'),
                Step(Action.WRITE, player_device, 'INIT_FIFO'),
                Step(Action.WRITE, player_device, 'OUTSEL'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player0 m2.mp3'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player1 a2.mp3'),
                Step(Action.WRITE, player_device, 'CLK normal'),
                Step(Action.WRITE, player_device, 'STATUS player'),
                Step(Action.WAIT_FOR, player_device, 'Set PLAYER STATUS END'),
                Step(Action.WRITE, player_device, 'VOL -200 -200 -200'),
                Step(Action.WRITE, player_device, 'AMP_MUTE false'),
                Step(Action.WRITE, player_device, 'INITP 0 24000 2 16'),
                Step(Action.WRITE, player_device, 'INITP 1 44100 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY true true'),
                Step(Action.WAIT_FOR, player_device, 'Player1 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 3 sec'),
                Step(Action.WRITE, player_device, 'MUTE 0 1 0'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 6 sec'),
                Step(Action.WRITE, player_device, 'MUTE 0 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 10 sec'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 1 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Stopped'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 0 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Player0 Stopped'),
                Step(Action.WRITE, player_device, 'AMP_MUTE true'),
                Step(Action.WRITE, player_device, 'STATUS ready'),
                Step(Action.WRITE, player_device, 'POWER off'),
                Step(Action.WRITE, player_device, 'FREQ_LOCK false'),
                Step(Action.WRITE, player_device, 'DIR close'),
                Step(Action.WRITE, player_device, 'DELETE'),
                Step(Action.WRITE, player_device, 'FIN_MEM'),
                Step(Action.WRITE, player_device, 'QUIT'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
            ],
            teardown=player_teardown(player_device),
        )

        tc_02_player1_wav_mute = Test(
            name='tc_02_player1_wav_mute',
            timeout=timeout,
            setup=player_setup(player_device, player_reboot_monitor, player_error_monitor),
            test=[
                Step(Action.WRITE, player_device, 'audio_extensions_test'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_START),
                Step(Action.WRITE, player_device, 'INIT_MEM player'),
                Step(Action.WRITE, player_device, 'CREATE'),
                Step(Action.WRITE, player_device, 'DIR open'),
                Step(Action.WRITE, player_device, 'FREQ_LOCK true'),
                Step(Action.WRITE, player_device, 'POWER on'),
                Step(Action.WRITE, player_device, 'INIT_FIFO'),
                Step(Action.WRITE, player_device, 'OUTSEL'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player0 a2.mp3'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player1 w2.wav'),
                Step(Action.WRITE, player_device, 'CLK normal'),
                Step(Action.WRITE, player_device, 'STATUS player'),
                Step(Action.WAIT_FOR, player_device, 'Set PLAYER STATUS END'),
                Step(Action.WRITE, player_device, 'VOL -200 -200 -200'),
                Step(Action.WRITE, player_device, 'AMP_MUTE false'),
                Step(Action.WRITE, player_device, 'INITP 0 AUTO 2 16'),
                Step(Action.WRITE, player_device, 'INITP 1 24000 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY true true'),
                Step(Action.WAIT_FOR, player_device, 'Player1 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 3 sec'),
                Step(Action.WRITE, player_device, 'MUTE 0 1 0'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 6 sec'),
                Step(Action.WRITE, player_device, 'MUTE 0 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 10 sec'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 0 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Player0 Stopped'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 1 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Stopped'),
                Step(Action.WRITE, player_device, 'AMP_MUTE true'),
                Step(Action.WRITE, player_device, 'STATUS ready'),
                Step(Action.WRITE, player_device, 'POWER off'),
                Step(Action.WRITE, player_device, 'FREQ_LOCK false'),
                Step(Action.WRITE, player_device, 'DIR close'),
                Step(Action.WRITE, player_device, 'DELETE'),
                Step(Action.WRITE, player_device, 'FIN_MEM'),
                Step(Action.WRITE, player_device, 'QUIT'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
            ],
            teardown=player_teardown(player_device),
        )

        step_player_both_normal_stop = [
                Step(Action.WAIT_FOR, player_device, 'Set READY STATUS END'),
                Step(Action.EXECUTE, player_device, '------------------------------------------------', test_print_log),
                Step(Action.EXECUTE, player_device, 'STEPS [step_player_both_normal_stop] executed', test_print_log),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player0 m2.mp3'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player1 a3.mp3'),
                Step(Action.WRITE, player_device, 'CLK normal'),
                Step(Action.WRITE, player_device, 'STATUS player'),
                Step(Action.WAIT_FOR, player_device, 'Set PLAYER STATUS END'),
                Step(Action.WRITE, player_device, 'VOL -200 -200 -200'),
                Step(Action.WRITE, player_device, 'AMP_MUTE false'),
                Step(Action.WRITE, player_device, 'INITP 0 24000 2 16'),
                Step(Action.WRITE, player_device, 'INITP 1 AUTO 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY true true'),
                Step(Action.WAIT_FOR, player_device, 'Player0 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player1 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 3 sec'),
                Step(Action.EXECUTE, player_device, 'Player0 VOLUME -> -1020', test_print_log),
                Step(Action.WRITE, player_device, 'VOL -1020 255 255'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 4 sec'),
                Step(Action.EXECUTE, player_device, 'Player0 VOLUME -> -500', test_print_log),
                Step(Action.WRITE, player_device, 'VOL -500 255 255'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 5 sec'),
                Step(Action.EXECUTE, player_device, 'Player0 VOLUME -> 0', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 0 255 255'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 6 sec'),
                Step(Action.EXECUTE, player_device, 'Player0 VOLUME -> 120', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 120 255 255'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 7 sec'),
                Step(Action.EXECUTE, player_device, 'Player0 VOLUME -> -200(default)', test_print_log),
                Step(Action.WRITE, player_device, 'VOL -200 255 255'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 9 sec'),
                Step(Action.EXECUTE, player_device, 'Player1 VOLUME -> -1020', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 255 -1020 255'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 10 sec'),
                Step(Action.EXECUTE, player_device, 'Player1 VOLUME -> -500', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 255 -500 255'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 11 sec'),
                Step(Action.EXECUTE, player_device, 'Player1 VOLUME -> 0', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 255 0 255'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 12 sec'),
                Step(Action.EXECUTE, player_device, 'Player1 VOLUME -> 120', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 255 120 255'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 13 sec'),
                Step(Action.EXECUTE, player_device, 'Player1 VOLUME -> -200(default)', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 255 -200 255'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 15 sec'),
                Step(Action.EXECUTE, player_device, 'Stop players NORMALLY.', test_print_log),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 1 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Stopped'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 0 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Player0 Stopped'),
                Step(Action.WRITE, player_device, 'AMP_MUTE true'),
                Step(Action.WRITE, player_device, 'STATUS ready'),
        ]

        step_player_both_natural_stop = [
                Step(Action.WAIT_FOR, player_device, 'Set READY STATUS END'),
                Step(Action.EXECUTE, player_device, '------------------------------------------------', test_print_log),
                Step(Action.EXECUTE, player_device, 'STEPS [step_player_both_natural_stop] executed', test_print_log),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player0 m2.mp3'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player1 a3.mp3'),
                Step(Action.WRITE, player_device, 'CLK normal'),
                Step(Action.WRITE, player_device, 'STATUS player'),
                Step(Action.WAIT_FOR, player_device, 'Set PLAYER STATUS END'),
                Step(Action.WRITE, player_device, 'VOL -200 -200 -200'),
                Step(Action.WRITE, player_device, 'AMP_MUTE false'),
                Step(Action.WRITE, player_device, 'INITP 0 24000 2 16'),
                Step(Action.WRITE, player_device, 'INITP 1 AUTO 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY true true'),
                Step(Action.WAIT_FOR, player_device, 'Player0 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player1 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 3 sec'),
                Step(Action.EXECUTE, player_device, 'Player0 VOLUME -> -1020', test_print_log),
                Step(Action.WRITE, player_device, 'VOL -1020 255 255'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 4 sec'),
                Step(Action.EXECUTE, player_device, 'Player0 VOLUME -> -500', test_print_log),
                Step(Action.WRITE, player_device, 'VOL -500 255 255'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 5 sec'),
                Step(Action.EXECUTE, player_device, 'Player0 VOLUME -> 0', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 0 255 255'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 6 sec'),
                Step(Action.EXECUTE, player_device, 'Player0 VOLUME -> 120', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 120 255 255'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 7 sec'),
                Step(Action.EXECUTE, player_device, 'Player0 VOLUME -> -200(default)', test_print_log),
                Step(Action.WRITE, player_device, 'VOL -200 255 255'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 9 sec'),
                Step(Action.EXECUTE, player_device, 'Player1 VOLUME -> -1020', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 255 -1020 255'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 10 sec'),
                Step(Action.EXECUTE, player_device, 'Player1 VOLUME -> -500', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 255 -500 255'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 11 sec'),
                Step(Action.EXECUTE, player_device, 'Player1 VOLUME -> 0', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 255 0 255'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 12 sec'),
                Step(Action.EXECUTE, player_device, 'Player1 VOLUME -> 120', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 255 120 255'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 13 sec'),
                Step(Action.EXECUTE, player_device, 'Player1 VOLUME -> -200(default)', test_print_log),
                Step(Action.WRITE, player_device, 'VOL 255 -200 255'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 15 sec'),
                Step(Action.WAIT_FOR, player_device, 'Player0 End of audio file'),
                Step(Action.EXECUTE, player_device, 'Player0 End of audio file', test_print_log),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 0 ESEND'),
                Step(Action.WAIT_FOR, player_device, 'Player0 Stopped'),
                Step(Action.WAIT_FOR, player_device, 'Player1 End of audio file'),
                Step(Action.EXECUTE, player_device, 'Player1 End of audio file', test_print_log),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 1 ESEND'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Stopped'),
                Step(Action.WRITE, player_device, 'AMP_MUTE true'),
                Step(Action.WRITE, player_device, 'STATUS ready'),
        ]

        step_player0_pause = [
                Step(Action.WAIT_FOR, player_device, 'Set READY STATUS END'),
                Step(Action.EXECUTE, player_device, '------------------------------------------------', test_print_log),
                Step(Action.EXECUTE, player_device, 'STEPS [step_player0_pause] executed', test_print_log),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player0 m2.mp3'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player1 a3.mp3'),
                Step(Action.WRITE, player_device, 'CLK normal'),
                Step(Action.WRITE, player_device, 'STATUS player'),
                Step(Action.WAIT_FOR, player_device, 'Set PLAYER STATUS END'),
                Step(Action.WRITE, player_device, 'VOL -200 -200 -200'),
                Step(Action.WRITE, player_device, 'AMP_MUTE false'),
                Step(Action.WRITE, player_device, 'INITP 0 24000 2 16'),
                Step(Action.WRITE, player_device, 'INITP 1 AUTO 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY true true'),
                Step(Action.WAIT_FOR, player_device, 'Player0 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player1 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 3 sec'),
                Step(Action.EXECUTE, player_device, 'PAUSE player0', test_print_log),
                Step(Action.WRITE, player_device, 'PAUSE player0'),
                # paused for 5 seconds and continue play
                Step(Action.WAIT_FOR, player_device, 'Player0 Paused',
                     lambda test, source, data: test_sleep(test, source, data, 5)),
                Step(Action.EXECUTE, player_device, 'Play player0', test_print_log),
                Step(Action.WRITE, player_device, 'PLAY true true'),
                # play for 6 seconds and stop
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 6 sec'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 0 ESEND'),
                Step(Action.WAIT_FOR, player_device, 'Player0 Stopped'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 1 ESEND'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Stopped'),
                Step(Action.WRITE, player_device, 'AMP_MUTE true'),
                Step(Action.WRITE, player_device, 'STATUS ready'),
        ]

        step_player1_pause = [
                Step(Action.WAIT_FOR, player_device, 'Set READY STATUS END'),
                Step(Action.EXECUTE, player_device, '------------------------------------------------', test_print_log),
                Step(Action.EXECUTE, player_device, 'STEPS [step_player1_pause] executed', test_print_log),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player0 m2.mp3'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player1 a3.mp3'),
                Step(Action.WRITE, player_device, 'CLK normal'),
                Step(Action.WRITE, player_device, 'STATUS player'),
                Step(Action.WAIT_FOR, player_device, 'Set PLAYER STATUS END'),
                Step(Action.WRITE, player_device, 'VOL -200 -200 -200'),
                Step(Action.WRITE, player_device, 'AMP_MUTE false'),
                Step(Action.WRITE, player_device, 'INITP 0 24000 2 16'),
                Step(Action.WRITE, player_device, 'INITP 1 AUTO 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY true true'),
                Step(Action.WAIT_FOR, player_device, 'Player0 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player1 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 3 sec'),
                Step(Action.EXECUTE, player_device, 'PAUSE player1', test_print_log),
                Step(Action.WRITE, player_device, 'PAUSE player1'),
                # paused for 5 seconds and continue play
                Step(Action.WAIT_FOR, player_device, 'Player1 Paused',
                     lambda test, source, data: test_sleep(test, source, data, 5)),
                Step(Action.EXECUTE, player_device, 'Play player1', test_print_log),
                Step(Action.WRITE, player_device, 'PLAY true true'),
                # play for 6 seconds and stop
                Step(Action.WAIT_FOR, player_device, 'Player1 escaped 6 sec'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 0 ESEND'),
                Step(Action.WAIT_FOR, player_device, 'Player0 Stopped'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 1 ESEND'),
                Step(Action.WAIT_FOR, player_device, 'Player1 Stopped'),
                Step(Action.WRITE, player_device, 'AMP_MUTE true'),
                Step(Action.WRITE, player_device, 'STATUS ready'),
        ]

        step_player0_beep = [
            Step(Action.WAIT_FOR, player_device, 'Set READY STATUS END'),
            Step(Action.EXECUTE, player_device, '------------------------------------------------', test_print_log),
            Step(Action.EXECUTE, player_device, 'STEPS [step_player0_beep] executed', test_print_log),
            Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player0 m2.mp3'),
            Step(Action.WRITE, player_device, 'CLK normal'),
            Step(Action.WRITE, player_device, 'STATUS player'),
            Step(Action.WAIT_FOR, player_device, 'Set PLAYER STATUS END'),
            Step(Action.WRITE, player_device, 'VOL -200 -200 -200'),
            Step(Action.WRITE, player_device, 'AMP_MUTE false'),
            Step(Action.WRITE, player_device, 'INITP 0 24000 2 16'),
            Step(Action.WAIT_FOR, player_device, 'Player0 Param setting finished'),
            Step(Action.WRITE, player_device, 'PLAY true false'),
            Step(Action.WAIT_FOR, player_device, 'Player0 playing'),
            Step(Action.WAIT_FOR, player_device, 'Player0 escaped 1 sec'),
            Step(Action.WRITE, player_device, 'BEEP enable -60 2000'),
            Step(Action.WAIT_FOR, player_device, 'Audio beep enabled'),
            Step(Action.WAIT_FOR, player_device, 'Player0 escaped 3 sec'),
            Step(Action.WRITE, player_device, 'VOL -1020 255 255'),
            Step(Action.WAIT_FOR, player_device, 'Player0 escaped 4 sec'),
            Step(Action.WRITE, player_device, 'VOL -500 255 255'),
            Step(Action.WAIT_FOR, player_device, 'Player0 escaped 5 sec'),
            Step(Action.WRITE, player_device, 'VOL 0 255 255'),
            Step(Action.WAIT_FOR, player_device, 'Player0 escaped 6 sec'),
            Step(Action.WRITE, player_device, 'VOL 120 255 255'),
            Step(Action.WAIT_FOR, player_device, 'Player0 escaped 7 sec'),
            Step(Action.WRITE, player_device, 'VOL -200 255 255'),
            Step(Action.WRITE, player_device, 'BEEP disable'),
            Step(Action.WAIT_FOR, player_device, 'Audio beep disabled'),
            Step(Action.WRITE, player_device, 'STOP_PLAYER 0 NORMAL'),
            Step(Action.WAIT_FOR, player_device, 'Player0 Stopped'),
            Step(Action.WRITE, player_device, 'AMP_MUTE true'),
            Step(Action.WRITE, player_device, 'STATUS ready'),
        ]

        step_player1_beep = [
            Step(Action.WAIT_FOR, player_device, 'Set READY STATUS END'),
            Step(Action.EXECUTE, player_device, '------------------------------------------------', test_print_log),
            Step(Action.EXECUTE, player_device, 'STEPS [step_player1_beep] executed', test_print_log),
            Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player1 a3.mp3'),
            Step(Action.WRITE, player_device, 'CLK normal'),
            Step(Action.WRITE, player_device, 'STATUS player'),
            Step(Action.WAIT_FOR, player_device, 'Set PLAYER STATUS END'),
            Step(Action.WRITE, player_device, 'VOL -200 -200 -200'),
            Step(Action.WRITE, player_device, 'AMP_MUTE false'),
            Step(Action.WRITE, player_device, 'INITP 1 AUTO 2 16'),
            Step(Action.WAIT_FOR, player_device, 'Player1 Param setting finished'),
            Step(Action.WRITE, player_device, 'PLAY false true'),
            Step(Action.WAIT_FOR, player_device, 'Player1 playing'),
            Step(Action.WAIT_FOR, player_device, 'Player1 escaped 1 sec'),
            Step(Action.WRITE, player_device, 'BEEP enable -60 2000'),
            Step(Action.WAIT_FOR, player_device, 'Audio beep enabled'),
            Step(Action.WAIT_FOR, player_device, 'Player1 escaped 3 sec'),
            Step(Action.WRITE, player_device, 'VOL 255 -1020 255'),
            Step(Action.WAIT_FOR, player_device, 'Player1 escaped 5 sec'),
            Step(Action.WRITE, player_device, 'VOL 255 -500 255'),
            Step(Action.WAIT_FOR, player_device, 'Player1 escaped 7 sec'),
            Step(Action.WRITE, player_device, 'VOL 255 0 255'),
            Step(Action.WAIT_FOR, player_device, 'Player1 escaped 9 sec'),
            Step(Action.WRITE, player_device, 'VOL 255 120 255'),
            Step(Action.WAIT_FOR, player_device, 'Player1 escaped 11 sec'),
            Step(Action.WRITE, player_device, 'VOL 255 -200 255'),
            Step(Action.WAIT_FOR, player_device, 'Player1 escaped 13 sec'),
            Step(Action.WRITE, player_device, 'BEEP disable'),
            Step(Action.WAIT_FOR, player_device, 'Audio beep disabled'),
            Step(Action.WRITE, player_device, 'STOP_PLAYER 1 NORMAL'),
            Step(Action.WAIT_FOR, player_device, 'Player1 Stopped'),
            Step(Action.WRITE, player_device, 'AMP_MUTE true'),
            Step(Action.WRITE, player_device, 'STATUS ready'),
        ]

        tc_03_steps = [
            Step(Action.WRITE, player_device, 'audio_extensions_test'),
            Step(Action.WAIT_FOR, player_device, player_device.PLAYER_START),
            Step(Action.WRITE, player_device, 'INIT_MEM player'),
            Step(Action.WRITE, player_device, 'CREATE'),
            Step(Action.WAIT_FOR, player_device, 'Create audio sub system succeed.'),
            Step(Action.WRITE, player_device, 'DIR open'),
            Step(Action.WRITE, player_device, 'FREQ_LOCK true'),
            Step(Action.WRITE, player_device, 'POWER on'),
            Step(Action.WRITE, player_device, 'INIT_FIFO'),
            Step(Action.WRITE, player_device, 'OUTSEL'),
            Step(Action.WAIT_FOR, player_device, 'INIT OUTPUT SELECT finished'),
            Step(Action.WRITE, player_device, 'STATUS ready'),
        ]

        for index in range(6):
            step_chosen = random.choice([
                step_player_both_normal_stop,
                step_player_both_natural_stop,
                step_player0_pause,
                step_player1_pause,
                step_player0_beep,
                step_player1_beep,
                ])
            tc_03_steps.extend(step_chosen)

        tc_03_steps.extend(
            [Step(Action.WRITE, player_device, 'POWER off'),
                Step(Action.WRITE, player_device, 'FREQ_LOCK false'),
                Step(Action.WRITE, player_device, 'DIR close'),
                Step(Action.WRITE, player_device, 'DELETE'),
                Step(Action.WRITE, player_device, 'FIN_MEM'),
                Step(Action.WRITE, player_device, 'QUIT'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
             ])

        tc_03_player_both_random = Test(
            name='tc_03_player_both_random',
            timeout=600,
            setup=player_setup(player_device, player_reboot_monitor, player_error_monitor),
            test=tc_03_steps,
            teardown=player_teardown(player_device),
        )

        tc_04_steps = [
                Step(Action.WRITE, player_device, 'audio_extensions_test'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_START),
            ]

        for index in range(5):
            steps = [
                Step(Action.EXECUTE, player_device, '------------------------------------------------', test_print_log),
                Step(Action.EXECUTE, player_device, 'ROUND[' + str(index + 1) + '] recording, please make some noises.', test_print_log),
                Step(Action.WRITE, player_device, 'INIT_MEM recorder'),
                Step(Action.WRITE, player_device, 'CREATE'),
                Step(Action.WAIT_FOR, player_device, 'Create audio sub system succeed.'),
                Step(Action.WRITE, player_device, 'DIR open'),
                Step(Action.WRITE, player_device, 'POWER on'),
                Step(Action.WRITE, player_device, 'INIT_FIFO'),
                Step(Action.WRITE, player_device, 'MICGAIN 210'),
                Step(Action.WRITE, player_device, 'STATUS recorder'),
                Step(Action.WRITE, player_device, 'INITR mp3 16K stereo'),
                Step(Action.WRITE, player_device, 'START_RECORDER rec' + str(index + 1) + '.mp3'),
                Step(Action.WAIT_FOR, player_device, 'Recorder Recording'),
                Step(Action.WAIT_FOR, player_device, 'Recorder escaped 15 sec'),
                Step(Action.WRITE, player_device, 'STOP_RECORDER'),
                Step(Action.WAIT_FOR, player_device, 'Recorder Stopped'),
                Step(Action.WRITE, player_device, 'DIR close'),
                Step(Action.WRITE, player_device, 'STATUS ready'),
                Step(Action.WAIT_FOR, player_device, 'Set READY STATUS END'),
                Step(Action.WRITE, player_device, 'POWER off'),
                Step(Action.WAIT_FOR, player_device, 'POWER OFF finished'),
                Step(Action.WRITE, player_device, 'DELETE'),
                Step(Action.WAIT_FOR, player_device, 'Delete audio sub system finished'),
                Step(Action.WRITE, player_device, 'FIN_MEM'),
                Step(Action.WAIT_FOR, player_device, 'Finalize libraries finished'),

                Step(Action.EXECUTE, player_device, 'ROUND[' + str(index + 1) + '] playing back.', test_print_log),
                Step(Action.WRITE, player_device, 'INIT_MEM player'),
                Step(Action.WRITE, player_device, 'CREATE'),
                Step(Action.WAIT_FOR, player_device, 'Create audio sub system succeed.'),
                Step(Action.WRITE, player_device, 'DIR open'),
                Step(Action.WRITE, player_device, 'FREQ_LOCK true'),
                Step(Action.WRITE, player_device, 'POWER on'),
                Step(Action.WRITE, player_device, 'INIT_FIFO'),
                Step(Action.WRITE, player_device, 'OUTSEL'),
                Step(Action.WRITE, player_device, 'OPEN_PLAY_FILE player0 rec' + str(index + 1) + '.mp3'),
                Step(Action.WRITE, player_device, 'CLK normal'),
                Step(Action.WRITE, player_device, 'STATUS player'),
                Step(Action.WAIT_FOR, player_device, 'Set PLAYER STATUS END'),
                Step(Action.WRITE, player_device, 'VOL -100 -100 -100'),
                Step(Action.WRITE, player_device, 'AMP_MUTE false'),
                Step(Action.WRITE, player_device, 'INITP 0 AUTO 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Player0 Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY true false'),
                Step(Action.WAIT_FOR, player_device, 'Player0 playing'),
                Step(Action.WAIT_FOR, player_device, 'Player0 escaped 9 sec'),
                Step(Action.WRITE, player_device, 'STOP_PLAYER 0 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Player0 Stopped'),
                Step(Action.WRITE, player_device, 'AMP_MUTE true'),
                Step(Action.WRITE, player_device, 'STATUS ready'),
                Step(Action.WRITE, player_device, 'POWER off'),
                Step(Action.WRITE, player_device, 'FREQ_LOCK false'),
                Step(Action.WRITE, player_device, 'DIR close'),
                Step(Action.WRITE, player_device, 'DELETE'),
                Step(Action.WRITE, player_device, 'FIN_MEM'),
                Step(Action.WAIT_FOR, player_device, 'Finalize libraries finished'),
            ]
            tc_04_steps.extend(steps)

        tc_04_steps.extend([
            Step(Action.WRITE, player_device, 'QUIT'),
            Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
        ])

        tc_04_player_recorder = Test(
            name='tc_04_player_recorder',
            timeout=300,
            setup=player_setup(player_device, player_reboot_monitor, player_error_monitor),
            test=tc_04_steps,
            teardown=player_teardown(player_device),
        )

        audio = TestGroup(
            name='audio',
            devices=[player_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_01_player0_mp3_mute,
                tc_01_player0_wav_mute,
                tc_02_player1_mp3_mute,
                tc_02_player1_wav_mute,
                tc_03_player_both_random,
                tc_04_player_recorder,
            ]
        )

        test_groups = [
            audio,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()

    parser.add_argument('--player_bin', help='Player binary file path. Available only when --flash_bin available.')
    parser.add_argument('--player', metavar='SERIAL_PORT', help='Set player')
    parser.add_argument('--player_fs_ready', action='store_true',
                        help='Player folders and files already created')
    parser.add_argument('--preserve_player_fs', action='store_true',
                        help='Do not remove player folders and files after test')
    parser.add_argument('--flash_bin', action='store_true',
                        help='Flash bin before run test')
    parser.add_argument('--reboot_each_tc', action='store_true',
                        help='Reboot before each test case run')

    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    player = dev_manager.get_devices_by_serials(args.player)[0]

    # Create test runner instance
    tc_runner = PlayerTestTcRunner(config, player=player)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
