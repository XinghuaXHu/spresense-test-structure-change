/****************************************************************************
 * test/sqa/combination/step_audio_lte/lte.c
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

#include <sdk/config.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include "netutils/cJSON.h"
#include "lte/lte_api.h"
//#include "netutils/webclient.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define APP_MQUEUE_NAME    "lte_http_get_sample_queue"
#define APP_MAX_MQUEUE_MSG 1
#define APP_MQUEUE_MODE    0666
#define APP_IOBUFFER_LEN   512
#define APP_SESSION_ID     1

/* APN settings */

#ifdef CONFIG_EXAMPLES_LTE_HTTP_GET_APN_NAME
#  define APP_APN_NAME     CONFIG_EXAMPLES_LTE_HTTP_GET_APN_NAME
#else
#  define APP_APN_NAME     "lte_http_get_sample_apn"
#endif

#ifdef CONFIG_EXAMPLES_LTE_HTTP_GET_APN_IPTYPE
#  define APP_APN_IPTYPE   CONFIG_EXAMPLES_LTE_HTTP_GET_APN_IPTYPE
#else
#  define APP_APN_IPTYPE   LTE_APN_IPTYPE_IP
#endif

#ifdef CONFIG_EXAMPLES_LTE_HTTP_GET_APN_AUTHTYPE
#  define APP_APN_AUTHTYPE CONFIG_EXAMPLES_LTE_HTTP_GET_APN_AUTHTYPE
#else
#  define APP_APN_AUTHTYPE LTE_APN_AUTHTYPE_NONE
#endif

#ifdef CONFIG_EXAMPLES_LTE_HTTP_GET_APN_USERNAME
#  define APP_APN_USR_NAME CONFIG_EXAMPLES_LTE_HTTP_GET_APN_USERNAME
#else
#  define APP_APN_USR_NAME ""
#endif

#ifdef CONFIG_EXAMPLES_LTE_HTTP_GET_APN_PASSWD
#  define APP_APN_PASSWD   CONFIG_EXAMPLES_LTE_HTTP_GET_APN_PASSWD
#else
#  define APP_APN_PASSWD   ""
#endif

#define APP_WGET_URL       "http://example.com/index.html"

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct app_message_header_s
{
  int           result;
  unsigned int  param_size;
};

struct app_message_s
{
  struct app_message_header_s header;
  unsigned char               payload;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

//static char g_app_iobuffer[APP_IOBUFFER_LEN];
extern uint32_t step_count;
static int g_http_header_num = 0;
static const char *g_http_headers[10];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

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
}

/****************************************************************************
 * Name: app_mq_create
 ****************************************************************************/

