/****************************************************************************
 * test/lte_socket/api/shutdown_test.c
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
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "apitest.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define SOCKET_FD  0
#define ACCEPT_FD  1
#define INVALID_FD -1

#define INVALID_HOW -1

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_shutdown_s
{
  /* Input parameters */

  FAR const char *test_num;
  int fd;
  int how;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;

  /* Host behavior */

  bool do_accept;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

struct apitest_shutdown_s g_apitest_shutdown_param[] =
{
  {"12-01", SOCKET_FD, SHUT_RD,  { VERIFY_EQ, 0}, false, { VERIFY_EQ, DONTCARE_VAL}, false},
  {"12-02", ACCEPT_FD, SHUT_RD,  { VERIFY_EQ, 0}, false, { VERIFY_EQ, DONTCARE_VAL}, true },
  {"12-03", INVALID_FD, SHUT_RD, { VERIFY_EQ, -1}, true, { VERIFY_EQ, EBADF},        false },

  {"12-06", ACCEPT_FD,  SHUT_RDWR,   { VERIFY_EQ, 0}, false, { VERIFY_EQ, DONTCARE_VAL}, false },
  {"12-04", ACCEPT_FD, SHUT_RD,      { VERIFY_EQ, 0}, false, { VERIFY_EQ, DONTCARE_VAL}, false },
  {"12-05", ACCEPT_FD,  SHUT_WR,     { VERIFY_EQ, 0}, false, { VERIFY_EQ, DONTCARE_VAL}, false },
  {"12-07", ACCEPT_FD,  INVALID_HOW, { VERIFY_EQ, -1}, true, { VERIFY_EQ, EINVAL},        false },
};

static int g_sock_fd = -1;
static int g_accept_fd = -1;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void teardown_shutdown(void)
{
  if (g_sock_fd != -1)
    {
      close(g_sock_fd);
      g_sock_fd = -1;
    }

  if (g_accept_fd != -1)
    {
      close(g_accept_fd);
      g_accept_fd = -1;
    }
}

#ifdef HOST_MAKE

int setup_shutdown_host(char *host, char *port_no)
{
  int                  ret;
  int                  sockfd;
  int                  val = 1;
  struct addrinfo      hints;
  FAR struct addrinfo *ainfo = NULL;

  memset(&hints, 0, sizeof(hints));

  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  hints.ai_flags  = AI_PASSIVE;

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
      return -1;
    }
  printf("socket success: sockfd = %d\n", sockfd);

  ret = connect(sockfd, ainfo->ai_addr, ainfo->ai_addrlen);
  if (ret < 0)
    {
      printf("conncet error:%d\n", errno);
      goto errout_close;
    }
  printf("connect success: ret = %d\n", ret);

  return ret;
errout_close:
  close(sockfd);
  return -1;
}

#else

static int apitest_shutdown_getfd(int flg)
{
  int ret;
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
      ret = -1;
    }

  return ret;
}

static int setup_shutdown(int port_no, bool cnct)
{
  int                ret;
  int                family = AF_INET;
  int                socktype = SOCK_STREAM;
  int                protocol = 0;
  struct sockaddr    *addr;
  socklen_t          addrlen;
  struct sockaddr    *cli_addr;
  socklen_t          cli_addrlen;
  struct sockaddr_in src_addr4;
  struct sockaddr_in cli_addr4;

  memset(&src_addr4, 0, sizeof(struct sockaddr_in));
  src_addr4.sin_family      = AF_INET;
  src_addr4.sin_port        = htons(port_no);
  src_addr4.sin_addr.s_addr = htonl(INADDR_ANY);

  addr     = (struct sockaddr*)&src_addr4;
  addrlen  = sizeof(struct sockaddr_in);

  memset(&cli_addr4, 0, sizeof(struct sockaddr_in));

  cli_addr     = (struct sockaddr*)&cli_addr4;
  cli_addrlen  = sizeof(struct sockaddr_in);
  socktype     = SOCK_STREAM;

  g_sock_fd = socket(family, socktype, protocol);
  if (g_sock_fd < 0)
    {
      printf("socket(%d, %d, %d) error:%d\n",
             family, socktype, protocol, errno);
      goto errout_close;
    }
  if (cnct)
    {
      ret = bind(g_sock_fd, addr, addrlen);
      if (ret < 0)
        {
          printf("bind error:%d\n", errno);
          goto errout_close;
        }

      ret = listen(g_sock_fd, 1);
      if (ret < 0)
        {
          printf("listen error:%d\n", errno);
          goto errout_close;
        }
      printf("listen success: ret = %d\n", ret);

      g_accept_fd = accept(g_sock_fd, cli_addr, &cli_addrlen);
      if (g_accept_fd < 0)
        {
          printf("accept error:%d\n", g_accept_fd);
          goto errout_close;
        }
    }

  return 0;

errout_close:
  if (g_accept_fd < 0)
    {
      close(g_accept_fd);
      g_accept_fd = -1;
    }

  if (g_sock_fd < 0)
    {
      close(g_sock_fd);
      g_sock_fd = -1;
    }
  return -1;
}

#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/
#ifdef HOST_MAKE
int apitest_shutdown_main(char *host, char *port_no)
#else
int apitest_shutdown_main(int port_no)
#endif
{
  int i;
  int ret;
  int result = 0;

  printf("+----------- START SHUTDOWN TEST-.- -----------+\n");

  
#ifdef HOST_MAKE
  ret = setup_shutdown_host(host, port_no);

  if (ret < 0)
    {
      printf("setup_shutdown() failed: %d\n", ret);
    }

#else
  for (i = 0; i < TABLE_NUM(g_apitest_shutdown_param); i++)
    {
      printf("[%s] test start\n", g_apitest_shutdown_param[i].test_num);

      setup_shutdown(port_no, g_apitest_shutdown_param[i].do_accept);
      ret = shutdown(apitest_shutdown_getfd(g_apitest_shutdown_param[i].fd), g_apitest_shutdown_param[i].how);

      /* Check return code */

      switch(g_apitest_shutdown_param[i].verify_ret.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(ret, &g_apitest_shutdown_param[i].verify_ret);
            break;

          case VERIFY_NE:
            result = apitest_verify_ne(ret, &g_apitest_shutdown_param[i].verify_ret);
            break;

          case VERIFY_LT:
            result = apitest_verify_lt(ret, &g_apitest_shutdown_param[i].verify_ret);
            break;

          case VERIFY_LE:
            result = apitest_verify_le(ret, &g_apitest_shutdown_param[i].verify_ret);
            break;

          case VERIFY_GT:
            result = apitest_verify_gt(ret, &g_apitest_shutdown_param[i].verify_ret);
            break;

          case VERIFY_GE:
            result = apitest_verify_ge(ret, &g_apitest_shutdown_param[i].verify_ret);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_shutdown_param[i].verify_ret.verify_flag);
            result = -1;
            break;
        }

      if ((result == 0) && (g_apitest_shutdown_param[i].verify_err_flag))
        {
          /* Check errno */

          switch(g_apitest_shutdown_param[i].verify_err.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(errno, &g_apitest_shutdown_param[i].verify_err);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_apitest_shutdown_param[i].verify_err.verify_flag);
                result = -1;
                break;
            }
        }

      if (-1 == result)
        {
          printf("[%s] error\n", g_apitest_shutdown_param[i].test_num);
        }

    }
  teardown_shutdown();

#endif

  printf("+----------- END SHUTDOWN TEST^v^ -----------+\n");

  return result;
}
