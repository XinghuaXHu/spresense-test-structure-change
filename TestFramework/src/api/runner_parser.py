#! /usr/bin/env python3
from argparse import ArgumentParser


class RunnerParser(ArgumentParser):
    def __init__(self):
        super(RunnerParser, self).__init__(description='Test runner script')
        self.add_argument('-c', '--config', help='Config file path')
        self.add_argument('-v', '--verbose', action='store_true', help='Log verbosity')
        self.add_argument('-i', '--include_tags', metavar='TAG_N', nargs='+',
                          help='Include tests that should be run')
        self.add_argument('-e', '--exclude_tags', metavar='TAG_N', nargs='+',
                          help='Exclude tests which should not be run')
        self.add_argument('-d', '--dut_bin', help='Device Under Test binary file path')
        self.add_argument('-r', '--release', action='store_true',
                          help='Build binary files in release')
        self.add_argument('-s', '--skip_flash', action='store_true', help='Binary already flashed')
        self.add_argument('--dut_device', metavar='SERIAL_PORT', help='Set Device Under Test')
        self.add_argument('-rc', '--repeat_count', type=int, help='Number of repeats')