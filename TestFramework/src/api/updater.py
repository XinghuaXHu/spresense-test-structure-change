#!/usr/bin/python3

import os
import signal
import subprocess
import sys

from subprocess import TimeoutExpired

START_UPDATER_TIMEOUT = 120

def flash(port='/dev/ttyUSB0', baudrate=115200, bin_path=""):
    # Get this script file path
    api_path = os.path.dirname(__file__)

    # Get tools path
    tools_path = "%s/../tools" % api_path

    # Get Platform
    if sys.platform == 'linux':
        platform = "linux"
    else:
        platform = "windows"

    # Set flash_writer path
    flash_writer = "%s/flash_writer/%s/flash_writer" % (tools_path, platform)

    # Execute command
    cmd = [flash_writer,
           "-s",
           "-c", port,
           "-b", str(baudrate),
           "-d",
           "-n", bin_path]

    # Print execute command
    print(' '.join(cmd))

    # Prepare flash_writer process
    proc = subprocess.Popen(cmd)

    # Run process with timeout
    try:
        outs, errs = proc.communicate(timeout=START_UPDATER_TIMEOUT)
    except TimeoutExpired:
        os.kill(proc.pid+1, signal.SIGKILL)
        proc.terminate()

