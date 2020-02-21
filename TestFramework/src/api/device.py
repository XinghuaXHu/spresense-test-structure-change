#! /usr/bin/env python3
import serial

from api.tester_exception import TesterException
from api.base import NonCloneable
from api.updater import flash 


BAUDRATE = 115200
TIMEOUT = 1


class DeviceException(TesterException):
    module = 'Device'


class Device(NonCloneable):
    """Device keeps info about single device like: serial or sdk_path."""
    def __init__(self, address="", serial="", jlink="", chip_revision="", tools=None, gdb_port='', ext_type=""):
        """Initialises the Device module object.

        :type address: str
        :type serial: strprint
        :type jlink: str
        :type chip_revision: str
        :type tools: tools config object
        :type gdb_port: str
        :param address: BLE address
        :param serial: device serial
        :param jlink: jlink's number
        :param chip_revision: device's chip revision
        :param tools: tools configuration object
        :param gdb_port: gdb_port number
        """
        self.ble_address = address
        self.serial = serial
        self.jlink = jlink
        self.chip_revision = chip_revision
        self.tools = tools
        self.io = None
        self.baudrate = BAUDRATE
        self.gdb_port = gdb_port
        self.ext_type = ext_type

    def __str__(self):
        """ Method of printing the device serial as string.

        :rtype: str
        :return: device serial
        """
        return "%s" % self.serial

    def set_baudrate(self, baudrate):
        self.baudrate = baudrate

    def get_ble_address(self):
        """ Get the BLE address.

        :rtype: str
        :return: BLE address
        """
        return self.ble_address

    def get_ble_address_formatted(self):
        """ Get formatted BLE address.

        :rtype: str
        :return: formatted BLE address
        """
        return '{0}{1}:{2}{3}:{4}{5}:{6}{7}:{8}{9}:{10}{11}'.format(*self.ble_address).upper()

    def get_serial(self):
        """ Get the device serial.

        :rtype: str
        :return: device serial
        """
        return self.serial

    def get_jlink(self):
        """ Get the jlink serial.

        :rtype: str
        :return: jlink serial
        """
        return self.jlink

    def get_chip_revision(self):
        """ Get the chip revision

        :rtype: str
        :return: chip revision
        """
        return self.chip_revision

    def flash(self, name, log=None, bin_path=""):
        """Flashes device with given bin image.

        :type name: str
        :type bin_path: str
        :param name: device name
        :param bin_path: path to bin image
        :param log: logger object
        """
        try:
            flash(self.get_serial(), self.baudrate, bin_path)
        except RuntimeError as e:
            log.error(e)
        except IOError as e:
            log.error(e)

    def open(self, timeout=TIMEOUT):
        """ Open serial port with defined self.baudrate and TIMEOUT.

        :type timeout: int
        :param timeout: timeout after open serial port
        """
        self.io = serial.Serial(self.serial, timeout=timeout, baudrate=self.baudrate)
        self.io.setDTR(False)
        self.io.setDTR(True)
        self.io.setDTR(False)

    def close(self):
        """ Method for close serial port. """
        return self.io.close()

    def read(self):
        """ Read line from serial port. """
        return self.io.readline().decode("Cp1252", errors="ignore")

    def write(self, data, terminator=None):
        """ Write data to serial port.

        :type data: str
        :param data: data for sending via serial port
        """
        return self.io.write(data.encode('utf-8') + (b'\r\n' if terminator is None else terminator.encode('utf-8')))

    # noinspection PyMethodMayBeStatic
    def check_device_config(self, name, builtin_apps, log=None):
        lines = []
        apps_to_check = {}
        configured = []

        for app in builtin_apps:
            if app == 'zmodem':
                apps_to_check['rz'] = False
                apps_to_check['sz'] = False
            elif app == 'audio_player_objif':
                apps_to_check['player_objif'] = False
            else:
                apps_to_check[app] = False

        try:
            self.open()

            while True:
                line = self.read()
                if 'nsh>' in line:
                    break
                if 'Mismatched version' in line:
                    raise TesterException(
                        '{} firmware version mismatch (installed: {}, required: {})! '
                        'Update loader.espk'.format(name, line.split('(')[1].split(')')[0],
                                                    line.split('(')[2].split(')')[0]))

            self.write("help")

            while True:
                line = self.read()
                lines.append(line)
                if 'nsh>' in line:
                    break

        finally:
            self.close()

        for app in apps_to_check:
            for line in lines:
                if app in line:
                    apps_to_check[app] = True

        for app in apps_to_check:
            configured.append(apps_to_check[app])

        if all(configured):
            log.info('{} configured properly'.format(name))
        else:
            raise TesterException('{} not configured properly'.format(name))
