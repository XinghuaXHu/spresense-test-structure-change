#! /usr/bin/env python3
import sys
import os
import subprocess
import time


SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.audio_player_extensions_test.player_device import PlayerDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger, Timer
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
PLAYER_APPS = ['player_test', 'zmodem']
PLAYER_APPS_TO_BUILTIN = ['examples/player_test', 'zmodem']

UPDATER = 'updater.py'
UPDATER_PATH = 'autotest/src/api'
DSP_PATH = 'sdk/modules/audio/dsp'
DECODER = 'MP3DEC'
DECODER_WAV = 'WAVDEC'
SRC = 'SRC'

ZMODEM_DEFCONFIG = 'defconfig/zmodem-defconfig'
AUDIO_FILE = ['input/test.mp3', 'input/Sound.mp3', 'input/m2.mp3', 'input/w2.wav']

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
def sz_send_file(test, source, data, path):
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
def mv_file(test, source, data, file, dest_path):
    source.write('mv {} {}'.format(file, dest_path))
    test.add_user_event(source, 'File {} moved'.format(file))
    return True


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
        binaries = dict(player_bin=None)

        binaries['player_bin'] = os.path.normpath(arguments.player_bin)\
            if arguments.player_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.player, binaries['player_bin'], log, PLAYER_APPS)

    # noinspection PyUnresolvedReferences
    def build(self, debug=True, log=None, **kwargs):
        player_bin = kwargs.get('player_bin', None)
        toolbox = None

        if not all([player_bin]):
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
        timeout = 300

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
            if arguments.player_fs_ready:
                return setup_steps(device, reboot_monitor, error_monitor) +\
                       verify_sd_card_mount(device)
            else:
                return setup_steps(device, reboot_monitor, error_monitor) +\
                       verify_sd_card_mount(device) + prepare_player_fs_steps(device)

        def test_setup(device, reboot_monitor, error_monitor):
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

        def prepare_player_fs_steps(device):
            ret = [
                Step(Action.WAIT_FOR, device, device.NSH_PROMPT,
                     lambda test, source, data: execute_command(test, source, data, 'cd sd0',
                                                                True)),
                Step(Action.WAIT_FOR, device, 'Command cd sd0 executed',
                     lambda test, source, data: sz_send_file(test, source, data, self.audio_file[0])),
            ]
            for index in range(len(AUDIO_FILE) - 1):
                ret.append(Step(Action.WAIT_FOR, device,
                                'File {} successfully transferred'.format(AUDIO_FILE[index].split('/')[1]),
                                lambda test, source, data, i=index: sz_send_file(test, source, data, self.audio_file[i+1])))
            ret.extend(
                [
                    Step(Action.WAIT_FOR, device,
                         'File {} successfully transferred'.format(AUDIO_FILE[len(AUDIO_FILE) - 1].split('/')[1]),
                         lambda test, source, data: sz_send_file(test, source, data,
                                                                 self.audio_decoder)),
                    Step(Action.WAIT_FOR, device, 'File {} successfully transferred'.format(DECODER),
                         lambda test, source, data: sz_send_file(test, source, data,
                                                                 self.audio_decoder_wav)),
                    Step(Action.WAIT_FOR, device, 'File {} successfully transferred'.format(DECODER_WAV),
                         lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                    Step(Action.WAIT_FOR, device, device.NSH_LS,
                         lambda test, source, data: create_dirs(test, source, data, PLAYER_FOLDERS,
                                                                AUDIO_EXAMPLE_PATH)),
                    Step(Action.WAIT_FOR, device, 'Directories created',
                         lambda test, source, data: mv_file(test, source, data, DECODER, BIN)),
                    Step(Action.WAIT_FOR, device, 'File {} moved'.format(DECODER),
                         lambda test, source, data: mv_file(test, source, data, DECODER_WAV, BIN)),
                    Step(Action.WAIT_FOR, device, 'File {} moved'.format(DECODER_WAV),
                         lambda test, source, data: mv_file(test, source, data,
                                                            AUDIO_FILE[0].split('/')[1], AUDIO)),
                ])
            for index in range(len(AUDIO_FILE) - 1):
                ret.append(Step(Action.WAIT_FOR, device, 'File {} moved'.format(AUDIO_FILE[index].split('/')[1]),
                                lambda test, source, data, i=index: mv_file(test, source, data,
                                                                            AUDIO_FILE[i+1].split('/')[1], AUDIO)))
            return ret

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
                     lambda test, source, data: execute_command(test, source, data, 'ls', False)),
                Step(Action.WAIT_FOR, device, device.NSH_LS),
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

        tc_01_player0_mp3_play_stop = Test(
            name='tc_01_player0_mp3_play_stop',
            timeout=timeout,
            setup=player_setup(player_device, player_reboot_monitor, player_error_monitor),
            test=[
                Step(Action.WRITE, player_device, 'player_test'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_START),
                Step(Action.WRITE, player_device, 'PARAM 0 m2.mp3 24000 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY 0'),
                Step(Action.WAIT_FOR, player_device, 'Playing'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 3 sec'),
                Step(Action.WRITE, player_device, 'MUTE 1 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 6 sec'),
                Step(Action.WRITE, player_device, 'MUTE 0 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 15 sec'),
                Step(Action.WRITE, player_device, 'STOP 0 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Stopped'),
                Step(Action.WRITE, player_device, 'QUIT'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
            ],
            # teardown=player_teardown(player_device),
            teardown=None,
        )

        tc_02_player0_wav_play_stop = Test(
            name='tc_02_player0_wav_play_stop',
            timeout=timeout,
            setup=test_setup(player_device, player_reboot_monitor, player_error_monitor),
            test=[
                Step(Action.WRITE, player_device, 'player_test'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_START),
                Step(Action.WRITE, player_device, 'PARAM 0 w2.wav 24000 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY 0'),
                Step(Action.WAIT_FOR, player_device, 'Playing'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 3 sec'),
                Step(Action.WRITE, player_device, 'MUTE 1 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 6 sec'),
                Step(Action.WRITE, player_device, 'MUTE 0 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 15 sec'),
                Step(Action.WRITE, player_device, 'STOP 0 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Stopped'),
                Step(Action.WRITE, player_device, 'QUIT'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
            ],
            # teardown=player_teardown(player_device),
            teardown=None,
        )

        tc_03_player1_play_stop = Test(
            name='tc_03_player1_play_stop',
            timeout=timeout,
            # setup=player_setup(player_device, player_reboot_monitor, player_error_monitor),
            setup=test_setup(player_device, player_reboot_monitor, player_error_monitor),
            test=[
                Step(Action.WRITE, player_device, 'player_test'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_START),
                Step(Action.WRITE, player_device, 'PARAM 1 Sound.mp3 48000 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY 1'),
                Step(Action.WAIT_FOR, player_device, 'Playing'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 3 sec'),
                Step(Action.WRITE, player_device, 'MUTE 0 1 0'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 5 sec'),
                Step(Action.WRITE, player_device, 'MUTE 0 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 7 sec'),
                Step(Action.WRITE, player_device, 'STOP 1 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Stopped'),
                Step(Action.WRITE, player_device, 'QUIT'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
            ],
            teardown=None,
            # teardown=player_teardown(player_device),
        )

        tc_04_player_both_play_stop = Test(
            name='tc_04_player_both_play_stop',
            timeout=timeout,
            # setup=player_setup(player_device, player_reboot_monitor, player_error_monitor),
            setup=test_setup(player_device, player_reboot_monitor, player_error_monitor),
            test=[
                Step(Action.WRITE, player_device, 'player_test'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_START),
                Step(Action.WRITE, player_device, 'PARAM 0 m2.mp3 24000 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Param setting finished'),
                Step(Action.WRITE, player_device, 'PARAM 1 Sound.mp3 48000 2 16'),
                Step(Action.WAIT_FOR, player_device, 'Param setting finished'),
                Step(Action.WRITE, player_device, 'PLAY 0'),
                Step(Action.WAIT_FOR, player_device, 'Playing'),
                Step(Action.WRITE, player_device, 'PLAY 1'),
                Step(Action.WAIT_FOR, player_device, 'Playing'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 3 sec'),
                Step(Action.WRITE, player_device, 'MUTE 1 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 5 sec'),
                Step(Action.WRITE, player_device, 'MUTE 0 1 0'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 7 sec'),
                Step(Action.WRITE, player_device, 'MUTE 0 0 0'),
                Step(Action.WAIT_FOR, player_device, 'Escaped 8 sec'),
                Step(Action.WRITE, player_device, 'STOP 1 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Stopped'),
                Step(Action.WRITE, player_device, 'STOP 0 NORMAL'),
                Step(Action.WAIT_FOR, player_device, 'Stopped'),
                Step(Action.WRITE, player_device, 'QUIT'),
                Step(Action.WAIT_FOR, player_device, player_device.PLAYER_EXIT),
            ],
            teardown=player_teardown(player_device),
        )

        audio = TestGroup(
            name='audio',
            devices=[player_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_01_player0_mp3_play_stop,
                tc_02_player0_wav_play_stop,
                tc_03_player1_play_stop,
                tc_04_player_both_play_stop,
            ]
        )

        test_groups = [
            audio,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()

    parser.add_argument('--player_bin', help='Player binary file path')
    parser.add_argument('--player', metavar='SERIAL_PORT', help='Set player')
    parser.add_argument('--player_fs_ready', action='store_true',
                        help='Player folders and files already created')
    parser.add_argument('--preserve_player_fs', action='store_true',
                        help='Do not remove player folders and files after test')

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
