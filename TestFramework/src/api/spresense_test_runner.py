#! /usr/bin/env python3

import os
import json

from api.tester_exception import TesterException
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.dut_test_device import DutTestDevice
from api.peer_test_device import PeerTestDevice
from api.config import Config
from api.runner_parser import RunnerParser
from api.device_manager import DeviceManager
from api.test import TestLogger
from api.nuttshell_device import *

REBOOT_TIMEOUT = 10
JSONFILLE = "testscript.json"
TESTSW = 'testsw'
TESTSW_PATH = '../../../../TestSoftware'
DEVICES = ['dut', 'peer']
BUILD_PATH = 'build_path'
SKETCH_PATH = 'sketch_path'
PROJECT = 'spresense'


class SpresenseTestRunner(TestRunner):
    def __init__(self):
        self.args = RunnerParser().parse_args()
        self.config = self.get_config_object()
        super(SpresenseTestRunner, self).__init__(self.config)

        # Configure logger
        self.logger = TestLogger(log_name=self.__class__.__name__, console_debug=self.args.verbose)
        self.json_data = self.get_test_script_json_data()
        self.device_num = self.get_test_device_count()
        self.dut_device = DutTestDevice(self.get_device_serial_list()[0], 'DUT_DEVICE') \
                          if self.get_device_serial_list()[0] else None
        self.peer_device = PeerTestDevice(self.get_device_serial_list()[1], 'PEER_DEVICE') \
                          if self.get_device_serial_list()[1] else None 

        if self.device_num == 2:
            if not all(self.get_devices_list()):
                raise TesterException('At least two devices are needed!')
        else:
            if not all(self.get_devices_list()):
                raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        
         # Run test groups from main function arguments
        self.run(self.args,self.logger)
        
    @property
    def name(self):
        return self.__class__.__name__

    def get_device_serial_list(self):
        # Create Device Manager
        dev_manager = DeviceManager(self.config)

        # Assign devices according to role
        dut_device, peer_device= dev_manager.get_devices_by_serials(self.args.dut_device,self.args.peer_device)
        return [dut_device, peer_device]

    # Main configuration file
    def get_config_object(self):
        if self.args.config is not None:
            return Config(os.path.abspath(self.args.config))
        else:
            return Config(os.path.join('../../../../TestFramework/', Config.DEFAULT_CONFIG_FILE))

    def get_run_parameter(self):
        return [self.args, self.logger]

    def get_test_script_json_data(self):
        filepath = os.path.join(os.getcwd(),JSONFILLE)
        if os.path.exists(filepath) != True:
            log.info('No testscript.json file ')
            return
        
        return json.load(open(filepath,encoding='utf-8'))
    
    def get_test_device_count(self):
        con = len(self.json_data[TESTSW])
        if con != 0:
          return con
        else:
          log.info('no test device ')

    # noinspection PyMethodMayBeStatic
    def get_devices_list(self):
        device_list = []
        device_list.append(self.dut_device)

        if self.device_num == 2:
            device_list.append(self.peer_device)

        return device_list

    # noinspection PyUnusedLocal
    def dut_reboot_monitor(selp, test, source, data):
        if source.NUTTSHELL in data:
            test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

    # noinspection PyMethodMayBeStatic
    def setup(self, args, log=None):
        if self.json_data != None:
           testsw_data = self.json_data[TESTSW]
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
            log.info('testscript.json file no data')
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
            
            if device_data[BUILD_PATH] != None:
                print('not device_bin')
                log.info('Building {}'.format(device))
                apps_path = []
                defconfig_path = None
                for defconfig in device_data[BUILD_PATH]:
                    if 'examples' == defconfig.split('/')[0]:
                        apps_path.append(defconfig) 
                    elif 'defconfig' == defconfig.split('/')[-1].split('-')[-1]:
                        defconfig_path = os.path.join(os.getcwd(),TESTSW_PATH,defconfig)
                        apps_path.append(defconfig_path.split('/')[-1].split('-')[0]) 

                build_bin = self.__build_binary(toolbox, apps_path, device,defconfig_path)
            elif device_data[SKETCH_PATH] != None:
                 pass
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



