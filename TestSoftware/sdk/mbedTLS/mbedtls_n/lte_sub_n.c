/****************************************************************************
 * test/mbedtls/lte_sub.c
 *
 *   Copyright 2019 Sony Corporation
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
 * 3. Neither the name NuttX nor Sony nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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
#include <sdk/config.h>
#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#ifdef CONFIG_EXTERNALS_MBEDTLS
#include <sys/time.h>
#endif

#include "lte/lte_api.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LTE_SAMPLE_MQUEUE_NAME_API  "lte_sample_queue_api"
#define MAX_MQUEUE_MSG_API          1
#define MQUEUE_MODE                 0666

/* APN settings */

#define APP_APN_NAME     "iijmobile.biz"
#define APP_APN_IPTYPE   LTE_APN_IPTYPE_IPV4V6
#define APP_APN_AUTHTYPE LTE_APN_AUTHTYPE_PAP
//#define APP_APN_AUTHTYPE LTE_APN_AUTHTYPE_CHAP

#define APP_APN_USR_NAME "mobile@iij"
#define APP_APN_PASSWD   "iij"

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

/****************************************************************************
 * Global variables
 ***************************************************************************/

struct lte_apn_setting  g_apnsetting;
struct lte_pdn g_pdnsetting;

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

/****************************************************************************
 * Name: app_init
 ****************************************************************************/

static int app_init(void)
{
  int errcode;
  mqd_t mqd;
  struct mq_attr mq_attr;

  mq_attr.mq_maxmsg  = MAX_MQUEUE_MSG_API;
  mq_attr.mq_msgsize = sizeof(int);
  mq_attr.mq_flags   = 0;

  /* Create message queue resource */

  mqd = mq_open(LTE_SAMPLE_MQUEUE_NAME_API, (O_RDWR | O_CREAT),
                MQUEUE_MODE, &mq_attr);
  if (mqd < 0)
    {
      errcode = errno;
      printf("mq_open() failed: %d\n", errcode);
      return -1;
    }
  mq_close(mqd);

  return 0;
}

/****************************************************************************
 * Name: app_fin
 ****************************************************************************/

static void app_fin(void)
{
  mq_unlink(LTE_SAMPLE_MQUEUE_NAME_API);
}

/****************************************************************************
 * Name: app_notify_response
 ****************************************************************************/

static void app_notify_response(int response)
{
  int ret;
  mqd_t mqd;
  int errcode;
  int buffer = response;

  /* Open message queue for send */

  mqd = mq_open(LTE_SAMPLE_MQUEUE_NAME_API, O_WRONLY);
  if (mqd < 0)
    {
      errcode = errno;
      printf("mq_open() failed: %d\n", errcode);
      return;
    }

  /* Send response */

  ret = mq_send(mqd, (FAR const char*)&buffer, sizeof(buffer), 0);
  if (ret < 0)
    {
      errcode = errno;
      printf("mq_send() failed: %d\n", errcode);
      mq_close(mqd);
      return;
    }
  mq_close(mqd);
}

/****************************************************************************
 * Name: app_wait_response
 ****************************************************************************/

static int app_wait_response(int *response)
{
  int ret;
  mqd_t mqd;
  int errcode;
  int resp;

  /* Open message queue for receive */

  mqd = mq_open(LTE_SAMPLE_MQUEUE_NAME_API, O_RDONLY);
  if (mqd < 0)
    {
      errcode = errno;
      printf("mq_open() failed: %d\n", errcode);
      return -1;
    }

  /* Receive response */

  ret = mq_receive(mqd, (FAR char*)&resp, sizeof(resp), 0);
  if (ret < 0)
    {
      errcode = errno;
      printf("mq_send() failed: %d\n", errcode);
      mq_close(mqd);
      return -1;
    }
  mq_close(mqd);

  *response = resp;

  return 0;
}

/****************************************************************************
 * Name: app_errorcase_print
 ***************************************************************************/

