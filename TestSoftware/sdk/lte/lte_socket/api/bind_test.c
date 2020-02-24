/****************************************************************************
 * test/lte_socket/api/bind_test.c
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
#include <netinet/in.h>
#include <arpa/inet.h>

#include "apitest.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ADDR_PTN_NULL     0
#define ADDR_PTN_IPV4     1
#define ADDR_PTN_IPV6     2
#define ADDRLEN_NULL      0
#define SOCKET_FD         0
#define INVALID_FD        -1

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_bind_s
{
  /* Input parameters */

  FAR const char *test_num;
  int            fd;
  int            addr_ptn;
  int            addrlen;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct apitest_bind_s g_apitest_bind_param[] =
{
  { "02-01", SOCKET_FD,  ADDR_PTN_IPV4, sizeof(struct sockaddr_in),      { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "02-02", INVALID_FD, ADDR_PTN_IPV4, sizeof(struct sockaddr_in),      { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, EBADF        }},
  { "02-03", SOCKET_FD,  ADDR_PTN_NULL, sizeof(struct sockaddr_in),      { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, EINVAL       }},
  { "02-04", SOCKET_FD,  ADDR_PTN_IPV4, sizeof(struct sockaddr_in),      { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "02-05", SOCKET_FD,  ADDR_PTN_IPV6, sizeof(struct sockaddr_in6),     { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "02-06", SOCKET_FD,  ADDR_PTN_IPV4, sizeof(struct sockaddr_in) - 1,  { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, EINVAL       }},
  { "02-07", SOCKET_FD,  ADDR_PTN_IPV4, sizeof(struct sockaddr_in),      { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "02-08", SOCKET_FD,  ADDR_PTN_IPV6, sizeof(struct sockaddr_in6) - 1, { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, EINVAL       }},
  { "02-09", SOCKET_FD,  ADDR_PTN_IPV6, sizeof(struct sockaddr_in6),     { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
};

static int g_bind_fd = INVALID_FD;
static struct sockaddr g_bind_addr;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * setup_bind
 ****************************************************************************/

static int setup_bind(int addr_ptn)
{
  int                  ret;
  int                  val = 1;

  if (ADDR_PTN_IPV6 == addr_ptn)
    {
      g_bind_fd = socket(AF_INET6, SOCK_STREAM, 0);
    }
  else
    {
      g_bind_fd = socket(AF_INET, SOCK_STREAM, 0);
    }
  if (g_bind_fd < 0)
    {
      printf("socket() error: %d\n", errno);
      return -1;
    }
  printf("socket success: sockfd = %d\n", g_bind_fd);

  ret = setsockopt(g_bind_fd, SOL_SOCKET, SO_REUSEADDR,
                   (const int *)&val, sizeof(val));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
      goto errout_close;
    }
  printf("setsockopt(%d, %d, %d, %d) success: ret = %d\n",
         g_bind_fd, SOL_SOCKET, SO_REUSEADDR, val, ret);

  return 0;

errout_close:
  close(g_bind_fd);
  g_bind_fd = INVALID_FD;

  return -1;
}

/****************************************************************************
 * teardown_bind
 ****************************************************************************/

static int teardown_bind(void)
{
  if (INVALID_FD != g_bind_fd)
    {
      close(g_bind_fd);
      g_bind_fd = INVALID_FD;
    }

  return 0;
}

/****************************************************************************
 * get_bind_fd
 ****************************************************************************/

static int get_bind_fd(int fd)
{
  int ret = INVALID_FD;

  if (INVALID_FD != fd)
    {
      ret = g_bind_fd;
    }

  return ret;
}

/****************************************************************************
 * get_bind_addr
 ****************************************************************************/

static FAR struct sockaddr *get_bind_addr(int port_no, int addr_ptn)
{
  struct sockaddr_in   *addr4;
  struct sockaddr_in6  *addr6;

  if (addr_ptn == ADDR_PTN_NULL)
    {
      return NULL;
    }
  else if (addr_ptn == ADDR_PTN_IPV4)
    {
      addr4 = (FAR struct sockaddr_in *)&g_bind_addr;
      memset(addr4, 0, sizeof(struct sockaddr_in));
      addr4->sin_family      = AF_INET;
      addr4->sin_port        = htons(port_no);
      addr4->sin_addr.s_addr = htonl(INADDR_ANY);
    }
  else
    {
      addr6 = (FAR struct sockaddr_in6 *)&g_bind_addr;
      memset(addr6, 0, sizeof(struct sockaddr_in6));
      addr6->sin6_family = AF_INET6;
      addr6->sin6_port   = htons(port_no);
      addr6->sin6_addr   = in6addr_any;
    }

  return &g_bind_addr;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int apitest_bind_main(int port_no)
{
  int i;
  int ret;
  int result = 0;

  printf("+++++++++++++++++++++bind test start+++++++++++++++++++++\n");

  for (i = 0; i < TABLE_NUM(g_apitest_bind_param); i++)
    {
      setup_bind(g_apitest_bind_param[i].addr_ptn);

      ret = bind(get_bind_fd(g_apitest_bind_param[i].fd),
                 get_bind_addr(port_no, g_apitest_bind_param[i].addr_ptn),
                 g_apitest_bind_param[i].addrlen);

      /* Check return code */

      switch(g_apitest_bind_param[i].verify_ret.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(ret, &g_apitest_bind_param[i].verify_ret);
            break;

          case VERIFY_NE:
            result = apitest_verify_ne(ret, &g_apitest_bind_param[i].verify_ret);
            break;

          case VERIFY_LT:
            result = apitest_verify_lt(ret, &g_apitest_bind_param[i].verify_ret);
            break;

          case VERIFY_LE:
            result = apitest_verify_le(ret, &g_apitest_bind_param[i].verify_ret);
            break;

          case VERIFY_GT:
            result = apitest_verify_gt(ret, &g_apitest_bind_param[i].verify_ret);
            break;

          case VERIFY_GE:
            result = apitest_verify_ge(ret, &g_apitest_bind_param[i].verify_ret);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_bind_param[i].verify_ret.verify_flag);
            result = -1;
            break;
        }

      if ((result == 0) && (g_apitest_bind_param[i].verify_err_flag))
        {
          /* Check errno */

          switch(g_apitest_bind_param[i].verify_err.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(errno, &g_apitest_bind_param[i].verify_err);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_apitest_bind_param[i].verify_err.verify_flag);
                result = -1;
                break;
            }
        }

      if (-1 == result)
        {
          printf("[%s] error\n", g_apitest_bind_param[i].test_num);
        }
      teardown_bind();
    }

  printf("+++++++++++++++++++++bind test end+++++++++++++++++++++\n");
  return result;
}
