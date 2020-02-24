#! /usr/bin/env python3
import sys
import os
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)

from api.device_manager import DeviceManager
from tc.ostest_test.ostest_test_device import OsTestTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['ostest']

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if 'fpu_test: ERROR:' in data or 'waitpid failed with ECHILD' in data or 'PASS pthread_join failed with status=ESRCH' in data or 'nerrors=0' in data:
        test.add_user_event(source, 'skip')
    elif 'ERROR' in data or 'error' in data:
        test.set_fail(data)
    elif 'Failed' in data or 'failed' in data:
        test.set_fail(data)

def call_shell(test, source, data, command):
    test.log.info("start")
    os.system(command)
    test.log.info("end")

class OstestTestTcRunner(TestRunner):
    def __init__(self, conf, **kwargs):
        super(OstestTestTcRunner, self).__init__(conf)

        self.ostest_test_device = OsTestTestDevice(
            kwargs.get('dut_device', None), 'OSTEST_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/ostest-defconfig')

    def get_devices_list(self):
        return [self.ostest_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.ostest_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.ostest_test_device)

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
        device.check_device_config(str(device), APPS_TO_BUILTIN, log)

        if os.path.basename(dut_bin).startswith(str(device)):
            try:
                os.remove(dut_bin)
            except OSError as e:
                log.error(e)
                raise

    def generate_test_groups(self, arguments, log=None):
        timeout = 180
        #48 hours and a little margin

        ostest_test_device = self.get_devices_list()[0]


        setup = [
            Step(Action.WAIT_FOR, ostest_test_device, 'nsh>'),
            Step(Action.WRITE, ostest_test_device, 'reboot'),
            Step(Action.WAIT_FOR, ostest_test_device, 'NuttShell (NSH)'),
            Step(Action.EXECUTE, ostest_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, ostest_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]


        Nuttx_OsTest_15875_task_restart = Test(
            name='Nuttx_OsTest_15875_task_restart',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: task_restart test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'Test task_restart()'),
                Step(Action.WAIT_FOR, ostest_test_device, 'restart_main: Exiting'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16874_twaitpid = Test(
            name='Nuttx_OsTest_16874_twaitpid',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: waitpid test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'Test waitpid()'),
                Step(Action.WAIT_FOR, ostest_test_device, 'waitpid_last: PASS'),
                Step(Action.WAIT_FOR, ostest_test_device, 'Test waitid(P_PID)'),
                Step(Action.WAIT_FOR, ostest_test_device, 'waitpid_last: PASS'),
                Step(Action.WAIT_FOR, ostest_test_device, 'Test waitid(P_ALL)'),
                Step(Action.WAIT_FOR, ostest_test_device, 'waitpid_last: PASS'),
                Step(Action.WAIT_FOR, ostest_test_device, 'Test wait()'),
                Step(Action.WAIT_FOR, ostest_test_device, 'waitpid_last: PASS'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16875_mutex = Test(
            name='Nuttx_OsTest_16875_mutex',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: mutex test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'Errors	0	0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16877_pthread_cancel_test = Test(
            name='Nuttx_OsTest_16877_pthread_cancel_test',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: cancel test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'cancel_test: PASS'),
                Step(Action.WAIT_FOR, ostest_test_device, 'cancel_test: PASS'),
                Step(Action.WAIT_FOR, ostest_test_device, 'cancel_test: PASS'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16878_robust_test = Test(
            name='Nuttx_OsTest_16878_robust_test',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: robust test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'robust_test: Test complete with nerrors=0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16879_semaphore = Test(
            name='Nuttx_OsTest_16879_semaphore',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: semaphore test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'done'),
                Step(Action.WAIT_FOR, ostest_test_device, 'done'),
                Step(Action.WAIT_FOR, ostest_test_device, 'done'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16881_condition_variables = Test(
            name='Nuttx_OsTest_16881_condition_variables',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: condition variable test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'cond_test: Errors	0	0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )


        Nuttx_OsTest_16882_pthreads_rwlock_interfaces = Test(
            name='Nuttx_OsTest_16882_pthreads_rwlock_interfaces',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: pthread_rwlock test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16883_pthread_rwlock_cancel = Test(
            name='Nuttx_OsTest_16883_pthread_rwlock_cancel',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: pthread_rwlock_cancel test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'pthread_rwlock_cancel: Starting test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16885_timed_wait = Test(
            name='Nuttx_OsTest_16885_timed_wait',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: timed wait test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16886_message_queue = Test(
            name='Nuttx_OsTest_16886_message_queue',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: message queue test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'receiver_thread: returning nerrors=0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16887_timed_message_queue = Test(
            name='Nuttx_OsTest_16887_timed_message_queue',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: timed message queue test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'receiver_thread: returning nerrors=0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'timedmqueue_test: Test complete'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16888_signal_mask = Test(
            name='Nuttx_OsTest_16888_signal_mask',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: sigprocmask test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'sigprocmask_test: SUCCESS'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16889_signal_handler = Test(
            name='Nuttx_OsTest_16889_signal_handler',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: signal handler test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'done'),
                Step(Action.WAIT_FOR, ostest_test_device, 'done'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16890_nested_signal_handler = Test(
            name='Nuttx_OsTest_16890_nested_signal_handler',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: nested signal handler test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'signest_test: done'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16891_scheduler = Test(
            name='Nuttx_OsTest_16891_scheduler',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: round-robin scheduler test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'rr_test: Done'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16892_barrier = Test(
            name='Nuttx_OsTest_16892_barrier',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: barrier test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'completed with result=0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'completed with result=0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'completed with result=0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'completed with result=0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'completed with result=0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'completed with result=0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'completed with result=0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'completed with result=0'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        Nuttx_OsTest_16893_priority_inheritance = Test(
            name='Nuttx_OsTest_16893_priority_inheritance',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, ostest_test_device, 'ostest'),
                Step(Action.WAIT_FOR, ostest_test_device, 'user_main: priority inheritance test'),
                Step(Action.WAIT_FOR, ostest_test_device, 'priority_inheritance: Finished'),
                Step(Action.WAIT_FOR, ostest_test_device, 'End of test memory usage:'),
            ],
            teardown=teardown
        )

        ostest = TestGroup(
            name='ostest',
            devices=[ostest_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                Nuttx_OsTest_15875_task_restart,
                Nuttx_OsTest_16874_twaitpid,
                Nuttx_OsTest_16875_mutex,
                Nuttx_OsTest_16877_pthread_cancel_test,
                Nuttx_OsTest_16878_robust_test,
                Nuttx_OsTest_16879_semaphore,
                Nuttx_OsTest_16881_condition_variables,
                Nuttx_OsTest_16882_pthreads_rwlock_interfaces,
                Nuttx_OsTest_16883_pthread_rwlock_cancel,
                Nuttx_OsTest_16885_timed_wait,
                Nuttx_OsTest_16886_message_queue,
                Nuttx_OsTest_16887_timed_message_queue,
                Nuttx_OsTest_16888_signal_mask,
                Nuttx_OsTest_16889_signal_handler,
                Nuttx_OsTest_16890_nested_signal_handler,
                Nuttx_OsTest_16891_scheduler,
                Nuttx_OsTest_16892_barrier,
            ]
        )

        test_groups = [
            ostest,
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
    tc_runner = OstestTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