static void app_errorcase_print(uint32_t result, uint32_t errcause)
{
  if (LTE_RESULT_ERROR == result)
    {
      switch (errcause)
      {
        case LTE_ERR_WAITENTERPIN:
          {
            printf("[ERR] : Waiting for PIN enter\n");
          }
        break;
        case LTE_ERR_REJECT:
          {
            printf("[ERR] : Rejected from the network\n");
          }
        break;
        case LTE_ERR_MAXRETRY:
          {
            printf("[ERR] : No response from the network\n");
          }
        break;
        case LTE_ERR_BARRING:
          {
            printf("[ERR] : Network barring\n");
          }
        break;
        default:
          {
            printf("[ERR] : Unexpected cause\n");
          }
        break;
      }
    }
}

/****************************************************************************
 * Name: app_poweron_cb
 ****************************************************************************/

static void app_poweron_cb(uint32_t result)
{
  printf("%s called\n", __func__);

  app_notify_response(result);
}

/****************************************************************************
 * Name: app_poweroff_cb
 ****************************************************************************/

static void app_poweroff_cb(uint32_t result)
{
  printf("%s called\n", __func__);

  app_notify_response(result);
}

/****************************************************************************
 * Name: app_get_imsi_cb
 ****************************************************************************/

static void app_get_imsi_cb(uint32_t result, uint8_t errcause, int8_t *imsi)
{
  if (result == LTE_RESULT_OK)
    {
      printf("%s called: result:%d, imsi:%s\n", __func__,
         result, imsi);
    }
  else
    {
      printf("%s called: result:%d, errcause:%d\n", __func__,
        result, errcause);
      app_errorcase_print(result, errcause);
    }
  app_notify_response(result);
}

/****************************************************************************
 * Name: app_attach_net_cb
 ****************************************************************************/

static void app_attach_net_cb(uint32_t result, uint32_t errcause)
{
  if (LTE_RESULT_ERROR == result)
    {
      printf("%s called: result:%d errorcause:%d\n",
             __func__, result, errcause);
      app_errorcase_print(result, errcause);
    }
  else
    {
      printf("%s called: result:%d\n", __func__, result);
    }

  app_notify_response(result);
}

/****************************************************************************
 * Name: app_detach_net_cb
 ****************************************************************************/

static void app_detach_net_cb(uint32_t result)
{
  printf("%s called: result:%d\n", __func__, result);
  app_notify_response(result);
}

/****************************************************************************
 * Name: app_data_on_cb
 ****************************************************************************/

static void app_data_on_cb(uint32_t result, uint32_t errcause)
{
  if (LTE_RESULT_ERROR == result)
    {
      printf("%s called: result:%d errorcause:%d\n",
             __func__, result, errcause);
    }
  else
    {
      printf("%s called: result:%d\n", __func__, result);
    }

  app_notify_response(result);
}

/****************************************************************************
 * Name: app_data_off_cb
 ****************************************************************************/

static void app_data_off_cb(uint32_t result, uint32_t errcause)
{
  if (LTE_RESULT_ERROR == result)
    {
      printf("%s called: result:%d errorcause:%d\n",
             __func__, result, errcause);
    }
  else
    {
      printf("%s called: result:%d\n", __func__, result);
    }

  app_notify_response(result);
}


#ifdef CONFIG_EXTERNALS_MBEDTLS

/****************************************************************************
 * Name: app_lte_setlocaltime
 ****************************************************************************/

