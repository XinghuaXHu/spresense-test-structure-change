#! /usr/bin/env python3
import sys
import os
import time
import datetime

SCRIPT_PATH = os.path.dirname(__file__)
API_PATH = os.path.realpath(os.path.join(SCRIPT_PATH, "..", ".."))

sys.path.append(API_PATH)
from api.device_manager import DeviceManager
from api.tester_exception import TesterException
from tc.lteapi_test.lteapi_test_device import LteapiTestDevice
from api.utils import Toolbox
from api.runner import TestRunner, Tag
from api.test import Test, Step, Action, TestGroup, TestLogger
from api.config import Config
from api.runner_parser import RunnerParser


PROJECT = 'spresense'
APPS_TO_BUILTIN = ['lteapi']
PIN_CODE_DEFAULT = "0000"
PIN_CODE_CHANGED = "1111"
PIN2_CODE_DEFAULT = "9999"
PIN2_CODE_CHANGED = "8888"
PUK_CODE = "11111111"
OPERATOR_NAME = "R&S CMW 500 LTE"

# noinspection PyUnusedLocal
def dut_reboot_monitor(test, source, data):
    if source.NUTTSHELL in data:
        test.set_fail(' '.join(['Unexpected', str(source), 'reboot!']))

def localtime_monitor(test, source, data):
    if "[CB_VAL] lcltmrepo_cb" in data:
        test.set_fail("lcltmrepo_cb received!")

def netstat_monitor(test, source, data):
    if "[CB_VAL] ntstrepo_cb" in data:
        test.set_fail("ntstrepo_cb received!")

def simstat_monitor(test, source, data):
    if "[CB_VAL] smstrepo_cb simstat" in data:
        test.set_fail("smstrepo_cb received!")

def quality_monitor(test, source, data):
    if "[CB_VAL] qtyrepo_cb" in data:
        test.set_fail("qtyrepo_cb received!")

def cellinfo_monitor(test, source, data):
    if "[CB_VAL] clinfrepo_cb" in data:
        test.set_fail("clinfrepo_cb received!")

# noinspection PyUnusedLocal
def dut_monitor(test, source, data):
    test.log.debug(data)

def wait_sec(test, source, data, sec, description=None):
    if description is not None:
        test.log.info("")
        test.log.info("############################################################")
        test.log.info("## [{}sec] {}".format(str(sec), description))
        test.log.info("############################################################")
        test.log.info("")

    time.sleep(sec)
    test.add_user_event(source, 'wait sec completed')

def chk_in_cmd_str(test, source, data, expect_string):
    cmd = data.get_value('Command')
    if expect_string in cmd:
        return True
    else:
        return False

