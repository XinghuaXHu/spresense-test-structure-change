#! /usr/bin/env python3
from api.base import NonCloneable
from api.device import Device


class DeviceManager(NonCloneable):
    """Device Manager reads config files and creates devices list. """

    def __init__(self, config):
        """Device Manager constructor.

        :type config: :class:`.Config`
        :param config: configuration
        """
        self.device_list = []
        self.__read_config(config)

    def __str__(self):
        """ Method for return whole device list."""
        ret = "Device Manager info:\n"
        ret += "\nDevice list:"
        for device in self.device_list:
            ret += "\n" + str(device)

        return ret

    def __read_config(self, config):
        """ Reads needed entries from the config object.
        """
        for dev in config.devices:
            device = Device(address=dev.address, serial=dev.serial, jlink=dev.jlink,
                            chip_revision=dev.chip_rev, gdb_port=dev.gdb_port, ext_type=dev.ext_type)
            self.device_list.append(device)

    def get_device_list(self):
        """ Return list of devices.

        :rtype: list[:class:`.Device`]
        """
        return self.device_list

    def get_devices_by_serials(self, *serials):
        if any(serials):
            return list(map(lambda serial: self.__get_device_by_serial(serial), serials))
        else:
            device_list = self.device_list[:]
            return list(map(lambda serial: device_list.pop(0) if device_list else None, serials))

    def __get_device_by_serial(self, serial):
        return next((device for device in self.device_list if device.serial == serial), None)
