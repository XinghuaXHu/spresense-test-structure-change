#! /usr/bin/env python3
import sys
import os

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "../../../..","TestFramework", "src"))

sys.path.append(API_PATH)
from api.runner import  Tag
from api.test import Test, Step, Action, TestGroup
from api.spresense_test_runner import SpresenseTestRunner


class HelloTestTcRunner(SpresenseTestRunner):

    def __init__(self):
        super(HelloTestTcRunner, self).__init__()


    def generate_test_groups(self, arguments, log=None):
        timeout = 10

        hello_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, hello_test_device, hello_test_device.NSH_PROMPT),
            Step(Action.WRITE, hello_test_device, 'reboot'),
            Step(Action.WAIT_FOR, hello_test_device, hello_test_device.NUTTSHELL),
            Step(Action.EXECUTE, hello_test_device, ('dut_reboot_mon', super(HelloTestTcRunner, self).dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, hello_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        SDK_Example_test_15836 = Test(
            name='SDK_Example_test_15836',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, hello_test_device, 'hello'),
                Step(Action.WAIT_FOR, hello_test_device, 'Hello, World!!'),
            ],
            teardown=teardown
        )

        hello = TestGroup(
            name='hello',
            devices=[hello_test_device],
            tag=[Tag.POSITIVE],
            tests=[
                SDK_Example_test_15836,
            ]
        )

        test_groups = [
            hello,
        ]

        return test_groups


if __name__ == "__main__":

    # Create test runner instance
    tc_runner = HelloTestTcRunner()
