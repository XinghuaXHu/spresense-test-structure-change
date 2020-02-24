# Test Automation Script for sd_create_write example application

## Overview

These python scripts are an implementation for automated testing of __sd_create_write__ example application

## Features

- All necessary projects are built automatically.
- Binary file with example application is flashed automatically.
- All test cases are executed sequentially.

## Setup

- Connect Spresense Board to PC.

## Automated Testing

- To run the test go to __*/autotest/src/tc/sd_create_write_test/*__ directory and run
  __*sd_create_write_test_tc_runner.py*__ script. All tests will run automatically.

    __Note:__ It is important to run the script exactly from the folder mentioned above.

## Arguments and optional flags

Script can be executed with optional flags and arguments as described below:

Runner arguments:

- __-c__ or __--config__ - select path to custom config
- __-v__ or __--verbose__ - log verbosity to console
- __-i__ or __--include_tags__ - include tests that should be run
- __-e__ or __--exclude_tags__ - exclude tests which should not be run
- __-s__ or __--skip_flash__ - skip device flashing
- __-rc__ or __--repeat_count__ - number of repeats if test fails

Device Under Test arguments:

- __-d__ or __--dut_bin__ - Device Under Test binary file path
- __--dut_device__ - set Device Under Test
