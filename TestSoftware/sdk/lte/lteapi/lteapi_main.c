/****************************************************************************
 * test/lteapi/src/lteapi_main.c
 *
 *   Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/


/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>

#include <nuttx/arch.h>
#include <nuttx/sched.h>
#include <nuttx/mqueue.h>
#include <nuttx/config.h>
#include <sdk/config.h>

#include <sys/stat.h>

#include "lte/lte_api.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LTEAPI_CMD_APPNAME                  (0)
#define LTEAPI_CMD_APINAME                  (1)
#define LTEAPI_CMD_APIPARAM_1               (2)
#define LTEAPI_CMD_APIPARAM_2               (3)
#define LTEAPI_CMD_APIPARAM_3               (4)
#define LTEAPI_CMD_APIPARAM_4               (5)
#define LTEAPI_CMD_APIPARAM_5               (6)
#define LTEAPI_CMD_APIPARAM_6               (7)
#define LTEAPI_CMD_APIPARAM_7               (8)

#define LTEAPI_MAX_NUMBER_OF_PARAM         (12)
#define LTEAPI_MAX_API_MQUEUE              (16)
#define LTEAPI_MQUEUE_MODE               (0666)

#define LTEAPI_TASK_STACKSIZE            (2048)
#define LTEAPI_TASK_PRI                   (100)
#define LTEAPI_PINCODE_LEN                  (9)
#define LTEAPI_INVALID_VALUE               (99)
#define LTEAPI_STRTOL_BASE                 (10)
#define LTEAPI_STRTOL_BASE_HEX             (16)
#define LTEAPI_MQUEUE_PATH         ("lte_test")
#define LTEAPI_INVALID_ARG          ("INVALID")
#define LTEAPI_NULL_ARG                ("NULL")

#if defined(CONFIG_NSH_MAXARGUMENTS)
#  if (CONFIG_NSH_MAXARGUMENTS < 8)
#    error CONFIG_NSH_MAXARGUMENTS too small
#  endif
#endif

#if defined(CONFIG_NSH_LINELEN)
#  if (CONFIG_NSH_LINELEN < 255)
#    error CONFIG_NSH_LINELEN too small
#  endif
#endif

#define LTEAPI_GETFUNCNAME(func) ( #func )

#define LTEAPI_PRINT_RESULT(result) \
  do \
    { \
      show_result(result, __func__); \
    } \
  while(0)

#define LTEAPI_PRINT_ERROR(result, errcause) \
  do \
    { \
      show_error(result, errcause, __func__); \
    } \
  while(0)

#define LTEAPI_PRINT_STATE(result, state) \
  do \
    { \
      show_state(result, state, __func__); \
  } \
  while(0)

#define LTEAPI_TABLESIZE (sizeof(g_testfunt_table)/sizeof(g_testfunt_table[0]))

#define GET_STRING(val, name) (val < (sizeof(name)/sizeof(name[0]))? \
                               (name[val]) : ("NOTHING"))

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct lteapi_command_s
{
  int   argc;
  FAR char *argv[LTEAPI_MAX_NUMBER_OF_PARAM];
};

typedef int32_t(*lteapi_testfunc_t)(struct lteapi_command_s *command);
typedef void(*lteapi_help_t)(char* cmdname);

typedef struct lteapi_test_tbl_s
{
  char              cmd[64];
  lteapi_testfunc_t api_func;
  lteapi_help_t     help_func;
} test_tbl_t;

/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

static int32_t lteapi_help(struct lteapi_command_s *command);
static int32_t lteapi_test_init(struct lteapi_command_s *command);
static int32_t lteapi_test_fin(struct lteapi_command_s *command);

#ifdef CONFIG_TEST_LTEAPI_ENABLE_OLDAPI
static int32_t lteapi_test_pwrctl_on(struct lteapi_command_s *command);
static int32_t lteapi_test_pwrctl_off(struct lteapi_command_s *command);
static int32_t lteapi_test_atch(struct lteapi_command_s *command);
static int32_t lteapi_test_dtch(struct lteapi_command_s *command);
static int32_t lteapi_test_getnetstat(struct lteapi_command_s *command);
static int32_t lteapi_test_dataon(struct lteapi_command_s *command);
static int32_t lteapi_test_dataoff(struct lteapi_command_s *command);
static int32_t lteapi_test_getdtst(struct lteapi_command_s *command);
static int32_t lteapi_test_getdtcfg(struct lteapi_command_s *command);
static int32_t lteapi_test_setdtcfg(struct lteapi_command_s *command);
static int32_t lteapi_test_setapn(struct lteapi_command_s *command);
static int32_t lteapi_test_getapnset(struct lteapi_command_s *command);
static int32_t lteapi_test_setrepnetstat(struct lteapi_command_s *command);
#endif

static int32_t lteapi_test_getver(struct lteapi_command_s *command);
static int32_t lteapi_test_getphoneno(struct lteapi_command_s *command);
static int32_t lteapi_test_getimei(struct lteapi_command_s *command);
static int32_t lteapi_test_getimsi(struct lteapi_command_s *command);
static int32_t lteapi_test_getpinst(struct lteapi_command_s *command);
static int32_t lteapi_test_setpinenb(struct lteapi_command_s *command);
static int32_t lteapi_test_chgpin(struct lteapi_command_s *command);
static int32_t lteapi_test_entpin(struct lteapi_command_s *command);
static int32_t lteapi_test_getltime(struct lteapi_command_s *command);
static int32_t lteapi_test_getope(struct lteapi_command_s *command);
static int32_t lteapi_test_getedrx(struct lteapi_command_s *command);
static int32_t lteapi_test_setedrx(struct lteapi_command_s *command);
static int32_t lteapi_test_getpsm(struct lteapi_command_s *command);
static int32_t lteapi_test_setpsm(struct lteapi_command_s *command);
static int32_t lteapi_test_setrepsimstat(struct lteapi_command_s *command);
static int32_t lteapi_test_setrepltime(struct lteapi_command_s *command);
static int32_t lteapi_test_setrepqlty(struct lteapi_command_s *command);
static int32_t lteapi_test_setrepcelinf(struct lteapi_command_s *command);
static int32_t lteapi_test_setce(struct lteapi_command_s *command);
static int32_t lteapi_test_getce(struct lteapi_command_s *command);
static int32_t lteapi_test_poweron(struct lteapi_command_s *command);
static int32_t lteapi_test_powerff(struct lteapi_command_s *command);
static int32_t lteapi_test_setrestart(struct lteapi_command_s *command);
static int32_t lteapi_test_radioon(struct lteapi_command_s *command);
static int32_t lteapi_test_radiooff(struct lteapi_command_s *command);
static int32_t lteapi_test_actpdn(struct lteapi_command_s *command);
static int32_t lteapi_test_actpdn_cancel(struct lteapi_command_s *command);
static int32_t lteapi_test_deactpdn(struct lteapi_command_s *command);
static int32_t lteapi_test_getnetinfo(struct lteapi_command_s *command);
static int32_t lteapi_test_getimscap(struct lteapi_command_s *command);
static int32_t lteapi_test_dataallow(struct lteapi_command_s *command);
static int32_t lteapi_test_repnetinfo(struct lteapi_command_s *command);
static int32_t lteapi_test_getsiminfo(struct lteapi_command_s *command);
static int32_t lteapi_test_getdynamicedrx(struct lteapi_command_s *command);
static int32_t lteapi_test_getdynamicpsm(struct lteapi_command_s *command);
static int32_t lteapi_test_getquality(struct lteapi_command_s *command);
static int32_t  lteapi_test_geterr(struct lteapi_command_s *command);

static void help_actpdn(char *cmdname);
static void help_deactpdn(char *cmdname);
static void help_dataallow(char *cmdname);
static void help_getsiminfo(char *cmdname);
static void help_setpsm(char *cmdname);
static void help_setedrx(char *cmdname);
static void help_setce(char *cmdname);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static bool g_taskrunning;

static test_tbl_t g_testfunt_table[] =
{
  { "help",           lteapi_help,                NULL            },
  { "init",           lteapi_test_init,           NULL            },
  { "fin",            lteapi_test_fin,            NULL            },
#ifdef CONFIG_TEST_LTEAPI_ENABLE_OLDAPI
  { "pwron",          lteapi_test_pwrctl_on,      NULL            },
  { "pwroff",         lteapi_test_pwrctl_off,     NULL            },
  { "atchnet",        lteapi_test_atch,           NULL            },
  { "dtchnet",        lteapi_test_dtch,           NULL            },
  { "gtntst",         lteapi_test_getnetstat,     NULL            },
  { "dataon",         lteapi_test_dataon,         NULL            },
  { "dataoff",        lteapi_test_dataoff,        NULL            },
  { "gtdtst",         lteapi_test_getdtst,        NULL            },
  { "gtdtcfg",        lteapi_test_getdtcfg,       NULL            },
  { "stdtcfg",        lteapi_test_setdtcfg,       NULL            },
  { "stapn",          lteapi_test_setapn,         NULL            },
  { "gtapnst",        lteapi_test_getapnset,      NULL            },
  { "strpntst",       lteapi_test_setrepnetstat,  NULL            },
#endif
  { "gtver",          lteapi_test_getver,         NULL            },
  { "gtphno",         lteapi_test_getphoneno,     NULL            },
  { "gtimei",         lteapi_test_getimei,        NULL            },
  { "gtimsi",         lteapi_test_getimsi,        NULL            },
  { "gtpinst",        lteapi_test_getpinst,       NULL            },
  { "stpinenb",       lteapi_test_setpinenb,      NULL            },
  { "cngpin",         lteapi_test_chgpin,         NULL            },
  { "entpin",         lteapi_test_entpin,         NULL            },
  { "gtlcltm",        lteapi_test_getltime,       NULL            },
  { "gtoprtr",        lteapi_test_getope,         NULL            },
  { "getedrx",        lteapi_test_getedrx,        NULL            },
  { "setedrx",        lteapi_test_setedrx,        help_setedrx    },
  { "getpsm",         lteapi_test_getpsm,         NULL            },
  { "setpsm",         lteapi_test_setpsm,         help_setpsm     },
  { "repsimstat",     lteapi_test_setrepsimstat,  NULL            },
  { "strplcltm",      lteapi_test_setrepltime,    NULL            },
  { "strpqty",        lteapi_test_setrepqlty,     NULL            },
  { "strpclinf",      lteapi_test_setrepcelinf,   NULL            },
  { "setce",          lteapi_test_setce,          help_setce      },
  { "getce",          lteapi_test_getce,          NULL            },
  { "poweron",        lteapi_test_poweron,        NULL            },
  { "poweroff",       lteapi_test_powerff,        NULL            },
  { "restart",        lteapi_test_setrestart,     NULL            },
  { "radioon",        lteapi_test_radioon,        NULL            },
  { "radiooff",       lteapi_test_radiooff,       NULL            },
  { "actpdn",         lteapi_test_actpdn,         help_actpdn     },
  { "cancel",         lteapi_test_actpdn_cancel,  NULL            },
  { "deactpdn",       lteapi_test_deactpdn,       help_deactpdn   },
  { "getnetinfo",     lteapi_test_getnetinfo,     NULL            },
  { "getimscap",      lteapi_test_getimscap,      NULL            },
  { "dataallow",      lteapi_test_dataallow,      help_dataallow  },
  { "repnetinfo",     lteapi_test_repnetinfo,     NULL            },
  { "getsiminfo",     lteapi_test_getsiminfo,     help_getsiminfo },
  { "getdynamicedrx", lteapi_test_getdynamicedrx, NULL            },
  { "getdynamicpsm",  lteapi_test_getdynamicpsm,  NULL            },
  { "getquality",     lteapi_test_getquality,     NULL            },
  { "geterr",         lteapi_test_geterr,         NULL            }
};

static const char *def_edrx_cyc[] = {
  "LTE_EDRX_CYC_512",
  "LTE_EDRX_CYC_1024",
  "LTE_EDRX_CYC_2048",
  "LTE_EDRX_CYC_4096",
  "LTE_EDRX_CYC_6144",
  "LTE_EDRX_CYC_8192",
  "LTE_EDRX_CYC_10240",
  "LTE_EDRX_CYC_12288",
  "LTE_EDRX_CYC_14336",
  "LTE_EDRX_CYC_16384",
  "LTE_EDRX_CYC_32768",
  "LTE_EDRX_CYC_65536",
  "LTE_EDRX_CYC_131072",
  "LTE_EDRX_CYC_262144"
};

static const char *def_ptw_val[] = {
  "LTE_EDRX_PTW_128",
  "LTE_EDRX_PTW_256",
  "LTE_EDRX_PTW_384",
  "LTE_EDRX_PTW_512",
  "LTE_EDRX_PTW_640",
  "LTE_EDRX_PTW_768",
  "LTE_EDRX_PTW_896",
  "LTE_EDRX_PTW_1024",
  "LTE_EDRX_PTW_1152",
  "LTE_EDRX_PTW_1280",
  "LTE_EDRX_PTW_1408",
  "LTE_EDRX_PTW_1536",
  "LTE_EDRX_PTW_1664",
  "LTE_EDRX_PTW_1792",
  "LTE_EDRX_PTW_1920",
  "LTE_EDRX_PTW_2048"
};

static const char *def_activetime_unit[] = {
  "LTE_PSM_T3324_UNIT_2SEC",
  "LTE_PSM_T3324_UNIT_1MIN",
  "LTE_PSM_T3324_UNIT_6MIN",
  "LTE_PSM_T3324_UNIT_DEACT"
};

static const char *def_extperiodictautime_unit[] = {
  "LTE_PSM_T3412_UNIT_2SEC",
  "LTE_PSM_T3412_UNIT_30SEC",
  "LTE_PSM_T3412_UNIT_1MIN",
  "LTE_PSM_T3412_UNIT_10MIN",
  "LTE_PSM_T3412_UNIT_1HOUR",
  "LTE_PSM_T3412_UNIT_10HOUR",
  "LTE_PSM_T3412_UNIT_320HOUR",
  "LTE_PSM_T3412_UNIT_DEACT"
};

static const char *def_netinfo_stat[] = {
  "LTE_NETSTAT_NOT_REG_NOT_SEARCHING",
  "LTE_NETSTAT_REG_HOME",
  "LTE_NETSTAT_NOT_REG_SEARCHING",
  "LTE_NETSTAT_REG_DENIED",
  "LTE_NETSTAT_UNKNOWN",
  "LTE_NETSTAT_REG_ROAMING",
  "LTE_NETSTAT_REG_SMS_ONLY_HOME",
  "LTE_NETSTAT_REG_SMS_ONLY_ROAMING",
  "LTE_NETSTAT_NOT_REG_EMERGENCY",
  "LTE_NETSTAT_REG_CSFB_NOT_PREF_HOME",
  "LTE_NETSTAT_REG_CSFB_NOT_PREF_ROAMING"
};

static const char *def_report_simstat[] = {
  "LTE_SIMSTAT_REMOVAL",
  "LTE_SIMSTAT_INSERTION",
  "LTE_SIMSTAT_WAIT_PIN_UNLOCK",
  "LTE_SIMSTAT_PERSONAL_FAILED",
  "LTE_SIMSTAT_ACTIVATE",
  "LTE_SIMSTAT_DEACTIVATE"
};

static const char *def_netinfo_errtype[] =
{
  "LTE_NETERR_MAXRETRY",
  "LTE_NETERR_REJECT",
  "LTE_NETERR_NWDTCH"
};

static const char *def_netinfo_rejectcategory[] =
{
  "LTE_REJECT_CATEGORY_EMM",
  "LTE_REJECT_CATEGORY_ESM"
};

static const char *def_gtpinst_status[] =
{
  "LTE_PINSTAT_READY",
  "LTE_PINSTAT_SIM_PIN",
  "LTE_PINSTAT_SIM_PUK",
  "LTE_PINSTAT_PH_SIM_PIN",
  "LTE_PINSTAT_PH_FSIM_PIN",
  "LTE_PINSTAT_PH_FSIM_PUK",
  "LTE_PINSTAT_SIM_PIN2",
  "LTE_PINSTAT_SIM_PUK2",
  "LTE_PINSTAT_PH_NET_PIN",
  "LTE_PINSTAT_PH_NET_PUK",
  "LTE_PINSTAT_PH_NETSUB_PIN",
  "LTE_PINSTAT_PH_NETSUB_PUK",
  "LTE_PINSTAT_PH_SP_PIN",
  "LTE_PINSTAT_PH_SP_PUK",
  "LTE_PINSTAT_PH_CORP_PIN",
  "LTE_PINSTAT_PH_CORP_PUK"
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void show_result(uint32_t result, FAR const char *funcname)
{
  if (LTE_RESULT_OK == result)
    {
      printf("[CB_OK] %s result : \"%d\"\n", funcname, result);
    }
  else
    {
      printf("[CB_NG] %s result : \"%d\"\n", funcname, result);
    }
}

static void show_error(uint32_t result, uint32_t errcause
  , FAR const char *funcname)
{
  if (LTE_RESULT_ERROR == result)
    {
      printf("[CB_NG] %s error : \"%d\"\n", funcname, errcause);
    }
}

#ifdef CONFIG_TEST_LTEAPI_ENABLE_OLDAPI
static void show_state(uint32_t result, uint32_t state
  , FAR const char *funcname)
{
  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s state : \"%d\"\n", funcname, state);
    }
}
#endif
static void show_pdn(lte_pdn_t *pdn, const char *call_func)
{
  int i;
  printf("[CB_VAL] %s pdn.session_id : \"%d\"\n", call_func, pdn->session_id);
  printf("[CB_VAL] %s pdn.active : \"%d\" -> \"%s\"\n"
         , call_func, pdn->active
         , ((LTE_PDN_ACTIVE == pdn->active)?
           "LTE_PDN_ACTIVE" : "LTE_PDN_DEACTIVE"));

  printf("[CB_VAL] %s pdn.apn_type : \"%x\"\n", call_func, pdn->apn_type);
  printf("[CB_VAL] %s pdn.ipaddr_num : \"%d\"\n", call_func, pdn->ipaddr_num);
  for (i = 0; i < pdn->ipaddr_num; i++)
    {
      printf("[CB_VAL] %s pdn.ipaddr[%d].ip_type : \"%d\" -> \"%s\"\n"
             , call_func
             , i
             , pdn->address[i].ip_type
             , ((LTE_IPTYPE_V4 == pdn->address[i].ip_type)?
               "LTE_IPTYPE_V4" : "LTE_IPTYPE_V6"));
      printf("[CB_VAL] %s pdn.ipaddr[%d].addr : \"%s\"\n"
             , call_func
             , i
             , pdn->address[i].address);
    }
  printf("[CB_VAL] %s pdn.ims_register : \"%d\" -> \"%s\"\n"
         , call_func
         , pdn->ims_register
         , ((LTE_IMS_REGISTERED == pdn->ims_register)?
           "LTE_IMS_REGISTERED" : "LTE_IMS_NOT_REGISTERED"));
  printf("[CB_VAL] %s pdn.data_allow : \"%d\" -> \"%s\"\n"
         , call_func, pdn->data_allow
         , ((LTE_DATA_ALLOW == pdn->data_allow)?
           "LTE_DATA_ALLOW" : "LTE_DATA_DISALLOW"));
  printf("[CB_VAL] %s pdn.data_roaming_allow : \"%d\" -> \"%s\"\n"
         , call_func, pdn->data_roaming_allow
         , ((LTE_DATA_ALLOW == pdn->data_roaming_allow)?
           "LTE_DATA_ALLOW" : "LTE_DATA_DISALLOW"));
}

static int32_t lteapi_help(struct lteapi_command_s *command)
{
  uint32_t i = 0;

  printf("** LTE API's Valid Commands.\n");

  for (i = 0; i < LTEAPI_TABLESIZE; i++)
    {
      printf("** %s\n", g_testfunt_table[i].cmd);
    }

  printf("\n*********************************************\n");
  return 0;
}

static void help_actpdn(char *cmdname)
{
  printf("*** Usage [lte_activate_pdn] ***\n");
  printf("** Command : %s <apn> <apn_type> <ip_type> <auth_type> <user_name> <password>\n", cmdname);
  printf("** Arguments detail\n");
  printf("** apn : <string> max length is 100.\n");
  printf("** apn_type : <integer> Can set some types.\n");
  printf("** type LTE_APN_TYPE_UNKNOWN   : 0x01\n");
  printf("** type LTE_APN_TYPE_DEFAULT   : 0x02\n");
  printf("** type LTE_APN_TYPE_MMS       : 0x04\n");
  printf("** type LTE_APN_TYPE_SUPL      : 0x08\n");
  printf("** type LTE_APN_TYPE_DUN       : 0x10\n");
  printf("** type LTE_APN_TYPE_HIPRI     : 0x20\n");
  printf("** type LTE_APN_TYPE_FOTA      : 0x40\n");
  printf("** type LTE_APN_TYPE_IMS       : 0x80\n");
  printf("** type LTE_APN_TYPE_CBS       : 0x100\n");
  printf("** type LTE_APN_TYPE_IA        : 0x200\n");
  printf("** type LTE_APN_TYPE_EMERGENCY : 0x400\n");
  printf("** ip_type : <integer>\n");
  printf("** type LTE_APN_IPTYPE_IP     : 0\n");
  printf("** type LTE_APN_IPTYPE_IPV6   : 1\n");
  printf("** type LTE_APN_IPTYPE_IPV4V6 : 2\n");
  printf("** auth_type : <integer>\n");
  printf("** type LTE_APN_AUTHTYPE_NONE : 0\n");
  printf("** type LTE_APN_AUTHTYPE_PAP  : 1\n");
  printf("** type LTE_APN_AUTHTYPE_CHAP : 2\n");
  printf("** user_name : <string> max length is 63.\n");
  printf("** password : <string> max length is 31.\n");
  printf("*********************************************\n");
}

static void help_deactpdn(char *cmdname)
{
  printf("*** Usage [lte_deactivate_pdn]\n");
  printf("** Command : %s <session_id>\n", cmdname);
  printf("** Arguments detail\n");
  printf("** session_id : <integer> Deactivate target session id.\n");
  printf("** Check activated pdn session is use [getnetinfo] command.\n");
  printf("*********************************************\n");
}

static void help_dataallow(char *cmdname)
{
  printf("*** Usage [lte_data_allow]\n");
  printf("** Command : %s <session_id> <data_allow> <data_roam_allow>\n",
    cmdname);
  printf("** Arguments detail\n");
  printf("** session_id : <integer> Deactivate target session id.\n");
  printf("** Check activated pdn session is use [getnetinfo] command.\n");
  printf("** data_allow : <integer>\n");
  printf("** LTE_DATA_DISALLOW : 0\n");
  printf("** LTE_DATA_ALLOW    : 1\n");
  printf("** data_roam_allow : <integer>\n");
  printf("** LTE_DATA_DISALLOW : 0\n");
  printf("** LTE_DATA_ALLOW    : 1\n");
  printf("*********************************************\n");
}

static void help_getsiminfo(char *cmdname)
{
  printf("*** Usage [lte_get_siminfo]\n");
  printf("** Command : %s <option>\n", cmdname);
  printf("** Arguments detail\n");
  printf("** option : <integer> Indicates which parameter to get.\n");
  printf("** Bit setting definition is as below.\n");
  printf("** LTE_SIMINFO_GETOPT_MCCMNC  : 0x01\n");
  printf("** LTE_SIMINFO_GETOPT_SPN     : 0x02\n");
  printf("** LTE_SIMINFO_GETOPT_ICCID   : 0x04\n");
  printf("** LTE_SIMINFO_GETOPT_IMSI    : 0x08\n");
  printf("** LTE_SIMINFO_GETOPT_GID1    : 0x16\n");
  printf("** LTE_SIMINFO_GETOPT_GID2    : 0x32\n");
  printf("*********************************************\n");

}

static void help_setpsm(char *cmdname)
{
  printf("*** Usage [lte_set_psm]\n");
  printf("** Command : %s <enable> <req_active_time.unit> <req_active_time.time_val> ",
    cmdname);
  printf("<ext_periodic_tau_time.unit> <ext_periodic_tau_time.time_val>\n");
  printf("** Arguments detail\n");
  printf("** enable  : <integer>\n");
  printf("** LTE_DISABLE : 0\n");
  printf("** LTE_ENABLE  : 1\n");
  printf("** unit : <integer> Unit of timer value. Definition is as below.\n");
  printf("** When kind of timer is Requested Active Time.\n");
  printf("** LTE_PSM_T3324_UNIT_2SEC  : 0\n");
  printf("** LTE_PSM_T3324_UNIT_1MIN  : 1\n");
  printf("** LTE_PSM_T3324_UNIT_6MIN  : 2\n");
  printf("** When kind of timer is Extended periodic TAU Time.\n");
  printf("** LTE_PSM_T3412_UNIT_2SEC  : 0\n");
  printf("** LTE_PSM_T3412_UNIT_30SEC : 1\n");
  printf("** LTE_PSM_T3412_UNIT_1MIN  : 2\n");
  printf("** LTE_PSM_T3412_UNIT_10MIN : 3\n");
  printf("** LTE_PSM_T3412_UNIT_1HOUR : 4\n");
  printf("** time_val : <integer> Timer value (1-31) .\n");
  printf("*********************************************\n");
}

static void help_setedrx(char *cmdname)
{
  printf("*** Usage [lte_set_edrx]\n");
  printf("** Command : %s <act_type> <enable> <edrx_cycle> <ptw_val>\n",
    cmdname);
  printf("** Arguments detail\n");
  printf("** act_type : <integer> Definition is as below.\n");
  printf("** LTE_EDRX_ACTTYPE_WBS1  : 0\n");
  printf("** enable : <integer>\n");
  printf("** LTE_DISABLE : 0\n");
  printf("** LTE_ENABLE  : 1\n");
  printf("** edrx_cycle : eDRX cycle. Definition is as below.\n");
  printf("** LTE_EDRX_CYC_512     : 0\n");
  printf("** LTE_EDRX_CYC_1024    : 1\n");
  printf("** LTE_EDRX_CYC_2048    : 2\n");
  printf("** LTE_EDRX_CYC_4096    : 3\n");
  printf("** LTE_EDRX_CYC_6144    : 4\n");
  printf("** LTE_EDRX_CYC_8192    : 5\n");
  printf("** LTE_EDRX_CYC_10240   : 6\n");
  printf("** LTE_EDRX_CYC_12288   : 7\n");
  printf("** LTE_EDRX_CYC_14336   : 8\n");
  printf("** LTE_EDRX_CYC_16384   : 9\n");
  printf("** LTE_EDRX_CYC_32768   : 10\n");
  printf("** LTE_EDRX_CYC_65536   : 11\n");
  printf("** LTE_EDRX_CYC_131072  : 12\n");
  printf("** LTE_EDRX_CYC_262144  : 13\n");
  printf("** ptw_val : Paging time window. Definition is as below.\n");
  printf("** LTE_EDRX_PTW_128   : 0\n");
  printf("** LTE_EDRX_PTW_256   : 1\n");
  printf("** LTE_EDRX_PTW_384   : 2\n");
  printf("** LTE_EDRX_PTW_512   : 3\n");
  printf("** LTE_EDRX_PTW_640   : 4\n");
  printf("** LTE_EDRX_PTW_768   : 5\n");
  printf("** LTE_EDRX_PTW_896   : 6\n");
  printf("** LTE_EDRX_PTW_1024  : 7\n");
  printf("** LTE_EDRX_PTW_1152  : 8\n");
  printf("** LTE_EDRX_PTW_1280  : 9\n");
  printf("** LTE_EDRX_PTW_1408  : 10\n");
  printf("** LTE_EDRX_PTW_1536  : 11\n");
  printf("** LTE_EDRX_PTW_1664  : 12\n");
  printf("** LTE_EDRX_PTW_1792  : 13\n");
  printf("** LTE_EDRX_PTW_1920  : 14\n");
  printf("** LTE_EDRX_PTW_2048  : 15\n");
  printf("*********************************************\n");
}

static void help_setce(char *cmdname)
{
  printf("*** Usage [lte_set_ce]\n");
  printf("** Command : %s <enable(mode_a)> <enable(mode_b)>\n",
    cmdname);
  printf("** Arguments detail\n");
  printf("** enable : <integer>\n");
  printf("** LTE_DISABLE : 0\n");
  printf("** LTE_ENABLE  : 1\n");
  printf("*********************************************\n");
}

static bool cb_null_chk(struct lteapi_command_s *command)
{
  if (3 <= command->argc
    && 0 == strncmp(command->argv[(command->argc - 1)]
      , LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
    {
      return true;
    }

  return false;
}

static void print_retval(int32_t val, FAR char *str)
{
  if (0 != val)
    {
      printf("[API_NG] %s return val : \"%d\"\n",str, val);
    }
  else
    {
      printf("[API_OK] %s return val : \"%d\"\n",str, val);
    }
}

static int32_t lteapi_test_init(struct lteapi_command_s *command)
{
  int32_t ret = lte_initialize();
  print_retval(ret, LTEAPI_GETFUNCNAME(lte_initialize));
  return ret;
}

static int32_t lteapi_test_fin(struct lteapi_command_s *command)
{
  int32_t ret = lte_finalize();
  print_retval(ret, LTEAPI_GETFUNCNAME(lte_finalize));
  return ret;
}

#ifdef CONFIG_TEST_LTEAPI_ENABLE_OLDAPI
static void pwron_cb(uint32_t result)
{
  LTEAPI_PRINT_RESULT(result);
}

static int32_t lteapi_test_pwrctl_on(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_power_control(LTE_POWERON, NULL);

    }
  else
    {
      ret = lte_power_control(LTE_POWERON, CODE pwron_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_power_control));
  return ret;
}

static void pwroff_cb(uint32_t result)
{
  LTEAPI_PRINT_RESULT(result);
}

static int32_t lteapi_test_pwrctl_off(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_power_control(LTE_POWEROFF, NULL);
    }
  else
    {
      ret = lte_power_control(LTE_POWEROFF, CODE pwroff_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_power_control));
  return ret;
}

static void atchnet_cb(uint32_t result, uint32_t errcause)
{
  LTEAPI_PRINT_RESULT(result);
  LTEAPI_PRINT_ERROR(result, errcause);
}

static int32_t lteapi_test_atch(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_attach_network(NULL);
    }
  else
    {
      ret = lte_attach_network(CODE atchnet_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_attach_network));
  return ret;
}

static void dtchnet_cb(uint32_t result)
{
  LTEAPI_PRINT_RESULT(result);
}

static int32_t lteapi_test_dtch(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_detach_network(NULL);
    }
  else
    {
      ret = lte_detach_network(CODE dtchnet_cb);
    }

  print_retval(ret ,LTEAPI_GETFUNCNAME(lte_detach_network));
  return ret;
}

static void getnetst_cb(uint32_t result, uint32_t state)
{
  LTEAPI_PRINT_RESULT(result);
  LTEAPI_PRINT_STATE(result, state);
}

static int32_t lteapi_test_getnetstat(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_get_netstat(NULL);
    }
  else
    {
      ret = lte_get_netstat(CODE getnetst_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_netstat));
  return ret;
}

static void daton_cb(uint32_t result, uint32_t errcause)
{
  LTEAPI_PRINT_RESULT(result);
  LTEAPI_PRINT_ERROR(result, errcause);
}

static int32_t lteapi_test_dataon(struct lteapi_command_s *command)
{
  int32_t ret = -EINVAL;
  uint8_t session_id = 0;

  if (3 <= command->argc)
    {
      if ((0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG))))
        {
          session_id = LTEAPI_INVALID_VALUE;
        }
      else
        {
          session_id = (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_1],
            NULL, LTEAPI_STRTOL_BASE);
        }

      if (4 <= command->argc
        && 0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
          LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_data_on(session_id, NULL);
        }
      else
        {
          ret = lte_data_on(session_id, CODE daton_cb);
        }
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_data_on));
  return ret;
}

static void datoff_cb(uint32_t result, uint32_t errcause)
{
  LTEAPI_PRINT_RESULT(result);
  LTEAPI_PRINT_ERROR(result, errcause);
}

static int32_t lteapi_test_dataoff(struct lteapi_command_s *command)
{
  int32_t ret = -EINVAL;
  uint8_t session_id = 0;

  if (3 <= command->argc)
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG)))
        {
          session_id = LTEAPI_INVALID_VALUE;
        }
      else
        {
          session_id = (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_1],
            NULL, LTEAPI_STRTOL_BASE);
        }

      if (4 <= command->argc
        && 0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
          LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_data_off(session_id, NULL);
        }
      else
        {
          ret = lte_data_off(session_id, CODE datoff_cb);
        }
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_data_off));
  return ret;
}

static void getdtst_cb(uint32_t result, FAR lte_getdatastat_t *datastate)
{
  int i = 0;

  LTEAPI_PRINT_RESULT(result);

  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s listnum : \"%d\"\n"
             , __func__
             , (datastate->listnum));
      for (i = 0; i < (datastate->listnum); i++)
        {
          printf("[CB_VAL] %s state [session_id] : \"%d [%d]\"\n"
                 , __func__
                 , datastate->statelist[i].state
                 , datastate->statelist[i].session_id);
        }
    }
}

static int32_t lteapi_test_getdtst(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_get_datastat(NULL);
    }
  else
    {
      ret = lte_get_datastat(CODE getdtst_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_datastat));
  return ret;
}

static void getdtcfg_cb(uint32_t result, uint32_t data_type,
  bool general, bool roaming)
{
  LTEAPI_PRINT_RESULT(result);
  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s data_type : \"%d\"\n", __func__, data_type);
      printf("[CB_VAL] %s general : \"%d\"\n", __func__, general);
      printf("[CB_VAL] %s roaming : \"%d\"\n", __func__, roaming);
    }
}

static int32_t lteapi_test_getdtcfg(struct lteapi_command_s *command)
{
  int32_t ret = -EINVAL;
  uint32_t data_type;

  if (3 <= command->argc)
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        "LTE_DATA_TYPE_USER", strlen("LTE_DATA_TYPE_USER")))
        {
          data_type = LTE_DATA_TYPE_USER;
        }
      else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        "LTE_DATA_TYPE_IMS", strlen("LTE_DATA_TYPE_IMS")))
        {
          data_type = LTE_DATA_TYPE_IMS;
        }
      else
        {
          data_type = LTE_DATA_TYPE_IMS + 1;
        }

      if (4 <= command->argc
        && 0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
          LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_get_dataconfig(data_type, NULL);
        }
      else
        {
          ret = lte_get_dataconfig(data_type,CODE getdtcfg_cb);
        }
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_dataconfig));
  return ret;
}

static void setdtcfg_cb(uint32_t result)
{
  LTEAPI_PRINT_RESULT(result);
}

static int32_t lteapi_test_setdtcfg(struct lteapi_command_s *command)
{
  int32_t ret = -EINVAL;
  bool     general = false;
  bool     roaming = false;
  uint32_t data_type;

  if (5 <= command->argc)
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        "LTE_DATA_TYPE_USER", strlen("LTE_DATA_TYPE_USER")))
        {
          data_type = LTE_DATA_TYPE_USER;
        }
      else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        "LTE_DATA_TYPE_IMS", strlen("LTE_DATA_TYPE_IMS")))
        {
          data_type = LTE_DATA_TYPE_IMS;
        }
      else
        {
          data_type = LTE_DATA_TYPE_IMS + 1;
        }

      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
        "LTE_ENABLE", strlen("LTE_ENABLE")))
        {
          general = LTE_ENABLE;
        }
      else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
        "LTE_DISABLE", strlen("LTE_DISABLE")))
        {
          general = LTE_DISABLE;
        }
      else
        {
          general = LTE_DISABLE + 1;
        }

      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_3],
        "LTE_ENABLE", strlen("LTE_ENABLE")))
        {
          roaming = LTE_ENABLE;
        }
      else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_3],
        "LTE_DISABLE", strlen("LTE_DISABLE")))
        {
          roaming = LTE_DISABLE;
        }
      else
        {
          roaming = LTE_DISABLE + 1;
        }

      if (6 <= command->argc
          && 0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_4],
            LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_set_dataconfig(data_type, general, roaming, NULL);
        }
      else
        {
          ret = lte_set_dataconfig(data_type, general, roaming, CODE setdtcfg_cb);
        }
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_set_dataconfig));
  return ret;
}

static void setapn_cb(uint32_t result)
{
  LTEAPI_PRINT_RESULT(result);
}

static int32_t lteapi_test_setapn(struct lteapi_command_s *command)
{
  uint8_t    session_id       = 0;
  uint8_t    ip_type;
  uint8_t    auth_type;
  int8_t     apnname_tmp[102] = {0};
  int8_t     usrname_tmp[65]  = {0};
  int8_t     passwrd_tmp[32]  = {0};
  FAR int8_t *apnname         = NULL;
  FAR int8_t *usrname         = NULL;
  FAR int8_t *passwrd         = NULL;
  int32_t    ret              = -EINVAL;

  if (8 <= command->argc)
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG)))
        {
          session_id = LTEAPI_INVALID_VALUE;
        }
      else
        {
          session_id = (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_1],
            NULL, LTEAPI_STRTOL_BASE);
        }

      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
        LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          apnname = NULL;
        }
      else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
        LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG)))
        {
          memset(apnname_tmp, 'X', LTE_APN_LEN);
          apnname = apnname_tmp;
        }
      else
        {
          strcpy((FAR char *)apnname_tmp,
            (FAR char *)command->argv[LTEAPI_CMD_APIPARAM_2]);
          apnname = apnname_tmp;
        }

    if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_3],
      "LTE_APN_IPTYPE_IPV6", strlen("LTE_APN_IPTYPE_IPV6")))
      {
        ip_type = LTE_APN_IPTYPE_IPV6;
      }
    else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_3],
      "LTE_APN_IPTYPE_IPV4V6", strlen("LTE_APN_IPTYPE_IPV4V6")))
      {
        ip_type = LTE_APN_IPTYPE_IPV4V6;
      }
    else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_3],
      "LTE_APN_IPTYPE_IP", strlen("LTE_APN_IPTYPE_IP")))
      {
        ip_type = LTE_APN_IPTYPE_IP;
      }
    else
      {
        ip_type = LTE_APN_IPTYPE_IPV4V6 + 1;
      }

    if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_4],
      "LTE_APN_AUTHTYPE_NONE", strlen("LTE_APN_AUTHTYPE_NONE")))
      {
        auth_type = LTE_APN_AUTHTYPE_NONE;
      }
    else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_4],
      "LTE_APN_AUTHTYPE_PAP", strlen("LTE_APN_AUTHTYPE_PAP")))
      {
        auth_type = LTE_APN_AUTHTYPE_PAP;
      }
    else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_4],
      "LTE_APN_AUTHTYPE_CHAP", strlen("LTE_APN_AUTHTYPE_CHAP")))
      {
        auth_type = LTE_APN_AUTHTYPE_CHAP;
      }
    else
      {
        auth_type = LTE_APN_AUTHTYPE_CHAP + 1;
      }

    if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_5],
      LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
      {
        usrname = NULL;
      }
    else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_5],
      LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG)))
      {
        memset(usrname_tmp, 'X', LTE_APN_USER_NAME_LEN);
        usrname = usrname_tmp;
      }
    else
      {
        strcpy((FAR char *)usrname_tmp,
          (FAR char *)command->argv[LTEAPI_CMD_APIPARAM_5]);
        usrname = usrname_tmp;
      }

    if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_6],
      LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
      {
        passwrd = NULL;
      }
    else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_6],
      LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG)))
      {
        memset(passwrd_tmp, 'X', LTE_APN_PASSWD_LEN);
        passwrd = passwrd_tmp;
      }
    else
      {
        strcpy((FAR char *)passwrd_tmp,
          (FAR char *)command->argv[LTEAPI_CMD_APIPARAM_6]);
        passwrd = passwrd_tmp;
      }

    if (9 <= command->argc
        && 0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_7],
          LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
      {
        ret = lte_set_apn(session_id, apnname, ip_type, auth_type, usrname, passwrd, NULL);
      }
    else
      {
        ret = lte_set_apn(session_id, apnname, ip_type, auth_type, usrname, passwrd, CODE setapn_cb);
      }
  }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_set_apn));
  return ret;
}

static void getapnst_cb(uint32_t result, FAR lte_getapnset_t *apn)
{
  int i = 0;
  LTEAPI_PRINT_RESULT(result);

  if (LTE_RESULT_OK == result)
  {
    printf("[CB_VAL] %s listnum : \"%d\"\n", __func__, (apn->listnum));
    for (i = 0; i < (apn->listnum); i++)
      {
        printf("[CB_VAL] %s [%d] session_id : \"%d\"\n"
               , __func__
               , i
               , (apn->apnlist[i].session_id));
        printf("[CB_VAL] %s [%d] apn : \"%s\"\n"
               , __func__
               , i
               , (apn->apnlist[i].apn));
        printf("[CB_VAL] %s [%d] ip_type : \"%d\"\n"
               , __func__
               , i
               , (apn->apnlist[i].ip_type));
      }
  }
}

static int32_t lteapi_test_getapnset(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_get_apnset(NULL);
    }
  else
    {
      ret = lte_get_apnset(CODE getapnst_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_apnset));
  return ret;
}

static void ntstrepo_cb(uint32_t netstat)
{
  printf("[CB_VAL] %s netstat : \"%d\"\n", __func__, netstat);
}

static int32_t lteapi_test_setrepnetstat(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_set_report_netstat(NULL);
    }
  else
    {
      ret = lte_set_report_netstat(CODE ntstrepo_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_set_report_netstat));
  return ret;
}

#endif

static void gtver_cb(uint32_t result, FAR lte_version_t *version)
{
  LTEAPI_PRINT_RESULT(result);
  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s bb_product : \"%s\"\n"
             , __func__
             , version->bb_product);
      printf("[CB_VAL] %s np_package : \"%s\"\n"
             , __func__
             , version->np_package);
    }
}

static int32_t lteapi_test_getver(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_get_version(NULL);
    }
  else
    {
      ret = lte_get_version(CODE gtver_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_version));
  return ret;
}

static void gtphno_cb(uint32_t result, uint8_t errcause, FAR int8_t *phoneno)
{
  LTEAPI_PRINT_RESULT(result);
  LTEAPI_PRINT_ERROR(result, errcause);

  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s phoneno : \"%s\"\n", __func__, phoneno);
    }
}

static int32_t lteapi_test_getphoneno(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_get_phoneno(NULL);
    }
  else
    {
      ret = lte_get_phoneno(CODE gtphno_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_phoneno));
  return ret;
}

static void gtimei_cb(uint32_t result, FAR int8_t *imei)
{
  LTEAPI_PRINT_RESULT(result);
  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s imei : \"%s\"\n", __func__, imei);
    }
}

static int32_t lteapi_test_getimei(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_get_imei(NULL);
    }
  else
    {
      ret = lte_get_imei(CODE gtimei_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_imei));
  return ret;
}

static void gtimsi_cb(uint32_t result, uint8_t errcause, FAR int8_t *imsi)
{
  LTEAPI_PRINT_RESULT(result);
  LTEAPI_PRINT_ERROR(result, errcause);

  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s imsi : \"%s\"\n", __func__, imsi);
    }
}

static int32_t lteapi_test_getimsi(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_get_imsi(NULL);
    }
  else
    {
      ret = lte_get_imsi(CODE gtimsi_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_imsi));
  return ret;
}

static void gtpinst_cb(uint32_t result, FAR lte_getpin_t *pinset)
{
  LTEAPI_PRINT_RESULT(result);
  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s enable : \"%d\" -> \"%s\"\n"
             , __func__
             , pinset->enable
             , ((LTE_PIN_ENABLE == pinset->enable)?
               "LTE_PIN_ENABLE" : "LTE_PIN_DISABLE"));
      if (LTE_PIN_ENABLE == pinset->enable)
        {
          printf("[CB_VAL] %s status : \"%d\" -> \"%s\"\n"
                 , __func__
                 , pinset->status
                 , GET_STRING(pinset->status, def_gtpinst_status));
        }
      printf("[CB_VAL] %s pin_attemptsleft : \"%d\"\n"
             , __func__
             , pinset->pin_attemptsleft);
      printf("[CB_VAL] %s puk_attemptsleft : \"%d\"\n"
             , __func__
             , pinset->puk_attemptsleft);
      printf("[CB_VAL] %s pin2_attemptsleft : \"%d\"\n"
             , __func__
             , pinset->pin2_attemptsleft);
      printf("[CB_VAL] %s puk2_attemptsleft : \"%d\"\n"
             , __func__
             , pinset->puk2_attemptsleft);
    }
}

static int32_t lteapi_test_getpinst(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_get_pinset(NULL);
    }
  else
    {
      ret = lte_get_pinset(CODE gtpinst_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_pinset));
  return ret;
}

static void stpnenbl_cb(uint32_t result, uint8_t attemptsleft)
{
  LTEAPI_PRINT_RESULT(result);
  if (LTE_RESULT_ERROR == result)
    {
      printf("[CB_VAL] %s attemptsleft : \"%d\"\n", __func__
        , attemptsleft);
    }
}

static int32_t lteapi_test_setpinenb(struct lteapi_command_s *command)
{
  int32_t    ret             = -EINVAL;
  bool       enable          = false;
  FAR int8_t *pincode        = NULL;
  int8_t     pincode_tmp[10] = {0};

  if (4 <= command->argc)
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        "LTE_PIN_ENABLE", strlen("LTE_PIN_ENABLE")))
        {
          enable = LTE_ENABLE;
        }
      else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        "LTE_PIN_DISABLE", strlen("LTE_PIN_DISABLE")))
        {
          enable = LTE_DISABLE;
        }
      else
        {
          enable = LTE_DISABLE + 1;
        }

      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
        LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          pincode = NULL;
        }
      else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
        LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG)))
        {
          memset(pincode_tmp, '0', LTEAPI_PINCODE_LEN);
          pincode = pincode_tmp;
        }
      else
        {
          strcpy((FAR char *)pincode_tmp,
            (FAR char *)command->argv[LTEAPI_CMD_APIPARAM_2]);
          pincode = pincode_tmp;
        }

      if (5 <= command->argc
      && 0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_3],
        LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_set_pinenable(enable, pincode, NULL);
        }
      else
        {
          ret = lte_set_pinenable(enable, pincode, CODE stpnenbl_cb);
        }
    }

  print_retval(ret , LTEAPI_GETFUNCNAME(lte_set_pinenable));
  return ret;
}

static void cngpin_cb(uint32_t result, uint8_t attemptsleft)
{
  LTEAPI_PRINT_RESULT(result);
  if (LTE_RESULT_ERROR == result)
    {
      printf("[CB_VAL] %s attemptsleft : \"%d\"\n", __func__
        , attemptsleft);
    }
}

static int32_t lteapi_test_chgpin(struct lteapi_command_s *command)
{
  int32_t    ret                = -EINVAL;
  int8_t     tgt_pin;
  FAR int8_t *pincode           = NULL;
  FAR int8_t *newpincode        = NULL;
  int8_t     pincode_tmp[10]    = {0};
  int8_t     newpincode_tmp[10] = {0};

  if (5 <= command->argc)
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        "LTE_TARGET_PIN2", strlen("LTE_TARGET_PIN2")))
        {
          tgt_pin = LTE_TARGET_PIN2;
        }
      else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        "LTE_TARGET_PIN", strlen("LTE_TARGET_PIN")))
        {
          tgt_pin = LTE_TARGET_PIN;
        }
      else
        {
          tgt_pin = LTE_TARGET_PIN2 + 1;
        }

      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
        LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          pincode = NULL;
        }
      else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
        LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG)))
        {
          memset(pincode_tmp, '0', LTEAPI_PINCODE_LEN);
          pincode = pincode_tmp;
        }
      else
        {
          strcpy((FAR char *)pincode_tmp,
            (FAR char *)command->argv[LTEAPI_CMD_APIPARAM_2]);
          pincode = pincode_tmp;
        }

      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_3],
        LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          newpincode = NULL;
        }
      else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_3],
        LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG)))
        {
          memset(newpincode_tmp, '0', LTEAPI_PINCODE_LEN);
          newpincode = newpincode_tmp;
        }
      else
        {
          strcpy((FAR char *)newpincode_tmp,
            (FAR char *)command->argv[LTEAPI_CMD_APIPARAM_3]);
          newpincode = newpincode_tmp;
        }

      if (6 <= command->argc && 0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_4],
          LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_change_pin(tgt_pin, pincode, newpincode, NULL);
        }
      else
        {
          ret = lte_change_pin(tgt_pin, pincode, newpincode, CODE cngpin_cb);
        }
     }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_change_pin));
  return ret;
}

static void entpin_cb(uint32_t result, uint8_t pinstat
  , uint8_t attemptsleft)
{
  LTEAPI_PRINT_RESULT(result);
  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s pinstat : \"%d\"\n", __func__, pinstat);
    }
  else
    {
      printf("[CB_VAL] %s attemptsleft : \"%d\"\n", __func__, attemptsleft);
    }
}

static int32_t lteapi_test_entpin(struct lteapi_command_s *command)
{
  int32_t    ret                = -EINVAL;
  FAR int8_t *pincode           = NULL;
  FAR int8_t *newpincode        = NULL;
  int8_t     pincode_tmp[10]    = {0};
  int8_t     newpincode_tmp[10] = {0};

  if (3 <= command->argc)
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          pincode = NULL;
        }
      else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG)))
        {
          memset(pincode_tmp, '0', LTEAPI_PINCODE_LEN);
          pincode = pincode_tmp;
        }
      else
        {
          strcpy((FAR char *)pincode_tmp,
            (FAR char *)command->argv[LTEAPI_CMD_APIPARAM_1]);
          pincode = pincode_tmp;
        }
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
        LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          newpincode = NULL;
        }
      else if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
        LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG)))
        {
          memset(newpincode_tmp, '0', LTEAPI_PINCODE_LEN);
          newpincode = newpincode_tmp;
        }
      else
        {
          strcpy((char *)newpincode_tmp,
            (char *)command->argv[LTEAPI_CMD_APIPARAM_2]);
          newpincode = newpincode_tmp;
        }

      if (5 <= command->argc
          && 0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_3],
            LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_enter_pin(pincode, newpincode, NULL);
        }
      else
        {
          ret = lte_enter_pin(pincode, newpincode, CODE entpin_cb);
        }
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_enter_pin));
  return ret;
}

static void gtlcltm_cb(uint32_t result, FAR lte_localtime_t *localtime)
{
  LTEAPI_PRINT_RESULT(result);
  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s time : \"%02d/%02d/%02d : %02d:%02d:%02d\"\n"
        , __func__
        , localtime->year
        , localtime->mon
        , localtime->mday
        , localtime->hour
        , localtime->min
        , localtime->sec);
      printf("[CB_VAL] %s time_zone : \"%d\"\n", __func__
        , localtime->tz_sec);
    }
}

static int32_t lteapi_test_getltime(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_get_localtime(NULL);
    }
  else
    {
      ret = lte_get_localtime(CODE gtlcltm_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_localtime));
  return ret;
}

static void gtoprtr_cb(uint32_t result, FAR int8_t *oper)
{
  LTEAPI_PRINT_RESULT(result);
  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s oper : \"%s\"\n", __func__, oper);
    }
}

static int32_t lteapi_test_getope(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_get_operator(NULL);
    }
  else
    {
      ret = lte_get_operator(CODE gtoprtr_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_operator));
  return ret;
}

static void getedrx_cb(uint32_t result, FAR lte_edrx_setting_t *settings)
{
  LTEAPI_PRINT_RESULT(result);
  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s act_type : ", __func__);
      switch(settings->act_type)
        {
          case LTE_EDRX_ACTTYPE_WBS1:
            printf("LTE_EDRX_ACTTYPE_WBS1[\"%d\"]\n",settings->act_type);
            break;
          case LTE_EDRX_ACTTYPE_NBS1:
            printf("LTE_EDRX_ACTTYPE_NBS1[\"%d\"]\n",settings->act_type);
            break;
          case LTE_EDRX_ACTTYPE_ECGSMIOT:
            printf("LTE_EDRX_ACTTYPE_ECGSMIOT[\"%d\"]\n",settings->act_type);
            break;
          case LTE_EDRX_ACTTYPE_GSM:
            printf("LTE_EDRX_ACTTYPE_GSM[\"%d\"]\n",settings->act_type);
            break;
          case LTE_EDRX_ACTTYPE_IU:
            printf("LTE_EDRX_ACTTYPE_IU[\"%d\"]\n",settings->act_type);
            break;
          case LTE_EDRX_ACTTYPE_NOTUSE:
            printf("LTE_EDRX_ACTTYPE_NOTUSE[\"%d\"]\n",settings->act_type);
            break;
          default:
            printf("\"%d\"\n", settings->act_type);
            break;
        }
      printf("[CB_VAL] %s enable : \"%d\"\n", __func__, settings->enable);
      if (LTE_ENABLE == settings->enable)
        {
          printf("[CB_VAL] %s edrx_cycle : \"%d\" -> \"%s\"\n"
                 , __func__
                 , settings->edrx_cycle
                 , GET_STRING(settings->edrx_cycle, def_edrx_cyc));
          printf("[CB_VAL] %s ptw_val : \"%d\" -> \"%s\"\n"
                 , __func__
                 , settings->ptw_val
                 , GET_STRING(settings->ptw_val, def_ptw_val));
        }
    }
}

static int32_t lteapi_test_getedrx(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_get_edrx(NULL);
    }
  else
    {
      ret = lte_get_edrx(CODE getedrx_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_edrx));
  return ret;
}

static void setedrx_cb(uint32_t result)
{
  LTEAPI_PRINT_RESULT(result);
}

static int32_t lteapi_test_setedrx(struct lteapi_command_s *command)
{
  int32_t                ret            = -EINVAL;
  FAR lte_edrx_setting_t *edrx_settings = NULL;
  lte_edrx_setting_t     edrx_stg_tmp;

  if (6 <= command->argc)
    {
      edrx_stg_tmp.act_type = (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_1],
        NULL, LTEAPI_STRTOL_BASE);
      edrx_stg_tmp.enable = (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_2],
        NULL, LTEAPI_STRTOL_BASE);
      edrx_stg_tmp.edrx_cycle = (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_3],
        NULL, LTEAPI_STRTOL_BASE);
      edrx_stg_tmp.ptw_val = (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_4],
        NULL, LTEAPI_STRTOL_BASE);
      edrx_settings = &edrx_stg_tmp;

      if (7 <= command->argc
        && 0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_5],
          LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_set_edrx(edrx_settings, NULL);
        }
      else
        {
          ret = lte_set_edrx(edrx_settings, CODE setedrx_cb);
        }
    }
  else if (3 <= command->argc)
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_set_edrx(NULL, CODE setedrx_cb);
        }
    }
  else
    {
      ;;
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_set_edrx));

  return ret;
}

static void getpsm_cb(uint32_t result, FAR lte_psm_setting_t *settings)
{
  LTEAPI_PRINT_RESULT(result);
  printf("[CB_VAL] %s enable : \"%d\"\n"
    , __func__
    , settings->enable);

  if (LTE_ENABLE == settings->enable)
    {
      printf("[CB_VAL] %s req_active_time.unit : \"%d\" -> \"%s\"\n"
             , __func__
             , settings->req_active_time.unit
             , GET_STRING(settings->req_active_time.unit
                          , def_activetime_unit));
      printf("[CB_VAL] %s req_active_time.time_val : \"%d\"\n"
             , __func__
             , settings->req_active_time.time_val);
      printf("[CB_VAL] %s ext_periodic_tau_time.unit : \"%d\" -> \"%s\"\n"
             , __func__
             , settings->ext_periodic_tau_time.unit
             , GET_STRING(settings->ext_periodic_tau_time.unit
                          , def_extperiodictautime_unit));
      printf("[CB_VAL] %s ext_periodic_tau_time.time_val : \"%d\"\n"
             , __func__
             , settings->ext_periodic_tau_time.time_val);
    }
}

static int32_t lteapi_test_getpsm(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_get_psm(NULL);
    }
  else
    {
      ret = lte_get_psm(CODE getpsm_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_psm));
  return ret;
}

static void setpsm_cb(uint32_t result)
{
  LTEAPI_PRINT_RESULT(result);
}

static int32_t lteapi_test_setpsm(struct lteapi_command_s *command)
{
  int32_t               ret           = -EINVAL;
  FAR lte_psm_setting_t *psm_settings = NULL;
  lte_psm_setting_t     psm_stg_tmp;
  uint8_t               timeval_tmp;

  if (7 <= command->argc)
    {

      psm_stg_tmp.enable = (uint8_t)strtol(
        command->argv[LTEAPI_CMD_APIPARAM_1],
        NULL,
        LTEAPI_STRTOL_BASE);
      psm_stg_tmp.req_active_time.unit = (uint8_t)strtol(
        command->argv[LTEAPI_CMD_APIPARAM_2],
        NULL,
        LTEAPI_STRTOL_BASE);
      timeval_tmp = (uint8_t)strtol(
        command->argv[LTEAPI_CMD_APIPARAM_3],
        NULL,
        LTEAPI_STRTOL_BASE);
      psm_stg_tmp.req_active_time.time_val = timeval_tmp;
      psm_stg_tmp.ext_periodic_tau_time.unit = (uint8_t)strtol(
        command->argv[LTEAPI_CMD_APIPARAM_4],
        NULL,
        LTEAPI_STRTOL_BASE);
      timeval_tmp = (uint8_t)strtol(
        command->argv[LTEAPI_CMD_APIPARAM_5],
        NULL,
        LTEAPI_STRTOL_BASE);
      psm_stg_tmp.ext_periodic_tau_time.time_val = timeval_tmp;

      psm_settings = &psm_stg_tmp;

      if ((8 <= command->argc)
        && (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_6]
          , LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG))))
        {
          ret = lte_set_psm(psm_settings, NULL);
        }
      else
        {
          ret = lte_set_psm(psm_settings, CODE setpsm_cb);
        }
    }
  else if (3 <= command->argc)
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1]
        , LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret =lte_set_psm(NULL, CODE setpsm_cb);
        }
    }
  else
    {
      ;;
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_set_psm));

  return ret;
}

static void smstrepo_cb(uint32_t simstat)
{
  printf("[CB_VAL] %s simstat : \"%d\" -> \"%s\"\n"
         , __func__
         , simstat
         , GET_STRING(simstat, def_report_simstat));
}

static int32_t lteapi_test_setrepsimstat(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_set_report_simstat(NULL);
    }
  else
    {
      ret = lte_set_report_simstat(CODE smstrepo_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_set_report_simstat));
  return ret;
}

static void lcltmrepo_cb(FAR lte_localtime_t *localtime)
{
  printf("[CB_VAL] %s time : \"%02d/%02d/%02d : %02d:%02d:%02d\"\n"
         , __func__
         , localtime->year
         , localtime->mon
         , localtime->mday
         , localtime->hour
         , localtime->min
         , localtime->sec);
  printf("[CB_VAL] %s time_zone : \"%d\"\n", __func__
    , localtime->tz_sec);
}

static int32_t lteapi_test_setrepltime(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  if (cb_null_chk(command))
    {
      ret = lte_set_report_localtime(NULL);
    }
  else
    {
      ret = lte_set_report_localtime(CODE lcltmrepo_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_set_report_localtime));
  return ret;
}

static void qtyrepo_cb(FAR lte_quality_t *quality)
{
  printf("[CB_VAL] %s valid : \"%d\"\n", __func__, quality->valid);

if (LTE_VALID == quality->valid)
    {
      printf("[CB_VAL] %s rsrp : \"%d\"\n", __func__, quality->rsrp);
      printf("[CB_VAL] %s rsrq : \"%d\"\n", __func__, quality->rsrq);
      printf("[CB_VAL] %s sinr : \"%d\"\n", __func__, quality->sinr);
      printf("[CB_VAL] %s rssi : \"%d\"\n", __func__, quality->rssi);
    }
}

static int32_t lteapi_test_setrepqlty(struct lteapi_command_s *command)
{
  int32_t  ret = -EINVAL;
  uint32_t period;

  if (3 <= command->argc)
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG)))
        {
          period = 0;
        }
      else
        {
          period = (uint32_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_1],
            NULL, LTEAPI_STRTOL_BASE);
        }

      if (4 <= command->argc
        && 0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
          LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_set_report_quality(NULL, period);
        }
      else
        {
          ret = lte_set_report_quality(CODE qtyrepo_cb, period);
        }
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_set_report_quality));
  return ret;
}

static void clinfrepo_cb(FAR lte_cellinfo_t *cellinfo)
{
  int i = 0;

  printf("[CB_VAL] %s valid : \"%d\"\n", __func__, cellinfo->valid);

  if (LTE_VALID == cellinfo->valid)
    {
      printf("[CB_VAL] %s phycell_id : \"%d\"\n"
             , __func__
             , cellinfo->phycell_id);
      printf("[CB_VAL] %s earfcn : \"%d\"\n", __func__, cellinfo->earfcn);

      for (i = 0; i < LTE_CELLINFO_MCC_DIGIT; i++)
        {
          printf("[CB_VAL] %s mcc[%d] : \"%d\"\n"
                 , __func__
                 , i
                 , cellinfo->mcc[i]);
        }

      printf("[CB_VAL] %s mnc_digit : \"%d\"\n"
             , __func__
             , cellinfo->mnc_digit);

      for (i = 0; i < cellinfo->mnc_digit; i++)
        {
          printf("[CB_VAL] %s mnc[%d] : \"%d\"\n"
                 , __func__
                 , i
                 , cellinfo->mnc[i]);
        }
    }
}

static int32_t lteapi_test_setrepcelinf(struct lteapi_command_s *command)
{
  int32_t  ret = -EINVAL;
  uint32_t period;

  if (3 <= command->argc)
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1],
        LTEAPI_INVALID_ARG, strlen(LTEAPI_INVALID_ARG)))
        {
          period = 0;
        }
      else
        {
          period = (uint32_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_1],
            NULL, LTEAPI_STRTOL_BASE);
        }

      if (4 <= command->argc
        && 0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_2],
          LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_set_report_cellinfo(NULL, period);
        }
      else
        {
          ret = lte_set_report_cellinfo(CODE clinfrepo_cb, period);
        }
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_set_report_cellinfo));
  return ret;
}

static void setce_cb(uint32_t result)
{
  LTEAPI_PRINT_RESULT(result);
}

static int32_t lteapi_test_setce(struct lteapi_command_s *command)
{
  int32_t              ret          = -EINVAL;
  FAR lte_ce_setting_t *ce_settings = NULL;
  lte_ce_setting_t     ce_stg_tmp;

  if (4 <= command->argc)
    {
      ce_stg_tmp.mode_a_enable =
        (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_1],
        NULL,
        LTEAPI_STRTOL_BASE);
      ce_stg_tmp.mode_b_enable =
        (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_2],
        NULL,
        LTEAPI_STRTOL_BASE);

      ce_settings = &ce_stg_tmp;

     if (5 <= command->argc
      && 0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_3],
        LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_set_ce(ce_settings, NULL);
        }
      else
        {
          ret = lte_set_ce(ce_settings, CODE setce_cb);
        }
    }
  else
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APIPARAM_1]
        , LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          ret = lte_set_ce(NULL, CODE setce_cb);
        }
   }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_set_ce));

  return ret;
}

static void getce_cb(uint32_t result, FAR lte_ce_setting_t *settings)
{
  LTEAPI_PRINT_RESULT(result);
  if (LTE_RESULT_OK == result)
    {
      printf("[CB_VAL] %s mode_a_enable : \"%d\" -> \"%s\"\n"
             , __func__
             , settings->mode_a_enable
             , ((LTE_ENABLE == settings->mode_a_enable)?
               "LTE_ENABLE" : "LTE_DISABLE"));
      printf("[CB_VAL] %s mode_b_enable : \"%d\" -> \"%s\"\n"
             , __func__
             , settings->mode_b_enable
             , ((LTE_ENABLE == settings->mode_b_enable)?
               "LTE_ENABLE" : "LTE_DISABLE"));
    }
}

static int32_t lteapi_test_getce(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  if (cb_null_chk(command))
    {
      ret = lte_get_ce(NULL);
    }
  else
    {
      ret = lte_get_ce(CODE getce_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_ce));
  return ret;
}

static int32_t lteapi_test_poweron(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  ret = lte_power_on();
  print_retval(ret, LTEAPI_GETFUNCNAME(lte_powr_on));

  return ret;
}

static int32_t lteapi_test_powerff(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  ret = lte_power_off();
  print_retval(ret, LTEAPI_GETFUNCNAME(lte_power_off));

  return ret;
}

static void restart_cb(uint32_t reason)
{
  printf("[CB_VAL] %s reason : \"%d\"\n", __func__, reason);
}

static int32_t lteapi_test_setrestart(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  if (cb_null_chk(command))
    {
      ret = lte_set_report_restart(NULL);
    }
  else
    {
      ret = lte_set_report_restart(CODE restart_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_set_report_restart));
  return ret;
}

static void radioon_cb(uint32_t result)
{
  LTEAPI_PRINT_RESULT(result);
}

static int32_t lteapi_test_radioon(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  if (cb_null_chk(command))
    {
      ret = lte_radio_on(NULL);
    }
  else
    {
      ret = lte_radio_on(CODE radioon_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_radio_on));
  return ret;
}

static void radiooff_cb(uint32_t result)
{
  LTEAPI_PRINT_RESULT(result);
}

static int32_t lteapi_test_radiooff(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  if (cb_null_chk(command))
    {
      ret = lte_radio_off(NULL);
    }
  else
    {
      ret = lte_radio_off(CODE radiooff_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_radio_off));
  return ret;
}

static void actpdn_cb(uint32_t result, lte_pdn_t *pdn)
{
  LTEAPI_PRINT_RESULT(result);
  if (result == LTE_RESULT_OK)
    {
      show_pdn(pdn, __func__);
    }
}

static int32_t lteapi_test_actpdn(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  lte_apn_setting_t apn;

  if (command->argc < 5)
    return -EINVAL;

  if (cb_null_chk(command))
    {
      ret = lte_activate_pdn(&apn, NULL);
    }
  else
    {
      apn.apn = (int8_t *)command->argv[LTEAPI_CMD_APIPARAM_1];
      apn.apn_type = strtol(command->argv[LTEAPI_CMD_APIPARAM_2],
                                NULL, LTEAPI_STRTOL_BASE_HEX);

      apn.ip_type = (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_3],
        NULL,
        LTEAPI_STRTOL_BASE);
      apn.auth_type = (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_4],
        NULL,
        LTEAPI_STRTOL_BASE);
      apn.user_name = NULL;
      apn.password = NULL;
      if (command->argc >= 6 && 0 != strncasecmp(command->argv[LTEAPI_CMD_APIPARAM_5],
          LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          apn.user_name = (int8_t *)command->argv[LTEAPI_CMD_APIPARAM_5];
        }
      if (command->argc >= 7 && 0 != strncasecmp(command->argv[LTEAPI_CMD_APIPARAM_6],
          LTEAPI_NULL_ARG, strlen(LTEAPI_NULL_ARG)))
        {
          apn.password = (int8_t *)command->argv[LTEAPI_CMD_APIPARAM_6];
        }

      ret = lte_activate_pdn(&apn, CODE actpdn_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_activate_pdn));
  return ret;
}

static int32_t lteapi_test_actpdn_cancel(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  ret = lte_activate_pdn_cancel();
  print_retval(ret, LTEAPI_GETFUNCNAME(lte_activate_pdn_cancel));

  return ret;
}

static void deactpdn_cb(uint32_t result)
{
   LTEAPI_PRINT_RESULT(result);
}

static int32_t lteapi_test_deactpdn(struct lteapi_command_s *command)
{
  uint8_t session_id = 0;
  int32_t ret = 0;

  if (command->argc < 3)
    {
      return -EINVAL;
    }

  if (cb_null_chk(command))
    {
      ret = lte_deactivate_pdn(session_id, NULL);
    }
  else
    {
      session_id = strtol(command->argv[LTEAPI_CMD_APIPARAM_1],
                          NULL, LTEAPI_STRTOL_BASE);
      ret = lte_deactivate_pdn(session_id, CODE deactpdn_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_deactivate_pdn));
  return ret;
}

static void getnetinfo_cb(uint32_t result, lte_netinfo_t *info)
{
  int i;

  LTEAPI_PRINT_RESULT(result);
  if (result == LTE_RESULT_OK)
    {
     if (!info)
      {
        printf("net info is null\n");
        return;
      }
      printf("[CB_VAL] %s nw_stat : \"%d\" -> \"%s\"\n"
             , __func__
             , info->nw_stat
             , GET_STRING(info->nw_stat, def_netinfo_stat));
      if (LTE_NETSTAT_REG_DENIED == info->nw_stat)
        {
          printf("[CB_VAL] %s err_type : \"%d\" -> \"%s\"\n"
                 , __func__
                 , info->nw_err.err_type
                 , GET_STRING(info->nw_err.err_type, def_netinfo_errtype));
          if (LTE_NETERR_REJECT == info->nw_err.err_type)
            {
              printf("[CB_VAL] %s reject cause category : \"%d\" -> \"%s\"\n"
                     , __func__
                     , info->nw_err.reject_cause.category
                     , GET_STRING(info->nw_err.reject_cause.category
                                  , def_netinfo_rejectcategory));
              printf("[CB_VAL] %s value : \"%d\" \n"
                     , __func__
                     , info->nw_err.reject_cause.value);
            }
        }

      printf("[CB_VAL] %s pdn_num : \"%d\"\n", __func__, info->pdn_num);
      for (i = 0; i < info->pdn_num; i++)
        {
          printf("[CB_VAL] %s pdn [%d]\n", __func__, (i + 1));
          show_pdn(&info->pdn_stat[i], __func__);
        }
    }
}

static int32_t lteapi_test_getnetinfo(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  if (cb_null_chk(command))
    {
      ret = lte_get_netinfo(NULL);
    }
  else
    {
      ret = lte_get_netinfo(CODE getnetinfo_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_netinfo));
  return ret;
}

static void getimscap_cb(uint32_t result, bool imscap)
{
  LTEAPI_PRINT_RESULT(result);
  printf("[CB_VAL] %s imscap : \"%d\" -> \"%s\"\n"
         , __func__
         , imscap
         , ((imscap == LTE_ENABLE)? "LTE_ENABLE" : "LTE_DISABLE"));
}

static int32_t lteapi_test_getimscap(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  if (cb_null_chk(command))
    {
      ret = lte_get_imscap(NULL);
    }
  else
    {
      ret = lte_get_imscap(CODE getimscap_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_imscap));
  return ret;
}

static void dataallow_cb(uint32_t result)
{
  LTEAPI_PRINT_RESULT(result);
}

static int32_t lteapi_test_dataallow(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  uint8_t session_id = 0;
  uint8_t allow = 0;
  uint8_t roamallow = 0;

  if (command->argc < 5)
    {
      return -EINVAL;
    }

  if (cb_null_chk(command))
    {
      ret = lte_data_allow(session_id, allow, roamallow, NULL);
    }
  else
    {
      session_id = (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_1],
                                   NULL, LTEAPI_STRTOL_BASE);
      allow = (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_2],
                              NULL, LTEAPI_STRTOL_BASE);
      roamallow = (uint8_t)strtol(command->argv[LTEAPI_CMD_APIPARAM_3],
                                  NULL, LTEAPI_STRTOL_BASE);
      ret = lte_data_allow(session_id, allow, roamallow, CODE dataallow_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_data_allow));
  return ret;
}

static void repnetinfo_cb(lte_netinfo_t *info)
{
  int i;

  if (info)
    {
      printf("[CB_VAL] %s nw_stat : \"%d\" -> \"%s\"\n"
             , __func__
             , info->nw_stat
             , GET_STRING(info->nw_stat, def_netinfo_stat));
      printf("[CB_VAL] %s pdn_num : \"%d\"\n"
             , __func__
             , info->pdn_num);

      if (LTE_NETSTAT_REG_DENIED == info->nw_stat)
        {
          printf("[CB_VAL] %s err_type : \"%d\" -> \"%s\"\n"
                 , __func__
                 , info->nw_err.err_type
                 , GET_STRING(info->nw_err.err_type, def_netinfo_errtype));
          if (LTE_NETERR_REJECT == info->nw_err.err_type)
            {
              printf("[CB_VAL] %s reject cause category : \"%d\" -> \"%s\"\n"
                     , __func__
                     , info->nw_err.reject_cause.category
                     , GET_STRING(info->nw_err.reject_cause.category
                                  , def_netinfo_rejectcategory));
              printf("[CB_VAL] %s value : \"%d\" \n"
                     , __func__
                     , info->nw_err.reject_cause.value);
            }
        }
      for (i = 0; i < info->pdn_num; i++)
        {
          printf("[CB_VAL] %s pdn [%d]\n", __func__, (i + 1));
          show_pdn(&info->pdn_stat[i], __func__);
        }
    }
}

static int32_t lteapi_test_repnetinfo(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  if (cb_null_chk(command))
    {
      ret = lte_set_report_netinfo(NULL);
    }
  else
    {
      ret = lte_set_report_netinfo(CODE repnetinfo_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_set_report_netinfo));
  return ret;
}

static void getsiminfo_cb(uint32_t result, lte_siminfo_t *info)
{
  int i;
  LTEAPI_PRINT_RESULT(result);

  if (result == LTE_RESULT_OK && info)
    {

      printf("[CB_VAL] %s option : \"%04X\"\n", __func__, info->option);

      if (LTE_SIMINFO_GETOPT_MCCMNC & info->option)
        {
          printf("[CB_VAL] %s MCC : \"", __func__);
          for (i = 0; i < LTE_SIMINFO_MCC_DIGIT; i++)
            {
              printf("%c", (char)info->mcc[i]);
            }
          printf("\"\n");

          printf("[CB_VAL] %s MNC digit : \"%d\"\n"
                 , __func__
                 , info->mnc_digit);
          printf("[CB_VAL] %s MNC : \"", __func__);
          for (i = 0; i < info->mnc_digit; i++)
            {
              printf("%c", (char)info->mnc[i]);
            }
          printf("\"\n");
        }

      if (LTE_SIMINFO_GETOPT_SPN & info->option)
        {
          printf("[CB_VAL] %s SPN len : \"%d\"\n", __func__, info->spn_len);
          printf("[CB_VAL] %s SPN : \"", __func__);
          for (i = 0; i < info->spn_len; i++)
            {
              printf("%c", (char)info->spn[i]);
            }
          printf("\"\n");
        }

      if (LTE_SIMINFO_GETOPT_ICCID & info->option)
        {
          printf("[CB_VAL] %s ICCID len : \"%d\"\n"
                 , __func__
                 , info->iccid_len);
          printf("[CB_VAL] %s ICCID : \"", __func__);
          for (i = 0; i < info->iccid_len; i++)
            {
              printf("%02x", info->iccid[i]);
            }
          printf("\"\n");
        }

      if (LTE_SIMINFO_GETOPT_IMSI & info->option)
        {
          printf("[CB_VAL] %s IMSI len : \"%d\"\n", __func__, info->imsi_len);
          printf("[CB_VAL] %s IMSI : \"", __func__);
          for (i = 0; i < info->imsi_len; i++)
            {
              printf("%c", info->imsi[i]);
            }
          printf("\"\n");
        }

       if (LTE_SIMINFO_GETOPT_GID1 & info->option)
        {
          printf("[CB_VAL] %s GID1 len : \"%d\"\n", __func__, info->gid1_len);
          printf("[CB_VAL] %s GID1 : \"", __func__);
          for (i = 0; i < info->gid1_len; i++)
            {
              printf("%c", info->gid1[i]);
            }
          printf("\"\n");
        }
       if (LTE_SIMINFO_GETOPT_GID2 & info->option)
        {
          printf("[CB_VAL] %s GID2 len : \"%d\"\n", __func__, info->gid2_len);
          printf("[CB_VAL] %s GID2 : \"", __func__);
          for (i = 0; i < info->gid2_len; i++)
            {
              printf("%c", info->gid2[i]);
            }
          printf("\"\n");
        }

    }
}

static int32_t lteapi_test_getsiminfo(struct lteapi_command_s *command)
{
  int32_t ret = 0;
  int32_t sim_info_opt = 0;

  if (command->argc < 3)
    {
      return -EINVAL;
    }
  else
    {
      if (cb_null_chk(command))
        {
          ret = lte_get_siminfo(sim_info_opt, NULL);
        }
      else
        {
          sim_info_opt = strtol(command->argv[LTEAPI_CMD_APIPARAM_1],
                                NULL, LTEAPI_STRTOL_BASE_HEX);

          ret = lte_get_siminfo(sim_info_opt, CODE getsiminfo_cb);
        }
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_siminfo));
  return ret;
}

static void getdynamic_edrxparam_cb(uint32_t result, lte_edrx_setting_t *set)
{
  LTEAPI_PRINT_RESULT(result);

  if (LTE_RESULT_OK == result && set)
    {
      printf("[CB_VAL] %s act_type : ", __func__);
      switch(set->act_type)
        {
          case LTE_EDRX_ACTTYPE_WBS1:
            printf("LTE_EDRX_ACTTYPE_WBS1[\"%d\"]\n",set->act_type);
            break;
          case LTE_EDRX_ACTTYPE_NBS1:
            printf("LTE_EDRX_ACTTYPE_NBS1[\"%d\"]\n",set->act_type);
            break;
          case LTE_EDRX_ACTTYPE_ECGSMIOT:
            printf("LTE_EDRX_ACTTYPE_ECGSMIOT[\"%d\"]\n",set->act_type);
            break;
          case LTE_EDRX_ACTTYPE_GSM:
            printf("LTE_EDRX_ACTTYPE_GSM[\"%d\"]\n",set->act_type);
            break;
          case LTE_EDRX_ACTTYPE_IU:
            printf("LTE_EDRX_ACTTYPE_IU[\"%d\"]\n",set->act_type);
            break;
          case LTE_EDRX_ACTTYPE_NOTUSE:
            printf("LTE_EDRX_ACTTYPE_NOTUSE[\"%d\"]\n",set->act_type);
            break;
          default:
            printf("\"%d\"\n", set->act_type);
            break;
        }
      printf("[CB_VAL] %s enable : \"%d\"\n", __func__, set->enable);
      if (LTE_ENABLE == set->enable)
        {
          printf("[CB_VAL] %s edrx_cycle : \"%d\" -> \"%s\"\n"
                 , __func__
                 , set->edrx_cycle
                 , GET_STRING(set->edrx_cycle, def_edrx_cyc));
          printf("[CB_VAL] %s ptw_val : \"%d\" -> \"%s\"\n"
                 , __func__
                 , set->ptw_val
                 , GET_STRING(set->ptw_val, def_ptw_val));
        }
    }
}

static int32_t lteapi_test_getdynamicedrx(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  if (cb_null_chk(command))
    {
      ret = lte_get_dynamic_edrx_param(NULL);
    }
  else
    {
      ret = lte_get_dynamic_edrx_param(CODE getdynamic_edrxparam_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_dynamic_edrx_param));
  return ret;
}

static void getdynamic_psmparam_cb(uint32_t result, lte_psm_setting_t *set)
{
  LTEAPI_PRINT_RESULT(result);
  if (LTE_RESULT_OK == result && set)
    {
      printf("[CB_VAL] %s enable : \"%d\"\n", __func__, set->enable);

      if (LTE_ENABLE == set->enable)
        {
          printf("[CB_VAL] %s req_active_time.unit : \"%d\" -> \"%s\"\n"
                 , __func__
                 , set->req_active_time.unit
                 , GET_STRING(set->req_active_time.unit
                              , def_activetime_unit));
          printf("[CB_VAL] %s req_active_time.time_val : \"%d\"\n"
                 , __func__
                 , set->req_active_time.time_val);
          printf("[CB_VAL] %s ext_periodic_tau_time.unit : \"%d\" -> \"%s\"\n"
                 , __func__
                 , set->ext_periodic_tau_time.unit
                 , GET_STRING(set->ext_periodic_tau_time.unit
                              , def_extperiodictautime_unit));
          printf("[CB_VAL] %s ext_periodic_tau_time.time_val : \"%d\"\n"
                  , __func__
                  , set->ext_periodic_tau_time.time_val);
        }
    }
}

static int32_t lteapi_test_getdynamicpsm(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  if (cb_null_chk(command))
    {
      ret = lte_get_dynamic_psm_param(NULL);
    }
  else
    {
      ret = lte_get_dynamic_psm_param(CODE getdynamic_psmparam_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_dynamic_psm_param));
  return ret;
}

static void get_quality_cb(uint32_t result, FAR lte_quality_t *quality)
{
  LTEAPI_PRINT_RESULT(result);
  printf("[CB_VAL] %s valid : \"%s\"\n", __func__, quality->valid ? "true" : "false");
  if (LTE_RESULT_OK == result && LTE_VALID == quality->valid)
    {
      printf("[CB_VAL] %s rsrp : \"%d\"\n", __func__, quality->rsrp);
      printf("[CB_VAL] %s rsrq : \"%d\"\n", __func__, quality->rsrq);
      printf("[CB_VAL] %s sinr : \"%d\"\n", __func__, quality->sinr);
      printf("[CB_VAL] %s rssi : \"%d\"\n", __func__, quality->rssi);
    }
}

static int32_t lteapi_test_getquality(struct lteapi_command_s *command)
{
  int32_t ret = 0;

  if (cb_null_chk(command))
    {
      ret = lte_get_quality(NULL);
    }
  else
    {
      ret = lte_get_quality(CODE get_quality_cb);
    }

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_quality));
  return ret;
}

static void get_err(void)
{
  int32_t ret;
  lte_errinfo_t info;

  ret = lte_get_errinfo(FAR &info);

  print_retval(ret, LTEAPI_GETFUNCNAME(lte_get_errinfo));

  if (0 < ret)
    {
      printf("[API_VAL] %s api_is_error \"%d\"\n", __func__, ret);
    }
  else
    {
      if ((LTE_ERR_INDICATOR_ERRCODE & info.err_indicator))
          printf("[API_VAL] %s error_code : \"%d\"\n"
                 , __func__
                 , info.err_result_code);

      if ((LTE_ERR_INDICATOR_ERRNO & info.err_indicator))
          printf("[API_VAL] %s error_no : \"%d\"\n", __func__, info.err_no);

      if ((LTE_ERR_INDICATOR_ERRSTR & info.err_indicator))
          printf("[API_VAL] %s error_str : \"%s\"\n"
                 , __func__
                 , info.err_string);
    }
}

static int32_t  lteapi_test_geterr(struct lteapi_command_s *command)
{
  get_err();
  return 0;
}

static void testapi_getfunc(struct lteapi_command_s *command, lteapi_testfunc_t *f, lteapi_help_t *h)
{
  uint8_t i;

  *f = NULL;
  *h = NULL;

  if (!command)
    {
      printf("null param\n");
      return;
    }

  for (i = 0; i < LTEAPI_TABLESIZE; i++)
    {
      if (0 == strncmp(command->argv[LTEAPI_CMD_APINAME],
          g_testfunt_table[i].cmd, strlen(g_testfunt_table[i].cmd)))
        {
          *f = g_testfunt_table[i].api_func;
          *h = g_testfunt_table[i].help_func;
          break;
        }
    }

  return;
}

static int lteapi_test_task(int argc, FAR char *argv[])
{
  int                     itr;
  mqd_t                   mqd;
  ssize_t                 ret;
  struct mq_attr          attr;
  int32_t api_rslt;
  struct lteapi_command_s *command;
  lteapi_testfunc_t       f;
  lteapi_help_t           help;

  g_taskrunning   = true;
  attr.mq_maxmsg  = LTEAPI_MAX_API_MQUEUE;
  attr.mq_msgsize = sizeof(struct lteapi_command_s *);
  mqd             = mq_open(LTEAPI_MQUEUE_PATH, O_RDWR,
                    LTEAPI_MQUEUE_MODE, &attr);

  while (g_taskrunning)
    {
      f = NULL;

      /* keep waiting until send commands */
      ret = mq_receive(mqd, (FAR char *)(&command)
        , sizeof(struct lteapi_command_s *), NULL);
      if (0 > ret)
        {
          printf("receive fail[%d]\n", ret);
          continue;
        }

      /* Get API function */
      testapi_getfunc(command, &f, &help);
      if (!f)
        {
          lteapi_help(NULL);
        }
      else
        {
          api_rslt = f(command);
          if (-EINVAL == api_rslt && help)
            {
              help(command->argv[LTEAPI_CMD_APINAME]);
            }
        }

      for (itr = 0; itr < (command->argc); itr++)
        {
          free(command->argv[itr]);
          command->argv[itr] = NULL;
        }
      free(command);
      command = NULL;
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
/****************************************************************************
 * lteapi_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int lteapi_main(int argc, char *argv[])
#endif
{
  FAR struct lteapi_command_s *command;
  struct mq_attr          attr;
  mqd_t                   mqd;
  int                     itr = 0;

  if ( 2 <= argc)
    {
      if (LTEAPI_MAX_NUMBER_OF_PARAM < argc)
        {
          printf("too many arguments\n");
          return -1;
        }

      command = malloc(sizeof(struct lteapi_command_s));

      memset(command, 0, sizeof(struct lteapi_command_s));
      command->argc = argc;

      for (itr = 0; itr < argc; itr++)
        {
          command->argv[itr] = malloc(strlen(argv[itr]) + 1);
          memset(command->argv[itr], '\0', (strlen(argv[itr]) + 1));
          strncpy((command->argv[itr]), argv[itr],  strlen(argv[itr]));
        }

      if (!g_taskrunning)
        {
          task_create("lte_test_task", LTEAPI_TASK_PRI
            , LTEAPI_TASK_STACKSIZE, lteapi_test_task, NULL);
        }

      attr.mq_maxmsg  = LTEAPI_MAX_API_MQUEUE;
      attr.mq_msgsize = (sizeof(struct lteapi_command_s *));
      mqd = mq_open(LTEAPI_MQUEUE_PATH, O_RDWR | O_CREAT
        , LTEAPI_MQUEUE_MODE, &attr);

      if (mq_send(mqd, (FAR const char *)(&command)
        , sizeof(struct lteapi_command_s *), 0) != 0)
        {
          printf("mq_send to lte_task_main Failed!!\n");
          for (itr = 0; itr < (command->argc); itr++)
            {
              free(command->argv[itr]);
              command->argv[itr] = NULL;
            }
          free(command);
        }
    }
  return 0;
}
