/****************************************************************************
 * test/lte_socket/api/getsockname_test.c
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
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "apitest.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define SOCKET_FD        0
#define INVALID_FD       -1
#define ADDR_PTN_NULL    0
#define ADDR_PTN_IPV4    1
#define ADDR_PTN_IPV6    2
#define ADDRLEN_PTN_NULL -1

#define ADDR_IPV4        "13.1.68.3"
#define ADDR_IPV6        "FEDC:BA98:7654:3210:FEDC:BA98:7654:3210"


/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_getsockname_s
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
  int                           verify_addrlen;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct sockaddr_in6 g_addr = {};
static socklen_t           g_addrlen = 0;
static int                 g_getsockname_fd = -1;

static struct sockaddr_in6 g_expect_addr    = {};
static socklen_t           g_expect_addrlen = 0;

static struct apitest_getsockname_s g_apitest_getsockname_param[] =
{
  { "04-01", SOCKET_FD,  ADDR_PTN_IPV4, sizeof(struct sockaddr_in),      { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "04-02", INVALID_FD, ADDR_PTN_IPV4, sizeof(struct sockaddr_in),      { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, EBADF        }},
  { "04-03", SOCKET_FD,  ADDR_PTN_IPV4, sizeof(struct sockaddr_in),      { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "04-04", SOCKET_FD,  ADDR_PTN_IPV4, ADDRLEN_PTN_NULL,                { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, EINVAL       }},
  { "04-05", SOCKET_FD,  ADDR_PTN_NULL, sizeof(struct sockaddr_in),      { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, EINVAL       }},
  { "04-06", SOCKET_FD,  ADDR_PTN_IPV4, sizeof(struct sockaddr_in),      { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "04-07", SOCKET_FD,  ADDR_PTN_IPV4, sizeof(struct sockaddr_in) - 1,  { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "04-08", SOCKET_FD,  ADDR_PTN_IPV6, sizeof(struct sockaddr_in6),     { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "04-09", SOCKET_FD,  ADDR_PTN_IPV6, sizeof(struct sockaddr_in6) - 1, { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * setup_getsockname
 ****************************************************************************/

static int setup_getsockname(int table_no, int port_no)
{
  int                  ret;
  int                  val = 1;
  struct sockaddr      *addr;
  struct sockaddr_in   addr4;
  struct sockaddr_in6  addr6;
  socklen_t            addrlen;
  int addr_ptn = g_apitest_getsockname_param[table_no].addr_ptn;

  if (ADDR_PTN_IPV6 == addr_ptn)
    {
      g_getsockname_fd = socket(AF_INET6, SOCK_STREAM, 0);
    }
  else
    {
      g_getsockname_fd = socket(AF_INET, SOCK_STREAM, 0);
    }

  if (g_getsockname_fd < 0)
    {
      printf("socket() error: %d\n", errno);
      return -1;
    }
  printf("socket success: sockfd = %d\n", g_getsockname_fd);

  ret = setsockopt(g_getsockname_fd, SOL_SOCKET, SO_REUSEADDR,
                   (const int *)&val, sizeof(val));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
      goto errout_close;
    }
  printf("setsockopt(%d, %d, %d, %d) success: ret = %d\n",
         g_getsockname_fd, SOL_SOCKET, SO_REUSEADDR, val, ret);

  if (ADDR_PTN_IPV6 == addr_ptn)
    {
      memset(&addr6, 0xFF, sizeof(addr6));
      addr6.sin6_family = AF_INET6;
      addr6.sin6_port   = htons(port_no);
      inet_pton(AF_INET6, ADDR_IPV6, &addr6.sin6_addr);

      addrlen  = sizeof(struct sockaddr_in6);
      addr = (FAR struct sockaddr *)&addr6;
    }
  else
    {
      memset(&addr4, 0xFF, sizeof(addr4));
      addr4.sin_family = AF_INET;
      addr4.sin_port   = htons(port_no);
      inet_pton(AF_INET, ADDR_IPV4, &addr4.sin_addr);

      addrlen = sizeof(struct sockaddr_in);
      addr = (FAR struct sockaddr *)&addr4;
    }

  ret = bind(g_getsockname_fd, addr, addrlen);
  if (ret < 0)
    {
      printf("bind error:%d\n", errno);
      goto errout_close;
    }
  printf("bind success: ret = %d\n", ret);

  memset(&g_addr, 0xFF, sizeof(g_addr));
  memset(&g_expect_addr, 0xFF, sizeof(g_expect_addr));
  if (ADDRLEN_PTN_NULL != g_apitest_getsockname_param[table_no].addrlen)
    {
      memcpy(&g_expect_addr, addr, g_apitest_getsockname_param[table_no].addrlen);
    }
  g_expect_addrlen = addrlen;

  return 0;

errout_close:
  close(g_getsockname_fd);
  g_getsockname_fd = INVALID_FD;

  return -1;
}

/****************************************************************************
 * teardown_getsockname
 ****************************************************************************/

