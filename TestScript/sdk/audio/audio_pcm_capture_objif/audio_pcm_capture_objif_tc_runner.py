#! /usr/bin/env python3
import sys
import os

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.audio_pcm_capture_objif.audio_test_device import AudioTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
AUIDO_APPS = ['audio_pcm_capture_objif', 'zmodem']
APPS_TO_BUILTIN = ['examples/audio_pcm_capture_objif','zmodem']
ZMODEM_DEFCONFIG = 'defconfig/zmodem-defconfig'

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))


class AudioPcmCaptureObjifTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AudioPcmCaptureObjifTestTcRunner, self).__init__(conf)

        self.audio_test_device = AudioTestDevice(
            kwargs.get('dut_device', None), 'AUDIO_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), ZMODEM_DEFCONFIG)

    def get_devices_list(self):
        return [self.audio_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.audio_test_device, binaries['dut_bin'], log, AUIDO_APPS)

    # noinspection PyUnresolvedReferences
    def build(self, debug=True, log=None, **kwargs):
        dut_bin = kwargs.get('dut_bin', None)
        toolbox = None

        if not all([dut_bin]):
            if log:
                log.info('Building project sources')

            toolbox = Toolbox(self.config, log)
            toolbox.init_builder_module()

        if not dut_bin:
            dut_bin = self.__build_binary(toolbox, APPS_TO_BUILTIN, self.audio_test_device, self.defconfig_path)

        return dict(dut_bin=dut_bin)

    @staticmethod
    def __build_binary(toolbox, apps, device, defconfig):
        bin_file = toolbox.builder.build_project(PROJECT, apps, device, defconfig)
        return os.path.normpath(bin_file)

    # noinspection PyMethodMayBeStatic
    def flash_n_check(self, device, dut_bin, log=None, apps=None):
        if log:
            log.info('Flashing ' + str(device))

        device.flash(str(device), log, dut_bin)
        device.check_device_config(str(device), apps, log)

        if os.path.basename(dut_bin).startswith(str(device)):
            try:
                os.remove(dut_bin)
            except OSError as e:
                log.error(e)
                raise

    def generate_test_groups(self, arguments, log=None):
        timeout = 60

        audio_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WRITE, audio_test_device, 'reboot'),
            Step(Action.WAIT_FOR, audio_test_device, audio_test_device.NUTTSHELL),
            Step(Action.EXECUTE, audio_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, audio_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        SDK_Example_test_17162 = Test(
            name='SDK_Example_test_17162',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, audio_test_device, 'audio_pcm_capture_objif'),
                Step(Action.WAIT_FOR, audio_test_device, 'Start PCM capture with object level i/f example'),
                Step(Action.WAIT_FOR, audio_test_device, 'Sampling rate  48kHz'),
                Step(Action.WAIT_FOR, audio_test_device, 'Channel number 4ch'),
                Step(Action.WAIT_FOR, audio_test_device, 'Bit length     16bit'),
                Step(Action.WAIT_FOR, audio_test_device, 'Running time is 10 sec'),
                Step(Action.WAIT_FOR, audio_test_device, 'Exit PCM capture with object level i/f example'),
            ],
            teardown=teardown
        )

        audio = TestGroup(
            name='audio',
            devices=[audio_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Example_test_17162,
            ]
        )

        test_groups = [
            audio,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()
    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    dut_device = dev_manager.get_devices_by_serials(args.dut_device)[0]

    # Create test runner instance
    tc_runner = AudioPcmCaptureObjifTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