static void app_lte_setlocaltime(FAR lte_localtime_t *localtime)
{
  int            ret;
  struct tm      calTime;
  struct timeval current_time = {0};

  /* lte_localtime_t -> struct tm */

  memset(&calTime, 0, sizeof(struct tm));
  calTime.tm_year = localtime->year + 100; /* 1900 + 100 + year(0-99) */
  calTime.tm_mon  = localtime->mon - 1;    /* mon(1-12) - 1 */
  calTime.tm_mday = localtime->mday;
  calTime.tm_hour = localtime->hour;
  calTime.tm_min  = localtime->min;
  calTime.tm_sec  = localtime->sec;

  /* struct tm -> struct time_t */

  current_time.tv_sec = mktime(&calTime);
  if (current_time.tv_sec < 0)
    {
      printf("%s: mktime falied\n");
      return;
    }

  /* Set time */

  ret = settimeofday(&current_time, NULL);
  if (ret < 0)
    {
      printf("%s: settimeofday falied: %d\n", errno);
      return;
    }

  printf("set localtime completed: %4d/%02d/%02d,%02d:%02d:%02d\n",
         localtime->year + 1900 + 100, localtime->mon, localtime->mday,
         localtime->hour, localtime->min, localtime->sec);
  sleep(2);
}

/****************************************************************************
 * Name: app_localtime_report_cb
 ****************************************************************************/

static void app_localtime_report_cb(FAR lte_localtime_t *localtime)
{
  printf("%s called: localtime : \"%02d/%02d/%02d : %02d:%02d:%02d\"\n",
         __func__, localtime->year, localtime->mon, localtime->mday,
         localtime->hour, localtime->min, localtime->sec);

  app_lte_setlocaltime(localtime);
//  app_notify_response(0);
}
#endif

/****************************************************************************
 * Name: app_lte_init
 ****************************************************************************/

static int app_lte_init(void)
{
  int ret;
  int response = LTE_RESULT_OK;

  /* Initialize the LTE library */

  ret = lte_initialize();
  if (ret < 0)
    {
      printf("Failed to initialize LTE library :%d\n", ret);
      goto errout;
    }

  /* Power on the modem
   * If it succeeds, it will be able to accept requests from all APIs */

#ifdef _OLD_API_
  ret = lte_power_control(LTE_POWERON, app_poweron_cb);
  if (ret < 0)
    {
      printf("Failed to power on the modem :%d\n", ret);
      goto errout_with_fin;
    }

  /* Wait until the modem startup completed */

  ret = app_wait_response(&response);
  if ((ret < 0) || (response != LTE_RESULT_OK))
    {
      goto errout_with_fin;
    }
#else
  ret = lte_set_report_restart(app_poweron_cb);
  if (ret < 0)
    {
      printf("Failed in set_report_restart :%d\n", ret);
      goto errout_with_fin;
    }

  ret = lte_power_on();
  if (ret < 0)
    {
      printf("Failed in power_on :%d\n", ret);
      goto errout_with_fin;
    }
  else
    {
      printf("power_on OK\n");
    }
  /* Wait until the modem startup completed */

  ret = app_wait_response(&response);
  if ((ret < 0) || (response != LTE_RESULT_OK))
    {
      goto errout_with_fin;
    }

#endif
  return 0;

errout_with_fin:
  lte_finalize();

errout:
  return ret;
}

/****************************************************************************
 * Name: app_lte_fin
 ****************************************************************************/

static int app_lte_fin(void)
{
  int ret;
  int response = LTE_RESULT_OK;

  /* Power off the modem
   * If it succeeds, it will be not able to accept requests from all APIs */

#ifdef _OLD_API_ 
  ret = lte_power_control(LTE_POWEROFF, app_poweroff_cb);
  if (ret < 0)
    {
      printf("Failed to power off the modem :%d\n", ret);
      goto errout_with_fin;
    }

  /* Wait until the modem shutdown completed */

  ret = app_wait_response(&response);
  if ((ret < 0) || (response != LTE_RESULT_OK))
    {
      goto errout_with_fin;
    }
#else
  ret = lte_power_off();
  if (ret < 0)
    {
      printf("Failed in power off the modem :%d\n", ret);
      goto errout_with_fin;
    }
  ret = app_wait_response(&response);
  if ((ret < 0) || (response != LTE_RESULT_OK))
    {
      goto errout_with_fin;
    }

#endif
  /* Finalize LTE library */

  ret = lte_finalize();
  if (ret < 0)
    {
      printf("Failed to finalize LTE library :%d\n", ret);
      goto errout;
    }


  return 0;

errout_with_fin:
  lte_finalize();

errout:
  return ret;
}

