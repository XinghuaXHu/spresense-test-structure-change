/****************************************************************************
 * test/lte_socket/api/getsockopt_test.c
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
#include <sys/time.h>

#include "apitest.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define VALID_FD       0
#define INVALID_FD      -1

#define INVALID_LEV    -1
#define INVALID_OPT    -1
#define VALID_OPTVAL   0
#define INVALID_OPTVAL -1
#define VALID_OPTLEN   0
#define INVALID_OPTLEN -1

#define BUFFLEN 16

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_getsockopt_s
{
  /* Input parameters */

  FAR const char *test_num;
  int fd;
  int level;
  int optname;
  int optlen;
  int optval;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static char g_optbuf[BUFFLEN] = {};
static int g_sockfd = -1;

struct apitest_getsockopt_s g_apitest_getsockopt_param[] =
{
  { "05-01", VALID_FD,  SOL_SOCKET, SO_ACCEPTCONN, sizeof(int), VALID_OPTVAL, { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL}},
  { "05-02", INVALID_FD, SOL_SOCKET, SO_ACCEPTCONN, sizeof(int), VALID_OPTVAL, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, EBADF} },

  { "05-03", VALID_FD, SOL_SOCKET,  SO_ACCEPTCONN, sizeof(int), VALID_OPTVAL, { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL} },
  { "05-09", VALID_FD, INVALID_LEV, SO_ACCEPTCONN, sizeof(int), VALID_OPTVAL, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, ENOPROTOOPT} },

  { "05-10", VALID_FD, SOL_SOCKET, SO_ACCEPTCONN, sizeof(int), VALID_OPTVAL, { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL} },
  { "05-11", VALID_FD, SOL_SOCKET, SO_ERROR,      sizeof(int), VALID_OPTVAL, { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL} },
  { "05-12", VALID_FD, SOL_SOCKET, SO_BROADCAST,  sizeof(int), VALID_OPTVAL, { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL} },
  { "05-13", VALID_FD, SOL_SOCKET, SO_KEEPALIVE,  sizeof(int), VALID_OPTVAL, { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL} },
  { "05-14", VALID_FD, SOL_SOCKET, SO_REUSEADDR,  sizeof(int), VALID_OPTVAL, { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL} },
  { "05-15", VALID_FD, SOL_SOCKET, SO_TYPE,       sizeof(int), VALID_OPTVAL, { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL} },
  { "05-16", VALID_FD, SOL_SOCKET, SO_RCVBUF,     sizeof(int), VALID_OPTVAL, { VERIFY_EQ, -1},  false, { VERIFY_EQ, ENOPROTOOPT} },

  { "05-18", VALID_FD, SOL_SOCKET, SO_LINGER,     sizeof(struct linger), VALID_OPTVAL, { VERIFY_EQ, -1},  false, { VERIFY_EQ, ENOPROTOOPT} },
  { "05-19", VALID_FD, SOL_SOCKET, SO_DEBUG,      sizeof(int), VALID_OPTVAL, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, ENOPROTOOPT} },
  { "05-20", VALID_FD, SOL_SOCKET, SO_OOBINLINE,  sizeof(int), VALID_OPTVAL, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, ENOPROTOOPT} },
  { "05-21", VALID_FD, SOL_SOCKET, SO_SNDBUF,     sizeof(int), VALID_OPTVAL, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, ENOPROTOOPT} },
  { "05-22", VALID_FD, SOL_SOCKET, SO_DONTROUTE,  sizeof(int), VALID_OPTVAL, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, ENOPROTOOPT} },
  { "05-23", VALID_FD, SOL_SOCKET, SO_RCVLOWAT,   sizeof(int), VALID_OPTVAL, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, ENOPROTOOPT} },
  { "05-24", VALID_FD, SOL_SOCKET, SO_SNDLOWAT,   sizeof(int), VALID_OPTVAL, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, ENOPROTOOPT} },
  { "05-25", VALID_FD, SOL_SOCKET, SO_RCVTIMEO,   sizeof(struct timeval), VALID_OPTVAL, { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL} },
  { "05-26", VALID_FD, SOL_SOCKET, SO_SNDTIMEO,   sizeof(struct timeval), VALID_OPTVAL, { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL} },
  { "05-27", VALID_FD, SOL_SOCKET, INVALID_OPT,   sizeof(int), VALID_OPTVAL, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, EINVAL} },

  { "05-46", VALID_FD, SOL_SOCKET, SO_ACCEPTCONN, sizeof(int), INVALID_OPTVAL, { VERIFY_EQ, -1},  true, { VERIFY_EQ, EINVAL} },
  { "05-47", VALID_FD, SOL_SOCKET, SO_ACCEPTCONN, sizeof(int), VALID_OPTVAL, { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL} },

  { "05-48", VALID_FD, SOL_SOCKET, SO_ACCEPTCONN, sizeof(int),   VALID_OPTVAL, { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL} },
  { "05-49", VALID_FD, SOL_SOCKET, SO_ACCEPTCONN, (sizeof(int) - 1), VALID_OPTVAL, { VERIFY_EQ, -1},  true, { VERIFY_EQ, EINVAL} },
};

