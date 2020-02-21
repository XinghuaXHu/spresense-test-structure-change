#! /usr/bin/env python3
import re
import unicodedata


class Enum(object):
    _names = ['ZERO']

    def __init__(self, key=0):
        self.__key = key

    def __str__(self):
        if self.__key < len(self._names):
            return "%s" % self._names[self.__key]
        return '%d (no str. repr.)' % self.__key

    def __cmp__(self, other):
        if self.__key != other.__key:
            return 1
        return 0


def normalize_string(instr):
    """ Normalizes string
    :type instr: str
    :param instr: input string to process
    :return: normalized string
    """
    string = instr
    string = unicodedata.normalize('NFKD', string).encode('ascii', 'ignore')
    string = re.sub(b'[^\w\s-]', '', string).strip().lower()
    string = re.sub(b'[-\s]+', '_', string)
    string = string.decode()
    return string
