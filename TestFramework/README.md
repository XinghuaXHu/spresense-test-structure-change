# Test Automation Framework for Spresense Platform

These python scripts are an implementation for automated testing of Spresense SDK applications.

## Features:

  * Builds the necessary projects
  * Programs devices
  * Runs tests automatically
  * Saves logs of the test cases execution

## Installation procedure

No installation is needed, this platform runs in python 3.4.

## Prerequisites

- Python 3.4.

- PySerial 3.4 module.

## Setup

- Connect Spresense Board to PC.

    __Note:__ Some Test Case Runners may require two Spresense Boards connected to PC

## Configuration

- Create __*config.xml*__ file based on provided __*example_config.xml*__ located in __*autotest*__ directory.
- Place __*config.xml*__ in __*autotest*__ directory.
- Fill projects setting:
  * Path: give path to Spresense project
  * Tools:
    + Binaries: Give path to SDK
    + SDK Tools: Give path to __tools__ folder in SDK
- Fill device settings:
    + Chip revision: Set __00__
    + Project: Give project name
    + Serial: Give associated serial port (eg. COM1 or /dev/ttyUSB0)

        __Note:__ Remaining entries are for future use
- Repeat the above configuration steps for the second device (if runner requires one).

# Directory structure

```
autotest
|-- logs                                - Logs from the tests
|  `-- <RunnerName>TcRunner             - Logs from <RunnerName> runner
|       `-- log_YYYY-MM-DD_HHhMMmSSs    - Logs from particular runner run
|-- src                                 - Source code
    |-- api                             - Modules used by test cases and test case runners
    `-- tc                              - Implementation of test cases
        `-- <RunnerName>                - <RunnerName> application test cases

```

## Automated Testing

- Go to a __*autotest/src/tc/\<RunnerName\>*__ folder.

- Run __*<runner_name>_tc_runner.py*__ using command `python3 <runner_name>_tc_runner.py`. Tests
  will run automatically.

    __Note:__ It is important to run the script exactly from the folder mentioned above.

## Saved Log files

- To see the log files created by the Test Case Runner run. Go to
  __*autotest/logs/\<RunnerName\>/*__ and then to the sub-directory __*log_YYYY-MM-DD_HHhMMmSSs*__.
  Each sub-derectory contains:

  * __*summary.txt*__ includes the summary of all the testcases that ran
  * separate log file for every test case run