static int app_mq_create(FAR const char *mq_name)
{
  int            errcode;
  mqd_t          mqd;
  struct mq_attr mq_attr;

  mq_attr.mq_maxmsg  = APP_MAX_MQUEUE_MSG;
  mq_attr.mq_msgsize = sizeof(int);
  mq_attr.mq_flags   = 0;

  /* Create message queue */

  mqd = mq_open(mq_name, (O_RDWR | O_CREAT), APP_MQUEUE_MODE, &mq_attr);
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
 * Name: app_mq_delete
 ****************************************************************************/

static void app_mq_delete(FAR const char *mq_name)
{
  /* Delete message queue */

  mq_unlink(mq_name);
}

/****************************************************************************
 * Name: app_mq_notify_result
 ****************************************************************************/

static void app_mq_notify_result(int result)
{
  int   ret;
  mqd_t mqd;
  int   errcode;
  int   buffer = result;

  /* Open message queue for send */

  mqd = mq_open(APP_MQUEUE_NAME, O_WRONLY);
  if (mqd < 0)
    {
      errcode = errno;
      printf("mq_open() failed: %d\n", errcode);
      return;
    }

  /* Send result of callback */

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
 * Name: app_wait_lte_callback
 ****************************************************************************/

static int app_wait_lte_callback(int *result)
{
  int   ret;
  mqd_t mqd;
  int   errcode;
  int   buffer;

  /* Open message queue for receive */

  mqd = mq_open(APP_MQUEUE_NAME, O_RDONLY);
  if (mqd < 0)
    {
      errcode = errno;
      printf("mq_open() failed: %d\n", errcode);
      return -1;
    }

  /* Receive result of callback */

  ret = mq_receive(mqd, (FAR char*)&buffer, sizeof(buffer), 0);
  if (ret < 0)
    {
      errcode = errno;
      printf("mq_send() failed: %d\n", errcode);
      mq_close(mqd);
      return -1;
    }
  mq_close(mqd);

  *result = buffer;

  return 0;
}

/****************************************************************************
 * Name: app_mq_notify_parameter
 ****************************************************************************/

static void app_mq_notify_parameter(int result, void* param,
                                    unsigned int size)
{
  int                   ret;
  mqd_t                 mqd;
  int                   errcode;
  struct app_message_s *buffer;

  buffer = malloc(sizeof(struct app_message_header_s) + size);
  if (buffer == NULL)
    {
      printf("failed to allocate memory\n");
      return;
    }

  /* Fill the message */

  buffer->header.result     = result;
  buffer->header.param_size = size;
  memcpy((void*)&buffer->payload, param, size);

  /* Open message queue for send */

  mqd = mq_open(APP_MQUEUE_NAME, O_WRONLY);
  if (mqd < 0)
    {
      errcode = errno;
      printf("mq_open() failed: %d\n", errcode);
      return;
    }

  /* Send result of callback */

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
 * Name: app_wait_lte_callback_with_parameter
 ****************************************************************************/

static int app_wait_lte_callback_with_parameter(int *result, void *param)
{
  int                   ret;
  mqd_t                 mqd;
  int                   errcode;
  struct app_message_s *buffer;

  /* Open message queue for receive */

  mqd = mq_open(APP_MQUEUE_NAME, O_RDONLY);
  if (mqd < 0)
    {
      errcode = errno;
      printf("mq_open() failed: %d\n", errcode);
      return -1;
    }

  /* Receive result of callback */

  ret = mq_receive(mqd, (FAR char*)&buffer, sizeof(buffer), 0);
  if (ret < 0)
    {
      errcode = errno;
      printf("mq_send() failed: %d\n", errcode);
      mq_close(mqd);
      return -1;
    }
  mq_close(mqd);

  *result = buffer->header.result;
  memcpy(param, (void*)&buffer->payload, buffer->header.param_size);

  free(buffer);

  return 0;
}

/****************************************************************************
 * Name: app_restart_cb
 *
 * Description:
 *   This callback is called when the startup is completed
 *   after power on the modem.
 ****************************************************************************/

static void app_restart_cb(uint32_t reason)
{
  char *reson_string[] =
    {
      "Modem restart by application.",
      "Modem restart by self."
    };
  printf("%s called. reason:%s\n", __func__, reson_string[reason]);

  /* Notify the result to the lte_http_get sample application task */

  app_mq_notify_result(reason);
}

/****************************************************************************
 * Name: app_show_errinfo
 *
 * Description:
 *   Show error information.
 ****************************************************************************/

static void app_show_errinfo(void)
{
  int           ret;
  lte_errinfo_t info = {0};

  ret = lte_get_errinfo(&info);
  if (ret == 0)
    {
      if (info.err_indicator & LTE_ERR_INDICATOR_ERRCODE)
        {
          printf("err_result_code : %d\n", info.err_result_code);
        }
      if (info.err_indicator & LTE_ERR_INDICATOR_ERRNO)
        {
          printf("err_no          : %d\n", info.err_no);
        }
      if (info.err_indicator & LTE_ERR_INDICATOR_ERRSTR)
        {
          printf("err_string      : %s\n", info.err_string);
        }
    }
}

/****************************************************************************
 * Name: app_show_pdn
 *
 * Description:
 *   Show PDN information.
 ****************************************************************************/

static void app_show_pdn(lte_pdn_t *pdn)
{
  int i;

  printf("pdn.session_id : %d\n", pdn->session_id);
  printf("pdn.active     : %d\n", pdn->active);
  printf("pdn.apn_type   : 0x%x\n", pdn->apn_type);

  for (i = 0; i < pdn->ipaddr_num; i++)
    {
      printf("pdn.ipaddr[%d].addr : %s\n", i, pdn->address[i].address);
    }
}

/****************************************************************************
 * Name: app_radio_on_cb
 *
 * Description:
 *   This callback is called when the radio on is completed.
 ****************************************************************************/

static void app_radio_on_cb(uint32_t result)
{
  printf("%s called. result: %d\n", __func__, result);

  if (result == LTE_RESULT_ERROR)
    {
      app_show_errinfo();
    }
  /* Notify the result to the lte_http_get sample application task */

  app_mq_notify_result(result);
}

/****************************************************************************
 * Name: app_radio_off_cb
 *
 * Description:
 *   This callback is called when the radio off is completed.
 ****************************************************************************/

static void app_radio_off_cb(uint32_t result)
{
  printf("%s called. result: %d\n", __func__, result);

  if (result == LTE_RESULT_ERROR)
    {
      app_show_errinfo();
    }

  /* Notify the result to the lte_http_get sample application task */

  app_mq_notify_result(result);
}

/****************************************************************************
 * Name: app_activate_pdn_cb
 *
 * Description:
 *   This callback is called when the connected to the PDN.
 ****************************************************************************/

static void app_activate_pdn_cb(uint32_t result, lte_pdn_t *pdn)
{
  printf("%s called. result: %d\n", __func__, result);

  if (result == LTE_RESULT_OK)
    {
      app_show_pdn(pdn);
    }
  if (result == LTE_RESULT_ERROR)
    {
      app_show_errinfo();
    }
  /* Notify the result to the lte_http_get sample application task */

  app_mq_notify_parameter(result, (void*)&pdn->session_id,
                          sizeof(pdn->session_id));
}

/****************************************************************************
 * Name: app_deactivate_pdn_cb
 *
 * Description:
 *   This callback is called when the disconnected from PDN.
 ****************************************************************************/

static void app_deactivate_pdn_cb(uint32_t result)
{
  printf("%s called. result: %d\n", __func__, result);

  if (result == LTE_RESULT_ERROR)
    {
      app_show_errinfo();
    }

  /* Notify the result to the lte_http_get sample application task */

  app_mq_notify_result(result);
}

/****************************************************************************
 * Name: app_localtime_report_cb
 *
 * Description:
 *   This callback is called when local time has changed.
 ****************************************************************************/

static void app_localtime_report_cb(FAR lte_localtime_t *localtime)
{
  printf("%s called: localtime : \"%02d/%02d/%02d : %02d:%02d:%02d\"\n",
         __func__, localtime->year, localtime->mon, localtime->mday,
         localtime->hour, localtime->min, localtime->sec);

  app_lte_setlocaltime(localtime);
}

/****************************************************************************
 * Name: app_wget_cb
 *
 * Description:
 *   As data is obtained from the host, this function is to output of
 *   each block of file data as it is received.
 ****************************************************************************/

//static void app_wget_cb(FAR char **buffer, int offset, int datend,
//                        FAR int *buflen, FAR void *arg)
//{
//  /* Write HTTP data to standard output */
//
//  (void)write(1, &((*buffer)[offset]), datend - offset);
//}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * lte_http_get_main
 ****************************************************************************/

int lte_http_post(int argc, char *argv[])
{
  int                     ret;
  int                     result = LTE_RESULT_OK;
  FAR char               *url    = APP_WGET_URL;
  struct lte_apn_setting  apnsetting;
  uint8_t                 data_pdn_sid = LTE_PDN_SESSIONID_INVALID_ID;
  cJSON *root;
  cJSON *step;
  char *out = NULL;
  char *host;
  char *path;
  int port;
  int option;
  int i;

  printf("%s %d\n", __func__, argc);

#if 0
  for (i=0; i<argc; i++)
    printf("argv[%d] [%s]\n", i, argv[i]);
#endif

  /* Parse input parameters */

  while ((option = getopt(argc-1, &argv[1], "H:")) != ERROR)
    {
      switch (option)
        {
          case 'H':
            g_http_headers[g_http_header_num] = optarg;
            g_http_header_num++;
            break;

          default:
          case '?':
            fprintf(stderr, "ERROR: Unrecognized option\n");
            break;
        }
    }

  /* There should be three final parameters remaining on the command line */

  if ((optind + 3) >= argc)
    {
      printf("ERROR: Missing required 'host' or 'port' or 'path' argument\n");
      return -1;
    }

  optind++;
  host = argv[optind++];
  port = atoi(argv[optind++]);
  path = argv[optind++];

  for( i = 0; i < g_http_header_num; i++)
    {
      printf("%s headers[%d] (%s)\n", __func__, i, g_http_headers[i]);
    }
  printf("%s host[%s] port[%d] path[%s]\n", __func__, host, port, path);

  /* This application is a sample that connect to the LTE network,
   * get a file with wget, and disconnect the LTE network.
   * The URL is specified by the second argument.
   * If URL is not specified, use the default URL.
   * The URL starts with "http://"
   * (eg, http://example.com/index.html, or http://192.0.2.1:80/index.html) */

  if (argc > 1)
    {
      url = argv[1];
    }

  /* Create a message queue. It is used to receive result from the
   * asynchronous API callback.*/

  ret = app_mq_create(APP_MQUEUE_NAME);
  if (ret < 0)
    {
      goto errout;
    }

  /* Initialize the LTE library */

  ret = lte_initialize();
  if (ret < 0)
    {
      printf("Failed to initialize LTE library :%d\n", ret);
      goto errout_with_fin;
    }

  /* Register callback for modem restart.
   * It must call this function after lte_initialize.
   * The callback will be invoked if the modem starts successfully
   * after calling lte_power_on.
   */

  ret = lte_set_report_restart(app_restart_cb);
  if (ret < 0)
    {
      printf("Failed to set report restart :%d\n", ret);
      goto errout_with_fin;
    }

  /* Power on the LTE modem */

  ret = lte_power_on();
  if (ret < 0)
    {
      printf("Failed to power on the modem :%d\n", ret);
      goto errout_with_lte_fin;
    }

  /* Wait until the modem startup normally and notification
   * comes from the callback(app_restart_cb)
   * registered by lte_set_report_restart. */

  ret = app_wait_lte_callback(&result);
  if (ret < 0)
    {
      goto errout_with_lte_fin;
    }

  /* Enable to receive events of local time change */

  ret = lte_set_report_localtime(app_localtime_report_cb);
  if (ret < 0)
    {
      printf("Failed to set report local time :%d\n", ret);
      goto errout_with_lte_fin;
    }

  /* Radio on and start to search for network */

  ret = lte_radio_on(app_radio_on_cb);
  if (ret < 0)
    {
      printf("Failed to set radio on :%d\n", ret);
      goto errout_with_lte_fin;
    }

  /* Wait until the radio on is completed and notification
   * comes from the callback(app_radio_on_cb)
   * registered by lte_radio_on. */

  ret = app_wait_lte_callback(&result);
  if ((ret < 0) || (result == LTE_RESULT_ERROR))
    {
      goto errout_with_lte_fin;
    }

  /* Attach to the LTE network and connect to the data PDN */

  apnsetting.apn       = (int8_t*)APP_APN_NAME;
  apnsetting.ip_type   = APP_APN_IPTYPE;
  apnsetting.auth_type = APP_APN_AUTHTYPE;
  apnsetting.apn_type  = LTE_APN_TYPE_DEFAULT | LTE_APN_TYPE_IA;
  apnsetting.user_name = (int8_t*)APP_APN_USR_NAME;
  apnsetting.password  = (int8_t*)APP_APN_PASSWD;

  ret = lte_activate_pdn(&apnsetting, app_activate_pdn_cb);
  if (ret < 0)
    {
      printf("Failed to activate PDN :%d\n", ret);
      goto errout_with_lte_fin;
    }

  /* Wait until the connect completed and notification
   * comes from the callback(app_activate_pdn_cb)
   * registered by lte_activate_pdn. */

  ret = app_wait_lte_callback_with_parameter(&result, &data_pdn_sid);
  if ((ret < 0) || (result == LTE_RESULT_ERROR))
    {
      goto errout_with_lte_fin;
    }

//  /* Retrieve the file with the specified URL. */
//
//  wget(url, g_app_iobuffer, APP_IOBUFFER_LEN, app_wget_cb, NULL);

      NT_http_init();

  for(;;)
    {
      sleep(10);

      root = cJSON_CreateObject();
      cJSON_AddStringToObject(root, "host", "spritzer");
      cJSON_AddStringToObject(root, "ts_period", "10");
      cJSON_AddStringToObject(root, "ts_unit", "s");
      cJSON_AddItemToObject(root, "step", step = cJSON_CreateObject());
      cJSON_AddNumberToObject(step, "step", step_count);
      out = cJSON_Print(root);
      printf("%s\n", out);
      /* Send HTTP request to server using POST method */

      http_post_request(0, host, port, path,
                        g_http_headers, g_http_header_num,
                        out, strlen(out));

      /* Delete JSON object */

      cJSON_Delete(root);
      free(out);
    }
      
  /* Disconnect from the data PDN and Detach from the LTE network */

  ret = lte_deactivate_pdn(data_pdn_sid, app_deactivate_pdn_cb);
  if (ret < 0)
    {
      printf("Failed to deactivate PDN :%d\n", ret);
      goto errout_with_lte_fin;
    }

  /* Wait until the deactivate PDN is completed and notification
   * comes from the callback(app_deactivate_pdn_cb)
   * registered by lte_deactivate_pdn. */

  ret = app_wait_lte_callback(&result);
  if ((ret < 0) || (result == LTE_RESULT_ERROR))
    {
      goto errout_with_lte_fin;
    }

  /* Radio off */

  ret = lte_radio_off(app_radio_off_cb);
  if (ret < 0)
    {
      printf("Failed to set radio off :%d\n", ret);
      goto errout_with_lte_fin;
    }

  /* Wait until the radio off is completed and notification
   * comes from the callback(app_radio_off_cb)
   * registered by lte_radio_off. */

  ret = app_wait_lte_callback(&result);
  if ((ret < 0) || (result == LTE_RESULT_ERROR))
    {
      goto errout_with_lte_fin;
    }

  /* Power off the modem. If asynchronous API has not notified
   * the result by callback, it will be canceled */

  ret = lte_power_off();
  if (ret < 0)
    {
      printf("Failed to power off the modem :%d\n", ret);
      goto errout_with_lte_fin;
    }

  /* Finalize LTE library
   * If this function is called while the modem power is on, shutdown the modem */

  ret = lte_finalize();
  if (ret < 0)
    {
      printf("Failed to finalize LTE library :%d\n", ret);
      goto errout_with_fin;
    }

  /* Delete a message queue */

  app_mq_delete(APP_MQUEUE_NAME);

  return 0;

errout_with_lte_fin:
  lte_finalize();

errout_with_fin:
  app_mq_delete(APP_MQUEUE_NAME);

errout:
  return -1;
}
