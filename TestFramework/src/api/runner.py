#! /usr/bin/env python3

import os
import json

from api.output_handler import OutputHandler
from api.tester_exception import TesterException
from api.test_logger import TestLogger
from api.helpers import normalize_string
from api.test import Test
from api.utils import Toolbox

REBOOT_TIMEOUT = 10
JSONFILLE = "testscript.json"
TESTSW = 'testsw'
TESTSW_PATH = '../../../../TestSoftware'
DEVICES = ['dut', 'peer']
PROJECT = 'spresense'
# Class used to simulate NULL device
class NoLogging():
    class NullLogger():
        def __init__(self):
            pass

        def dummy(self, *args):
            pass

        def __getattr__(self, item):
            return self.dummy

        def __nonzero__(self):
            return True

    def open_log(self, *argc):
        null_logger = self.NullLogger()
        return null_logger

    def close_log(self, *argc):
        pass


class Tag(object):
    POSITIVE = 'positive'
    NEGATIVE = 'negative'
    STRESS = 'stress'
    CORNER = 'corner'
    SANITY = 'sanity'


class TestRunner(object):
    def __init__(self, config):
        self.__name = self.__class__.__name__

        # Main configuration file
        self.config = config

        # Sniffer handles
        self.ftm = None
        self.fts = None
        self.runner_log = None
        self.include_tags = []
        self.exclude_tags = []

    @property
    def name(self):
        return self.__name

    def print_runner_results(self, groups, logger, summary=True):
        # All groups results
        passed = []
        failed = []
        not_run = []

        if summary:
            logger.info(TestLogger.header('%s RUNNER SUMMARY' % self.name))

        for group in groups:
            gr_passed, gr_failed, gr_not_run = TestRunner.__print_test_group_results(group, logger,
                                                                                     summary=False)
            passed = passed + gr_passed
            failed = failed + gr_failed
            not_run = not_run + gr_not_run

        if summary:
            TestLogger.print_test_summary(len(passed), len(failed), len(not_run), logger)

        return passed, failed, not_run

    @staticmethod
    def __print_test_group_results(group, logger, summary=True):
        # Group results
        passed = []
        failed = []
        not_run = []

        if not len(group.tests):
            return passed, failed, not_run

        if summary:
            logger.info(TestLogger.header('TEST GROUP SUMMARY'))
        else:
            logger.info(TestLogger.tag('GROUP: %s' % group.name))

        for test in group.tests:
            TestLogger.print_test_summary_item(group.tests.index(test) + 1, len(group.tests), test,
                                               logger)

            if test.reason:
                logger.info(TestLogger.reason(test.reason))

            if test.verdict == Test.VERDICT_PASS:
                passed.append(test)
            elif test.verdict == Test.VERDICT_FAIL:
                failed.append(test)
            elif test.verdict == Test.VERDICT_NOT_RUN:
                not_run.append(test)

        if summary:
            TestLogger.print_test_summary(len(passed), len(failed), len(not_run), logger)

        return passed, failed, not_run

    # noinspection PyMethodMayBeStatic
    def get_devices_list(self):
        raise NotImplementedError()

    # noinspection PyMethodMayBeStatic
    def setup(self, args, log=None):
        filepath = os.path.join(os.getcwd(),JSONFILLE)

        if os.path.exists(filepath) != True:
            log.info('No testscript.json file ')
            return
        
        json_data=open(filepath,encoding='utf-8')
        data=json.load(json_data)
        if data != None:
           testsw_data = data[TESTSW]
           if testsw_data!= None:
               keys = testsw_data.keys()
               num = 0
               for key in keys:
                   device_data = testsw_data[key]
                   
                   debug = not args.release
                   device = eval('self.' + DEVICES[num] + '_device')
                   build_bin = os.path.normpath(eval('args.' + DEVICES[num] +'_bin'))\
                       if eval('args.' + DEVICES[num] +'_bin') else None
                   
                   num+=1
                   binaries = self.build(debug, log, device_data, build_bin, device) 
                   self.flash_n_check(device, binaries, log, device_data['appscheck'] if 'appscheck' in device_data.keys() else None )
           else:
                raise TesterException('testsw no data')
        else:
            raise TesterException('testscript.json file no data')
            return

    # noinspection PyUnresolvedReferences
    def build(self, debug=True, log=None, device_data=None, device_bin=None, device=None):
        toolbox = None
        build_bin = device_bin
        if not device_bin:
            if log:
                log.info('Building project sources')

            toolbox = Toolbox(self.config, log)
            toolbox.init_builder_module()

            print('not device_bin')
            log.info('Building {}'.format(device))
            apps_path = []
            defconfig_path = None
            for defconfig in device_data['build_path']:
                if 'examples' == defconfig.split('/')[0]:
                    apps_path.append(defconfig) 
                elif 'defconfig' == defconfig.split('/')[-1].split('-')[-1]:
                    defconfig_path = os.path.join(os.getcwd(),TESTSW_PATH,defconfig)
                    apps_path.append(defconfig_path.split('/')[-1].split('-')[0]) 

            build_bin = self.__build_binary(toolbox, apps_path, device,defconfig_path)

        return build_bin

    @staticmethod
    def __build_binary(toolbox, apps, device, defconfig):
        bin_file = toolbox.builder.build_project(PROJECT, apps, device, defconfig)
        return os.path.normpath(bin_file)

    # noinspection PyMethodMayBeStatic
    def flash_n_check(self, device, bin_path, log=None, apps=None):
        if log:
            log.info('Flashing ' + str(device))

        device.flash(str(device), log, bin_path)
        
        if apps != None:
           device.check_device_config(str(device), apps, log)

        if os.path.basename(bin_path).startswith(str(device)):
            try:
                os.remove(bin_path)
            except OSError as e:
                log.error(e)
                raise

    # noinspection PyMethodMayBeStatic
    def generate_test_groups(self, args, log=None):
        raise NotImplementedError()

    # noinspection PyMethodMayBeStatic
    #def build(self, debug=True, log=None, **kwargs):
        #pass

    # noinspection PyMethodMayBeStatic
    def __setup(self, args):
        """Fill in self.test_lists while implementing this method"""

        if hasattr(args, 'exclude_tags') and args.exclude_tags is not None:
            self.exclude_tags = [i.lower() for i in args.exclude_tags]

        if hasattr(args, 'include_tags') and args.include_tags is not None:
            self.include_tags = [i.lower() for i in args.include_tags]
            # noinspection PyMethodMayBeStatic
            # it - include tags, et - exclude tags, tt - test tags

    # noinspection PyMethodMayBeStatic
    def _check_tags_to_run_test(self, it, et, tt):
        if not tt:
            return True

        it_condition = True

        if it:
            include = list(set(it) - set(et))
            it_condition = any([i in tt for i in include])

        return it_condition and not any([i in tt for i in et])

    def run(self, args, logger=None, summary=True):
        """Run tests from the given test group"""
        groups = []

        # if logger is None use simulated NULL device
        log_device = logger if logger else NoLogging()

        try:
            self.runner_log = log_device.open_log('summary')
            self.runner_log.info(TestLogger.section('%s SETUP' % self.name))

            self.__setup(args)

            if hasattr(args, 'skip_flash') and not args.skip_flash:
                self.setup(args, self.runner_log)

            test_lists = self.generate_test_groups(args, self.runner_log)

            for group in test_lists:
                groups.append(group.clone())

            for group in groups:
                test_indexes_to_remove = []
                if None not in group.devices:
                    test_group_log_printed = False
                    for idx, test in enumerate(group.tests):
                        # Convert test.tag + group.tag (without duplicates) to lowercase
                        test_tag = [i.lower() for i in list(set(test.tag + group.tag))]

                        if not self._check_tags_to_run_test(self.include_tags, self.exclude_tags,
                                                            test_tag):
                            test_indexes_to_remove.insert(0, idx)
                            continue

                        try:
                            if len(group.tests) != 0 and group.prepare is not None:
                                group.prepare()
                        except Exception as e:
                            self.runner_log.info(TestLogger.header("TEST GROUP: %s" % group.name))
                            for test in group.tests:
                                test.verdict = Test.VERDICT_FAIL
                                reason = str(e)
                                test.reason = 'Test Group Setup failed with error: %s' % reason
                            continue

                        if not test_group_log_printed:
                            test_group_log_printed = True
                            self.runner_log.info(TestLogger.header("TEST GROUP: %s" % group.name))

                        if hasattr(args, 'repeat_count') and args.repeat_count is not None:
                            run_count = args.repeat_count + 1
                        else:
                            run_count = 1

                        log = log_device.open_log(normalize_string(test.name))

                        for j in range(run_count):
                            if j > 0:
                                log.info("Test will be repeated [R{}].".format(j + 1))
                                log.info('')
                                # reset test stage:
                                test.stage = Test.STAGE_NONE
                                test.reason = None

                            test.run(group.devices, log)

                            if test.verdict != Test.VERDICT_FAIL:
                                break

                        log_device.close_log(log)
                else:
                    for test in group.tests:
                        test.reason = 'No enough devices. Needed: {0}, found: {1}'.\
                            format(len(group.devices), sum(i is not None for i in group.devices))

                for i in test_indexes_to_remove:
                    group.tests.pop(i)

                self.__print_test_group_results(group, self.runner_log, summary=True)

            if summary:
                self.print_runner_results(groups, self.runner_log, summary=True)

        finally:
            # If exception happens in phase other than the test itself (setup, teardown) this
            # can affect all other tests from the runner thus we clean things up but exception is
            # propagated up the function call hierarchy.
            self.runner_log.info(TestLogger.section('%s TEARDOWN' % self.name))
            self.__teardown(self.runner_log)
            self.teardown(self.runner_log)

            log_device.close_log(self.runner_log)
            self.runner_log = None

        return groups

    # noinspection PyMethodMayBeStatic
    def teardown(self, log=None):
        pass

    # noinspection PyMethodMayBeStatic
    def __teardown(self, log=None):
        """If something in setup() needs to be cleaned, this is the right place."""

        if self.ftm is not None:
            self.ftm.release_sniffer(self.fts)
