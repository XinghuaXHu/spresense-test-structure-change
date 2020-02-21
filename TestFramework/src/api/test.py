#! /usr/bin/env python3
import copy
import time
from threading import Timer


from api.helpers import Enum
from api.message import Parser
from api.output_handler import OutputHandler, ProcessOutputHandler
from api.test_logger import TestLogger
from api.mainloop import Mainloop


# noinspection PyClassHasNoInit
class Action(object):
    """Action class represents definitions of possible actions to perform test case step."""
    WRITE, WAIT_FOR, EXECUTE, NONE = range(4)


class Event(object):
    """Event class contains information about source and data received from UART."""
    def __init__(self, source, data):
        """Initialises the Event object with given parameters.

        :type source: :class:`.Device`
        :type data: object
        :param source: device on which action is performed
        :param data: data needed to perform action
        """

        self.source = source
        self.data = data


class Step(object):
    """Step class define structure of test case step."""

    def __init__(self, action, source, data=None, callback=None, terminator=None):
        """Initialises the Step object with given parameters.

        :type action: :class:`Action`
        :type source: :class:`.Device`
        :type data: object
        :type callback: callback function
        :param action: action performed on the device
        :param source: device on which action is performed
        :param data: data needed to perform action
        :param callback: callback function

        :Example:

        >>> Step(Action.WRITE, AUTOTEST_DEV1, 'echo autotest_demo'),
        """
        self.action = action
        self.source = source
        self.data = data
        self.callback = callback
        self.terminator = terminator
        self.storage = Storage()

    def __deepcopy__(self, memo):
        """Makes a deep copy of the Step object

        :param memo: Dictionary of objects already copied
        :return: new object instance
        """
        return type(self)(self.action, self.source, copy.copy(self.data), self.callback, self.terminator)


class Storage(object):
    """Empty storage container to hold any data in single place inside the test object."""
    pass


