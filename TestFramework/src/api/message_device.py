#! /usr/bin/env python3

from api.test_device import TestDevice


class ParsingException(Exception):
    pass


class MessageTemplate(object):
    """ This class is used to describe application messages. """
    def __init__(self, opcode, *parsing_steps):
        """
        :param opcode: opcode of MessageObject returned from parsing.
        :param parsing_steps: Sets: (expected_key(string), user_key(string), parser(method)):
            expected_key: key that validates line;
            user_key: MessageObject key where value is stored;
            parser: method for parsing value.
        Example: ('Connection index', 'Index', MessageTemplate.parse_string)
        """
        self.opcode = opcode
        self.steps = parsing_steps

    @staticmethod
    def _parse_line(line, expected_key, user_key):
        """
        Function used by parsers to parse key and value from read line.

        :param line: String with line from read.
        :param expected_key: key that validates line
        :param user_key: MessageObject key where value is stored
        :return: Set (key(string), value(string))
            key: if user_key is None: parsed key, otherwise user_key.
            value: parsed value.
        """
        try:
            parsed_key, parsed_value = map((lambda c: c.strip()), line.split(':', 1))
        except:
            raise ParsingException("Data separator not found.")

        if parsed_key != expected_key:
            raise ParsingException("Parsed line doesn't match expected line.")

        return user_key if user_key else parsed_key, parsed_value

    @staticmethod
    def validate_line(lines, expected_key, user_key, msg):
        """
        Function checks if expected string is in line.

        :param lines: Cached lines iterator
        :param expected_key: string expected in line
        :param user_key: MessageObject key where value is stored (should be None)
        :param msg: MessageObject where parsed values are stored
        """
        line = next(lines)
        if expected_key not in line:
            raise ParsingException("Parsed line doesn't match expected line.")

    @staticmethod
    def parse_string(lines, expected_key, user_key, msg):
        """
        Function adds parsed value as string to MessageObject.

        :param lines: Cached lines iterator
        :param expected_key: key that validates line
        :param user_key: MessageObject key where value is stored
        :param msg: MessageObject where parsed values are stored
        """
        line = next(lines)
        key, value = MessageTemplate._parse_line(line, expected_key, user_key)
        msg.set_value(key, str(value))

    def _parse(self, lines):
        """
        This function is calling parsers defined in steps.

        :param lines: Cached lines iterator
        :return: Parsed MessageObject
        """
        msg = MessageObject()
        for expected_key, user_key, parser in self.steps:
            parser(lines, expected_key, user_key, msg)
        msg.set_opcode(self.opcode)
        return msg


class MessageObject(object):
    """
    This class is data container for parsed messages.
    MessageObject object is returned by MessageDevice if parsing ended correctly.
    Accessing to MessageObject data should be performed with get_value method which returns None
    if value is not present.
    """
    def __init__(self, opcode=None):
        self.__opcode__ = opcode
        self.__data__ = {}

    def __str__(self):
        return "[%s] %s" % (self.__opcode__, self.__data__)

    def __contains__(self, item):
        return item in self.__opcode__

    def __eq__(self, other):
        if isinstance(other, MessageObject):
            if other.__opcode__ == self.__opcode__:
                return ((other.__data__ > self.__data__) - (other.__data__ < self.__data__)) == 0
        else:
            raise TypeError("Can not compare '%s' with '%s'" % (type(other), type(self)))
        return False

    def rstrip(self):
        return self.__str__()

    def set_value(self, key, value):
        self.__data__[key] = value

    def get_value(self, key):
        try:
            return self.__data__[key]
        except KeyError:
            return None

    def set_opcode(self, opcode):
        self.__opcode__ = opcode


class MessageDevice(TestDevice):
    """
    This class extends read method to parse related lines (messages) defined in msg_templates.
    """

    msg_templates = []

    def __init__(self, device, name):
        TestDevice.__init__(self, device, name)
        self.lines = []

    # noinspection PyProtectedMember
    def read(self):
        """
        :return: MessageObject or not matching line
        """
        while True:
            # First check for messages in cached lines (if exist)
            if self.lines:
                for template in self.msg_templates:
                    try:
                        lines = iter(self.lines)
                        message = template._parse(lines)
                        self.lines = list(lines)
                        return message
                    except StopIteration:
                        # This exception means that parser has matched message but has not enough
                        # lines to complete parsing.
                        break
                    except ParsingException:
                        # This exception means that received lines does not match template.
                        continue
                else:
                    # If this procedure ends without any match, device can return oldest line.
                    return self.lines.pop(0)

            line = super(MessageDevice, self).__getattr__('_device').read()

            # Check for timeout
            if not line:
                return line

            # Skip lines that have no content
            if line == '\r\n':
                continue

            self.lines.append(line)
