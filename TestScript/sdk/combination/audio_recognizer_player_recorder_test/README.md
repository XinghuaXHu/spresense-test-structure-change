# Test Automation Script for audio_player and audio_recorder example applications

## Overview

These python scripts are an implementation for automated testing of __audio_player_objif__ and
__audio_recorder__ example applications

## Features

- All necessary projects are built automatically.
- Binary files with example applications are flashed automatically.
- All test cases are executed sequentially.

## Prerequisites

- Installed __lrzsz__ package.

- NumPy 1.14 Python package.

## Setup

- Connect two Spresense Boards to PC.
- Insert Micro SD card into Micro SD card slot (CN4) on both Spresense Boards.
- Connect Headphones connector (CN7) on Spresense Board configured as __player__ with Microphone
pins (JP10) on Spresense Board configured as __recorder__ (GND->GND, LEFT or RIGHT->MICA).

## Automated Testing

- To run the test go to __*/autotest/src/tc/audio_aac_test/*__ directory and run
  __*audio_aac_test_tc_runner.py*__ script. All tests will run automatically.

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
- __--recorder_bin__ - recorder binary file path
- __--player__ - set player device serial port
- __--recorder__ - set recorder device serial port
- __--player_fs_ready__ - player folders and files already created
- __--recorder_fs_ready__ - recorder folders and files already created
- __--preserve_player_fs__ - do not remove player folders and files after test
- __--preserve_recorder_fs__ - do not remove recorder folders and files after test
- __--input_dirname - play file directory name
