#! /usr/bin/env python3
import os
import logging
import sys
import time

from api.base import NonCloneable


class TestLogger(NonCloneable):
    """This class provides simple logging functionality. It allows the user to simultaneously open
    multiple logs using :func:`open_log` and close them in any order with  :func:`close_log`.
    By default all info, error and debug type messages are stored in files, but debug information
    are not sent to user's console until logger is configured to do so.

    :Usage Example:

    >>> logger = TestLogger(log_name='my_logs', console_debug=True)
    >>> log1 = logger.open_log('log_nr1')
    >>> log1.info(TestLogger.header('EXAMPLE HEADER MESSAGE'))
    INFO     ========== EXAMPLE HEADER MESSAGE ==========

    The above example would store logs in the following file:
    ``./logs/my_logs/<timestamp>/log_nr1.txt``

    """

    def __init__(self, log_name=None, console_debug=False, timestamp=None):
        """Initialises the logger module. Logs will be stored in :param log_name: subdirectory of
        logs folder.

        :type timestamp: str
        :type console_debug: bool
        :type log_name: str
        :param log_name: main log folder name
        :param console_debug: if debug messages should be printed to user's console
        :param timestamp: timestamp to be used as part of the folder's name (current time will be
               used if not provided)
        """
        self.console_debug = console_debug
        self.file_handlers = {}
        self.console_formatter = None

        def create_dir(dir_path):
            """Helper function for directory creation

            :type dir_path: str
            :param dir_path: directory to create
            """
            if not os.path.exists(dir_path):
                os.makedirs(dir_path)

        def find_logs():
            """This function helps finding logs directory by going up the folder tree hierarchy.

            :return: path to logs folder
            """
            dirname = os.path.normpath('./logs')
            d = 1

            while d < 5:
                if os.path.exists(dirname):
                    return os.path.normpath(dirname)
                d += 1
                dirname = os.path.join('../', dirname)

            return dirname

        dir_name = find_logs()
        date_str = 'log_' + \
                   (time.strftime("%Y-%m-%d_%Hh%Mm%Ss") if timestamp is None else timestamp)
        if log_name is None:
            self.log_file_path = os.path.join(dir_name, date_str)
        else:
            # Avoid duplicated log_name(e.g TestRunner)
            tc_filename = os.path.basename(sys.argv[0]).replace("_tc_runner.py", "")
            tc_dirname = os.path.basename(os.path.dirname(sys.argv[0]))
            log_name = "%s-%s" % (tc_dirname, tc_filename)
            self.log_file_path = os.path.join(dir_name, log_name, date_str)

        create_dir(self.log_file_path)

        self.console_formatter = logging.Formatter('<%(asctime)s> %(levelname)-8s %(message)s')
        self.console_handler = logging.StreamHandler()
        self.console_handler.setFormatter(self.console_formatter)

        if not console_debug:
            self.console_handler.setLevel(logging.INFO)

    def __logtofile(self, log_name):
        """Enables storing logs to file.

        :type log_name: str
        :param log_name: name of the log which should have file storage enabled
        """
        logger = logging.getLogger(log_name)

        file_path = os.path.join(self.log_file_path, log_name + '.txt')

        formatter = logging.Formatter('<%(asctime)s> %(levelname)-8s %(message)s',
                                      datefmt='%y-%m-%d %H:%M:%S')
        self.file_handlers[logger] = logging.FileHandler(file_path, mode='w')
        self.file_handlers[logger].setFormatter(formatter)
        self.file_handlers[logger].setLevel(logging.DEBUG)
        logger.addHandler(self.file_handlers[logger])

        logger.info('SAVING LOGS IN: %s' % file_path)

    def open_log(self, log_name='autotest'):
        """Opens new log with the given name.

        :type log_name: str
        :param log_name: log name
        :rtype: :class:`.TestLogger
        :return: logger object
        """
        logger = logging.getLogger(log_name)
        logger.setLevel(logging.DEBUG)
        logger.addHandler(self.console_handler)

        self.__logtofile(log_name)

        return logger

    def close_log(self, log):
        """Closes the log object. This removes the file and console handlers from logger.

        :type log: logging.Logger
        :param log: logger object to close
        """
        log.removeHandler(self.file_handlers[log])
        log.removeHandler(self.console_handler)

    @staticmethod
    def print_test_summary_item(tc_num, tests_count, test, log):
        verdict = str(test.verdict)
        if test.run_count > 1:
            verdict += ' [R{}]'.format(test.run_count)

        log.info('{0:<9} {1.name:<60} {2:9} {1.time:>7.1f}s'.
                 format('[{0}/{1}]'.format(tc_num, tests_count), test, verdict))

    @staticmethod
    def print_test_summary(passed_num, failed_num, not_run_num, log):
        log.info(TestLogger.line())
        log.info('ALL: {0:>4}{4}PASS: {1:>4}{4}FAIL: {2:>4}{4}NOT RUN: {3:>4}'.
                 format(failed_num + passed_num + not_run_num, passed_num, failed_num,
                           not_run_num, 16 * ' '))
        log.info(TestLogger.line())
        log.info('')

    @staticmethod
    def header(msg):
        """Creates the following header from the given message:

        ``=============== MESSAGE TEXT ===============``

        :type msg: str
        :param msg: text to format
        :rtype: str
        :return: formatted message
        """
        return '{:=^90}'.format(' ' + msg + ' ')

    @staticmethod
    def line():
        """Formats horizontal line made of '=' signs:

        ``============================================``

        :rtype: str
        :return: horizontal line
        """
        return 90*'='

    @staticmethod
    def section(msg):
        """Format msg into section:

        ``--------------- MESSAGE TEXT ---------------``

        :type msg: str
        :param msg: text to format
        :rtype: str
        :return: formatted message
        """
        return '{:-^90}'.format(' ' + msg + ' ')

    @staticmethod
    def tag(msg):
        """Format message into tag:

        ``> MESSAGE TEXT``

        :type msg: str
        :param msg: text to format
        :rtype: str
        :return: formatted message
        """
        return '> ' + msg

    @staticmethod
    def reason(reason_message):
        """Format reason_message into reason:

        ``         `--> Reason: Reason message``

        :type reason_message: str
        :param reason_message: text to format
        :rtype: str
        :return: formatted message
        """
        return '{0}`--> Reason: {1}'.format(9 * ' ', reason_message)
