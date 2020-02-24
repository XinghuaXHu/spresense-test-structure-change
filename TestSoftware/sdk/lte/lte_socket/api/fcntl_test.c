/****************************************************************************
 * test/lte_socket/api/fcntl_test.c
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

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "apitest.h"

#define SOCKET_FD        0
#define ACCEPT_FD        1
#define INVALID_FD      -1

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_fcntl_s
{
  /* Input parameters */

  FAR const char *test_num;
  int fd;
  int cmd;
  int arg;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;

  /* accept flag */
  bool accept;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int g_sock_fd = -1;
static int g_accept_fd = -1;
static int g_chkval = -1;

static struct apitest_fcntl_s g_apitest_fcntl_param[] =
{
  { "29-01", SOCKET_FD,  F_SETFL, 0,          {VERIFY_EQ, 0},     false, {VERIFY_EQ, DONTCARE_VAL}, false },
  { "29-02", ACCEPT_FD,  F_SETFL, 0,          {VERIFY_EQ, 0},     false, {VERIFY_EQ, DONTCARE_VAL}, true  },
  { "29-03", INVALID_FD, F_SETFL, 0,          {VERIFY_EQ, EBADF}, false, {VERIFY_EQ, DONTCARE_VAL}, false },
  { "29-04", SOCKET_FD,  F_GETFL, 0,          {VERIFY_EQ, 0},     false, {VERIFY_EQ, DONTCARE_VAL}, false },
  { "29-05", SOCKET_FD,  F_SETFL, 0,          {VERIFY_EQ, 0},     false, {VERIFY_EQ, DONTCARE_VAL}, false },
  { "29-06", SOCKET_FD,  F_SETFL, 0,          {VERIFY_EQ, 0},     false, {VERIFY_EQ, DONTCARE_VAL}, false },
  { "29-07", SOCKET_FD,  F_SETFL, O_NONBLOCK, {VERIFY_EQ, 0},     false, {VERIFY_EQ, DONTCARE_VAL}, false },
  { "29-08", SOCKET_FD,  F_SETFL, -1,         {VERIFY_EQ, 0},     false, {VERIFY_EQ, DONTCARE_VAL}, false },
};

/****************************************************************************
 * Private Function
 ****************************************************************************/

static int apitest_fcntl_getfd(int flg)
{
  int ret = -1;
  if (SOCKET_FD == flg)
    {
      ret = g_sock_fd;
    }
  else if (ACCEPT_FD == flg)
    {
      ret = g_accept_fd;
    }
  else
    {
      ;;
    }

  return ret;
}

static int apitest_fcntl_chkrtnval(int rtnval)
{
  printf("default val = %d / set fcnt val = %d / get fcntl val = %d\n",
    g_chkval, O_NONBLOCK, rtnval);

  if (g_chkval == rtnval)
    {
      return -1;
    }

  if (rtnval != O_NONBLOCK)
    {
      return -1;
    }

  return 0;
}

#ifdef HOST_MAKE
static void teardown_fcntl_host(int fd)
{
  close(fd);
}

static int setup_fcntl_host(char *host,char *port_no)
{
  int                  ret;
  int                  sockfd;
  int                  val = 1;
  struct addrinfo      hints;
  FAR struct addrinfo *ainfo = NULL;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_flags  = AI_PASSIVE;
  hints.ai_socktype = SOCK_STREAM;
  ret = getaddrinfo(host, port_no, &hints, &ainfo);
  if (ret != 0)
    {
      printf("getaddrinfo error = %d\n",ret);
      return -1;
    }

  sockfd = socket(ainfo->ai_family, ainfo->ai_socktype,
                  ainfo->ai_protocol);
  if (sockfd < 0)
    {
      printf("socket(%d, %d, %d) error:%d\n",
             ainfo->ai_family, ainfo->ai_socktype,
             ainfo->ai_protocol, errno);
      goto err_with_close;
    }
  printf("socket success: sockfd = %d\n", sockfd);

  ret = connect(sockfd, ainfo->ai_addr, ainfo->ai_addrlen);
  if (ret < 0)
    {
      printf("conncet error:%d\n", errno);
      goto err_with_close;
    }

  printf("connect success: ret = %d\n", ret);
  freeaddrinfo(ainfo);
  return sockfd;
err_with_close:
  close(sockfd);
  freeaddrinfo(ainfo);
  return -1;
}
#else
static void teardown_fcntl(void)
{
  if (g_sock_fd != -1)
    {
      close(g_sock_fd);
    }

  if (g_accept_fd != -1)
    {
      close(g_accept_fd);
      g_accept_fd = -1;
    }
}

