#! /usr/bin/env python3
import subprocess
import os

from api.base import NonCloneable


class Process(NonCloneable):
    """Class allows easy starting and stopping processes"""
    def __init__(self):
        """Creates empty Process object."""
        self.pr = None
        self.cmd = None

    def wait(self):
        """Function waits for process complete and return

        :return: process output and it's return code
        :rtype: basestring, int
        """
        (out, err) = self.pr.communicate()
        self.cmd = None
        return out, self.pr.returncode

    def poll(self):
        """Check if process has terminated

        :return: return code or None if process still working
        :rtype: int
        """
        if self.pr:
            return self.pr.poll()

        return 0

    def open(self, cmd):
        """Start process with the given command

        :param cmd: Command to run
        :type cmd: str
        """

        if isinstance(cmd, str):
            cmd = cmd.split()

        self.cmd = cmd
        self.cmd[0] = os.path.normpath(self.cmd[0])
        self.pr = subprocess.Popen(self.cmd, shell=False, stdout=subprocess.PIPE,
                                   stdin=subprocess.PIPE, stderr=subprocess.STDOUT,
                                   universal_newlines=True)

    def read(self):
        """Read process stdout

        :return: line from the process output
        :rtype: basestring
        """
        return self.pr.stdout.readline()

    def kill(self):
        self.pr.kill()

    def write(self, data):
        """Write data to process stdin

        :param data: data written to stdin
        :type data: str
        """
        return self.pr.stdin.write(data)

    def __repr__(self):
        """String often used to represent the event source

        :rtype: basestring
        """
        if isinstance(self.cmd, str):
            return self.cmd.split()[0]

        return str(self.cmd).split()[0]
