/****************************************************************************
 * test/lwm2m/lwm2m_main.c
 *
 *   Copyright 2018 Sony Corporation
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

#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "lte_connection.h"
#include "lwm2mclient.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LWM2M_TEST_SERVER            0
#define LWM2M_TEST_NOKIA             1
#define LWM2M_TEST_LIFETIME          10
#define LWM2M_TEST_DTLS              16
#define LWM2M_TEST_DTLS_ERR          17
#define LWM2M_TEST_NAME_NOKIA        "selftest.iot.nokia.com"
#define LWM2M_TEST_PORT_NOKIA        "5683"
#define LWM2M_TEST_NAME_DTLS         "leshan.eclipse.org"
#define LWM2M_TEST_PORT_DTLS         "5684"
#define LWM2M_TEST_LIFETIME_DEFAULT  "300"
#define LWM2M_TEST_LIFETIME_TEST     "30"
#define LWM2M_TEST_ARGC_MAX          16
#define LWM2M_TEST_ARGV_MAX          64

#define LWM2MTEST_FUNC_LOOP          0
#define LWM2MTEST_FUNC_CONNECT       1

#define LWM2MTEST_FLAG_FAIL          0	/* Need to fail test */
#define LWM2MTEST_FLAG_SUCCESS       1	/* Need to success test */

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int g_status =  0;
struct wstest_t
{
  char* code;
  int   func;
  int   loop;
  int   host;
  int   flag;
};
static struct wstest_t wstest_site[] = 
{
  { "01", LWM2MTEST_FUNC_LOOP      , 10, LWM2M_TEST_LIFETIME, LWM2MTEST_FLAG_SUCCESS },
  { "10", LWM2MTEST_FUNC_CONNECT   ,  1, LWM2M_TEST_SERVER, LWM2MTEST_FLAG_SUCCESS },
  { "11", LWM2MTEST_FUNC_CONNECT   ,  1, LWM2M_TEST_NOKIA, LWM2MTEST_FLAG_SUCCESS },
  { "12", LWM2MTEST_FUNC_CONNECT   ,  1, LWM2M_TEST_DTLS, LWM2MTEST_FLAG_SUCCESS },
  { "13", LWM2MTEST_FUNC_CONNECT   ,  1, LWM2M_TEST_DTLS_ERR, LWM2MTEST_FLAG_FAIL },
  { NULL, 0, 0, 0, LWM2MTEST_FLAG_SUCCESS },
};
static volatile int scene_state = 0;
static int func_argc = 0;
static char *func_argv[LWM2M_TEST_ARGC_MAX];
static char scene_argv[LWM2M_TEST_ARGC_MAX][LWM2M_TEST_ARGV_MAX];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int lwm2mtest_start(int server, int flag)
{
  int cnt = 0;
  int result;

  // state update
  scene_state = 1;

  /* Initialize */
  for (cnt=0; cnt<LWM2M_TEST_ARGC_MAX; cnt++)
    {
      memset(scene_argv[cnt], 0, LWM2M_TEST_ARGV_MAX);
      func_argv[cnt] = scene_argv[cnt];
    }
  func_argc = 0;
  strncpy(scene_argv[func_argc++], CONFIG_TEST_LWM2M_CLIENT_NAME,
          LWM2M_TEST_ARGV_MAX);

  /* Client parameter */
  strncpy(scene_argv[func_argc++], "-n", LWM2M_TEST_ARGV_MAX);
  strncpy(scene_argv[func_argc++], CONFIG_TEST_LWM2M_CLIENT_NAME,
          LWM2M_TEST_ARGV_MAX);
  strncpy(scene_argv[func_argc++], "-l", LWM2M_TEST_ARGV_MAX);
  strncpy(scene_argv[func_argc++], CONFIG_TEST_LWM2M_CLIENT_PORT,
          LWM2M_TEST_ARGV_MAX);

  /* Server parameter */
  switch (server)
    {
      case LWM2M_TEST_LIFETIME:
          strncpy(scene_argv[func_argc++], "-h", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], CONFIG_TEST_LWM2M_SERVER_HOST,
                  LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], "-p", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], CONFIG_TEST_LWM2M_SERVER_PORT,
                  LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], "-t", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], LWM2M_TEST_LIFETIME_TEST, LWM2M_TEST_ARGV_MAX);
          break;

      case LWM2M_TEST_DTLS:
          strncpy(scene_argv[func_argc++], "-h", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], LWM2M_TEST_NAME_DTLS, LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], "-p", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], LWM2M_TEST_PORT_DTLS, LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], "-i", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], CONFIG_TEST_LWM2M_LESHAN_IDENTITY,
                  LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], "-s", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], CONFIG_TEST_LWM2M_LESHAN_PSK,
                  LWM2M_TEST_ARGV_MAX);
          break;

      case LWM2M_TEST_DTLS_ERR:
          strncpy(scene_argv[func_argc++], "-h", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], LWM2M_TEST_NAME_DTLS, LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], "-p", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], LWM2M_TEST_PORT_DTLS, LWM2M_TEST_ARGV_MAX);
          break;

      case LWM2M_TEST_NOKIA:
          strncpy(scene_argv[func_argc++], "-h", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], LWM2M_TEST_NAME_NOKIA, LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], "-p", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], LWM2M_TEST_PORT_NOKIA, LWM2M_TEST_ARGV_MAX);
          break;

      case LWM2M_TEST_SERVER:
      default:
          strncpy(scene_argv[func_argc++], "-h", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], CONFIG_TEST_LWM2M_SERVER_HOST,
                  LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], "-p", LWM2M_TEST_ARGV_MAX);
          strncpy(scene_argv[func_argc++], CONFIG_TEST_LWM2M_SERVER_PORT,
                  LWM2M_TEST_ARGV_MAX);
          break;
    }

  /* Network parameter */
  strncpy(scene_argv[func_argc++], "-4", LWM2M_TEST_ARGV_MAX);

  /* Start */
  result = lwm2m_main(func_argc, func_argv);

  // state update
  scene_state = 0;

  if ((result == 0) && (flag == LWM2MTEST_FLAG_SUCCESS))
    {
      printf("[OK] lwm2m_main success\n");
    }
  else if ((result != 0) && (flag == LWM2MTEST_FLAG_FAIL))
    {
      printf("[OK] lwm2m_main error : %d\n", result);
    }
  else
    {
      printf("[ERR] lwm2m_main failed : %d\n", result);
    }

  return result;
}

