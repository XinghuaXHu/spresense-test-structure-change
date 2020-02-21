#! /usr/bin/env python3
import threading

from api.base import NonCloneable
from threading import Timer

DEFAULT_TIMEOUT = 30


class StoppableThread(threading.Thread, NonCloneable):
    """Thread class with a :meth:`stop` method."""
    def __init__(self):
        """Initialises the StoppableThread object."""
        super(StoppableThread, self).__init__()
        self.__thread_run = threading.Event()
        self._stopper = threading.Event()
        self.daemon = True

    def start(self):
        super(StoppableThread, self).start()
        self.__thread_run.wait()

    def run(self):
        self.__thread_run.set()

    def stopit(self):
        """Stops the thread."""
        self._stopper.set()


class OutputHandler(StoppableThread):
    """Class for reading from the :class:`.Device` object."""
    def run(self):
        """Reads data from device to data_cb"""
        super(OutputHandler, self).run()

        while not self._stopper.is_set():
            try:
                line = self.device.read()
                if line:
                    self.data_cb(self.device, line)
            except Exception as e:
                self._stopper.set()
                print('Read failed:' + str(e))

    def __init__(self, device, data_cb):
        """Initialises the OutputHandler with the device to read from, and callback function to call.

        :type device: :class:`.Device`
        :type data_cb: callable object
        :param device: device from which the handler reads
        :param data_cb: callback function called with data read from the device
        """
        super(OutputHandler, self).__init__()
        self.device = device
        self.data_cb = data_cb


class ProcessOutputHandler(OutputHandler):
    def __init__(self, process, data_cb):
        super(ProcessOutputHandler, self).__init__(process, data_cb)

        self.results = None

    def join(self, timeout=None):
        self.stopit()

        timeout = timeout if timeout is not None else DEFAULT_TIMEOUT

        timer = Timer(timeout, self.device.kill)
        timer.start()

        super(ProcessOutputHandler, self).join()
        self.results = self.device.wait()

        timer.cancel()