/****************************************************************************
 * Private Function
 ****************************************************************************/
static void setup_getsockopt(void)
{
  g_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (g_sockfd < 0)
    {
      printf("socket() error: %d\n", errno);
    }
  printf("socket success: sockfd = %d\n", g_sockfd);
}

static void teardown_getsockopt(void)
{
  if (g_sockfd != -1)
    {
      close(g_sockfd);
      g_sockfd = -1;
    }
}

static int apitest_getsockopt_getfd(int flg)
{
  int ret = -1;
  if (flg != INVALID_FD)
    {
      ret = g_sockfd;
    }

  return ret;
}

static void *apitest_getsockopt_getbuff(int flg)
{
  if (INVALID_OPTVAL == flg)
    {
      return NULL;
    }

  return (void *)&g_optbuf;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int apitest_getsockopt_main(void)
{
  int i;
  socklen_t len;
  int ret;
  int result = 0;

  printf("+----------- STAR GETSOCKOPT TEST-.- -----------+\n");
  setup_getsockopt();
  for (i = 0; i < TABLE_NUM(g_apitest_getsockopt_param); i++)
    {

      len = g_apitest_getsockopt_param[i].optlen;
      ret = getsockopt(
          apitest_getsockopt_getfd(g_apitest_getsockopt_param[i].fd),
          g_apitest_getsockopt_param[i].level,
          g_apitest_getsockopt_param[i].optname,
          apitest_getsockopt_getbuff(g_apitest_getsockopt_param[i].optval),
          &len);

      /* Check return code */

      switch(g_apitest_getsockopt_param[i].verify_ret.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(ret, &g_apitest_getsockopt_param[i].verify_ret);
            break;

          case VERIFY_NE:
            result = apitest_verify_ne(ret, &g_apitest_getsockopt_param[i].verify_ret);
            break;

          case VERIFY_LT:
            result = apitest_verify_lt(ret, &g_apitest_getsockopt_param[i].verify_ret);
            break;

          case VERIFY_LE:
            result = apitest_verify_le(ret, &g_apitest_getsockopt_param[i].verify_ret);
            break;

          case VERIFY_GT:
            result = apitest_verify_gt(ret, &g_apitest_getsockopt_param[i].verify_ret);
            break;

          case VERIFY_GE:
            result = apitest_verify_ge(ret, &g_apitest_getsockopt_param[i].verify_ret);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_getsockopt_param[i].verify_ret.verify_flag);
            result = -1;
            break;
        }

      if ((result == 0) && (g_apitest_getsockopt_param[i].verify_err_flag))
        {
          /* Check errno */

          switch(g_apitest_getsockopt_param[i].verify_err.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(errno, &g_apitest_getsockopt_param[i].verify_err);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_apitest_getsockopt_param[i].verify_err.verify_flag);
                result = -1;
                break;
            }
        }

      if (-1 == result)
        {
          printf("No.%s error\n", g_apitest_getsockopt_param[i].test_num);
        }
    }

  printf("+----------- END GETSOCKOPT TEST^v^ -----------+\n");

  teardown_getsockopt();
  return result;
}