static void show_pdn(lte_pdn_t *pdn)
{
  int i;
  printf("[CB_VAL] %s pdn.session_id : \"%d\"\n", __func__, pdn->session_id);
  g_pdnsetting.session_id = pdn->session_id;

  printf("[CB_VAL] %s pdn.active : \"%d\"\n", __func__, pdn->active);
  printf("[CB_VAL] pdn.active : \"%d\" -> %s\n", __func__, pdn->active,
    ((LTE_PDN_ACTIVE == pdn->active)? "LTE_PDN_ACTIVE" : "LTE_PDN_DEACTIVE"));

  printf("[CB_VAL] %s pdn.apn_type : \"%d\"\n", __func__, pdn->apn_type);
  printf("[CB_VAL] %s pdn.ipaddr_num : \"%d\"\n", __func__, pdn->ipaddr_num);
  for (i = 0; i < pdn->ipaddr_num; i++)
    {
      printf("[CB_VAL] %s pdn.ipaddr[%d].ip_type : \"%d\"\n", __func__,
        i, pdn->address[i].ip_type);
      printf("[CB_VAL] pdn.ipaddr[%d].ip_type : \"%d\" -> %s\n",
        i, pdn->address[i].ip_type,
        ((LTE_IPTYPE_V4 == pdn->address[i].ip_type)? "LTE_IPTYPE_V4" : "LTE_IPTYPE_V6"));
      printf("[CB_VAL] %s pdn.ipaddr[%d].addr : \"%s\"\n", __func__, i,
        pdn->address[i].address);
    }
  printf("[CB_VAL] %s pdn.session_id : \"%d\"\n", __func__, pdn->session_id);
  printf("[CB_VAL] %s pdn.ims_register : \"%d\"\n", __func__,
    pdn->ims_register);
  printf("[CB_VAL] pdn.ims_register : \"%d\" -> %s\n", pdn->ims_register,
    ((LTE_IMS_REGISTERED == pdn->ims_register)? "LTE_IMS_REGISTERED" : "LTE_IMS_NOT_REGISTERED"));
  printf("[CB_VAL] %s pdn.data_allow : \"%d\"\n", __func__, pdn->data_allow);
  printf("[CB_VAL] pdn.data_allow : \"%d\" -> %s\n",pdn->data_allow,
    ((LTE_DATA_ALLOW == pdn->data_allow)? "LTE_DATA_ALLOW" : "LTE_DATA_DISALLOW"));
  printf("[CB_VAL] %s pdn.data_roaming_allow : \"%d\"\n", __func__,
    pdn->data_roaming_allow);
  printf("[CB_VAL] pdn.data_roaming_allow : \"%d\" -> %s\n",
    pdn->data_roaming_allow,
    ((LTE_DATA_ALLOW == pdn->data_roaming_allow)? "LTE_DATA_ALLOW" : "LTE_DATA_DISALLOW"));
}
static void actpdn_cb(uint32_t result, lte_pdn_t *pdn)
{
  LTEAPI_PRINT_RESULT(result);
  if (result == LTE_RESULT_OK)
    {
      show_pdn(pdn);
    }
  app_notify_response(result);
}
static void radioon_cb(uint32_t result)
{
  LTEAPI_PRINT_RESULT(result);
  app_notify_response(result);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int lte_init(void)
{
  int response = LTE_RESULT_OK;
  int ret;

  ret = app_init();
  if (ret < 0)
    {
      printf("app_init error ret:%d \n",ret);
      return ret;
    }

  ret = app_lte_init();
  if (ret < 0)
    {
      printf("app_lte_init error ret:%d \n",ret);
      goto errout_app;
    }
#ifdef CONFIG_EXTERNALS_MBEDTLS
  ret = lte_set_report_localtime(app_localtime_report_cb);
  if (ret < 0)
    {
      printf("Failed to set report local time :%d\n", ret);
      goto errout_app;
    }
#endif
  ret = lte_get_imsi(app_get_imsi_cb);
  if (ret < 0)
    {
      printf("lte_get_imsi() failed :%d\n", ret);
      goto errout_off;
    }

  ret = app_wait_response(&response);
  if ((ret < 0) || (response != LTE_RESULT_OK))
    {
      goto errout_off;
    }
#ifdef _OLD_API_
  ret = lte_attach_network(app_attach_net_cb);
  if (ret < 0)
    {
      printf("Failed to attach to LTE network :%d\n", ret);
      goto errout_off;
    }

  ret = app_wait_response(&response);
  if ((ret < 0) || (response == LTE_RESULT_ERROR))
    {
      goto errout_off;
    }
  
  ret = lte_data_on(LTE_SESSION_ID_MIN, app_data_on_cb);
  if (ret < 0)
    {
      printf("Failed to Data-ON :%d\n", ret);
      goto errout_off;
    }

  ret = app_wait_response(&response);
  if ((ret < 0) || (response == LTE_RESULT_ERROR))
    {
      goto errout_off;
    }

#else
    ret = lte_radio_on(CODE radioon_cb);
    if (ret < 0)
      {
        printf("Failed in radioon %d\n", ret);
        goto errout_off;
      }
    ret = app_wait_response(&response);
    if ((ret < 0) || (response == LTE_RESULT_ERROR))
      {
        printf("response error ret:%d\n",ret);
        goto errout_off;
      }

    printf("set pdn parameter \n");
    /* Set the APN to be connected.
     * Check the APN settings of the carrier according to the your environment.
     * Note that need to set apn_type to LTE_APN_TYPE_DEFAULT | LTE_APN_TYPE_IA.
     * This means APN type for data traffic.
     */

    g_apnsetting.apn       = (int8_t*)APP_APN_NAME;
    g_apnsetting.apn_type  = LTE_APN_TYPE_DEFAULT | LTE_APN_TYPE_IA;
    g_apnsetting.ip_type   = APP_APN_IPTYPE;

    /* Depending on the APN, authentication may not be necessary.
     * In this case, set auth_type to LTE_APN_AUTHTYPE_NONE,
     * and set user_name, password to NULL.
     */

    g_apnsetting.auth_type = APP_APN_AUTHTYPE;
    g_apnsetting.user_name = (int8_t*)APP_APN_USR_NAME;
    g_apnsetting.password  = (int8_t*)APP_APN_PASSWD;


    /* Attach to the LTE network and connect to the data PDN */

    printf("called activate_pdn \n");
    ret = lte_activate_pdn(&g_apnsetting, actpdn_cb);
    if (ret < 0)
      {
        printf("Failed to activate PDN :%d\n", ret);
        goto errout_off;
      }
    ret = app_wait_response(&response);
    if ((ret < 0) || (response == LTE_RESULT_ERROR))
      {
        goto errout_off;
      }

 
#endif

  return 0;

errout_off:
  app_lte_fin();

errout_app:
  app_fin();

  if (ret == 0)
    {
      ret = -1;
    }
  return ret;
}

static void deactpdn_cb(uint32_t result)
{
   LTEAPI_PRINT_RESULT(result);
}

int lte_fin(void)
{
  int ret;
  int response = LTE_RESULT_OK;

#ifdef _OLD_API_
  ret = lte_data_off(LTE_SESSION_ID_MIN, app_data_off_cb);
  if (ret < 0)
    {
      printf("Failed to Data-OFF :%d\n", ret);
    }
  app_wait_response(&response);

  ret = lte_detach_network(app_detach_net_cb);
  if (ret < 0)
    {
      printf("Failed to detach to LTE network :%d\n", ret);
    }
  app_wait_response(&response);

#else
  ret = lte_deactivate_pdn(g_pdnsetting.session_id, deactpdn_cb); 
  if (ret < 0)
    {
      printf("Failed in deactivate_pdn :%d\n", ret);
    }
  app_wait_response(&response);
#endif
  app_lte_fin();

  app_fin();

  return 0;
}

