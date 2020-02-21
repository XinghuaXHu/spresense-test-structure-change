#! /usr/bin/env python3
from api.base import NonCloneable


class TestDevice(NonCloneable):
    """Wrapper for :class:`.Device` object overriding :meth:`__str__` method."""
    def __init__(self, device, name):
        """Creates Device instance wrapper.

        :type name: str
        :type device: :class:`.Device`
        :param device: device returned from DeviceManager
        :param name: new device alias
        """
        self._device = device
        self._name = name
        self._input_handler = None

    def __getattr__(self, attr):
        """Function returns reference to requested attribute. If TestDevice instance contains
        the attribute, it is returned. Otherwise _device attribute is returned.

        :type attr: str
        :param attr: name of attribute

        :return: requested attribute
        """
        if attr in self.__dict__:
            return getattr(self, attr)
        return getattr(self._device, attr)

    def __str__(self):
        """Function returns TestDevice name.

        :rtype: str
        :return: device name
        """
        return self._name

    def open(self, input_handler=None):
        """Function sets input handler and calls `open` method form device.

        :type input_handler: function callback
        :param input_handler: function called each time write method is performed
        """
        self._input_handler = input_handler
        return self._device.open()

    def write(self, data, terminator=None):
        """Function calls input_handler then `write` method from device with data parameter.

        :type data: str
        :param data: data written in to device
        """
        if self._input_handler:
            self._input_handler(self, data)
        return self._device.write(data,terminator)

    def close(self):
        """Function calls `close` method from device and assures that input handler is set to None.
        """
        try:
            return self._device.close()
        finally:
            self._input_handler = None
