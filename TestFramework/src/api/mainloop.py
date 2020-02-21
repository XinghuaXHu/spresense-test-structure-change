#! /usr/bin/env python3
from queue import Queue, Empty

from api.base import NonCloneable


class Mainloop(NonCloneable):
    """Class providing mainloop functionality"""
    TIMEOUT = 0.1

    def __init__(self):
        """Creates new instance of mainloop"""
        self.__active = False
        self.__events = Queue()

    def run(self):
        """Method runs loop and process events until quit() method is called"""
        self.__active = True

        while self.__active:
            try:
                (callback, data) = self.__events.get(timeout=Mainloop.TIMEOUT)
            except Empty:
                continue

            callback(data)

    def add(self, callback, data):
        """Method add new callback to be executed with given data

        :type callback: callable object
        :type data: object
        :param callback: callback to be executed
        :param data: object passed to callback()
        """
        self.__events.put((callback, data))

    # noinspection PyUnusedLocal
    def __quit(self, data):
        """Quit callback. It sets __active flag to false and creates new __events queue"""
        self.__active = False
        self.__events = Queue()

    def quit(self):
        """Method allows exit from run() method"""
        self.add(self.__quit, None)
