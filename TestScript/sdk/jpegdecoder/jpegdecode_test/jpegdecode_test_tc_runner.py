#! /usr/bin/env python3
import sys
import os
import time
import datetime
import re

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.jpegdecode_test.jpegdecode_test_device import JpegDecodeTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['jpeg_decode']
JPG_FILES = ['qvga', 'vga', 'orig1', 'quad_vga', 'uxga', 'orig2', 'orig3', 'orig4']
IMG_WIDTH = 320
IMG_HEIGHT = 240
YUV_LINE_MD5_VALUES = [
    ['02a1d1a12565cfb236c23571b7edbbad',  # qvga
     'ca546ef436af6075efe54b123370fa52',
     'ffeeb85c0790450e3c6505ef65cee1a9',
     '18cf595fca87ee0b5671823c72a517e8',
     'b6cc434746f949f6033fd5972951ebc4',
     'c47993fdebc7ebb3387247d83abaebfb',
     '2ce3188d58a07faf7f3a7bde14990388',
     'eeed766be7961819f97b66b402f195ce',
     ],
    ['ee04c957055170decb1b1c8438ee0380',  # vga
     '111daf8e084bc55bf35207c95bfc8a4f',
     '53ae2218ec7c83ed9d73dd7146e27696',
     'cda9048fb3722fcbd4a1bb12d78d1065',
     '23f4702e9b5ba8727d046d0ccd1ee6fb',
     '494fc0b9973a3a25935589af8acadcf6',
     '3ca028f61afa78aeff057590a90457c5',
     '274b8cb83b399f58f065911e1bde2109',
     ],
    ['854a38979d6adea3e1027f91a7cc61dd',  # orig1
     '9b38cc1e907c346826ac9cca5be53a53',
     '43ad23f853abef2fd1fc9aaff7ffbe17',
     'e113c1a8ec6f63190bb8d53431652b3c',
     '0d429ecbd4aafc99990768d206434519',
     '92a0a36ae64b9eb31339d640be23eab0',
     '9f8cc3ab34e2067058758ee8c9db4ce9',
     'aa59900e27d6e8c72e86459363ed1809',
     ],
    ['5504e080f50849451296bac899c4fa6b',  # quad_vga
     'a6b6703f37466b71ac33b383dac2b1ed',
     'cf56c3bb8ebed52b8e3f2ac66eb0943d',
     '0a869bbc9da246689b777efa8e816420',
     'e9975fa23f8a3c56ba5b9d4e6f82a717',
     '889dd122bed30f2edf06304484f4181f',
     'a07c768881d7031edc3b21bd2b6f4ed3',
     '4db14ef49d2c0456ec5b2b161095a61b',
     ],
    ['d4134d0c17ce4f6573ae37b8fe0d523a',  # uxga
     '44ff6a1e033e895f5baf4b4ba1e57cab',
     'a021c6069d120c4f5fb73ffd9caf02f3',
     'f0379d4be8a5beec2ea125d7f4ef2933',
     '681f131394d9840c3a2701765ee21e4a',
     '6a6deaf4e6b30bf9b825c3ae2c47c63e',
     'f318f27bc275331f46e4fd3248545e64',
     'ffe8056783d738bb62dada0679d094b5',
     ],
    ['f4d0568a92b30f8a4a63fcfc2668bb97',  # orig2
     'f69e8f07e01c79910eb0ab4d469b6cbe',
     '885938c66f7e00d555b07b9acbb052aa',
     'a767d1d85495e355ad2404c4f0bac24c',
     'f6de49ae77818bb8414319ca95b3c312',
     '52d3787ef414d294dd951dc1e8b1a527',
     '4dc4e8e23f954d70c2636c6869f7f766',
     '2441e05c5b57dfad6806a6edd117e3d4',
     ],
    ['9805aa826f42097cc2d68b4ab66b41fb',  # orig3
     '9786e29104249fd554cada6d9113e791',
     '6fde8ffe085063b4ba5aa1526eba94f0',
     '53183371ba4b6b8365577c9e077f3254',
     '1d55b27e509b3743a55f062bf3ef945f',
     '3fd9643bb478da087fe41797a133c213',
     '20b0c35622fb0dbab0edfa09294bf15c',
     '897f48c20c57d529aa2958bf5f28848d',
     ],
    ['c3dfc6f8a192231ff31fb57a11e9ce84',  # orig4
     'e5f8dcfeaf9b92dd20d75fc20025585e',
     '342497c8e90d97e59e10d3a0d298b60f',
     '18e18b95a9fa0778ad16f2d23712a642',
     '63bcebd5c31a52cd9bb30023ed0cf2a7',
     '4e4d3083f2cf34fecc2fbdd5fa6e213d',
     'b1ca38b4e8533fc77a946df937a9ba7c',
     'c08011ddd1aa168be25c5da0e9918b45',
     ],
]