class Test(object):
    """Test class contains implementation of test case. Test consist of three phases: setup, test
    and teardown. Setup phase prepares test and reference devices for the test phase, in which
    the actual verification of the particular functionality is done. Teardown phase reverses
    operations done in setup to clear devices for the next test case. Each test phase consists of
    set of steps. During one step an action of one type is performed on the device. Test case has
    a unique name and a timeout which is applicable to each test phase separately.
    """

    class Verdict(Enum):
        """Verdict class represents possible test case result."""
        NOT_RUN, PASS, FAIL = range(3)
        _names = ['NOT RUN', 'PASS', 'FAIL']

    class Stage(Enum):
        """Stage class represents definitions of test case stage."""
        NONE, SETUP, TEST, TEARDOWN, DONE = range(5)
        _names = ['NONE', 'SETUP', 'TEST', 'TEARDOWN', 'DONE']

    VERDICT_PASS = Verdict(Verdict.PASS)
    VERDICT_FAIL = Verdict(Verdict.FAIL)
    VERDICT_NOT_RUN = Verdict(Verdict.NOT_RUN)

    STAGE_NONE = Stage(Stage.NONE)
    STAGE_SETUP = Stage(Stage.SETUP)
    STAGE_TEST = Stage(Stage.TEST)
    STAGE_TEARDOWN = Stage(Stage.TEARDOWN)
    STAGE_DONE = Stage(Stage.DONE)

    def __init__(self, name, start_condition=None, setup=None, test=None, teardown=None,
                 timeout=None, verdict=None, tag=None, fts_config=None):
        """Initialises the Test object with given parameters.

        :type name: str
        :type start_condition: callable object
        :type setup: list[:class:`Step`]
        :type test: list[:class:`Step`]
        :type teardown: list[:class:`Step`]
        :type timeout: int
        :type verdict: :class:`Verdict`
        :type tag: list[str]
        :type fts_config: :class:`FtsConfig`
        :param name: test case name
        :param start_condition: condition to start test
        :param setup: set of steps performed in setup phase
        :param test: set of steps performed in test phase
        :param teardown: set of steps performed in teardown phase
        :param timeout: timeout
        :param verdict: test case result
        :param tag: tags of test

        :Example:

        >>> test_01 = Test(
        >>>     name='TC_01: autotest_demo_communication_tc_01',
        >>>     timeout=15,
        >>>     tag=['positive', 'negative', 'user_tag'],
        >>>     setup=[
        >>>         Step(Action.EXECUTE, None, AUTOTEST_DEV1, self.reboot_device),
        >>>         Step(Action.WAIT_FOR, AUTOTEST_DEV1, self.app_started_str),
        >>>     ],
        >>>     test=[
        >>>         Step(Action.WRITE, AUTOTEST_DEV1, 'echo Example testing text'),
        >>>         Step(Action.WAIT_FOR, AUTOTEST_DEV1, 'Example testing text'),
        >>>     ],
        >>>     teardown=[
        >>>         Step(Action.EXECUTE, AUTOTEST_DEV1, AUTOTEST_DEV2,
        >>>             self.disconnect_with_device),
        >>>     ])
        """

        self.name = name
        self.start_condition = start_condition
        self.setup = setup if setup else []
        self.test = test if test else []
        self.teardown = teardown if teardown else []
        self.stage = Test.STAGE_NONE
        self.timeout = timeout
        self.timer = None
        self.verdict = verdict if verdict else Test.VERDICT_NOT_RUN
        self.reason = None
        self.tag = [self.name] + (tag if tag else [])
        self.steps = []
        self.time = 0
        self.run_count = 0
        self.mainloop = None
        self.output_handlers = {}
        self.process_output_handlers = {}
        self.log = None
        self.storage = Storage()
        self.monitors = None

    def clone(self):
        """Clone Test object."""
        test = Test(self.name, start_condition=self.start_condition,
                    setup=copy.deepcopy(self.setup), test=copy.deepcopy(self.test),
                    teardown=copy.deepcopy(self.teardown), timeout=self.timeout,
                    verdict=self.verdict, tag=copy.deepcopy(self.tag))

        test.storage = copy.copy(self.storage)

        return test

    def __fail(self, reason):
        """Set test case result to fail with given reason.

        :type reason: str
        :param reason: the reason of test case failure
        """
        self.verdict = Test.VERDICT_FAIL
        self.reason = reason
        self.steps = []

        self.log.error('FAIL TRIGGERED - REASON: %s' % reason)

        self.__next_stage()

    def set_fail(self, reason):
        """User function to set fail verdict with given reason.

        :type reason: str
        :param reason: the reason of test case failure

        :Example:

        >>> test.set_fail('The reason of test case failure')
        """
        if self.mainloop:
            self.mainloop.add(self.__fail, reason)

    def __set_stage(self, stage):
        """ Set given test case stage.

        :type stage: :class:`Stage`
        :param stage: stage which will be set
        """
        self.stage = stage

        if self.stage == Test.STAGE_SETUP:
            self.steps = list(self.setup)
        elif self.stage == Test.STAGE_TEST:
            self.steps = list(self.test)
        elif self.stage == Test.STAGE_TEARDOWN:
            self.steps = list(self.teardown)

        self.log.debug(TestLogger.tag('PHASE: %s' % self.__get_stage_name()))

    # noinspection PyUnusedLocal
    def __stage_timeout_cb(*args, **kwargs):
        """Function to set test case fail verdict with Timeout reason """
        test = args[0]
        test.set_fail('%s: Timeout (timeout = %d s)' % (test.name, test.timeout))

    def __next_stage(self):
        """Go to the next test case stage if all steps in current test case stage are finished
        properly (timeout does not occur). In case when teardown stage is reached, stage is set to
        Done and test case is finished."""
        if self.stage == Test.STAGE_DONE:
            # exit mainloop as test has been finished
            self.mainloop.quit()
            return

        # We need to make sure that steps are not moved to the next stage.
        assert (len(self.__get_steps()) == 0)

        # Restart the timer for new test stage
        if self.timer is not None:
            self.timer.cancel()
        self.log.debug('SET_TIMEOUT: For %s (%d s)' % (self.name, self.timeout))
        self.timer = Timer(self.timeout, self.__stage_timeout_cb, [self])
        self.timer.start()

        # State change
        if self.stage == Test.STAGE_NONE:
            # set the initial stage
            self.__set_stage(Test.STAGE_SETUP)
            # set the initial verdict
            self.verdict = Test.VERDICT_PASS
        elif self.stage == Test.STAGE_SETUP:
            if self.verdict == Test.VERDICT_FAIL:
                self.__set_stage(Test.STAGE_TEARDOWN)
            else:
                self.__set_stage(Test.STAGE_TEST)
        elif self.stage == Test.STAGE_TEST:
            self.__set_stage(Test.STAGE_TEARDOWN)
        elif self.stage == Test.STAGE_TEARDOWN:
            self.__set_stage(Test.STAGE_DONE)
            self.timer.cancel()
            self.timer = None
        else:
            assert False, 'Something is not right: Invalid test stage (%d) detected.' % self.stage

        if len(self.steps) == 0:
            self.__next_stage()

        self.mainloop.add(self.__next_step, None)

    def __get_stage_name(self):
        """Get current test case stage."""
        return str(self.stage)

    def __get_steps(self):
        """Get current test case steps."""
        return self.steps

    def __remove_step(self, step):
        """Remove given step.

        :type step: :class:`Step`
        :param step: current test case step
        """
        self.steps.remove(step)

        if len(self.steps) == 0:
            self.__next_stage()
        else:
            self.mainloop.add(self.__next_step, None)

    # noinspection PyUnusedLocal
    def __next_step(self, data):
        """Function used to go through test case steps for actions such as WRITE and EXECUTE with
        logging that actions. Data argument is not used.

        :type data: not used
        :param data: not used
        """
        steps = self.__get_steps()
        if len(steps) == 0:
            self.__next_stage()
            return

        step = steps[0]

        if step.action != Action.WAIT_FOR:
            if step.action == Action.WRITE:
                step.source.write(step.data, step.terminator)
            elif step.action == Action.EXECUTE and step.callback is not None:
                self.log.debug('[%s] EXECUTE: ' % step.source + step.callback.__name__)
                step.callback(self, step.source, step.data)

            self.__remove_step(step)

    def __process_parser_results(self, result, *args):
        event_source, step = args

        if step.callback is not None:
            # Conditional remove
            if step.callback(self, event_source, result):
                    self.__remove_step(step)
        else:
        # Always remove
            self.__remove_step(step)

    def __process_event(self, event):
        """Function used to go through nearest steps with WAIT_FOR action to catch them in any
        order on given source and with expected data.

        :type event: :class:`Event`
        :param event: source and data received from UART
        """
        steps = list(self.__get_steps())
        # Iterate over all nearest WAIT_FOR actions to catch them in any order
        while len(steps) != 0 and steps[0].action == Action.WAIT_FOR:
            # Remove action only if it matches the event
            if steps[0].source == event.source:
                if isinstance(steps[0].data, str):
                    # Process string argument
                    if steps[0].data in event.data:
                        if steps[0].callback:
                            # Conditional remove
                            if steps[0].callback(self, event.source, event.data):
                                self.__remove_step(steps[0])
                                break
                        else:
                            # Always remove
                            self.__remove_step(steps[0])
                            break
                elif isinstance(steps[0].data, Parser.MsgDescriptor):
                    Parser.try_message_parse(event.data, steps[0].data, steps[0].storage,
                                             self.__process_parser_results, event.source, steps[0])
                    break
                else:
                    # Process list of strings argument
                    if steps[0].data[0] in event.data:
                        if steps[0].callback:
                            # Conditional remove
                            if steps[0].callback(self, event.source, event.data):
                                steps[0].data = steps[0].data[1:]
                        else:
                            # Always remove
                            steps[0].data = steps[0].data[1:]

                        self.__remove_step(steps[0])
                        # Do not iterate over next WAIT_FOR steps if this one's still active
                        break

            steps = steps[1:]

        # Iterate over monitors dictionary to run callback methods
        for (source, callback) in self.monitors.values():
            if event.source == source:
                    callback(self, source, event.data)

    def __output_data_handler(self, source, data):
        """Function for handling data received via UART for a given source. It creates
        events from the source and data and pushes them into th main loop's queue.

        :type source: :class:`Device'
        :type data: str
        :param source: device on which action is performed
        :param data: data needed to perform action
        """
        self.log.debug('[%s] EVENT RECEIVED: ' % source + data.rstrip())
        self.mainloop.add(self.__process_event, Event(source, data))

    def __input_data_handler(self, source, data):
        """Function for handling data written via UART in to a given source. It logs each write
        action performed on source.

        :type source: :class:`Device'
        :type data: str
        :param source: device on which action is performed
        :param data: data needed to perform action
        """
        self.log.debug('[%s] EVENT SENT: ' % source + data.rstrip())

    def __output_handlers_start(self, devices):
        """Function which starts reading from the list[:class:`Device`] objects.

        :type devices: list[:class:`Device`]
        :param devices: list of devices from which the output handler start reads
        """
        self.output_handlers = {}

        for device in devices:
            # Open device
            try:
                device.open(self.__input_data_handler)
                device.io.read(device.io.inWaiting())
            except Exception:
                raise Exception("Could not open device: {}".format(device))
            # Add output handler for device
            self.output_handlers[device] = OutputHandler(device, self.__output_data_handler)
            self.output_handlers[device].start()

    def __output_handlers_stop(self):
        """Function which stops output handlers."""
        for device, output_handler in self.output_handlers.items():
            output_handler.stopit()
            output_handler.join()
            device.close()

        self.output_handlers = {}

        # Clear process output handlers if any left
        for process in self.process_output_handlers.keys():
            self.stop_process_handler(process)
        self.process_output_handlers = {}

    def __cleanup(self):
        if self.timer:
            if self.timer.isAlive():
                self.timer.cancel()
            self.timer = None
        self.monitors = None
        self.__output_handlers_stop()
        self.mainloop = None
        self.log = None

    def start_process_handler(self, process, cmd, monitored=True):
        process.is_monitored = monitored
        process.open(cmd)

        if monitored:
            output_handler = ProcessOutputHandler(process, self.__output_data_handler)
            self.process_output_handlers[process] = output_handler
            self.process_output_handlers[process].start()

    def stop_process_handler(self, process, timeout=None):
        if process.is_monitored:
            handler = self.process_output_handlers.pop(process)
            handler.join(timeout)
            return handler.results
        else:
            return process.wait()

    def run(self, devices, log):
        """Function to run all test case steps with logging.

        :type devices: list[:class:`.Device`]
        :type log: :class:`.TestLogger`
        :param devices: list of devices
        :param log: logger object
        """
        self.log = log
        storage_orig = copy.deepcopy(self.storage)

        if self.start_condition is not None:
            status, reason = self.start_condition(self)
            if not status:
                self.reason = reason
                self.log.info('TEST {0} WAS SKIPPED - REASON: {1}'.format(self.name, self.reason))
                self.log = None
                return

        self.run_count += 1
        self.mainloop = Mainloop()

        try:
            self.__output_handlers_start(devices)
        except Exception as e:
            self.verdict = Test.VERDICT_FAIL
            self.reason = e
            self.log.info('TEST {0} FAILED - REASON: {1}'.format(self.name, self.reason))
            self.__cleanup()
            return

        self.process_output_handlers = {}
        self.monitors = dict()

        try:
            self.log.info(TestLogger.section('%s STARTED' % self.name))
            self.mainloop.add(self.__next_step, None)
            self.time = time.time()
            self.mainloop.run()
        except Exception as e:
            self.log.error("Exception raised: %s", str(e))
            self.__fail("Exception raised during test phase.")
        finally:
            self.time = time.time() - self.time
            self.log.info(TestLogger.section('%s FINISHED: %s' % (self.name, self.verdict)))
            self.storage = storage_orig
            self.__cleanup()

    def stop(self):
        """ Stop test case which is currently running. """
        if self.mainloop is not None:
            self.mainloop.quit()

    def assert_true(self, expr, msg=None):
        """Assert given expression and return given message with assert result.

        :type expr: bool
        :type msg: str
        :param expr: assert expression
        :param msg: message to be returned with compare result

        :Example:
        >>> test.assert_true(10 == 10,'First compare')
        DEBUG    First compare success
        >>> test.assert_true(9 == 10,'Second compare')
        FAIL TRIGGERED - REASON: Second compare failure
        """
        passed = False
        if expr:
            passed = True

        reason = 'Unspecified'
        if msg:
            reason = '%s %s' % (msg, 'success' if passed else 'failure')

        if not passed:
            self.set_fail(reason)
        elif reason != 'Unspecified':
            self.log.debug(reason)

        return passed

    def assert_equal(self, first, second, msg=None):
        """Compare given arguments and return assert result with given message.
        If the result is negative, fail is triggered.

        :type first: str
        :type second: str
        :type msg: str
        :param first: string to be compare
        :param second: string to be compare
        :param msg: message to be returned with compare result

        :Example:

        >>> test.assert_equal('Example string','Example string','Some string compare')
        DEBUG    Some string compare success
        >>> test.assert_equal('Example','String','Another string compare')
        FAIL TRIGGERED - REASON: Another string compare failure
        """
        return self.assert_true(first == second, msg)

    def add_user_event(self, source, data):
        """Add new event to mainloop.

        :type source: Object
        :type data: str
        :param source: source on which action is performed
        :param data: data needed to perform action
        """

        self.__output_data_handler(source, data)

    @staticmethod
    def __copy__():
        raise Exception("Test has been copied")

    @staticmethod
    def __deepcopy__(memo):
        raise Exception("Test has been copied", memo)

    @staticmethod
    def add_monitor(test, source, named_monitor):
        (mon_name, callback) = named_monitor
        """Enables monitor on a given source, under the given name, which acts as a handle.
        The same handle is used to stop monitoring this event source.

        User can enable multiple monitoring callbacks on a single event source using
        different monitor names.
        """
        if test.monitors is not None:
            test.monitors[mon_name] = (source, callback)

    @staticmethod
    def remove_monitor(test, source, mon_name):
        """Removes the monitor by the given name.
        """
        if test.monitors is not None:
            test.monitors.pop(mon_name, None)


