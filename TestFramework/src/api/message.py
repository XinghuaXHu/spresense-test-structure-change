#! /usr/bin/env python3
from io import StringIO

from api.helpers import normalize_string
from api.tester_exception import TesterException


class MsgObject(object):
    """Empty container for parsed messages."""
    pass


class Parser(object):
    """ Parsing facility which allows users to define their own message parsers. It provides the
    necessary building blocks to support any kind of message.

    As there are no keywords or rules on how messages are constructed, each message has to be parsed
    accordingly. This means that during the test we expect a specific message and provide the
    specific message parsing function which serialises the message into the :class:`.MsgObject`
    instance. The boundaries of the message buffer which contain the expected data are denoted by
    the message descriptor.

    Parser provides necessary functionality to check the incoming data against the expected message
    descriptor. When the message is considered fully cached the parsing function is called and
    resulting data is returned by caller's callback function.

    Framework starts message caching if 'open_tag' is found and ends either when 'line_count' number
    of lines is cached ('open_tag' is also counted) or 'close_tag' is found. Cached buffer is then
    passed to the parsing function.

    When 'close_tag' and 'line_count' are both provided the cached message buffer will contain not
    only the whole message from the 'start_tag' to the 'end_tag' but also 'lines_count' - 1 lines
    after the 'end_tag' ('end_tag' is counted just like the 'start_tag' was). This is particularly
    helpful when the 'end_tag' denotes a few lines long ending sequence rather than just being the
    ending tag.
    """

    # Generic sections
    TAG_BLE_EVT_STATUS = 'Status: '
    TAG_BLE_EVT_ITEM_TYPE = 'Item type: '
    TAG_BLE_EVT_UUID = 'UUID: '

    # Generic message descriptor
    class MsgDescriptor(object):
        """ Message descriptor defines message boundaries and the parsing function. """

        def __init__(self, open_tag, parser, close_tag=None, line_count=0):
            """ Defines the message to be parsed.

            :param open_tag: String value which triggers message caching
            :param parser: Parsing function which is called once the full message is cached
            :param close_tag: Closing tag denoting the end of message caching
            :param line_count: Number of additional lines which are cached after open_tag (when
            'close_tag' is not provided) or after 'close_tag' if it's provided.

            :Simple Example:
            >>> def my_parsing_function(message_buffer):
            >>>     msg = MsgObject()
            >>>     setattr(msg, 'my_message_value', int(message_buffer.readlines()[1]))
            >>>     return msg
            >>>
            >>> my_msg_descriptor = Parser.MsgDescriptor('MY_MSG_START', my_parsing_function(),
            >>>                                          'MY_MESAGE_END')
            >>>

            The 'message_buffer' passed to the parsing function from the above example contains the
            open_tag, close_tag and everything in-between. This buffer can be processed by the
            parsing function to create proper MsgObject() representing the parsed message.
            """

            self.open_tag = open_tag
            self.parser = parser
            self.close_tag = close_tag
            self.line_count = line_count

            if not open_tag:
                raise TesterException("Invalid open_tag parameter for the message parsing module!")

            if line_count == 0 and close_tag is None:
                raise TesterException("Invalid parameters for the message parsing module!")

    # Generic data parsing functions
    @staticmethod
    def parse_integer(data):
        return int(data, 0)

    @staticmethod
    def parse_uuid(data):
        return data

    @staticmethod
    def parse_data_buffer(data):
        return data

    @staticmethod
    def parse_char_props(data):
        return data

    @staticmethod
    def parse_status(data):
        return data

    @staticmethod
    def parse_line(line_buff, line_descriptor):
        """ Performs single line parsing action on the 'line_buf' parameter. It slices the message
        to discard the 'tag' and passes the rest of the 'line_buf' to the parsing function (both
        the tag and the function are defined in the 'line_descriptor'.

        This should be used for parsing single values like this:
        >>> message_line = 'Status: 0'

        Where descriptor for such line looks something to this:
        >>> status_desc = [Parser.TAG_BLE_EVT_STATUS, Parser.parse_uuid]

        This parser provides a set of common parsing functions, like Parser.parse_uuid().

        :param line_buff: line of text to be parsed
        :param line_descriptor: Line descriptor defining the attribute and value portions of the
        text and providing low level parsing function.
        :return: parsed value
        """

        # search for label
        idx = line_buff.find(line_descriptor[0])
        if idx < 0:
            raise Exception("Parsing error.")

        # parse if parser is available
        if line_descriptor[1]:
            return line_descriptor[1](' '.join(line_buff[idx:].split(': ')[1:]).strip())

        return None

    @staticmethod
    def parse_section(section_descriptor, text_buffer, obj):
        """ Parses the whole section of the message sequence, and sets the attributes of the 'obj'
        to the values returned by the parsers. Attribute names are generated basing on the tag value
        of each descriptor.

        >>> DESC_BLE_EVT_GATTC_ITEM_TYPE_INCLUDE = [
        >>>                     ('Item type: ', Parser.parse_char_props),
        >>>                     ('Handle: ', Parser.parse_integer),
        >>>                     ('UUID: ', Parser.parse_uuid),
        >>>                     ('Start handle: ', Parser.parse_integer),
        >>>                     ('End handle: ', Parser.parse_integer)]

        The successfully parsed object should contain the following relevant attributes:
        >>> obj.item_type
        >>> obj.handle
        >>> obj.uuid
        >>> obj.start_handle
        >>> obj.end_handle

        Note that attribute names are nothing more then a normalised tag strings.

        :param section_descriptor: An array of descriptors to be parsed
        :param text_buffer: Buffer to be parsed.
        :param obj: Object which is augmented with attributes generated from the parsed message.
        """

        for desc in section_descriptor:
            line = text_buffer.readline()
            setattr(obj, normalize_string(desc[0]), Parser.parse_line(line, desc))

        Parser.__discard_empty_lines(text_buffer)

    @staticmethod
    def peek(text, buf):
        """ Checks if nex line in the buffer contains the provided text. """
        pos = buf.tell()
        line = buf.readline()
        buf.seek(pos)

        return text in line

    @staticmethod
    def __discard_empty_lines(text_buffer):
        pos = text_buffer.tell()
        line = text_buffer.readline()

        while not line.strip() and pos is not text_buffer.len:
            pos = text_buffer.tell()
            line = text_buffer.readline()

        text_buffer.seek(pos)

    @staticmethod
    def try_message_parse(data, msg_descr, storage, callback=None, *callback_args):
        """ Main message buffering and parsing function. It is called by the testing framework
        when it expects the message to be parsed (WAIT_FOR step of the test).

        This function is called for each line when the parser is about to be used. It checks
        if the message contains opening or closing tags and performs message buffering in
        boundaries described by tags in 'msg_descr' parameter. When it's done caching the message it
        triggers the parsing routine (which in most cases parses individual sections using
        predefined low level parsing functions). The result of parsing is passed to the callback
        function. The callback function takes variable number of arguments and the caller of this
        function can decide on what additional information it needs in this callback (autotest
        framework uses this mechanism to pass the actual test step and event source, which allows
        him to call the user's callback function in which it is decide dto pass the current test
        step or fail).

        :param data: text buffer
        :param msg_descr: message descriptor which 'data' is checked against
        :param storage: place where the message caching is done
        :param callback: callback used for returning the parsing results
        :param callback_args: additional user parameters passed to the callback function - depends
        on the parser usage.
        """

        buffer_attr = 'msg_buff'
        line_count = 'line_count'
        decrement_flag = 'do_decrement'

        if not hasattr(storage, buffer_attr) or getattr(storage, buffer_attr) is None:
            if msg_descr.open_tag == str(data).strip():
                setattr(storage, buffer_attr, StringIO())
                setattr(storage, line_count, msg_descr.line_count)

        if hasattr(storage, buffer_attr) and getattr(storage, buffer_attr) is not None:
            getattr(storage, buffer_attr).write(data)

            if not hasattr(storage, decrement_flag):
                setattr(storage, decrement_flag, False)

            if msg_descr.close_tag is None or msg_descr.close_tag in str(data).strip():
                setattr(storage, decrement_flag, True)

            if getattr(storage, decrement_flag):
                setattr(storage, line_count, getattr(storage, line_count) - 1)

                # Check if done collecting all data
                if getattr(storage, line_count) <= 0:
                    getattr(storage, buffer_attr).seek(0, 0)
                    result = msg_descr.parser(getattr(storage, buffer_attr))

                    if callback is not None:
                        callback(result, *callback_args)

                    setattr(storage, buffer_attr, None)
                    setattr(storage, decrement_flag, False)
                    setattr(storage, line_count, 0)

