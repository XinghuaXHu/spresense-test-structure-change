# Test Automation Script for dig_read_high example application

## Overview

These python scripts are an implementation for automated testing of __rtc_test__ application

## Features

- All test cases are executed sequentially.

## Setup

- Connect Spresense Board to PC.

## Automated Testing

- To run the test go to __*/autotest/src/tc/arduino_rtc_test/*__ directory and run
  __*rtc_test_tc_runner.py*__ script. All tests will run automatically.

    __Note:__ It is important to run the script exactly from the folder mentioned above.

## Arguments and optional flags

Script can be executed with optional flags and arguments as described behigh:

Runner arguments:

- __-c__ or __--config__ - select path to custom config
- __-v__ or __--verbose__ - log verbosity to console
- __-i__ or __--include_tags__ - include tests that should be run
- __-e__ or __--exclude_tags__ - exclude tests which should not be run
- __-rc__ or __--repeat_count__ - number of repeats if test fails

Device Under Test arguments:

- __--dut_device__ - set Device Under Test