static void lwm2mtest_loop(int loop, int flag)
{
  int result = 0;
  int cntc = 0;

  /* Main loop */
  for (cntc=0; cntc<loop; cntc++)
    {
      /* Start */
      if ((result = lwm2mtest_start(LWM2M_TEST_LIFETIME, flag)) < 0)
        {
          break;
        }

      /* Wait until connection re-established */
      printf("Loop count [%02d/%02d] finished\n", cntc+1, loop);
      sleep(1);
    }

  return;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int lwm2m_main(int argc, char *argv[])
#endif
{
  int ret;
  int exe = 0;
  struct wstest_t *site;

  if ((g_status == 1) && (argc > 1) && (strncmp(argv[1], "off", 3) == 0))
    {
      /* LTE finish only */
      g_status = 0;
      app_disconnect_from_lte();
      return 0;
    }

  else if (g_status == 0)
    {
      /* LTE connect */ 
      printf("Start to LTE connect\n");
      ret = app_connect_to_lte();
      if (ret < 0)
        {
          return -1;
        }
      g_status = 1;
    }

  site = &wstest_site[0];
  while (site->code)
    {
      /* Main TLS tests */
      if ((argc > 1) && (strncmp(argv[1], site->code, 2) == 0))
        {
          switch(site->func)
            {
              case LWM2MTEST_FUNC_LOOP:
                  lwm2mtest_loop(site->loop, site->flag);
                  exe++;
                  break;
              case LWM2MTEST_FUNC_CONNECT:
                  lwm2mtest_start(site->host, site->flag);
                  exe++;
                  break;
              default:
                  break;
            }
        }
      site++;
    }
 
  if (exe == 0)
    {
      printf("Not exist test funcion : %s\n", argv[1]);
      printf("Funcion list :\n");
      printf("[Library] : 01\n");
      printf("[Client]  : 10 - 13\n");
    }

  return 0;
}
