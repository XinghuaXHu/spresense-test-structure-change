# Test Automation Script for audio_player and audio_extensions_test applications

## Overview

These python scripts are an implementation for automated testing of __audio_extensions_test__ applications

## Features

- All necessary projects are built automatically.
- Binary files with example applications are flashed automatically.
- All test cases are executed sequentially.

## Prerequisites

- Installed __lrzsz__ package.

- NumPy 1.14 Python package.

## Setup

- Connect a Spresense Board to PC.
- Insert Micro SD card into Micro SD card slot (CN4) on Spresense Board.
- Connect Headphones connector (CN7) on Spresense Board configured as __player__ with Microphone

## Automated Testing

- To run the test, copy autotest/src/tc/audio_extension_test/input/* to SD Card's AUDIO directory and copy spresense/sdk/modules/audio/dsp/* to SD Card's BIN directory 
  Go to __*/autotest/src/tc/audio_extension_test/*__ directory and run
  __*audio_extensions_test_tc_runner.py*__ --flash_bin --reboot_each_tc --player_fs_ready. All tests will run automatically.

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

Devices Under Test arguments:

- __--player_bin__ - player binary file path
- __--player__ - set player device serial port
- __--player_fs_ready__ - player folders and files already created
- __--preserve_player_fs__ - do not remove player folders and files after test
