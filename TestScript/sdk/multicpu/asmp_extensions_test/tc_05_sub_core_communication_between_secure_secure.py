#! /usr/bin/env python3
import sys
import os

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.asmp_extensions_test.asmp_extensions_test_device import AsmpExtensionsTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
TEST_DEVICE_APP_NAME = 'asmp_extensions'
APPS_TO_BUILTIN = ['asmp_extensions']


# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

class AsmpExtensionsTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(AsmpExtensionsTestTcRunner, self).__init__(conf)

        self.asmp_test_device = AsmpExtensionsTestDevice(
            kwargs.get('dut_device', None), 'ASMP_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/asmp_extensions-defconfig')

    def get_devices_list(self):
        return [self.asmp_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.asmp_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.asmp_test_device)

        return dict(dut_bin=dut_bin)

    @staticmethod
    def __build_binary(toolbox, defconfig, device):
        bin_file = toolbox.builder.build_project(PROJECT, APPS_TO_BUILTIN, device, defconfig)
        return os.path.normpath(bin_file)

    # noinspection PyMethodMayBeStatic
    def flash_n_check(self, device, dut_bin, log=None):
        if log:
            log.info('Flashing ' + str(device))

        device.flash(str(device), log, dut_bin)
#        device.check_device_config(str(device), APPS_TO_BUILTIN, log)

        device.check_device_config(str(device), TEST_DEVICE_APP_NAME, log)

        if os.path.basename(dut_bin).startswith(str(device)):
            try:
                os.remove(dut_bin)
            except OSError as e:
                log.error(e)
                raise

    def generate_test_groups(self, arguments, log=None):
        timeout = 10

        asmp_test_device = self.get_devices_list()[0]

        setup = [
            # Step(Action.WAIT_FOR, asmp_test_device, asmp_test_device.NSH_PROMPT),
            Step(Action.WRITE, asmp_test_device, 'reboot'),
            Step(Action.WAIT_FOR, asmp_test_device, asmp_test_device.NUTTSHELL),
            Step(Action.EXECUTE, asmp_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, asmp_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        tc_05_sub_core_communication_between_secure_secure = Test(
            name='tc_05_sub_core_communication_between_secure_secure',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, asmp_test_device, 'asmp_extensions'),
                Step(Action.WAIT_FOR, asmp_test_device, 'asmp app start'),
                Step(Action.WRITE, asmp_test_device, 'SHM_INIT --index 0 --keyid 1 --size 1024'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'SHM_ATTATCH --index 0 --flag 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),

                Step(Action.WRITE, asmp_test_device, 'MT_INIT --index 0 --keyid 3'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),

                Step(Action.WRITE, asmp_test_device, 'TASK_INIT_SEC --index 0 --filename worker_extensions'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_ASSIGN --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_INIT_SEC --index 1 --filename worker_extensions'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_ASSIGN --index 1'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_INIT --index 2 --filename worker_extensions'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_ASSIGN --index 2'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_INIT --index 3 --filename worker_extensions'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_ASSIGN --index 3'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_INIT --index 4 --filename worker_extensions'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_ASSIGN --index 4'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),

                Step(Action.WRITE, asmp_test_device, 'MQ_INIT --index 0 --keyid 2 --cpuid 3'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_INIT --index 1 --keyid 2 --cpuid 4'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_INIT --index 2 --keyid 2 --cpuid 5'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_INIT --index 3 --keyid 2 --cpuid 6'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_INIT --index 4 --keyid 2 --cpuid 7'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),


                Step(Action.WRITE, asmp_test_device, 'TASK_BIND --index 0 --mutex 0 --mq 0 --shm 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_BIND --index 1 --mutex 0 --mq 1 --shm 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_BIND --index 2 --mutex 0 --mq 2 --shm 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_BIND --index 3 --mutex 0 --mq 3 --shm 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_BIND --index 4 --mutex 0 --mq 4 --shm 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),


                Step(Action.WRITE, asmp_test_device, 'TASK_EXEC --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_EXEC --index 1'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_EXEC --index 2'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_EXEC --index 3'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_EXEC --index 4'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                #supervisor work0
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 0 --lock'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'LOCK DONE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 0 --write'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'WRITE SHM DONE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 0 --unlock'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'UNLOCK DONE'),
                Step(Action.WRITE, asmp_test_device, 'MT_LOCK --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'RD_SHM --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'Hello,CPU'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MT_UNLOCK --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                #supervisor work1
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 1 --lock'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 1'),
                Step(Action.WAIT_FOR, asmp_test_device, 'LOCK DONE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 1 --write'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 1'),
                Step(Action.WAIT_FOR, asmp_test_device, 'WRITE SHM DONE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 1 --unlock'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 1'),
                Step(Action.WAIT_FOR, asmp_test_device, 'UNLOCK DONE'),
                Step(Action.WRITE, asmp_test_device, 'MT_LOCK --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'RD_SHM --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'Hello,CPU'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MT_UNLOCK --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                #work0 to work1
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 0 --lock'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'LOCK DONE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 0 --write'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'WRITE SHM DONE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 0 --unlock'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'UNLOCK DONE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 0 --send --value 4'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'SEND DONE'),

                #work1 to work0
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 1 --lock'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 1'),
                Step(Action.WAIT_FOR, asmp_test_device, 'LOCK DONE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 1 --write'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 1'),
                Step(Action.WAIT_FOR, asmp_test_device, 'WRITE SHM DONE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 1 --unlock'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 1'),
                Step(Action.WAIT_FOR, asmp_test_device, 'UNLOCK DONE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 1 --rev --value 3'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_RECV --index 1'),
                Step(Action.WAIT_FOR, asmp_test_device, 'REV DONE'),


                #end
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 1 --quit'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 0 --quit'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 2 --quit'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 3 --quit'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_SEND --index 4 --quit'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),


                Step(Action.WRITE, asmp_test_device, 'TASK_DESTROY --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_DESTROY --index 1'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_DESTROY --index 2'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_DESTROY --index 3'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'TASK_DESTROY --index 4'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),

                Step(Action.WRITE, asmp_test_device, 'SHM_DETATCH --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'SHM_DESTROY --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MT_DESTROY --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_DESTROY --index 0'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_DESTROY --index 2'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_DESTROY --index 3'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'MQ_DESTROY --index 4'),
                Step(Action.WAIT_FOR, asmp_test_device, 'TRUE'),
                Step(Action.WRITE, asmp_test_device, 'QUIT'),
                Step(Action.WAIT_FOR, asmp_test_device, 'asmp app end'),
            ],
            teardown=teardown
        )

        asmp = TestGroup(
            name='asmp',
            devices=[asmp_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                tc_05_sub_core_communication_between_secure_secure,
            ]
        )

        test_groups = [
            asmp,
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
    tc_runner = AsmpExtensionsTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