class TestGroup(object):
    """Test Group class contains groups of test cases"""
    def __init__(self, name, devices, tests, tag=None, prepare=None):
        """ Initialises test group with given name, needed devices and test cases.

        :type name: str
        :type devices: list[:class:`.Device`]
        :type tests: list[:class:`Test`]
        :type tag: list[str]
        :type prepare: callback
        :param name: test cases group name
        :param devices: list of devices needed to run test group
        :param tests: set of test cases belong to test group
        :param tag: tags of test group
        :param prepare: setup function for TestGroup

        :Example:

        >>> TestGroup(
        >>>     name='Interactions with one device (AUTOTEST_DEV1)',
        >>>     devices=[AUTOTEST_DEV1],
        >>>     tag=['positive', 'negative', 'user_tag'],
        >>>     tests=[
        >>>        test_01,
        >>>        test_02,
        >>>        test_03
        >>>     ]
        """

        self.name = name
        self.devices = devices
        self.tag = [self.name] + (tag if tag else [])
        self.tests = tests
        self.prepare = prepare

    def clone(self):
        """Clone test cases and return TestGroup."""
        tests = [test.clone() for test in self.tests]

        return TestGroup(self.name, self.devices, tests, tag=copy.deepcopy(self.tag), prepare=self.prepare)