YUV_MCU_MD5_VALUES = [
     ['02a1d1a12565cfb236c23571b7edbbad',
      '3a3cd9cd80376e93967c53c0980c3fbb',
      'abc3a05cd758adc2c34efb37c135924f',
      '8450ce3227390eb255bdb373205ae48a',
      'a2f2ff7c8403da7d26118ca6660dc1cd',
      '22e77507427ba71207c060ef6b7fc1b4',
      '04ae33d2ecaf66eb340151c9a4df1005',
      '8de0d86119668d2b28bbcfd21e98264f',
      ],
      ['ee04c957055170decb1b1c8438ee0380',
      '05e98ceb589c31cbbb0c6fd1557ea5c4',
      '9574f7bc27c9ba27c380fd9b00dae824',
      '8c1e006957192bd57302d39a07649fe1',
      'fdf0beaabe1a2e79b9683d885937b95e',
      '11d72982c31b200f4e3929d6e0203191',
      '72dd01a4e8bd4d8615419f7adf3cf789',
      '4320806ad949530d8693694027ce56bc',
      ],
      ['854a38979d6adea3e1027f91a7cc61dd',
      '531dc9365832b8416fb25f9e104bf749',
      'd139ad81740c32fa41213d090370d44f',
      '74aaa963cc341db1e450525fc2ed4176',
      '25134fe99742f1ddef13e4256a4df9ab',
      'c54d4d1a52774fb532895dee866a8790',
      '57e87a4087f5f168d3a6d0276749e263',
      '6edcb492bb7c4fa6d9f997e50716602f',
      ],
      ['5504e080f50849451296bac899c4fa6b',
      '677fa77d9ba37d5df377ef8d411f542e',
      '761cd57675ab5cc5c6a6a42041ff0edf',
      'ae1692cb8efb8f3fd8b9bb84201ca403',
      '5c27760e53c6fd9eafd19e09e2ffe014',
      '1cad35ba13a30434188bf719eef010c6',
      '8cb9b9da9f1ccf94edec310c047a5d05',
      '92c9b2ea93660e4d47e47b48f30a5b6a',
      ],
      ['d4134d0c17ce4f6573ae37b8fe0d523a',
      '5bab329a0afc0063fffebb09ee9469cf',
      '1b388f273daaa66d1156dfa897c26117',
      'd3f2d7e54c9f164e5e1e651ca57b9ad5',
      '34d256524701f5e62fb20f3fbaf77c0a',
      '238be41dc2facc6ea09a68dc38969267',
      'a91590cd59b0b15e95d62410c941a052',
      'bb5c760b99ba3f7661f462f579b9ead5',
      ],
      ['f4d0568a92b30f8a4a63fcfc2668bb97',
      'f156c55a279987a03351691b87958e9c',
      '8d91645f81c207e995e4e73ef343822f',
      '7120f88a17c37f18cf2dabd10988241a',
      '7892d759f8a4add9e2dab4cdda5a1a8d',
      '7012499ad25fcad83711d74cdbeed2f0',
      '545d667116350ca9ec84c0e6d636276c',
      '60c36fc46d07e2a59c36316e20ca4b5d',
      ],
      ['9805aa826f42097cc2d68b4ab66b41fb',
      'fba4ad658378c871d94b46ed1ec3dd84',
      'a2d01bf6ceb75af020920c8e5134a191',
      'd9cedc5ae13934bbb57b29ee92a80af2',
      'ebb5bbbd9f75011c2c21490a14478a7f',
      'd2ff94b15d16feea2ded60a1f95dbe62',
      'a82fe643bffaf7fb142794556b7a301d',
      'c0318ced8fe22cf41ca985e303f80502',
      ],
      ['c3dfc6f8a192231ff31fb57a11e9ce84',
      'd7a7401d1b584625a8f88cb4ae7b98ae',
      '640c44c4c656e86b868b87696523a52d',
      'a108d313cdf3356430183b52088b587d',
      '19be5b542c138313a5641b2019c3633b',
      'c7edb49a1d3cd895d0f81f627451fb5e',
      '6919da4600d7b1c9ea11eb2f84d6cf44',
      'de7d18754cd88b07c1173e0a7dd201b7',
      ],
]
# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))
    elif '-rw-rw-rw-' in data:
        split_str = '/'
        array = re.split(split_str, data)
        filename = array[3]
        filename = filename.replace('\n', '')
        filename = filename.replace('\r', '')
        test.log.info(filename)
        #newfilename = '{}-{}-{}'.format(test.name, datetime.datetime.now().strftime("%m%d%H%M%S"), filename)
        #source.write('mv /mnt/sd0/{} /mnt/sd0/{}'.format(filename, newfilename))
        #time.sleep(5)

