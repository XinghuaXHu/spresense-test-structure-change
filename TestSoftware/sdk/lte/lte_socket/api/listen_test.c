/****************************************************************************
 * test/lte_socket/api/listen_test.c
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
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "apitest.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define INVALID_FD -1
#define BACKLOG_1  1
#define BACKLOG_10 10

#define ADDR_PTN_NULL     0
#define ADDR_PTN_NOT_NULL 1
#define ADDRLEN_NULL      0
#define VALID_FD         0
#define INVALID_FD        -1

/****************************************************************************
 * Private Types
 ****************************************************************************/
struct apitest_listen_s
{
  /* Input parameters */

  int fd;
  int backlog;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

struct apitest_listen_s g_apitest_listen_param[] =
{
  { VALID_FD,  BACKLOG_1,  { VERIFY_EQ, 0}, false, { VERIFY_EQ, DONTCARE_VAL} },
  { INVALID_FD, BACKLOG_1,  { VERIFY_EQ, -1}, true,  { VERIFY_EQ, EBADF} },
  { VALID_FD,  BACKLOG_1,  { VERIFY_EQ, 0}, false, { VERIFY_EQ, DONTCARE_VAL} },
  { VALID_FD,  BACKLOG_10, { VERIFY_EQ, 0}, false, { VERIFY_EQ, DONTCARE_VAL} },
};

static int g_listen_fd = -1;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int get_listenfd(int flg)
{
  if (flg == VALID_FD)
    {
      return g_listen_fd;
    }

  return -1;
}

static int setup_listen(int port_no)
{
  int                 ret;
  int                 val = 1;
  struct sockaddr     *addr;
  struct sockaddr_in  src_addr4;
  socklen_t           addrlen;

  g_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (g_listen_fd < 0)
    {
      printf("socket() error: %d\n", errno);
      return -1;
    }
  printf("socket success: sockfd = %d\n", g_listen_fd);

  ret = setsockopt(g_listen_fd, SOL_SOCKET, SO_REUSEADDR,
                   (const int *)&val, sizeof(val));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
      goto errout_close;
    }

  printf("setsockopt(%d, %d, %d, %d) success: ret = %d\n",
         g_listen_fd, SOL_SOCKET, SO_REUSEADDR, val, ret);

  memset(&src_addr4, 0, sizeof(struct sockaddr_in));
  src_addr4.sin_family      = AF_INET;
  src_addr4.sin_port        = htons(port_no);
  src_addr4.sin_addr.s_addr = htonl(INADDR_ANY);

  addr    = (struct sockaddr*)&src_addr4;
  addrlen = sizeof(struct sockaddr_in);

  ret = bind(g_listen_fd, addr, addrlen);
  if (ret < 0)
    {
      printf("bind error:%d\n", errno);
      goto errout_close;
    }
  printf("bind success: ret = %d\n", ret);

  return 0;

errout_close:
  close(g_listen_fd);
  g_listen_fd = -1;

  return -1;
}

/****************************************************************************
 * teardown_listen
 ****************************************************************************/

int teardown_listen(int fd)
{
  if (fd != -1)
    {
      close(fd);
    }

  if (g_listen_fd != -1)
    {
      close(g_listen_fd);
      g_listen_fd = -1;
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int apitest_listen_main(int port)
{
  int i;
  int ret;
  int result = 0;

  printf("+----------- START LISTEN TEST-.- -----------+\n");
  for (i = 0; i < TABLE_NUM(g_apitest_listen_param); i++)
    {
      setup_listen(port);

      ret = listen(get_listenfd(g_apitest_listen_param[i].fd), g_apitest_listen_param[i].backlog);

      /* Check return code */

      switch(g_apitest_listen_param[i].verify_ret.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(ret, &g_apitest_listen_param[i].verify_ret);
            break;

          case VERIFY_NE:
            result = apitest_verify_ne(ret, &g_apitest_listen_param[i].verify_ret);
            break;

          case VERIFY_LT:
            result = apitest_verify_lt(ret, &g_apitest_listen_param[i].verify_ret);
            break;

          case VERIFY_LE:
            result = apitest_verify_le(ret, &g_apitest_listen_param[i].verify_ret);
            break;

          case VERIFY_GT:
            result = apitest_verify_gt(ret, &g_apitest_listen_param[i].verify_ret);
            break;

          case VERIFY_GE:
            result = apitest_verify_ge(ret, &g_apitest_listen_param[i].verify_ret);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_listen_param[i].verify_ret.verify_flag);
            result = -1;
            break;
        }

      if ((result == 0) && (g_apitest_listen_param[i].verify_err_flag))
        {
          /* Check errno */

          switch(g_apitest_listen_param[i].verify_err.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(errno, &g_apitest_listen_param[i].verify_err);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_apitest_listen_param[i].verify_err.verify_flag);
                result = -1;
                break;
            }
        }

      teardown_listen(ret);
    }
  printf("+----------- END LISTEN TEST^v^ -----------+\n");

  return result;
}
