#! /usr/bin/env python3
import os
import re
import stat
import subprocess
from shutil import copyfile, move

from api.base import NonCloneable
from api.tester_exception import TesterException


TEMP_FILE_PREFIX = 'autotest.'


class ToolboxException(TesterException):
    module = 'Toolbox'


class Toolbox(NonCloneable):
    """Toolbox is a set of tools for common operation on tree and source files.

    Toolbox incorporates the concept of a workspace. Workspace is nothing more than a temporary
    directory in which the Toolbox can operate. Toolbox can work inside the already existing
    directory or create one. If Toolbox is the creator of such workspace directory, it owns it.
    It means that when user calls :func:`cleanup`. workspace directory is deleted along with it's
    content. Toolbox cannot cleanup the directory it does not own.

    If Toolbox's function take path string as parameters those paths should be relative to the
    current workspace directory. Toolbox should not have access to to any files outside of it's
    workspace sandbox.

    """

    def __init__(self, config, logger=None):
        """Initialises Toolbox in the given directory.

        :type logger: logging.Logger
        :param logger: logger object for debug messages
        """
        self.builder = None
        self.logger = logger
        self.config = config
        self.projects = Toolbox.__get_projects(config)

    # noinspection PyMethodMayBeStatic
    def __copy__(self, instance):
        """Copy inhibitor.

        :param instance:
        :return: the same instance
        """
        return instance

    # noinspection PyUnusedLocal
    def __deepcopy__(self, memo):
        """Copy inhibitor.

        :param memo:
        :return: the same instance
        """
        return self

    @staticmethod
    def __get_projects(config):
        """Get list of projects from config"""

        # check if local repository path is provided in the config file

        config_projects = config.projects
        if not any(config_projects):
            return None

        for project in config_projects:
            if not os.path.exists(project.path):
                raise ToolboxException('Invalid path for project "' + project.name +
                                       '" (' + project.path + ')')

        return config_projects

    def get_project_path(self, project_name):
        """Get project path by name

        :type: str
        :param project_name: project's name
        :return: project path defined in config
        """
        if self.projects is None:
            raise ToolboxException('No projects defined in configuration')

        for project in self.projects:
            if project.name == project_name:
                return project.path
        else:
            raise ToolboxException('Project "' + project_name + '" not found in configuration')

    def cleanup(self):
        """Cleanup the workspace."""

        if self.builder is not None:
            self.builder.cleanup()

    def init_builder_module(self):
        """Initialises source code building module.
        """
        self.builder = Toolbox.Builder(self, self.logger)

    def backup_file(self, project, sdk_dir, file_name):
        """If file exists in sdk_dir backup of the file is created.

        :type project: str
        :type sdk_dir: str
        :type file_name: str
        :param project: project's name
        :param sdk_dir: relative dir path in sdk
        :param file_name: filename
        """

        if file_name.startswith('.'):
            prefix = TEMP_FILE_PREFIX.strip('.')
        else:
            prefix = TEMP_FILE_PREFIX

        project_path = self.get_project_path(project)
        target_file = os.path.join(project_path, sdk_dir, file_name)
        backup_file = os.path.join(project_path, sdk_dir, prefix + file_name)

        if os.path.isfile(target_file):
            try:
                copyfile(target_file, backup_file)
            except OSError as e:
                self.logger.error(e)
                raise

    def replace_file(self, project, sdk_dir, file):
        """If target file exists in sdk_dir it is replaced with source file, if not source file is
        just copied to sdk_dir.

        :type project: str
        :type sdk_dir: str
        :type file: str
        :param project: project's name
        :param sdk_dir: relative dir path in sdk
        :param file: source file path
        """

        project_path = self.get_project_path(project)
        target_file = os.path.join(project_path, sdk_dir, os.path.basename(file))

        try:
            copyfile(file, target_file)
        except OSError as e:
            self.logger.error(e)
            raise

    def restore_file(self, project, sdk_dir, file_name):
        """Restores file in sdk dir from backup copy.

        :type project: str
        :type sdk_dir: str
        :type file_name: str
        :param sdk_dir: relative dir path in sdk
        :param project: project's name
        :param file_name: file name
        """

        if os.path.basename(file_name).startswith('.'):
            prefix = TEMP_FILE_PREFIX.strip('.')
        else:
            prefix = TEMP_FILE_PREFIX

        project_path = self.get_project_path(project)
        file_copy = os.path.join(project_path, sdk_dir, prefix + file_name)

        if os.path.isfile(file_copy):
            file = os.path.join(project_path, sdk_dir, file_name)
            move(file_copy, file)

    def replace_line(self, project, file_name, line_num, text):
        """Replaces given line inside the file.

        :type project: str
        :type file_name: str
        :type line_num: int
        :type text: str
        :param project: project's name
        :param file_name: file's location (relative to project directory)
        :param line_num: line number to replace in the given file
        :param text: text string which should be put in this line
        """
        file_path = os.path.join(self.get_project_path(project), os.path.normpath(file_name))

        with open(file_path, 'r') as in_file:
            lines = in_file.readlines()
            lines[line_num] = text + '\n'

        with open(file_path, 'w') as out_file:
            out_file.writelines(lines)

    def find_line(self, project, file_name, text_re):
        """Searches file for text matching the given regular expression and return line number.

        :type file_name: str
        :type text_re: str
        :param file_name: file's location (relative to project directory)
        :param text_re: regular expression
        :return: line number on :param text_re: match, else -1
        :rtype: int
        """
        file_path = os.path.join(self.get_project_path(project), os.path.normpath(file_name))

        with open(file_path, 'r') as in_file:
            ln = 0
            for line in in_file:
                found = re.search(text_re, line)
                if found and found.group(0):
                    return ln
                ln += 1

        return -1

    # noinspection PyUnusedLocal
    @staticmethod
    def _remove_readonly(func, path, exc):
        """Changes file's mode to enable deleting.

        :type exc: Exception
        :type path: str
        :type func: callable
        :param func: function to call after file's mode is changed (i.e. rmtree)
        :param path: path to file which triggers the error
        :param exc: exception
        """
        os.chmod(path, stat.S_IWRITE)
        func(path)

    class Builder(object):
        """Builder class is meant to help the user with the project's source code building process.
        """

        def __init__(self, parent, logger=None):
            """Initialises the code building module object.

            :type parent: :class:`.Toolbox`
            :type logger: logging.Logger
            :param logger: logger for debug message printouts
            :param parent: Toolbox object
            """
            self.parent = parent

            self.owned_workspaces = {}
            self.logger = logger

        def execute_make_cmd(self, project_path, command):
            """Execute make command.

            :param project_path: project path
            :param command: make command to execute
            """

            if command == "distclean":
                fail = 'Failed to clean project'
                success = 'Project successfully cleaned'
            elif command == "olddefconfig":
                fail = 'Failed to restore project configuration'
                success = 'Project configuration successfully restored'

            makeline = ["make", command, "-C", project_path]

            pr = subprocess.Popen(makeline, shell=False, stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            (out, error) = pr.communicate()

            if out:
                out = out.decode()
                for line in out.split('\n'):
                    if line not in '\n':
                        self.logger.debug(line)

            if error:
                error = error.decode()
                for line in error.split('\n'):
                    if line not in '\n':
                        self.logger.error(line)

            if error:
                raise ToolboxException(fail)
            elif self.logger:
                self.logger.info(success)

        def configure_project(self, project_path, apps):
            """Configure project.

            :param project_path: project path
            :param apps: list of builtin applications to include in configuration
            """

            cwd = os.getcwd()
            os.chdir(project_path)

            configureline = ["tools/config.py"] + apps

            pr = subprocess.Popen(configureline, shell=False, stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            (out, error) = pr.communicate()

            if out:
                out = out.decode()
                for line in out.split('\n'):
                    if line not in '\n':
                        self.logger.debug(line)

            if error:
                error = error.decode()
                for line in error.split('\n'):
                    if line not in '\n':
                        self.logger.error(line)

            if error:
                raise ToolboxException('Failed to configure project')
            elif self.logger:
                self.logger.info('Project successfully configured')

            os.chdir(cwd)

        def build_project(self, project, apps, device, defconfig=None):
            """Initiate project build.

            :param project: project name
            :param apps: list of applications to include in built
            :param defconfig: custom defconfig file required to configure project
            """

            project_path = os.path.join(self.parent.get_project_path(project), 'sdk')
            spk_file = None
            restore_defconfig = False

            self.parent.backup_file(project, 'sdk', 'nuttx.spk')
            self.parent.backup_file(project, 'sdk', '.config')

            if self.logger:
                applications = ''
                if len(apps) == 1:
                    postfix = 'application'
                else:
                    postfix = 'applications'

                for app in apps:
                    applications = applications + "'" + app + "' "

                self.logger.info("Building sources for {}builtin {}".format(applications, postfix))

            self.execute_make_cmd(project_path, "distclean")

            if defconfig:
                defconfig_dest = os.path.join(self.parent.get_project_path(project), 'sdk',
                                              'configs', os.path.basename(defconfig))

                if os.path.exists(defconfig_dest):
                    restore_defconfig = True
                    self.parent.backup_file(project, 'sdk/configs', os.path.basename(defconfig))

                try:
                    copyfile(defconfig, defconfig_dest)
                except OSError as e:
                    self.logger.error(e)
                    raise

            self.configure_project(project_path, apps)

            makeline = ["make", "-C", project_path]

            pr = subprocess.Popen(makeline, shell=False, stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            (out, error) = pr.communicate()

            if out:
                out = out.decode()
                for line in out.split('\n'):
                    if 'SPK:' in line:
                        spk_file = line.split()[1]
                    if line not in '\n':
                        self.logger.debug(line)

            err = re.search(b'[eE]rror', error)

            if error and not err:
                if b'No targets specified and no makefile found' in error:
                    err = True

            if error and err:
                error = error.decode()
                for line in error.split('\n'):
                    if line not in '\n':
                        self.logger.error(line)

            if err:
                applications = ''
                pref = 'application '
                if len(apps) > 1:
                    pref = 'applications: '
                for app in apps:
                    applications = applications + "'" + app + "' "
                message = 'Build for builtin ' + pref + applications + 'failed'
                raise ToolboxException(message)
            elif self.logger:
                self.logger.info('Build succeeded')

            if spk_file:
                if spk_file:
                    os.rename(os.path.join(project_path, spk_file),
                              os.path.join(project_path, str(device) + '.' + spk_file))
                    spk_file = os.path.join(project_path, str(device) + '.' + spk_file)

            self.execute_make_cmd(project_path, "distclean")

            self.parent.restore_file(project, 'sdk', 'nuttx.spk')
            self.parent.restore_file(project, 'sdk', '.config')

            if defconfig:
                if restore_defconfig is True:
                    self.parent.restore_file(project, 'sdk/configs', os.path.basename(defconfig))

                if restore_defconfig is False:
                    if os.path.exists(defconfig):
                        try:
                            os.remove(os.path.join(project_path, 'configs',
                                                   os.path.basename(defconfig)))
                        except OSError as e:
                            self.logger.error(e)
                            raise

            self.execute_make_cmd(project_path, "olddefconfig")

            return spk_file

        def buildKernelApp_project(self, project, apps, device, defconfig=None, nuttxconfig=None):
            """Initiate project build.

            :param project: project name
            :param apps: list of applications to include in built
            :param defconfig: custom defconfig file required to configure project
            """

            project_path = os.path.join(self.parent.get_project_path(project), 'sdk')
            spk_file = None
            restore_defconfig = False
            restore_nuttxconfig = False

            self.parent.backup_file(project, 'sdk', 'nuttx.spk')
            self.parent.backup_file(project, 'sdk', '.config')

            if self.logger:
                applications = ''
                if len(apps) == 1:
                    postfix = 'application'
                else:
                    postfix = 'applications'

                for app in apps:
                    applications = applications + "'" + app + "' "

                self.logger.info("Building sources for {}builtin {}".format(applications, postfix))

            self.execute_make_cmd(project_path, "distclean")

            if nuttxconfig:
                nuttxconfig_dest = os.path.join(self.parent.get_project_path(project), 'nuttx', '.config')
                if os.path.exists(nuttxconfig_dest):
                    restore_nuttxconfig = True
                    self.parent.backup_file(project, 'nuttx', '.config')
                    try:
                        copyfile(nuttxconfig, nuttxconfig_dest)
                    except OSError as e:
                        self.logger.error(e)
                        raise
                    self.logger.info("Building kernel")
                    makeline = ["make", 'buildkernel', "-C", project_path]
                    pn = subprocess.Popen(makeline, shell=False, stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    (out, error) = pn.communicate()
                    if out:
                        out.decode()

                    build_kernel_failed = False

                    if error:
                        error = error.decode()
                        for line in error.split('\n'):
                            if 'error' in line:
                                build_kernel_failed = True
                                self.logger.error(line)

                    if build_kernel_failed:
                        raise ToolboxException('Failed to build kernel')
                    else:
                        self.logger.info("Build kernel succeeded")

            if defconfig:
                defconfig_dest = os.path.join(self.parent.get_project_path(project), 'sdk',
                                              'configs', os.path.basename(defconfig))

                if os.path.exists(defconfig_dest):
                    restore_defconfig = True
                    self.parent.backup_file(project, 'sdk/configs', os.path.basename(defconfig))

                try:
                    copyfile(defconfig, defconfig_dest)
                except OSError as e:
                    self.logger.error(e)
                    raise

            self.configure_project(project_path, apps)

            makeline = ["make", "-C", project_path]

            pr = subprocess.Popen(makeline, shell=False, stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            (out, error) = pr.communicate()

            if out:
                out = out.decode()
                for line in out.split('\n'):
                    if 'SPK:' in line:
                        spk_file = line.split()[1]
                    if line not in '\n':
                        self.logger.debug(line)

            err = re.search(b'[eE]rror', error)

            if error and not err:
                if b'No targets specified and no makefile found' in error:
                    err = True

            if error and err:
                error = error.decode()
                for line in error.split('\n'):
                    if line not in '\n':
                        self.logger.error(line)

            if err:
                applications = ''
                pref = 'application '
                if len(apps) > 1:
                    pref = 'applications: '
                for app in apps:
                    applications = applications + "'" + app + "' "
                message = 'Build for builtin ' + pref + applications + 'failed'
                raise ToolboxException(message)
            elif self.logger:
                self.logger.info('Build succeeded')

            if spk_file:
                if spk_file:
                    os.rename(os.path.join(project_path, spk_file),
                              os.path.join(project_path, str(device) + '.' + spk_file))
                    spk_file = os.path.join(project_path, str(device) + '.' + spk_file)

            self.execute_make_cmd(project_path, "distclean")

            self.parent.restore_file(project, 'sdk', 'nuttx.spk')
            self.parent.restore_file(project, 'sdk', '.config')

            if nuttxconfig:
                if restore_nuttxconfig is True:
                    self.parent.restore_file(project, 'nuttx', '.config')
                    makeline = ["make", 'buildkernel', 'KERNCONF=release', "-C", project_path]
                    pn = subprocess.Popen(makeline, shell=False, stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                    pn.communicate()
                    self.logger.info("Rebuild kernel succeeded")

            if defconfig:
                if restore_defconfig is True:
                    self.parent.restore_file(project, 'sdk/configs', os.path.basename(defconfig))

                if restore_defconfig is False:
                    if os.path.exists(defconfig):
                        try:
                            os.remove(os.path.join(project_path, 'configs',
                                                   os.path.basename(defconfig)))
                        except OSError as e:
                            self.logger.error(e)
                            raise

            self.execute_make_cmd(project_path, "olddefconfig")

            return spk_file