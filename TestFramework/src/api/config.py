#! /usr/bin/env python3
import os
from xml.etree import ElementTree

DEFAULT = dict(BDADDR='000000000000',
               CHIP_REV='REVISION',
               PROJECT_NAME='spresense',
               GDB_PORT=2331)

CHIP_REV_LIST = {'00': '00000000'}

ENTRY_DEVICE = dict(DEVICE='device',
                    ADDRESS='address',
                    SERIAL='serial',
                    JLINK='jlink',
                    CHIP_REV='chip_rev',
                    SDK_PATH='sdk',
                    PROJECT='project',
                    GDB_PORT='gdb_port',
                    EXT_TYPE='ext_type')

ENTRY_PROJECTS = dict(PROJECTS='projects',
                      PATH='path',
                      TOOLS='tools',
                      BINARIES='binaries',
                      SDK_TOOLS='sdk_tools')


class ConfigException(Exception):
    __module__ = 'Config'

    def __init__(self, message, invalid_entry=None):
        super(ConfigException, self).__init__(message)

        self.invalid_entry = invalid_entry


class ConfigDevice(object):
    """ ConfigDevice class represents device node from a config file. Each of sub-element can be
        read or set using proper properties."""

    def __init__(self, element):
        """ Initialise ConfigDevice.

        :param element: single device node from config file
        :type element: element object
        """
        self.element = element

    @property
    def address(self):
        return self.element.find(ENTRY_DEVICE.get('ADDRESS')).text

    @address.setter
    def address(self, new_addr):
        self.element.find(ENTRY_DEVICE.get('ADDRESS')).text = new_addr

    @property
    def serial(self):
        return self.element.find(ENTRY_DEVICE.get('SERIAL')).text

    @serial.setter
    def serial(self, new_serial):
        self.element.find(ENTRY_DEVICE.get('SERIAL')).text = new_serial

    @property
    def jlink(self):
        return self.element.find(ENTRY_DEVICE.get('JLINK')).text

    @jlink.setter
    def jlink(self, new_jlink):
        self.element.find(ENTRY_DEVICE.get('JLINK')).text = new_jlink

    @property
    def chip_rev(self):
        return self.element.find(ENTRY_DEVICE.get('CHIP_REV')).text

    @chip_rev.setter
    def chip_rev(self, new_chip_rev):
        self.element.find(ENTRY_DEVICE.get('CHIP_REV')).text = new_chip_rev

    @property
    def project(self):
        return self.element.find(ENTRY_DEVICE.get('PROJECT')).text

    @project.setter
    def project(self, new_project):
        self.element.find(ENTRY_DEVICE.get('PROJECT')).text = new_project

    @property
    def gdb_port(self):
        return self.element.find(ENTRY_DEVICE.get('GDB_PORT')).text

    @gdb_port.setter
    def gdb_port(self, new_gdb_port):
        self.element.find(ENTRY_DEVICE.get('GDB_PORT')).text = new_gdb_port
    
    @property
    def ext_type(self):
        if None == self.element.find(ENTRY_DEVICE.get('EXT_TYPE')):
            return 'ext'
        else:
            return  self.element.find(ENTRY_DEVICE.get('EXT_TYPE')).text

    @ext_type.setter
    def ext_type(self, new_ext_type):
        if None == self.element.find(ENTRY_DEVICE.get('EXT_TYPE')):
            return
        else:
            self.element.find(ENTRY_DEVICE.get('EXT_TYPE')).text = new_ext_type


class ConfigProject(object):
    """ ConfigProject class represents project node from a config file. Each of sub-element can be
        read or set using proper properties."""

    def __init__(self, element):
        """ Initialise ConfigProject.

        :param element: single project node from config file
        :type element: element object
        """
        self.element = element

    @property
    def name(self):
        return self.element.tag

    @name.setter
    def name(self, new_name):
        self.element.tag = new_name

    @property
    def path(self):
        return self.element.find(ENTRY_PROJECTS.get('PATH')).text

    @path.setter
    def path(self, new_path):
        self.element.find(ENTRY_PROJECTS.get('PATH')).text = new_path

    @property
    def __tools(self):
        return self.element.find(ENTRY_PROJECTS.get('TOOLS'))

    @property
    def tools_binaries(self):
        return self.__tools.find(ENTRY_PROJECTS.get('BINARIES')).text

    @tools_binaries.setter
    def tools_binaries(self, new_binaries):
        self.__tools.find(ENTRY_PROJECTS.get('BINARIES')).text = new_binaries

    @property
    def tools_sdk_tools(self):
        return self.__tools.find(ENTRY_PROJECTS.get('SDK_TOOLS')).text

    @tools_sdk_tools.setter
    def tools_sdk_tools(self, new_sdk_tools):
        self.__tools.find(ENTRY_PROJECTS.get('SDK_TOOLS')).text = new_sdk_tools


