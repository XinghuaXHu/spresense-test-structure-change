#! /usr/bin/env python3


class TesterException(Exception):
    module = 'Tester'

    def __init__(self, message):
        super(TesterException, self).__init__(self.module + ': ' + message)