class LteapiTestTcRunner(TestRunner):

    def __init__(self, conf, **kwargs):
        super(LteapiTestTcRunner, self).__init__(conf)

        self.lteapi_test_device = LteapiTestDevice(
            kwargs.get('dut_device', None), 'LTEAPI_TEST_DEVICE') if kwargs.get('dut_device', None)\
            else None

        if not all(self.get_devices_list()):
            raise TesterException('At least one device is needed!')

        self.writer_path = os.path.join(self.config.projects[0].path, 'test/autotest/src/api/updater.py')
        self.defconfig_path = os.path.join(os.getcwd(), 'defconfig/lteapi-defconfig')

    def get_devices_list(self):
        return [self.lteapi_test_device]

    def setup(self, arguments, log=None):
        debug = not arguments.release
        binaries = dict(dut_bin=None, bac_bin=None)

        binaries['dut_bin'] = os.path.normpath(arguments.dut_bin) if arguments.dut_bin else None

        binaries = self.build(debug, log, **binaries)

        self.flash_n_check(self.lteapi_test_device, binaries['dut_bin'], log)

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
            dut_bin = self.__build_binary(toolbox, self.defconfig_path, self.lteapi_test_device)

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
        timeout = 300

        lteapi_test_device = self.get_devices_list()[0]

        setup = [
            Step(Action.WAIT_FOR, lteapi_test_device, lteapi_test_device.NSH_PROMPT),
            Step(Action.WRITE, lteapi_test_device, 'lteapi init'),
            Step(Action.WAIT_FOR, lteapi_test_device, '[API_OK]'),
            Step(Action.WRITE, lteapi_test_device, 'lteapi pwron'),
            Step(Action.WAIT_FOR, lteapi_test_device, '[CB_OK]'),
            Step(Action.WRITE, lteapi_test_device, 'lteapi stpinenb LTE_PIN_DISABLE {}'.format(PIN_CODE_DEFAULT)),
            Step(Action.EXECUTE, lteapi_test_device, None,
                 lambda test, source, data: wait_sec(test, source, data, 3)),
            Step(Action.WAIT_FOR, lteapi_test_device, 'wait sec completed'),
            Step(Action.WRITE, lteapi_test_device, 'reboot'),
            Step(Action.WAIT_FOR, lteapi_test_device, lteapi_test_device.NUTTSHELL),
        ]

        teardown = [
        ]

        def loop_steps(count, steps):
            r = []
            for i in range(count):
                r = r + steps
            return r

        def wait_for_cmd_steps(device, expect_string):
            return [
                Step(Action.WAIT_FOR, device, device.NSH_PROMPT,
                    lambda test, source, data: chk_in_cmd_str(test, source, data, expect_string)),
            ]

        def wait_sec_steps(device, sec, description=None):
            return [
                Step(Action.EXECUTE, device, None,
                     lambda test, source, data: wait_sec(test, source, data, sec, description)),
                Step(Action.WAIT_FOR, device, 'wait sec completed'),
            ]

        def localtime_monitor_steps(device, monitor_start):
            if monitor_start == True:
                return [
                    Step(Action.EXECUTE, device, ('localtime_mon', localtime_monitor), Test.add_monitor),
                ]
            else:
                return [
                    Step(Action.EXECUTE, device, 'localtime_mon', Test.remove_monitor),
                ]

        def netstat_monitor_steps(device, monitor_start):
            if monitor_start == True:
                return [
                    Step(Action.EXECUTE, device, ('netstat_mon', netstat_monitor), Test.add_monitor),
                ]
            else:
                return [
                    Step(Action.EXECUTE, device, 'netstat_mon', Test.remove_monitor),
                ]

        def simstat_monitor_steps(device, monitor_start):
            if monitor_start == True:
                return [
                    Step(Action.EXECUTE, device, ('simstat_mon', simstat_monitor), Test.add_monitor),
                ]
            else:
                return [
                    Step(Action.EXECUTE, device, 'simstat_mon', Test.remove_monitor),
                ]

        def quality_monitor_steps(device, monitor_start):
            if monitor_start == True:
                return [
                    Step(Action.EXECUTE, device, ('quality_mon', quality_monitor), Test.add_monitor),
                ]
            else:
                return [
                    Step(Action.EXECUTE, device, 'quality_mon', Test.remove_monitor),
                ]

        def cellinfo_monitor_steps(device, monitor_start):
            if monitor_start == True:
                return [
                    Step(Action.EXECUTE, device, ('cellinfo_mon', cellinfo_monitor), Test.add_monitor),
                ]
            else:
                return [
                    Step(Action.EXECUTE, device, 'cellinfo_mon', Test.remove_monitor),
                ]

        def sqa_init(device):
            return [
                Step(Action.WRITE, device, 'lteapi init'),
                Step(Action.WAIT_FOR, device, '[API_OK]'),
            ]

        def sqa_pwron(device):
            return [
                Step(Action.WRITE, device, 'lteapi pwron'),
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
            ]

        def sqa_gtnetst(device, state):
            return [
                Step(Action.WRITE, device, 'lteapi gtnetst'),
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
                Step(Action.WAIT_FOR, device, '[CB_VAL] getnetst_cb state : \"{}\"'.format(state)),
            ]

        def sqa_gtdtst(device, state_list):
            r = [
                Step(Action.WRITE, device, 'lteapi gtdtst'),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
                Step(Action.WAIT_FOR, device, '[CB_VAL] getdtst_cb listnum : {}'.format(str(len(state_list)))),
            ]
            for (idx, state) in zip(range(len(state_list)), state_list):
                r = r + [
                    Step(Action.WAIT_FOR, device, '[CB_VAL] getdtst_cb state [session_id] : \"{} [{}]\"'.format(state, str(idx+1))),
                ]
            return r

        # deta_type: "USER" or "IMS"
        # general  : True or False
        # roaming  : True or False
        def sqa_stdtcfg(device, data_type, general, roaming):
            gene = "LTE_ENABLE" if general == True else "LTE_DISABLE"
            roam = "LTE_ENABLE" if roaming == True else "LTE_DISABLE"
            return [
                Step(Action.WRITE, device, 'lteapi stdtcfg LTE_DATA_TYPE_{} {} {}'.format(data_type, gene, roam)),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
            ]

        # deta_type: "USER" or "IMS"
        def sqa_gtdtcfg(device, data_type, general, roaming):
            data_type_num = 0 if data_type == "USER" else 1
            return [
                Step(Action.WRITE, device, 'lteapi gtdtcfg LTE_DATA_TYPE_{}'.format(data_type)),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
                Step(Action.WAIT_FOR, device, '[CB_VAL] getdtcfg_cb data_type : \"{}\"'.format(str(data_type_num))),
                Step(Action.WAIT_FOR, device, '[CB_VAL] getdtcfg_cb general   : \"{}\"'.format(str(int(general)))),
                Step(Action.WAIT_FOR, device, '[CB_VAL] getdtcfg_cb roaming   : \"{}\"'.format(str(int(roaming)))),
            ]

        def sqa_stapn(device, session_id_list, apn_list):
            r = []
            for (session_id, apn) in zip(session_id_list, apn_list):
                r = r + [
                    Step(Action.WRITE, device, 'lteapi stapn {} {} LTE_APN_IPTYPE_IPV4V6 LTE_APN_AUTHTYPE_CHAP usr pwd00'.format(str(session_id), apn)),
                ]
                if session_id > 0 and session_id < 3:
                    r = r + wait_for_cmd_steps(device, '[API_OK]')
                    r.append(Step(Action.WAIT_FOR, device, '[CB_OK]'))
                elif session_id < 6:
                    r = r + wait_for_cmd_steps(device, '[API_OK]')
                    r.append(Step(Action.WAIT_FOR, device, '[CB_NG]'))
                else:
                    r.append(Step(Action.WAIT_FOR, device, '[API_NG]'))
            return r

        def sqa_gtapnst(device, session_id_list, apn_list):
            r = [
                Step(Action.WRITE, device, 'lteapi gtapnst'),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
            ]
            for (session_id, apn) in zip(session_id_list, apn_list):
                r = r + [
                    Step(Action.WAIT_FOR, device, 'session_id : \"{}\"'.format(session_id)),
                    Step(Action.WAIT_FOR, device, 'apn        : \"{}\"'.format(apn)),
                    Step(Action.WAIT_FOR, device, 'ip_type    : \"2\"'),
                ]
            return r

        def sqa_gtver(device):
            return [
                Step(Action.WRITE, device, 'lteapi gtver'),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
                Step(Action.WAIT_FOR, device, 'gtver_cb bb_product : '),
                Step(Action.WAIT_FOR, device, 'gtver_cb np_package : '),
            ]

        def sqa_gtphno(device):
            return [
                Step(Action.WRITE, device, 'lteapi gtphno'),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
                Step(Action.WAIT_FOR, device, 'gtphno_cb phoneno : '),
            ]

        def sqa_gtimsi(device, cmd_fail=False):
            r = [
                Step(Action.WRITE, device, 'lteapi gtimsi'),
            ] + wait_for_cmd_steps(device, '[API_OK]')

            if cmd_fail == True:
                r = r + [
                    Step(Action.WAIT_FOR, device, '[CB_NG] gtimsi_cb result : '),
                    Step(Action.WAIT_FOR, device, '[CB_NG] gtimsi_cb error : '),
                ]
            else:
                r = r + [
                    Step(Action.WAIT_FOR, device, '[CB_OK]'),
                    Step(Action.WAIT_FOR, device, 'gtimsi_cb imsi : '),
                ]
            return r

        def sqa_gtimei(device):
            return [
                Step(Action.WRITE, device, 'lteapi gtimei'),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
                Step(Action.WAIT_FOR, device, 'gtimei_cb imei : '),
            ]

        def sqa_gtpinst(device, enable, status):
            r = []
            r = r + [
                Step(Action.WRITE, device, 'lteapi gtpinst'),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK] gtpinst_cb'),
                Step(Action.WAIT_FOR, device, '[CB_VAL] gtpinst_cb enable            : \"{}\"'.format(enable)),
            ]

            if enable == "1":
                r = r + [
                    Step(Action.WAIT_FOR, device, '[CB_VAL] gtpinst_cb status            : \"{}\"'.format(status)),
                ]

            r = r + [
                Step(Action.WAIT_FOR, device, '[CB_VAL] gtpinst_cb pin_attemptsleft'),
                Step(Action.WAIT_FOR, device, '[CB_VAL] gtpinst_cb puk_attemptsleft'),
                Step(Action.WAIT_FOR, device, '[CB_VAL] gtpinst_cb pin2_attemptsleft'),
                Step(Action.WAIT_FOR, device, '[CB_VAL] gtpinst_cb puk2_attemptsleft'),
            ]
            return r

        def sqa_stpinenb(device, pin_enable, pin_code=PIN_CODE_DEFAULT, cmd_fail=False):
            en = "LTE_PIN_ENABLE" if pin_enable == True else "LTE_PIN_DISABLE"
            r = [
                Step(Action.WRITE, device, 'lteapi stpinenb {} {}'.format(en, pin_code)),
            ] + wait_for_cmd_steps(device, '[API_OK]')
            if cmd_fail == True:
                r = r + [
                    Step(Action.WAIT_FOR, device, '[CB_NG] stpnenbl_cb'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] stpnenbl_cb attemptsleft'),
                ]
            else:
                r = r + [
                    Step(Action.WAIT_FOR, device, '[CB_OK] stpnenbl_cb'),
                ]
            return r

        def sqa_gtlcltm(device):
            datestr = datetime.datetime.now().strftime("%Y/%m/%d")[2:]
            return [
                Step(Action.WRITE, device, 'lteapi gtlcltm'),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
                Step(Action.WAIT_FOR, device, '[CB_VAL] gtlcltm_cb time      : \"{} :'.format(datestr)),
                Step(Action.WAIT_FOR, device, '[CB_VAL] gtlcltm_cb time_zone :'),
            ]

        def sqa_strplcltm(device, rp_enable=True):
            r = []
            if rp_enable == True:
               r.append(Step(Action.WRITE, device, 'lteapi strplcltm'))
            else:
               r.append(Step(Action.WRITE, device, 'lteapi strplcltm NULL'))
            r = r + wait_for_cmd_steps(device, '[API_OK]')
            return r

        def sqa_rplcltm(device):
            datestr = datetime.datetime.now().strftime("%Y/%m/%d")[2:]
            return [
                Step(Action.WAIT_FOR, device, '[CB_VAL] lcltmrepo_cb time      : \"{} '.format(datestr)),
                Step(Action.WAIT_FOR, device, '[CB_VAL] lcltmrepo_cb time_zone :'),
            ]

        def sqa_gtoprtr(device):
            return [
                Step(Action.WRITE, device, 'lteapi gtoprtr'),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_VAL] gtoprtr_cb oper : \"{}\"'.format(OPERATOR_NAME)),
            ]

        # edrx_enable: True   or False
        # edrx_cyc   : "1024" or "2048"
        # edrx_pwt   : "256"  or "384"
        def sqa_stedrx(device, edrx_enable=True, edrx_cyc="1024", edrx_ptw="256"):
            if edrx_enable == False:
                enable = "LTE_DISABLE"
                cyc = "INVALID"
                ptw = "INVALID"
            else:
                enable = "LTE_ENABLE"
                cyc = "LTE_EDRX_CYC_{}".format(edrx_cyc)
                ptw = "LTE_EDRX_PTW_{}".format(edrx_ptw)

            return [
                Step(Action.WRITE, device, 'lteapi stedrx {} {} {}'.format(enable, cyc, ptw)),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK] setedrx_cb'),
            ]

        # edrx_enable: True   or False
        # edrx_cyc   : "1024" or "2048"
        # edrx_pwt   : "256"  or "384"
        def sqa_gtedrx(device, edrx_enable=True, edrx_cyc="1024", edrx_ptw="256"):
            r = [
                Step(Action.WRITE, device, 'lteapi gtedrx'),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
                Step(Action.WAIT_FOR, device, '[CB_VAL] getedrx_cb enable     : \"{}\"'.format(str(int(edrx_enable)))),
            ]
            if edrx_enable == True:
                cyc = "1" if edrx_cyc == "1024" else "2"
                ptw = "1" if edrx_ptw == "256" else "2"
                r = r + [
                    Step(Action.WAIT_FOR, device, '[CB_VAL] getedrx_cb edrx_cycle : \"{}\"'.format(cyc)),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] getedrx_cb ptw_val    : \"{}\"'.format(ptw)),
                ]
            return r

        def sqa_stpsm(device, psm_enable=True, t3324_unit="1", t3324_value="10", t3412_unit="1", t3412_value="10"):
            if psm_enable == True:
                enable = "LTE_ENABLE"
                t3324_u = "LTE_PSM_T3324_UNIT_1MIN" if t3324_unit == "1" else "LTE_PSM_T3324_UNIT_6MIN"
                t3324_v = t3324_value
                t3412_u = "LTE_PSM_T3412_UNIT_30SEC" if t3412_unit == "1" else "LTE_PSM_T3412_UNIT_1MIN"
                t3412_v = t3412_value
            else:
                enable = "LTE_DISABLE"
                t3324_u = "INVALID"
                t3324_v = "INVALID"
                t3412_u = "INVALID"
                t3412_v = "INVALID"

            return [
                Step(Action.WRITE, device, 'lteapi stpsm {} {} {} {} {}'.format(enable, t3324_u, t3324_v, t3412_u, t3412_v)),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK] setpsm_cb'),
            ]

        def sqa_gtpsm(device, psm_enable=True, t3324_unit="1", t3324_value="10", t3412_unit="1", t3412_value="10"):
            r = [
                Step(Action.WRITE, device, 'lteapi gtpsm'),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK] getpsm_cb'),
                Step(Action.WAIT_FOR, device, '[CB_VAL] getpsm_cb enable                         : \"{}\"'.format(str(int(psm_enable)))),
            ]
            if psm_enable == True:
                r = r + [
                    Step(Action.WAIT_FOR, device, '[CB_VAL] getpsm_cb req_active_time.unit           : \"{}\"'.format(t3324_unit)),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] getpsm_cb req_active_time.time_val       : \"{}\"'.format(t3324_value)),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] getpsm_cb ext_periodic_tau_time.unit     : \"{}\"'.format(t3412_unit)),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] getpsm_cb ext_periodic_tau_time.time_val : \"{}\"'.format(t3412_value)),
                ]
            return r

        def sqa_strpntst(device, rep_enable):
            r = []
            if rep_enable == True:
                r.append(Step(Action.WRITE, device, 'lteapi strpntst'))
            else:
                r.append(Step(Action.WRITE, device, 'lteapi strpntst NULL'))
            r = r + wait_for_cmd_steps(device, '[API_OK]')
            return r

        def sqa_strpsmst(device, rep_enable):
            r = []
            if rep_enable == True:
                r.append(Step(Action.WRITE, device, 'lteapi strpsmst'))
            else:
                r.append(Step(Action.WRITE, device, 'lteapi strpsmst NULL'))
            r = r + wait_for_cmd_steps(device, '[API_OK]')
            return r

        def sqa_strpqty(device, rep_interval, rep_enable):
            r = []
            if rep_enable == False:
                r = r + [
                    Step(Action.WRITE, device, 'lteapi strpqty {} NULL'.format(rep_interval)),
                ] + wait_for_cmd_steps(device, '[API_OK]')
            else:
                r = r + [
                    Step(Action.WRITE, device, 'lteapi strpqty {}'.format(rep_interval)),
                ] + wait_for_cmd_steps(device, '[API_OK]') + loop_steps(3, [
                    Step(Action.WAIT_FOR, device, '[CB_VAL] qtyrepo_cb valid'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] qtyrepo_cb rsrp'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] qtyrepo_cb rsrq'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] qtyrepo_cb sinr'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] qtyrepo_cb rssi'),
                ])
            return r

        def sqa_strpclinf(device, rep_interval, rep_enable):
            r = []
            if rep_enable == False:
                r = r + [
                    Step(Action.WRITE, device, 'lteapi strpclinf {} NULL'.format(rep_interval)),
                ] + wait_for_cmd_steps(device, '[API_OK]')
            else:
                r = r + [
                    Step(Action.WRITE, device, 'lteapi strpclinf {}'.format(rep_interval)),
                ] + wait_for_cmd_steps(device, '[API_OK]') + loop_steps(3, [
                    Step(Action.WAIT_FOR, device, '[CB_VAL] clinfrepo_cb valid'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] clinfrepo_cb phycell_id'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] clinfrepo_cb earfcn'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] clinfrepo_cb mcc[0]'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] clinfrepo_cb mcc[1]'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] clinfrepo_cb mcc[2]'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] clinfrepo_cb mnc_digit'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] clinfrepo_cb mnc[0]'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] clinfrepo_cb mnc[1]'),
                ])
            return r

        def sqa_dataoff(device, session_id, rep_net_stat=False):
            r = [
                Step(Action.WRITE, device, 'lteapi dataoff {}'.format(session_id)),
            ] + wait_for_cmd_steps(device, '[API_OK]')

            if rep_net_stat == True:
                r = r + sqa_ntstrepo(device, "3")

            r = r + [
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
            ]
            return r

        def sqa_entpin(device, cmd_fail=False):
            r = []
            if cmd_fail == True:
                r = r + [
                    Step(Action.WRITE, device, 'lteapi entpin {} {}'.format(PIN_CODE_CHANGED, PIN_CODE_CHANGED)),
                ] + wait_for_cmd_steps(device, '[API_OK]') + [
                    Step(Action.WAIT_FOR, device, '[CB_NG] entpin_cb result'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] entpin_cb attemptsleft'),
                ]
            else:
                r = r + [
                    Step(Action.WRITE, device, 'lteapi entpin {} {}'.format(PIN_CODE_DEFAULT, PIN_CODE_DEFAULT)),
                ] + wait_for_cmd_steps(device, '[API_OK]') + [
                    Step(Action.WAIT_FOR, device, '[CB_VAL] entpin_cb pinstat'),
                ]
            return r

        def sqa_entpuk(device):
            r = []
            r = r + [
                Step(Action.WRITE, device, 'lteapi entpin {} {}'.format(PUK_CODE, PIN_CODE_DEFAULT)),
            ] + wait_for_cmd_steps(device, '[API_OK]') + [
                Step(Action.WAIT_FOR, device, '[CB_OK] entpin_cb'),
                Step(Action.WAIT_FOR, device, '[CB_VAL] entpin_cb pinstat'),
            ]
            return r


        def sqa_atchnet(device, rep_net_stat=False):
            r = [
                Step(Action.WRITE, device, 'lteapi atchnet'),
            ] + wait_for_cmd_steps(device, '[API_OK]')

            if rep_net_stat == True:
                r = r + sqa_ntstrepo(device, "2") + sqa_ntstrepo(device, "0")

            r = r + [
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
            ]
            return r

        def sqa_atchnet_pinenable(device, rep_simstat=False):
            r = [
                Step(Action.WRITE, device, 'lteapi atchnet'),
            ] + wait_for_cmd_steps(device, '[API_OK]')

            if rep_simstat == True:
                r = r + sqa_smstrepo(device, "2")

            r = r + [
                Step(Action.WAIT_FOR, device, '[CB_NG] atchnet_cb result : '),
                Step(Action.WAIT_FOR, device, '[CB_NG] atchnet_cb error : '),
            ]
            return r

        def sqa_smstrepo(device, sim_stat):
            return [
                Step(Action.WAIT_FOR, device, '[CB_VAL] smstrepo_cb simstat : \"{}\"'.format(sim_stat)),
            ]

        def sqa_ntstrepo(device, net_stat):
            return [
                Step(Action.WAIT_FOR, device, '[CB_VAL] ntstrepo_cb netstat : \"{}\"'.format(net_stat)),
            ]

        def sqa_dataon(device, session_id, rep_net_stat=False):
            r = [
                Step(Action.WRITE, device, 'lteapi dataon {}'.format(session_id)),
            ] + wait_for_cmd_steps(device, '[API_OK]')

            if rep_net_stat == True:
                r = r + sqa_ntstrepo(device, "2")

            r = r + [
                Step(Action.WAIT_FOR, device, '[CB_OK]'),
            ]
            return r

        def sqa_cngpin(device, pin_no, recover=False, cmd_fail=False):
            if pin_no == 1:
                pin_target = "LTE_TARGET_PIN"
                pin_old = PIN_CODE_DEFAULT if recover == False else PIN_CODE_CHANGED
                pin_new = PIN_CODE_CHANGED if recover == False else PIN_CODE_DEFAULT
            else:
                pin_target = "LTE_TARGET_PIN2"
                pin_old = PIN2_CODE_DEFAULT if recover == False else PIN2_CODE_CHANGED
                pin_new = PIN2_CODE_CHANGED if recover == False else PIN2_CODE_DEFAULT

            r = [
                Step(Action.WRITE, device, 'lteapi cngpin {} {} {}'.format(pin_target, pin_old, pin_new)),
            ] + wait_for_cmd_steps(device, '[API_OK]')

            if cmd_fail == True:
                r = r + [
                    Step(Action.WAIT_FOR, device, '[CB_NG] cngpin_cb'),
                    Step(Action.WAIT_FOR, device, '[CB_VAL] cngpin_cb attemptsleft'),
                ]
            else:
                r = r + [
                    Step(Action.WAIT_FOR, device, '[CB_OK] cngpin_cb'),
                ]
            return r

        def sqa_dtchnet(device, rep_net_stat=False):
            r = [
                Step(Action.WRITE, device, 'lteapi dtchnet'),
            ] + wait_for_cmd_steps(device, '[API_OK]')

            if rep_net_stat == True:
                r = r + sqa_ntstrepo(device, "3") + sqa_ntstrepo(device, "1")

            r = r + [
                Step(Action.WAIT_FOR, device, '[CB_OK] dtchnet_cb'),
            ]
            return r

        def sqa_pwroff(device):
            return [
                Step(Action.WRITE, device, 'lteapi pwroff'),
                Step(Action.WAIT_FOR, device, '[CB_OK] pwroff_cb'),
            ]

        def sqa_fin(device):
            return [
                Step(Action.WRITE, device, 'lteapi fin'),
            ] + wait_sec_steps(device, 3)

        def get_mem(device):
            return [
                Step(Action.WRITE, device, 'free'),
                Step(Action.WAIT_FOR, device, 'Mem:'),
            ]

        def sqa_power_steps(device):
            return sqa_init(device)                                      +\
                   loop_steps(5, sqa_pwron(device) + sqa_pwroff(device)) +\
                   sqa_fin(device)

        def sqa_attach_steps(device):
            return sqa_init(device)                   +\
                   sqa_pwron(device)                  +\
                   loop_steps(5,                       \
                       sqa_atchnet(device)            +\
                       sqa_gtnetst(device, "0")       +\
                       sqa_gtdtst(device, ["2","3"])  +\
                       sqa_dtchnet(device)            +\
                       sqa_gtnetst(device, "1")       +\
                       sqa_gtdtst(device, ["3","3"])) +\
                   sqa_fin(device)

        def sqa_data_steps(device):
            return sqa_init(device)                   +\
                   sqa_pwron(device)                  +\
                   sqa_atchnet(device)                +\
                   sqa_gtdtst(device, ["2","3"])      +\
                   loop_steps(2,                       \
                       sqa_dataon(device, "1")        +\
                       sqa_gtdtst(device, ["2","3"])  +\
                       sqa_dataon(device, "2")        +\
                       sqa_gtdtst(device, ["2","2"])  +\
                       sqa_dataoff(device, "2")       +\
                       sqa_gtdtst(device, ["2","3"])  +\
                       sqa_dataoff(device, "1")       +\
                       sqa_gtdtst(device, ["3","3"])) +\
                   sqa_dataon(device, "1")            +\
                   sqa_dataon(device, "2")            +\
                   sqa_gtdtst(device, ["2","2"])      +\
                   sqa_dtchnet(device)                +\
                   sqa_gtdtst(device, ["3","3"])      +\
                   sqa_fin(device)

        def sqa_dataconfig_steps(device):
            return sqa_init(device)                              +\
                   sqa_pwron(device)                             +\
                   loop_steps(5,                                  \
                       sqa_stdtcfg(device, "USER", False, False) +\
                       sqa_gtdtcfg(device, "USER", False, False) +\
                       sqa_stdtcfg(device, "IMS", True, True)    +\
                       sqa_gtdtcfg(device, "IMS", True, True)    +\
                       sqa_stdtcfg(device, "USER", True, True)   +\
                       sqa_gtdtcfg(device, "USER", True, True)   +\
                       sqa_stdtcfg(device, "IMS", False, False)  +\
                       sqa_gtdtcfg(device, "IMS", False, False)) +\
                   sqa_fin(device)

        def sqa_apn_steps(device):
            return sqa_init(device)                            +\
                   sqa_pwron(device)                           +\
                   sqa_stapn(device, [1,2], ["apn1","apn2"])   +\
                   sqa_gtapnst(device, [1,2], ["apn1","apn2"]) +\
                   sqa_stapn(device, [1,2], ["apnA","apnB"])   +\
                   sqa_gtapnst(device, [1,2], ["apnA","apnB"]) +\
                   sqa_stapn(device, [1,2], ["apn1","apn2"])   +\
                   sqa_gtapnst(device, [1,2], ["apn1","apn2"]) +\
                   sqa_stapn(device, [3], ["apn3"])            +\
                   sqa_stapn(device, [6], ["apn6"])            +\
                   sqa_fin(device)

        def sqa_apn_short_steps(device):
            return sqa_init(device)                                              +\
                   sqa_pwron(device)                                             +\
                   sqa_stapn(device, [1,2,3,4], ["apn1","apn2","apn3","apn4"])   +\
                   sqa_gtapnst(device, [1,2,3,4], ["apn1","apn2","apn3","apn4"]) +\
                   sqa_stapn(device, [1,2,3,4], ["apnA","apnB","apnC","apnD"])   +\
                   sqa_gtapnst(device, [1,2,3,4], ["apnA","apnB","apnC","apnD"]) +\
                   sqa_stapn(device, [1,2,3,4], ["apn1","apn2","apn3","apn4"])   +\
                   sqa_gtapnst(device, [1,2,3,4], ["apn1","apn2","apn3","apn4"]) +\
                   sqa_fin(device)

        def sqa_version_steps(device):
            return sqa_init(device)                  +\
                   sqa_pwron(device)                 +\
                   loop_steps(10, sqa_gtver(device)) +\
                   sqa_fin(device)

        def sqa_phoneno_steps(device):
            return sqa_init(device)                   +\
                   sqa_pwron(device)                  +\
                   loop_steps(10, sqa_gtphno(device)) +\
                   sqa_fin(device)

        def sqa_imsi_steps(device):
            return sqa_init(device)                   +\
                   sqa_pwron(device)                  +\
                   loop_steps(10, sqa_gtimsi(device)) +\
                   loop_steps(10,                      \
                       sqa_gtpinst(device, "0", "0")  +\
                       sqa_gtimsi(device)             +\
                       sqa_stpinenb(device, True)     +\
                       sqa_gtpinst(device, "1", "0")  +\
                       sqa_gtimsi(device)             +\
                       sqa_pwroff(device)             +\
                       sqa_pwron(device)              +\
                       sqa_gtpinst(device, "1", "1")  +\
                       sqa_gtimsi(device, True)       +\
                       sqa_entpin(device, True)       +\
                       sqa_gtpinst(device, "1", "1")  +\
                       sqa_gtimsi(device, True)       +\
                       sqa_entpin(device)             +\
                       sqa_gtpinst(device, "1", "0")  +\
                       sqa_gtimsi(device)             +\
                       sqa_stpinenb(device, False))   +\
                   sqa_fin(device)

        def sqa_imei_steps(device):
            return sqa_init(device)                   +\
                   sqa_pwron(device)                  +\
                   loop_steps(10, sqa_gtimei(device)) +\
                   sqa_stpinenb(device, True)         +\
                   sqa_pwroff(device)                 +\
                   sqa_pwron(device)                  +\
                   loop_steps(10, sqa_gtimei(device)) +\
                   sqa_stpinenb(device, False)        +\
                   sqa_fin(device)

        def sqa_pin_enable_steps(device):
            return sqa_init(device)                                    +\
                   sqa_pwron(device)                                   +\
                   sqa_stpinenb(device, True)                          +\
                   sqa_gtpinst(device, "1", "0")                       +\
                   sqa_stpinenb(device, False, PIN_CODE_CHANGED, True) +\
                   sqa_gtpinst(device, "1", "0")                       +\
                   sqa_stpinenb(device, False)                         +\
                   sqa_gtpinst(device, "0", "0")                       +\
                   loop_steps(5,                                        \
                       sqa_stpinenb(device, True)                      +\
                       sqa_gtpinst(device, "1", "0")                   +\
                       sqa_stpinenb(device, False)                     +\
                       sqa_gtpinst(device, "0", "0"))                  +\
                   loop_steps(5,                                        \
                       sqa_stpinenb(device, True)                      +\
                       sqa_stpinenb(device, False))                    +\
                   sqa_stpinenb(device, True)                          +\
                   loop_steps(5,                                        \
                       sqa_gtpinst(device, "1", "0"))                  +\
                   sqa_stpinenb(device, False)                         +\
                   loop_steps(5,                                        \
                       sqa_gtpinst(device, "0", "0"))                  +\
                   sqa_fin(device)

        def sqa_pin_change_steps(device):
            return sqa_init(device)                         +\
                   sqa_pwron(device)                        +\
                   sqa_stpinenb(device, True)               +\
                   loop_steps(5,                             \
                       sqa_cngpin(device, 1)                +\
                       sqa_cngpin(device, 1, cmd_fail=True) +\
                       sqa_cngpin(device, 1, recover=True)  +\
                       sqa_pwroff(device)                   +\
                       sqa_pwron(device)                    +\
                       sqa_cngpin(device, 2)                +\
                       sqa_cngpin(device, 2, cmd_fail=True) +\
                       sqa_pwroff(device)                   +\
                       sqa_pwron(device)                    +\
                       sqa_cngpin(device, 2, recover=True)) +\
                   sqa_stpinenb(device, False)              +\
                   sqa_fin(device)

        def sqa_pin_enter_steps(device):
            return sqa_init(device)              +\
                   sqa_pwron(device)             +\
                   sqa_gtpinst(device, "0", "0") +\
                   sqa_stpinenb(device, True)    +\
                   sqa_gtpinst(device, "1", "0") +\
                   sqa_pwroff(device)            +\
                   sqa_pwron(device)             +\
                   sqa_gtpinst(device, "1", "1") +\
                   sqa_entpin(device, True)      +\
                   sqa_gtpinst(device, "1", "1") +\
                   sqa_entpin(device)            +\
                   sqa_gtpinst(device, "1", "0") +\
                   sqa_pwroff(device)            +\
                   sqa_pwron(device)             +\
                   sqa_gtpinst(device, "1", "1") +\
                   sqa_entpin(device, True)      +\
                   sqa_gtpinst(device, "1", "1") +\
                   sqa_entpin(device, True)      +\
                   sqa_gtpinst(device, "1", "1") +\
                   sqa_entpin(device, True)      +\
                   sqa_gtpinst(device, "1", "2") +\
                   sqa_entpuk(device)            +\
                   sqa_gtpinst(device, "1", "0") +\
                   sqa_stpinenb(device, False)   +\
                   sqa_gtpinst(device, "0", "0") +\
                   sqa_fin(device)

        def sqa_localtime_steps(device):
            return sqa_init(device)                       +\
                   sqa_pwron(device)                      +\
                   sqa_strplcltm(device)                  +\
                   sqa_atchnet(device)                    +\
                   sqa_rplcltm(device)                    +\
                   sqa_dtchnet(device)                    +\
                   sqa_atchnet(device)                    +\
                   sqa_rplcltm(device)                    +\
                   sqa_dtchnet(device)                    +\
                   sqa_strplcltm(device, False)           +\
                   localtime_monitor_steps(device, True)  +\
                   sqa_atchnet(device)                    +\
                   sqa_dtchnet(device)                    +\
                   localtime_monitor_steps(device, False) +\
                   sqa_strplcltm(device)                  +\
                   sqa_atchnet(device)                    +\
                   sqa_rplcltm(device)                    +\
                   sqa_dtchnet(device)                    +\
                   sqa_pwroff(device)                     +\
                   sqa_pwron(device)                      +\
                   localtime_monitor_steps(device, True)  +\
                   sqa_atchnet(device)                    +\
                   sqa_dtchnet(device)                    +\
                   localtime_monitor_steps(device, False) +\
                   sqa_strplcltm(device)                  +\
                   sqa_atchnet(device)                    +\
                   sqa_rplcltm(device)                    +\
                   loop_steps(10, sqa_gtlcltm(device))    +\
                   sqa_dtchnet(device)                    +\
                   sqa_fin(device)

        def sqa_operator_steps(device):
            return sqa_init(device)                    +\
                   sqa_pwron(device)                   +\
                   sqa_atchnet(device)                 +\
                   loop_steps(10, sqa_gtoprtr(device)) +\
                   sqa_dtchnet(device)                 +\
                   sqa_fin(device)

        def sqa_edrx_steps(device):
            return sqa_init(device)                            +\
                   sqa_pwron(device)                           +\
                   sqa_atchnet(device)                         +\
                   loop_steps(5,                                \
                       sqa_stedrx(device, True, "1024", "256") +\
                       sqa_gtedrx(device, True, "1024", "256") +\
                       sqa_stedrx(device, False)               +\
                       sqa_gtedrx(device, False)               +\
                       sqa_stedrx(device, True, "2048", "384") +\
                       sqa_gtedrx(device, True, "2048", "384") +\
                       sqa_stedrx(device, False)               +\
                       sqa_gtedrx(device, False))              +\
                   sqa_dtchnet(device)                         +\
                   sqa_fin(device)

        def sqa_psm_steps(device):
            return sqa_init(device)                                  +\
                   sqa_pwron(device)                                 +\
                   loop_steps(5,                                      \
                       sqa_stpsm(device, True, "1", "10", "1", "10") +\
                       sqa_gtpsm(device, True, "1", "10", "1", "10") +\
                       sqa_stpsm(device, True, "2", "20", "2", "20") +\
                       sqa_gtpsm(device, True, "2", "20", "2", "20") +\
                       sqa_stpsm(device, False)                      +\
                       sqa_gtpsm(device, False))                     +\
                   sqa_fin(device)

        def sqa_report_network_status_steps(device):
            return sqa_init(device)                     +\
                   sqa_pwron(device)                    +\
                   sqa_strpntst(device, True)           +\
                   sqa_atchnet(device, True)            +\
                   sqa_dataon(device, "2", True)        +\
                   sqa_dataoff(device, "2", True)       +\
                   sqa_dtchnet(device, True)            +\
                   sqa_strpntst(device, False)          +\
                   netstat_monitor_steps(device, True)  +\
                   sqa_atchnet(device)                  +\
                   sqa_dataon(device, "2")              +\
                   sqa_dataoff(device, "2")             +\
                   sqa_dtchnet(device)                  +\
                   netstat_monitor_steps(device, False) +\
                   sqa_strpntst(device, True)           +\
                   sqa_pwroff(device)                   +\
                   sqa_pwron(device)                    +\
                   netstat_monitor_steps(device, True)  +\
                   sqa_atchnet(device)                  +\
                   sqa_dataon(device, "2")              +\
                   sqa_dataoff(device, "2")             +\
                   sqa_dtchnet(device)                  +\
                   netstat_monitor_steps(device, False) +\
                   sqa_fin(device)

        def sqa_report_sim_status_steps(device):
            return sqa_init(device)                     +\
                   sqa_pwron(device)                    +\
                   sqa_stpinenb(device, True)           +\
                   sqa_pwroff(device)                   +\
                   sqa_pwron(device)                    +\
                   sqa_gtpinst(device, "1", "1")        +\
                   sqa_strpsmst(device, True)           +\
                   sqa_atchnet_pinenable(device, True)  +\
                   sqa_gtpinst(device, "1", "1")        +\
                   sqa_entpin(device)                   +\
                   sqa_smstrepo(device, "4")            +\
                   sqa_gtpinst(device, "1", "0")        +\
                   sqa_pwroff(device)                   +\
                   sqa_pwron(device)                    +\
                   simstat_monitor_steps(device, True)  +\
                   sqa_gtpinst(device, "1", "1")        +\
                   sqa_atchnet_pinenable(device, False) +\
                   sqa_gtpinst(device, "1", "1")        +\
                   sqa_entpin(device)                   +\
                   sqa_gtpinst(device, "1", "0")        +\
                   sqa_pwroff(device)                   +\
                   sqa_pwron(device)                    +\
                   sqa_gtpinst(device, "1", "1")        +\
                   sqa_strpsmst(device, False)          +\
                   sqa_atchnet_pinenable(device, False) +\
                   sqa_gtpinst(device, "1", "1")        +\
                   sqa_entpin(device)                   +\
                   sqa_gtpinst(device, "1", "0")        +\
                   sqa_stpinenb(device, False)          +\
                   sqa_gtpinst(device, "0", "0")        +\
                   simstat_monitor_steps(device, False) +\
                   sqa_fin(device)

        def sqa_report_sim_remove_steps(device):
            return sqa_init(device)                         +\
                   sqa_pwron(device)                        +\
                   sqa_strpsmst(device, True)               +\
                   sqa_atchnet(device)                      +\
                   wait_sec_steps(device, 3, "Remove sim")  +\
                   sqa_smstrepo(device, "0")                +\
                   wait_sec_steps(device, 20, "Insert sim") +\
                   sqa_dtchnet(device)                      +\
                   sqa_pwroff(device)                       +\
                   sqa_pwron(device)                        +\
                   simstat_monitor_steps(device, True)      +\
                   sqa_atchnet(device)                      +\
                   wait_sec_steps(device, 10, "Remove sim") +\
                   wait_sec_steps(device, 20, "Insert sim") +\
                   sqa_dtchnet(device)                      +\
                   sqa_pwroff(device)                       +\
                   sqa_pwron(device)                        +\
                   sqa_strpsmst(device, False)              +\
                   sqa_atchnet(device)                      +\
                   wait_sec_steps(device, 10, "Remove sim") +\
                   wait_sec_steps(device, 20, "Insert sim") +\
                   sqa_dtchnet(device)                      +\
                   simstat_monitor_steps(device, False)     +\
                   sqa_fin(device)

        def sqa_report_network_quality_steps(device):
            return sqa_init(device)                                   +\
                   sqa_pwron(device)                                  +\
                   sqa_atchnet(device)                                +\
                   sqa_strpqty(device, "1", True)                     +\
                   sqa_strpqty(device, "1", False)                    +\
                   quality_monitor_steps(device, True)                +\
                   wait_sec_steps(device, 5, "Stopping qtyrepo_cb?")  +\
                   quality_monitor_steps(device, False)               +\
                   sqa_strpqty(device, "10", True)                    +\
                   sqa_pwroff(device)                                 +\
                   sqa_pwron(device)                                  +\
                   quality_monitor_steps(device, True)                +\
                   wait_sec_steps(device, 10, "Stopping qtyrepo_cb?") +\
                   sqa_atchnet(device)                                +\
                   sqa_dtchnet(device)                                +\
                   quality_monitor_steps(device, False)               +\
                   sqa_fin(device)

        def sqa_report_cellinfo_steps(device):
            return sqa_init(device)                                   +\
                   sqa_pwron(device)                                  +\
                   sqa_atchnet(device)                                +\
                   sqa_strpclinf(device, "1", True)                   +\
                   sqa_strpclinf(device, "1", False)                  +\
                   cellinfo_monitor_steps(device, True)               +\
                   wait_sec_steps(device, 5, "Stopping qtyrepo_cb?")  +\
                   cellinfo_monitor_steps(device, False)              +\
                   sqa_strpclinf(device, "10", True)                  +\
                   sqa_pwroff(device)                                 +\
                   sqa_pwron(device)                                  +\
                   cellinfo_monitor_steps(device, True)               +\
                   wait_sec_steps(device, 10, "Stopping qtyrepo_cb?") +\
                   sqa_atchnet(device)                                +\
                   sqa_dtchnet(device)                                +\
                   cellinfo_monitor_steps(device, False)              +\
                   sqa_fin(device)

        sqa_power = Test(
            name='SDK_Lte_15984',
            timeout=timeout,
            setup=setup,
            test=sqa_power_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_attach = Test(
            name='SDK_Lte_15987',
            timeout=timeout,
            setup=setup,
            test=sqa_attach_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_data = Test(
            name='SDK_Lte_15994',
            timeout=timeout,
            setup=setup,
            test=sqa_data_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_dataconfig = Test(
            name='SDK_Lte_16002',
            timeout=timeout,
            setup=setup,
            test=sqa_dataconfig_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_apn = Test(
            name='SDK_Lte_16009',
            timeout=timeout,
            setup=setup,
            test=sqa_apn_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_version = Test(
            name='sqa_version',
            timeout=timeout,
            setup=setup,
            test=sqa_version_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_phoneno = Test(
            name='sqa_phoneno',
            timeout=timeout,
            setup=setup,
            test=sqa_phoneno_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_imsi = Test(
            name='SDK_Lte_16055',
            timeout=timeout,
            setup=setup,
            test=sqa_imsi_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_imei = Test(
            name='sqa_imei',
            timeout=timeout,
            setup=setup,
            test=sqa_imei_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_pin_enable = Test(
            name='SDK_Lte_16027',
            timeout=timeout,
            setup=setup,
            test=sqa_pin_enable_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_pin_change = Test(
            name='SDK_Lte_16044',
            timeout=timeout,
            setup=setup,
            test=sqa_pin_change_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_pin_enter = Test(
            name='SDK_Lte_16055',
            timeout=timeout,
            setup=setup,
            test=sqa_pin_enter_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_localtime = Test(
            name='sqa_localtime',
            timeout=timeout,
            setup=setup,
            test=sqa_localtime_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_operator = Test(
            name='sqa_operator',
            timeout=timeout,
            setup=setup,
            test=sqa_operator_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_edrx = Test(
            name='sqa_edrx',
            timeout=timeout,
            setup=setup,
            test=sqa_edrx_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_psm = Test(
            name='sqa_psm',
            timeout=timeout,
            setup=setup,
            test=sqa_psm_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_report_network_status = Test(
            name='sqa_report_network_status',
            timeout=timeout,
            setup=setup,
            test=sqa_report_network_status_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_report_sim_status = Test(
            name='sqa_report_sim_status',
            timeout=timeout,
            setup=setup,
            test=sqa_report_sim_status_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_report_sim_remove = Test(
            name='sqa_report_sim_remove',
            timeout=timeout,
            setup=setup,
            test=sqa_report_sim_remove_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_report_network_quality = Test(
            name='sqa_report_network_quality',
            timeout=timeout,
            setup=setup,
            test=sqa_report_network_quality_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_report_cellinfo = Test(
            name='sqa_report_cellinfo',
            timeout=timeout,
            setup=setup,
            test=sqa_report_cellinfo_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_phoneno_diff = Test(
            name='sqa_phoneno_diff',
            timeout=timeout,
            setup=setup,
            test=sqa_phoneno_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_apn_short = Test(
            name='SDK_Lte_16013',
            timeout=timeout,
            setup=setup,
            test=sqa_apn_short_steps(lteapi_test_device),
            teardown=teardown
        )

        sqa_lteapi_work = TestGroup(
            name='sqa_lteapi',
            devices=[lteapi_test_device],
            tag=['work'],
            tests=[
#                sqa_power,
#                sqa_attach,
#                sqa_data,
#                sqa_dataconfig,
#                sqa_apn,
#                sqa_version,
#                sqa_phoneno,
#                sqa_imsi,
#                sqa_imei,
#                sqa_pin_enable,
#                sqa_pin_change,
#                sqa_pin_enter,
#                sqa_localtime,
#                sqa_operator,
#                sqa_edrx,
#                sqa_psm,
#                sqa_report_network_status,
#                sqa_report_sim_status,
#                sqa_report_sim_remove,
#                sqa_report_network_quality,
#                sqa_report_cellinfo,
#                sqa_phoneno_diff,
            ]
        )

        sqa_lteapi_online = TestGroup(
            name='sqa_lteapi_online',
            devices=[lteapi_test_device],
            tag=['online'],
            tests=[
                sqa_attach,
                sqa_data,
                sqa_localtime,
                sqa_operator,
                sqa_edrx,
                sqa_report_network_status,
                sqa_report_sim_status,
                sqa_report_network_quality,
                sqa_report_cellinfo,
            ]
        )

        sqa_lteapi_offline = TestGroup(
            name='sqa_lteapi_offline',
            devices=[lteapi_test_device],
            tag=['offline'],
            tests=[
                sqa_power,
                sqa_dataconfig,
                sqa_apn,
                sqa_version,
                sqa_phoneno,
                sqa_imsi,
                sqa_imei,
                sqa_pin_enable,
                sqa_pin_change,
                sqa_pin_enter,
                sqa_psm,
            ]
        )

        sqa_lteapi_other = TestGroup(
            name='sqa_lteapi_other',
            devices=[lteapi_test_device],
            tag=['other'],
            tests=[
                sqa_report_sim_remove,
                sqa_phoneno_diff,
                sqa_apn_short,
            ]
        )

        test_groups = [
            sqa_lteapi_work,
            sqa_lteapi_offline,
            sqa_lteapi_online,
            sqa_lteapi_other,
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
    tc_runner = LteapiTestTcRunner(config, dut_device=dut_device)

    # Configure logger
    logger = TestLogger(log_name=tc_runner.name, console_debug=args.verbose)

    # Run test groups from main function arguments
    tc_runner.run(args, logger)