class ConfigBuilder(object):
    """ ConfigBuilder class represents builder node from a config file. Each of sub-element can be
        read or set using proper properties."""

    def __init__(self, element):
        """ Initialise ConfigBuilder.

        :param element: builder node from config file
        :type element: element object
        """
        self.element = element


class Config(object):
    """Autotest configuration object."""

    DEFAULT_CONFIG_FILE = 'config.xml'
    DEFAULT_CONFIG_TEMPLATE_FILE = 'example_config.xml'

    def __init__(self, config_file):
        """Creates the configuration object by parsing XML file.

        :param config_file: file's path
        :type config_file: basestring
        """
        self.tree = ElementTree.ElementTree()
        self.tree.parse(os.path.normpath(config_file))
        self.root = self.tree.getroot()

        self.verify_config()

    @property
    def devices(self):
        """ Devices property.

        :rtype: list[:class:`.ConfigDevice`]
        """
        devices = []
        for device in self.__findall(ENTRY_DEVICE.get('DEVICE')):
            devices.append(ConfigDevice(device))

        return devices

    def add_device(self, address, serial, jlink, chip_rev, project, gdb_port):
        """ Add device to the ElementTree with the given elements.

        :type address: str
        :type serial: str
        :type jlink: str
        :type chip_rev: str
        :type project: str
        :type gdb_port: str
        :param address: BLE address
        :param serial: device serial
        :param jlink: jlink's number
        :param chip_rev: device's chip revision
        :param project: device's project name
        :param gdb_port: gdb port number
        """
        elements = {ENTRY_DEVICE.get('ADDRESS'): address,
                    ENTRY_DEVICE.get('SERIAL'): serial,
                    ENTRY_DEVICE.get('JLINK'): jlink,
                    ENTRY_DEVICE.get('CHIP_REV'): chip_rev,
                    ENTRY_DEVICE.get('PROJECT'): project,
                    ENTRY_DEVICE.get('GDB_PORT'): gdb_port,
                    ENTRY_DEVICE.get('EXT_PYPE'): ext_type}

        for dev in self.devices:
            if all([dev.address == elements.get(ENTRY_DEVICE.get('ADDRESS')),
                    dev.serial == elements.get(ENTRY_DEVICE.get('SERIAL')),
                    dev.jlink == elements.get(ENTRY_DEVICE.get('JLINK')),
                    dev.chip_rev == elements.get(ENTRY_DEVICE.get('CHIP_REV')),
                    dev.project == elements.get(ENTRY_DEVICE.get('PROJECT'))]):
                raise ConfigException('Device already exists')

        self.__insert_element(ENTRY_DEVICE.get('DEVICE'), elements)

    def remove_device(self, device):
        """ Remove device from the ElementTree

        :type device: :class:`DeviceConfig`
        :param device: DeviceConfig object to be removed
        """
        self.__remove_element(device.element)

    @property
    def projects(self):
        """ Projects property.

        :rtype: list[:class:`.ConfigProject`]
        """
        projects = []
        for project in self.__find(ENTRY_PROJECTS.get('PROJECTS')):
            projects.append(ConfigProject(project))
        return projects

    def add_project(self, name, path, binaries, sdk_tools):
        """ Add new project to the ElementTree with given elements.

        :type name: str
        :type path: str
        :type binaries: str
        :type sdk_tools: str
        :param name: project name
        :param path: sdk path
        :param binaries: binaries path
        :param sdk_tools: sdk_tools path

        """
        if not any(self.projects):
            self.__insert_element(ENTRY_PROJECTS.get('PROJECTS'))

        if not name:
            raise ConfigException('Cannot add new project without the name!')

        if self.__find(ENTRY_PROJECTS.get('PROJECTS')).find(name) is None:
            tools_elements = {ENTRY_PROJECTS.get('BINARIES'): binaries,
                              ENTRY_PROJECTS.get('SDK_TOOLS'): sdk_tools}

            project = self.__find(ENTRY_PROJECTS.get('PROJECTS'))
            self.__insert_element(name, {ENTRY_PROJECTS.get('PATH'): path}, project)
            self.__insert_element(ENTRY_PROJECTS.get('TOOLS'), tools_elements, project.find(name))
        else:
            raise ConfigException('Name of project: %s already exist!' % name)

    def remove_project(self, project):
        """ Remove project from the ElementTree

        :type project: :class:`ConfigProject`
        :param project: ProjectConfig object to be removed
        """
        projects = self.root.find(ENTRY_PROJECTS.get('PROJECTS'))
        projects.remove(project.element)

    def verify_config(self, verify_paths=False):
        """Check entries, normalize paths, provide default values for the missing parameters.

        :type verify_paths: bool
        :param verify_path: Whether path strings should be verified for existence.
        """

        # Process all elements and strip white spaces
        for el in self.root.iter('*'):
            if hasattr(el, 'text'):
                if el.text is None:
                    el.text = ''
                else:
                    el.text = el.text.strip()

        # Verify each section
        self.__verify_devices_config(self.__findall(ENTRY_DEVICE.get('DEVICE')), verify_paths)
        # TODO: Verify other sections

    def __verify_chip_rev(self, chip_rev):
        if not chip_rev:
            chip_rev =  DEFAULT.get('CHIP_REV')
        elif chip_rev in CHIP_REV_LIST.keys():
            chip_rev = CHIP_REV_LIST[chip_rev]

        if chip_rev not in CHIP_REV_LIST.values():
            raise ConfigException('Unsupported chip revision: %s set in config file!' % chip_rev)

        return chip_rev

    def __verify_devices_config(self, elements, verify_paths):
        """Provide default values for the selected set of entries.

        :type elements: :class:`ElementTree.Element`
        :type verify_paths: bool
        :param elements: Device configuration elements to verify
        :param verify_path: Whether path strings should be verified for existence.
        """
        projects = self.__find(ENTRY_PROJECTS.get('PROJECTS'))
        # Try migrating old config value for the missing entry
        migrated_project = projects.findall('*')[0].tag if projects is not None else\
            DEFAULT.get('PROJECT_NAME')

        try:
            for dev in elements:
                verifiers = {
                    ENTRY_DEVICE.get('ADDRESS'): lambda val: val if val else DEFAULT.get('BDADDR'),
                    ENTRY_DEVICE.get('SERIAL'): lambda val: '' if val is None else val,
                    ENTRY_DEVICE.get('JLINK'): lambda val: '' if val is None else val,
                    ENTRY_DEVICE.get('CHIP_REV'): self.__verify_chip_rev,
                    ENTRY_DEVICE.get('PROJECT'): lambda val: val if val else migrated_project,
                    ENTRY_DEVICE.get('GDB_PORT'): lambda val: val if val else DEFAULT.get('GDB_PORT'),
                 }

                self.__verify_property_values(dev, verifiers)

                project_name = dev.find(ENTRY_DEVICE.get('PROJECT')).text
                # look for this project entry
                self.__verify_projects_config(projects.find(project_name), verify_paths)

        except ConfigException:
            raise

        except Exception as e:
            raise ConfigException('Configuration verification error: %s' % str(e),
                                  invalid_entry=ENTRY_DEVICE.get('DEVICE'))

    def __verify_projects_config(self, element, verify_paths):
        """Verifies the correctness of the projects section of the config.

        :type element: :class:`ElementTree.Element`
        :type verify_paths: bool
        :param element: Element to be verified
        :param verify_path: Whether path strings should be verified for existence.
        """
        entry = None
        # Migrate project path from projects[0].path
        verifiers = {
            ENTRY_PROJECTS.get('PATH'): lambda _: self.projects[0].path,
            ENTRY_PROJECTS.get('TOOLS'): lambda val: '' if val is None else val
        }

        self.__verify_property_values(element, verifiers)

        try:
            entry = ENTRY_PROJECTS.get('PATH')
            self.__normalise_path_properties(element, [entry], verify_paths)

            entry = ENTRY_PROJECTS.get('TOOLS')
            self.__verify_tools_config(element.find(entry),
                                       migration_path=element.find(ENTRY_PROJECTS.get('PATH')).text,
                                       verify_paths=verify_paths)

        except ConfigException:
            raise

        except Exception as e:
            ex = ConfigException('Configuration verification error: %s' % str(e))
            ex.invalid_entry = entry
            raise ex

    def __verify_tools_config(self, element, migration_path, verify_paths):
        """Verify tools paths for correctness.

        :type element: :class:`ElementTree.Element`
        :type migration_path: basestring
        :type verify_paths: bool
        :param element: Element of the configuration to be verified
        :param migration_path: Base path used to migrate the old config structure to the new one
        :param verify_path: Whether path strings should be verified for existence.
        """

        try:
            # Use migration path if old config is missing these entries
            verifiers = {
                ENTRY_PROJECTS.get('BINARIES'): lambda _: os.path.join(migration_path, 'binaries'),
                ENTRY_PROJECTS.get('SDK_TOOLS'):
                    lambda _: os.path.join(migration_path, 'tools'),
            }

            self.__verify_property_values(element, verifiers)
            self.__normalise_path_properties(element, verifiers.keys(), verify_paths)

        except Exception as e:
            ex = ConfigException('Configuration verification error: %s' % str(e))
            ex.invalid_entry = ENTRY_PROJECTS.get('TOOLS')
            raise ex

    @staticmethod
    def __normalise_path_properties(element, props, verify_paths=False):
        """Normalises paths passed by the user or read from the xml

        :type element: :class:`ElementTree.Element`
        :type props: dict[basestring:basestring]
        :type verify_paths: bool
        :param element: element tag
        :param props: Array of attribute tags
        :param verify_path: Whether path strings should be verified for existence.
        """
        for prop in props:
            entry = element.find(prop)
            if entry is not None:
                if entry.text != '':
                    entry.text = os.path.normpath(entry.text)
                    if verify_paths and not os.path.exists(entry.text):
                        raise ConfigException("Invalid path: %s." % entry.text, invalid_entry=prop)

    @staticmethod
    def __verify_property_values(element, props):
        """Fix invalid values and use default ones for the missing properties

        :type element: :class:`ElementTree.Element`
        :type props: dict[basestring:basestring]
        :param element: element tag
        :param props: Dictionary of attribute tags and value verifiers
        """
        for prop, verifier in props.items():
            entry = element.find(prop)
            if entry is None:
                print('WARNING: Deprecated or invalid configuration file. '
                      'Recovering missing entry: <%s>...' % prop)
                print('Please overwrite you configuration file in the GUI configuration tool or '
                      'create it from scratch to remove this warning.')
                entry = ElementTree.SubElement(element, prop)

            if verifier is not None:
                entry.text = verifier(entry.text)

    def __findall(self, element_tag):
        """Find all configuration entries with the given tag

        :type element_tag: basestring
        :param element_tag: Tag of the element
        :return: List of matching config elements
        """
        return self.root.findall(element_tag)

    def __find(self, element_tag):
        """Find the first entry with the given tag.

        :type element_tag: basestring
        :param element_tag: Tag of the element
        :return: Matching element
        """
        return self.root.find(element_tag)

    def __insert_element(self, element_tag, attributes=None, parent=None):
        """Insert the configuration section. It creates subsection if parent is provided.

        :param element_tag: Tag of the element to add
        :param attributes: Dictionary of key:value pairs for the attributes and their values
        :param parent: parent element if subsection is to be created
        :return: Newly created element which can act as parent for other subelements
        """
        if attributes is None:
            attributes = {}

        if parent is not None:
            el = ElementTree.SubElement(parent, element_tag)
        else:
            el = ElementTree.Element(element_tag)
            self.root.append(el)

        for key, val in attributes.iteritems():
            entry = ElementTree.SubElement(el, key)
            entry.text = val

        return el

    def __set_element_values(self, element_tag, attributes=None):
        """Fills the element with the provided content.

        :type attributes: dict[basestring:basestring]
        :type element_tag: basestring
        :param element_tag: Tag of the config section to fill with data
        :param attributes: Data to put inside the config section
        """
        if attributes is None:
            attributes = {}

        el = self.__find(element_tag)
        if el is None:
            el = self.__insert_element(element_tag)

        for key, val in attributes.iteritems():
            entry = el.find(key)
            if entry is None:
                entry = self.__insert_element(key, None, el)
            entry.text = val

    def __remove_element(self, element):
        self.root.remove(element)

    def __remove_all(self, element_tag):
        """Remove all elements with the given tag

        :type element_tag: basestring
        :param element_tag: Tag of the elements to remove from configuration
        """
        for el in self.__findall(element_tag):
            self.root.remove(el)

    @staticmethod
    def __indent( elem, level=0):
        """Recurrent indentation"""

        i = "\n" + level * "    "
        if len(elem):
            if not elem.text or not elem.text.strip():
                elem.text = i + "    "
            if not elem.tail or not elem.tail.strip():
                elem.tail = i
            for elem in elem:
                Config.__indent(elem, level + 1)
            if not elem.tail or not elem.tail.strip():
                elem.tail = i
        else:
            if level and (not elem.tail or not elem.tail.strip()):
                elem.tail = i

    def write(self, filename):
        """Saves the current configuration in a file

        :type filename: basestring
        :param filename: Path to the file to store the configuration in.
        :return: THe encoded string
        """
        self.__indent(self.root)
        return self.tree.write(filename)