static void teardown_getsockname(void)
{
  if (g_getsockname_fd != INVALID_FD)
    {
      close(g_getsockname_fd);
      g_getsockname_fd = INVALID_FD;
    }

  memset(&g_addr, 0xFF, sizeof(g_addr));
  g_addrlen = 0;
}

/****************************************************************************
 * get_getsockname_fd
 ****************************************************************************/

static int get_getsockname_fd(int fd)
{
  int ret = -1;

  if (INVALID_FD != fd)
    {
      ret = g_getsockname_fd;
    }

  return ret;
}

/****************************************************************************
 * get_getsockname_addr
 ****************************************************************************/

static FAR struct sockaddr *get_getsockname_addr(int addr_ptn)
{
  if (ADDR_PTN_NULL == addr_ptn)
    {
      return NULL;
    }

  return (FAR struct sockaddr *)&g_addr;
}

/****************************************************************************
 * check_getsockname_addr
 ****************************************************************************/

static int check_getsockname_addr(void)
{
  int i = 0;

  if (memcmp(&g_addr, &g_expect_addr, sizeof(g_expect_addr)))
    {
      printf("Check addr error.\n");
      printf("Expect addr:");
      for (i = 0; i < sizeof(g_expect_addr); i++)
        {
          printf("%02x", ((FAR char *)&g_expect_addr)[i]);
        }

      printf("\nGet addr   :");
      for (i = 0; i < sizeof(g_addr); i++)
        {
          printf("%02x", ((FAR char *)&g_addr)[i]);
        }

      printf("\nExpect addrlen:%d, Get addrlen:%d\n", (int)g_expect_addrlen, (int)g_addrlen);
      return -1;
    }

  return 0;
}

/****************************************************************************
 * get_getsockname_addrlen
 ****************************************************************************/

static FAR socklen_t *get_getsockname_addrlen(int addrlen)
{
  if (ADDRLEN_PTN_NULL == addrlen)
    {
      return NULL;
    }

  g_addrlen = addrlen;
  return &g_addrlen;
}

/****************************************************************************
 * check_getsockname_addrlen
 ****************************************************************************/

static int check_getsockname_addrlen(void)
{
  if (g_addrlen != g_expect_addrlen)
    {
      printf("Check addrlen error. Expect addrlen:%d, Get addrlen:%d\n", (int)g_expect_addrlen, (int)g_addrlen);
      return -1;
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int apitest_getsockname_main(int port_no)
{
  int i;
  int ret;
  int result = 0;
  int err = 0;

  printf("+++++++++++++++++++++getsockname test start+++++++++++++++++++++\n");

  for (i = 0; i < TABLE_NUM(g_apitest_getsockname_param); i++)
    {
      setup_getsockname(i, port_no);

      ret = getsockname(get_getsockname_fd(g_apitest_getsockname_param[i].fd),
        get_getsockname_addr(g_apitest_getsockname_param[i].addr_ptn),
        get_getsockname_addrlen(g_apitest_getsockname_param[i].addrlen));

      err = errno;
      /* Check return code */

      switch(g_apitest_getsockname_param[i].verify_ret.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(ret, &g_apitest_getsockname_param[i].verify_ret);
            break;

          case VERIFY_NE:
            result = apitest_verify_ne(ret, &g_apitest_getsockname_param[i].verify_ret);
            break;

          case VERIFY_LT:
            result = apitest_verify_lt(ret, &g_apitest_getsockname_param[i].verify_ret);
            break;

          case VERIFY_LE:
            result = apitest_verify_le(ret, &g_apitest_getsockname_param[i].verify_ret);
            break;

          case VERIFY_GT:
            result = apitest_verify_gt(ret, &g_apitest_getsockname_param[i].verify_ret);
            break;

          case VERIFY_GE:
            result = apitest_verify_ge(ret, &g_apitest_getsockname_param[i].verify_ret);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_getsockname_param[i].verify_ret.verify_flag);
            result = -1;
            break;
        }

      /* Check addr */

      if ((result == 0) && !(g_apitest_getsockname_param[i].verify_err_flag))
        {
          result = check_getsockname_addr();
        }

      /* Check addrlen */

      if ((result == 0) && !(g_apitest_getsockname_param[i].verify_err_flag))
        {
          result = check_getsockname_addrlen();
        }

      if ((result == 0) && (g_apitest_getsockname_param[i].verify_err_flag))
        {
          /* Check errno */

          switch(g_apitest_getsockname_param[i].verify_err.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(err, &g_apitest_getsockname_param[i].verify_err);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_apitest_getsockname_param[i].verify_err.verify_flag);
                result = -1;
                break;
            }
        }

      if (-1 == result)
        {
          printf("[%s] error. errno:%d\n", g_apitest_getsockname_param[i].test_num, err);
        }
      else
        {
          printf("[%s] success\n", g_apitest_getsockname_param[i].test_num);
        }
      teardown_getsockname();
    }

  printf("+++++++++++++++++++++getsockname test end+++++++++++++++++++++\n");
  return result;
}