static int setup_fcntl(int port_no, bool acptflg, int cmd)
{
  int                ret;
  int                val = 1;
  struct sockaddr    *addr;
  struct sockaddr_in src_addr4;
  socklen_t          addrlen;

  g_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (g_sock_fd < 0)
    {
      printf("socket() error: %d\n", errno);
      return -1;
    }

  ret = setsockopt(g_sock_fd, SOL_SOCKET, SO_REUSEADDR,
                   (const int *)&val, sizeof(val));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
      goto errout_close;
    }
  printf("setsockopt(%d, %d, %d, %d) success: ret = %d\n",
         g_sock_fd, SOL_SOCKET, SO_REUSEADDR, val, ret);

  memset(&src_addr4, 0, sizeof(struct sockaddr_in));
  src_addr4.sin_family      = AF_INET;
  src_addr4.sin_port        = htons(port_no);
  src_addr4.sin_addr.s_addr = htonl(INADDR_ANY);

  addr    = (struct sockaddr*)&src_addr4;
  addrlen = sizeof(struct sockaddr_in);

  ret = bind(g_sock_fd, addr, addrlen);
  if (ret < 0)
    {
      printf("bind error:%d\n", errno);
      goto errout_close;
    }
  printf("bind success: ret = %d\n", ret);

  ret = listen(g_sock_fd, 1);
  if (ret < 0)
    {
      printf("listen error:%d\n", errno);
      goto errout_close;
    }
  printf("listen success: ret = %d\n", ret);

  if (acptflg)
    {
      g_accept_fd = accept(g_sock_fd, addr, &addrlen);
      if (g_accept_fd < 0)
        {
          printf("accept error:%d\n", errno);
          goto errout_close;
        }
    }

  if (cmd == F_GETFL)
    {
      g_chkval = fcntl(g_sock_fd, cmd, 0);
      fcntl(g_sock_fd, F_SETFL, O_NONBLOCK);
    }

  return 0;

errout_close:
  close(g_sock_fd);
  g_sock_fd = -1;
  close(g_accept_fd);
  g_accept_fd = -1;

  return -1;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_fcntl_main(char *host, char *port_no)
#else
int apitest_fcntl_main(int port_no)
#endif
{
  int i;
  int ret;
  int result = 0;

  printf("+----------- START FCNTL TEST-.- -----------*\n");
#ifdef HOST_MAKE
  ret = setup_fcntl_host(host, port_no);
  if (ret < 0)
    {
      printf("setup_fcntl_host() failed: %d\n", ret);
      return -1;
    }

  teardown_fcntl_host(ret);

#else
  for (i = 0; i < TABLE_NUM(g_apitest_fcntl_param); i++)
    {

      setup_fcntl(port_no, g_apitest_fcntl_param[i].accept, g_apitest_fcntl_param[i].cmd);
      ret = fcntl(apitest_fcntl_getfd(g_apitest_fcntl_param[i].fd),
                   g_apitest_fcntl_param[i].cmd,
                   g_apitest_fcntl_param[i].arg);

      if (g_apitest_fcntl_param[i].cmd == F_GETFL)
        {
          result = apitest_fcntl_chkrtnval(ret);
        }

      /* Check return code */

      switch(g_apitest_fcntl_param[i].verify_ret.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(ret, &g_apitest_fcntl_param[i].verify_ret);
            break;

          case VERIFY_NE:
            result = apitest_verify_ne(ret, &g_apitest_fcntl_param[i].verify_ret);
            break;

          case VERIFY_LT:
            result = apitest_verify_lt(ret, &g_apitest_fcntl_param[i].verify_ret);
            break;

          case VERIFY_LE:
            result = apitest_verify_le(ret, &g_apitest_fcntl_param[i].verify_ret);
            break;

          case VERIFY_GT:
            result = apitest_verify_gt(ret, &g_apitest_fcntl_param[i].verify_ret);
            break;

          case VERIFY_GE:
            result = apitest_verify_ge(ret, &g_apitest_fcntl_param[i].verify_ret);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_fcntl_param[i].verify_ret.verify_flag);
            result = -1;
            break;
        }

      if ((result == 0) && (g_apitest_fcntl_param[i].cmd == F_GETFL))
        {
          /* Check return val */

          result = apitest_fcntl_chkrtnval(ret);
          if (0 > result)
            {
              printf("Invalid ret_code [ret_code=%d]", g_apitest_fcntl_param[i].verify_ret.verify_val);
            }
        }

      if (-1 == result)
        {
          printf("No.%s error\n", g_apitest_fcntl_param[i].test_num);
        }

      teardown_fcntl();
    }
#endif

  printf("+----------- END FCNTL TEST^v^ -----------*\n");

  return result;
}
