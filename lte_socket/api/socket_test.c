/****************************************************************************
 * test/lte_socket/api/socket_test.c
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

#ifndef HOST_MAKE
#  include <nuttx/config.h>
#  include <sdk/config.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>

#include "apitest.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_socket_s
{
  FAR const char *test_num;

  /* Input parameters */

  int  domain;
  int  type;
  int  protocol;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

struct apitest_socket_s g_apitest_socket_param[] =
{
  { "13-01", PF_UNSPEC, SOCK_STREAM,    0, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, EAFNOSUPPORT}},
  { "13-02", PF_UNIX,   SOCK_STREAM,    0, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, EAFNOSUPPORT}},
  { "13-03", PF_LOCAL,  SOCK_STREAM,    0, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, EAFNOSUPPORT}},
  { "13-04", PF_INET,   SOCK_STREAM,    0, { VERIFY_GE, 0},  false, { VERIFY_EQ, DONTCARE_VAL}},
  { "13-05", PF_INET6,  SOCK_STREAM,    0, { VERIFY_GE, 0},  false, { VERIFY_EQ, DONTCARE_VAL}},
  { "13-06", PF_PACKET, SOCK_STREAM,    0, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, EAFNOSUPPORT}},
  /* 13-07 : same as 13-04 */
  { "13-08", PF_INET,   SOCK_DGRAM,     0, { VERIFY_GE, 0},  false, { VERIFY_EQ, DONTCARE_VAL}},
  { "13-09", PF_INET,   SOCK_SEQPACKET, 0, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, EPROTONOSUPPORT}},
  { "13-10", PF_INET,   SOCK_RAW,       0, { VERIFY_GE, 0},  false, { VERIFY_EQ, DONTCARE_VAL}},
  { "13-11", PF_INET,   SOCK_RDM,       0, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, EPROTONOSUPPORT}},
  { "13-12", PF_INET,   SOCK_PACKET,    0, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, EPROTONOSUPPORT}},
  /* 13-13 : same as 13-04 */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int apitest_socket_main(void)
{
  int i;
  int ret;
  int result = 0;

  printf("+++++++++++++++++++++socket test start+++++++++++++++++++++\n");

  for (i = 0; i < TABLE_NUM(g_apitest_socket_param); i++)
    {
      printf("[%s] test start\n", g_apitest_socket_param[i].test_num);

      ret = socket(g_apitest_socket_param[i].domain,
                   g_apitest_socket_param[i].type,
                   g_apitest_socket_param[i].protocol);

      /* Check return code */

      switch(g_apitest_socket_param[i].verify_ret.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(ret, &g_apitest_socket_param[i].verify_ret);
            break;

          case VERIFY_NE:
            result = apitest_verify_ne(ret, &g_apitest_socket_param[i].verify_ret);
            break;

          case VERIFY_LT:
            result = apitest_verify_lt(ret, &g_apitest_socket_param[i].verify_ret);
            break;

          case VERIFY_LE:
            result = apitest_verify_le(ret, &g_apitest_socket_param[i].verify_ret);
            break;

          case VERIFY_GT:
            result = apitest_verify_gt(ret, &g_apitest_socket_param[i].verify_ret);
            break;

          case VERIFY_GE:
            result = apitest_verify_ge(ret, &g_apitest_socket_param[i].verify_ret);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_socket_param[i].verify_ret.verify_flag);
            result = -1;
            break;
        }

      if ((result == 0) && (g_apitest_socket_param[i].verify_err_flag))
        {
          /* Check errno */

          switch(g_apitest_socket_param[i].verify_err.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(errno, &g_apitest_socket_param[i].verify_err);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_apitest_socket_param[i].verify_err.verify_flag);
                result = -1;
                break;
            }
        }
      if (-1 == result)
        {
          printf("No.%d error\n", i);
        }
      if (ret != -1)
        {
          close(ret);
        }
    }

  printf("+++++++++++++++++++++socket test end+++++++++++++++++++++\n");

  return result;
}