def wait_sec(test, source, data, sec, description):
    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')

def checkSum_verify(test, source, data, checksum):
    if checksum in data:
        return True
    else:
        test.set_fail(' '.join(['checkSum is not right, data= ', data]))
    return False

class JpegDecodeTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(JpegDecodeTestTcRunner, self).__init__(conf)

        self.jpegdecode_test_device = JpegDecodeTestDevice(
            kwargs.get('dut_device', None), 'JPEGDECODE_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.path.dirname(__file__), 'defconfig/jpeg_decode-defconfig')

    def get_devices_list(self):
        return [self.jpegdecode_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.jpegdecode_test_device, binaries['dut_bin'], log)

    # noinspection PyUnresolvedReferences
    def build(self, debug=True, log=None, **kwargs):
        dut_bin = kwargs.get('dut_bin', None)
        toolbox = None

        if not all([dut_bin]):
            if log:
                log.info('Building project sources')

            toolbox = Toolbox(self.config, log)
            toolbox.init_builder_module()

        if not dut_bin:
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.jpegdecode_test_device)

        return dict(dut_bin=dut_bin)

    @staticmethod
    def __build_binary(toolbox, defconfig, device):
        bin_file = toolbox.builder.build_project(PROJECT, APPS_TO_BUILTIN, device, defconfig)
        return os.path.normpath(bin_file)

    # noinspection PyMethodMayBeStatic
    def flash_n_check(self, device, dut_bin, log=None):
        if log:
            log.info('Flashing ' + str(device))

        device.flash(str(device), log, dut_bin)
        device.check_device_config(str(device), APPS_TO_BUILTIN, log)

        if os.path.basename(dut_bin).startswith(str(device)):
            try:
                os.remove(dut_bin)
            except OSError as e:
                log.error(e)
                raise

    def generate_test_groups(self, arguments, log=None):
        timeout = 240
        def loop_steps(count=1):
            r = []
            for i in range(count):
                    r = r + [
                        Step(Action.WRITE, jpegdecode_test_device, ' jpeg_decode std_error'),
                        Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_str_error()'),
                        Step(Action.EXECUTE, jpegdecode_test_device, None,
                            lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                        Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode create_decompress'),
                        Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_create_decompress()'),
                        Step(Action.EXECUTE, jpegdecode_test_device, None,
                            lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),
                    ]
            return r

        jpegdecode_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, jpegdecode_test_device, jpegdecode_test_device.NSH_PROMPT),
            Step(Action.WRITE, jpegdecode_test_device, 'reboot'),
            Step(Action.WAIT_FOR, jpegdecode_test_device, jpegdecode_test_device.NUTTSHELL),
            Step(Action.EXECUTE, jpegdecode_test_device, ('dut_reboot_mon', dut_reboot_monitor),
                 Test.add_monitor),
        ]

        teardown = [
            Step(Action.EXECUTE, jpegdecode_test_device, 'dut_reboot_mon', Test.remove_monitor),
        ]

        decode_dev_test = Test(
            name='decode_dev_test',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode hello'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, jpegdecode_test_device.HELLOWORLD),
            ],
            teardown=teardown
        )

        decode_dev_test1 = Test(
            name='decode_dev_test',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode hello'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, jpegdecode_test_device.HELLOWORLD),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

            ],
            teardown=teardown
        )

        def decoder_memory_cmd(cmd, i, j):
            r = []
            r = r + [
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 4, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device,
                     'jpeg_decode filename /mnt/sd0/' + JPG_FILES[i] + '.jpg'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'infilename: /mnt/sd0/' + JPG_FILES[i] + '.jpg'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode select_decode_param {}'.format(cmd)),
                Step(Action.WAIT_FOR, jpegdecode_test_device, '{} decode'.format(cmd)),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, ' jpeg_decode std_error'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_str_error()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode create_decompress'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_create_decompress()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode mem_src'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode read_header'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image width  = ' + str(IMG_WIDTH * (i + 1))),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image height = ' + str(IMG_HEIGHT * (i + 1))),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode color_space cbycry'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'set color_space : JCS_CbYCrY'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode start_decompress ' + str(j + 1)),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_start_decompress'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode set_buf_mem'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'setting buf_mem done'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, ' jpeg_decode output ' + str(j + 1) + '_mem_' + cmd),
            ]
            if 'line' == cmd:
                r = r + [
                    #Step(Action.WAIT_FOR, jpegdecode_test_device, YUV_LINE_MD5_VALUES[i][j]),
                    Step(Action.WAIT_FOR, jpegdecode_test_device, 'checkSum',
                         lambda test, source, data: checkSum_verify(test, source, data, YUV_LINE_MD5_VALUES[i][j])),
                ]
            else:
                r = r + [
                    #Step(Action.WAIT_FOR, jpegdecode_test_device, YUV_MCU_MD5_VALUES[i][j]),
                    Step(Action.WAIT_FOR, jpegdecode_test_device, 'checkSum',
                         lambda test, source, data: checkSum_verify(test, source, data, YUV_MCU_MD5_VALUES[i][j])),
                ]
            r = r + [
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'output done'),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode finish_decompress'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_finish_decompress()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode destroy_decompress'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_destroy_decompress()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode fd_close'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'closed fd'),
            ]
            return r

        def decoder_df_cmd(cmd, i, j):
            r = []
            r = r + [
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 4, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device,
                     'jpeg_decode filename /mnt/sd0/' + JPG_FILES[i] + '.jpg'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'infilename: /mnt/sd0/' + JPG_FILES[i] + '.jpg'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode fopen'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode select_decode_param {}'.format(cmd)),
                Step(Action.WAIT_FOR, jpegdecode_test_device, '{} decode'.format(cmd)),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, ' jpeg_decode std_error'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_str_error()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode create_decompress'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_create_decompress()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode fd_src'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_fd_src()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode read_header'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image width  = ' + str(IMG_WIDTH * (i + 1))),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image height = ' + str(IMG_HEIGHT * (i + 1))),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode color_space cbycry'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'set color_space : JCS_CbYCrY'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode start_decompress ' + str(j + 1)),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_start_decompress'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode set_buf_mem'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'setting buf_mem done'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, ' jpeg_decode output ' + str(j + 1) + '_df_' + cmd),
            ]
            if 'line' == cmd:
                r = r + [
                    #Step(Action.WAIT_FOR, jpegdecode_test_device, YUV_LINE_MD5_VALUES[i][j]),
                    Step(Action.WAIT_FOR, jpegdecode_test_device, 'checkSum',
                         lambda test, source, data: checkSum_verify(test, source, data, YUV_LINE_MD5_VALUES[i][j])),
                ]
            else:
                r = r + [
                    #Step(Action.WAIT_FOR, jpegdecode_test_device, YUV_MCU_MD5_VALUES[i][j]),
                    Step(Action.WAIT_FOR, jpegdecode_test_device, 'checkSum',
                         lambda test, source, data: checkSum_verify(test, source, data, YUV_MCU_MD5_VALUES[i][j])),
                ]
            r = r + [
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'output done'),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode finish_decompress'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_finish_decompress()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode destroy_decompress'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_destroy_decompress()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode fd_close'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'closed fd'),
            ]
            return r

        Decode_16414 = Test(
            name='Decode_16414',
            timeout=90,
            setup=setup,
            test=[],
            teardown=teardown
        )

        Decode_16415 = Test(
            name='Decode_16415',
            timeout=90,
            setup=setup,
            test=[],
            teardown=teardown
        )

        def decoder_df_gp(cmd):
            g = []
            for i in [0, 1, 2, 3, 4, 5, 6, 7]:
                for j in [1, 0, 2, 3, 4, 5, 6, 7]:
                    temp = Decode_16414.clone()
                    temp.name = temp.name + '_' + cmd + '_' + str(i + 1) + '_' + str(j + 1)
                    temp.test = decoder_df_cmd(cmd, i, j)
                    g = g + [temp]
            return g

        def decoder_mem_gp(cmd):
            g = []
            for i in [0, 1, 2, 3, 4, 5, 6, 7]:
                for j in [1, 0, 2, 3, 4, 5, 6, 7]:
                    temp = Decode_16415.clone()
                    temp.name = temp.name + '_' + cmd + '_' + str(i + 1) + '_' + str(j + 1)
                    temp.test = decoder_memory_cmd(cmd, i, j)
                    g = g + [temp]
            return g




        Decode_16416 = Test(
            name='Decode_16416',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, jpegdecode_test_device, 'ls /mnt/sd0'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode filename /mnt/sd0/orig4.jpg'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'infilename: /mnt/sd0/orig4.jpg'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode select_decode_param line'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'line decode'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, ' jpeg_decode std_error'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_str_error()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode create_decompress'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_create_decompress()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode read_header'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'Improper call to JPEG library in state 200'),
            ],
            teardown=teardown
        )


        Decode_16417 = Test(
            name='Decode_16417',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, jpegdecode_test_device, 'ls /mnt/sd0'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode filename /mnt/sd0/orig4.jpg'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'infilename: /mnt/sd0/orig4.jpg'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode w_fopen'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode select_decode_param mcu'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'mcu decode'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, ' jpeg_decode std_error'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_str_error()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode create_decompress'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_create_decompress()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode fd_src'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_fd_src()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode read_header'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'Not a JPEG file: starts with '),
            ],
            teardown=teardown
        )

        Decode_16418= Test(
            name='Decode_16418',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, jpegdecode_test_device, 'ls /mnt/sd0'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode filename /mnt/sd0/test.txt'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'infilename: /mnt/sd0/test.txt'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode select_decode_param mcu'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'mcu decode'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, ' jpeg_decode std_error'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_str_error()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode create_decompress'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_create_decompress()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode mem_src'),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode read_header'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'Not a JPEG file: starts with '),
            ],
            teardown=teardown
        )



        Decode_16419 = Test(
            name='Decode_16419',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, jpegdecode_test_device, 'ls /mnt/sd0'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode filename /mnt/sd0/sample_not.jpg'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'infilename: /mnt/sd0/sample_not.jpg'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode select_decode_param mcu'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'mcu decode'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, ' jpeg_decode std_error'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_str_error()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode create_decompress'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'jpeg_create_decompress()'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 1, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode mem_src'),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode read_header'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'Not a JPEG file: starts with '),
            ],
            teardown=teardown
        )


        Decode_16420_01 = Test(
            name='Decode_16420_01',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode read_header'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'up_hardfault: PANIC!!! Hard fault:'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "command wait")),

            ],
            teardown=teardown
        )
        Decode_16420_02 = Test(
            name='Decode_16420_02',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode start_decomp'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'up_hardfault: PANIC!!! Hard fault:'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "command wait")),
            ],
            teardown=teardown
        )
        Decode_16420_03 = Test(
            name='Decode_16420_03',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode fd_src'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'up_hardfault: PANIC!!! Hard fault:'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "command wait")),
            ],
            teardown=teardown
        )
        Decode_16420_04 = Test(
            name='Decode_16420_04',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode mem_src'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'up_hardfault: PANIC!!! Hard fault:'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "command wait")),
            ],
            teardown=teardown
        )
        Decode_16420_05 = Test(
            name='Decode_16420_05',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode finish_decompress'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'up_hardfault: PANIC!!! Hard fault:'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                    lambda test, source, data: wait_sec(test, source, data, 3, "command wait")),
            ],
            teardown=teardown
        )



        Decode_16421 = Test(
            name='Decode_16421',
            timeout=300,
            setup=setup,
            test=loop_steps(100) + decoder_df_cmd('line', 7, 1),
            teardown=teardown
        )

        Decode_16422 = Test(
            name='Decode_16422',
            timeout=timeout,
            test=decoder_df_cmd('line', 7, 7) + [
               # work with Decode_16415_line_08
                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode finish_decompress'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'Improper call to JPEG library in state 0'),
                Step(Action.WRITE, jpegdecode_test_device, 'echo $?'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, '0'),
                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode destroy_decompress'),
                Step(Action.WRITE, jpegdecode_test_device, 'echo $?'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, '0'),
            ],
            teardown=teardown
        )

        Decode_16423 = Test(
            name='Decode_16423',
            timeout=timeout,
            setup=setup,
            test=[
                Step(Action.WRITE, jpegdecode_test_device, 'ls /mnt/sd0'),
                Step(Action.EXECUTE, jpegdecode_test_device, None,
                     lambda test, source, data: wait_sec(test, source, data, 5, "command wait")),

                Step(Action.WRITE, jpegdecode_test_device, 'jpeg_decode thread_create'),

                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image width  = 320'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image height = 240'),

                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image width  = 640'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image height = 480'),

                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image width  = 960'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image height = 720'),

                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image width  = 1280'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image height = 960'),

                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image width  = 1600'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'image height = 1200'),


                Step(Action.WAIT_FOR, jpegdecode_test_device, 'END'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'END'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'END'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'END'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'END'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, jpegdecode_test_device.NSH_PROMPT),
                Step(Action.WRITE, jpegdecode_test_device, 'echo $?'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, '0'),

                Step(Action.WRITE, jpegdecode_test_device, 'md5 -f /mnt/sd0/orig1.YUV'),
                Step(Action.WAIT_FOR, jpegdecode_test_device,'43ad23f853abef2fd1fc9aaff7ffbe17'),
                Step(Action.WRITE, jpegdecode_test_device, 'md5 -f /mnt/sd0/quad_vga.YUV'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'a6b6703f37466b71ac33b383dac2b1ed'),
                Step(Action.WRITE, jpegdecode_test_device, 'md5 -f /mnt/sd0/uxga.YUV'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, '44ff6a1e033e895f5baf4b4ba1e57cab'),
                Step(Action.WRITE, jpegdecode_test_device, 'md5 -f /mnt/sd0/qvga.YUV'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'eeed766be7961819f97b66b402f195ce'),
                Step(Action.WRITE, jpegdecode_test_device, 'md5 -f /mnt/sd0/vga.YUV'),
                Step(Action.WAIT_FOR, jpegdecode_test_device, 'cda9048fb3722fcbd4a1bb12d78d1065'),
            ],
            teardown=teardown
        )

        develop_gr = TestGroup(
            name='develop_gr',
            devices=[jpegdecode_test_device],
            tag=[Tag.POSITIVE],
            tests=[
            ]
        )

        jpegdecode = TestGroup(
            name='jpegdecode',
            devices=[jpegdecode_test_device],
            tag=[Tag.POSITIVE],
            tests=decoder_df_gp('mcu') + decoder_mem_gp('mcu') + decoder_df_gp('line') + decoder_mem_gp('line') + [
                #Decode_16421,
                #loop initialize test
                #half automation, check human eye
                Decode_16416,
                Decode_16417,
                Decode_16418,
                Decode_16419,

                Decode_16420_01,
                Decode_16420_02,
                Decode_16420_03,
                Decode_16420_04,
                Decode_16420_05,

                Decode_16421,
                Decode_16422,
                Decode_16423,
            ]
        )

        test_groups = [
            jpegdecode,
#            develop_gr,
        ]

        return test_groups


if __name__ == "__main__":
    parser = RunnerParser()
    args = parser.parse_args()

    if args.config is not None:
        config = Config(os.path.abspath(args.config))
    else:
        config = Config(os.path.join('../../../', Config.DEFAULT_CONFIG_FILE))

    # Create Device Manager
    dev_manager = DeviceManager(config)

    # Assign devices according to role
    dut_device = dev_manager.get_devices_by_serials(args.dut_device)[0]

    # Create test runner instance
    tc_runner = JpegDecodeTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
