/****************************************************************************
 * test/lte_socket/api/close_test.c
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

#define ADDR_PTN_NULL     0
#define ADDR_PTN_NOT_NULL 1
#define ADDRLEN_NULL      0
#define SOCKET_FD         0
#define ACCEPT_FD         1
#define INVALID_FD        -1
#define IPV4              0
#define IPV6              1

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_close_s
{
  FAR const char *test_num;

  /* Input parameters */

  int fd;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;

};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int g_sock_fd = INVALID_FD;

#ifdef HOST_MAKE
static FAR struct addrinfo *g_ainfo = NULL;
#else
static int g_accept_fd = INVALID_FD;
struct apitest_close_s g_apitest_close_param[] =
{
  { "28-01", SOCKET_FD,  { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL }},
  { "28-02", ACCEPT_FD,  { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL }},
  { "28-03", INVALID_FD, { VERIFY_EQ, -1}, true,  { VERIFY_EQ, EBADF        }},
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifdef HOST_MAKE

/****************************************************************************
 * setup_close_host
 ****************************************************************************/

static void setup_close_host(char *host, char *port_no)
{
  int                  ret;
  int                  val = 1;
  struct addrinfo      hints;

  memset(&hints, 0, sizeof(hints));

  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;
  hints.ai_flags  = AI_PASSIVE;

  ret = getaddrinfo(host, port_no, &hints, &g_ainfo);
  if (ret != 0)
    {
      printf("getaddrinfo error = %d\n",ret);
      return;
    }

  g_sock_fd = socket(g_ainfo->ai_family, g_ainfo->ai_socktype,
                  g_ainfo->ai_protocol);
  if (g_sock_fd < 0)
    {
      printf("socket(%d, %d, %d) error:%d\n",
             g_ainfo->ai_family, g_ainfo->ai_socktype,
             g_ainfo->ai_protocol, errno);
      return;
    }
  printf("socket success: sockfd = %d\n", g_sock_fd);
}

/****************************************************************************
 * teardown_close_host
 ****************************************************************************/

static int teardown_close_host(void)
{
  if (INVALID_FD != g_sock_fd)
    {
      close(g_sock_fd);
    }

  if (g_ainfo)
    {
      freeaddrinfo(g_ainfo);
    }

  return 0;
}

/****************************************************************************
 * do_connect_host
 ****************************************************************************/

static int do_connect_host(void)
{
  int ret;

  ret = connect(g_sock_fd, g_ainfo->ai_addr, g_ainfo->ai_addrlen);
  if (ret < 0)
    {
      printf("conncet error:%d\n", errno);
    }
  printf("connect success: ret = %d\n", ret);

  return 0;
}

#else

/****************************************************************************
 * setup_close
 ****************************************************************************/

void setup_close(int port_no)
{
  int                 ret;
  int                 val = 1;
  struct sockaddr_in  src_addr4;

  g_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (g_sock_fd < 0)
    {
      printf("socket() error: %d\n", errno);
      return;
    }
  printf("socket success: sockfd = %d\n", g_sock_fd);

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

  ret = bind(g_sock_fd, (FAR struct sockaddr *)&src_addr4, sizeof(struct sockaddr_in));
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

  g_accept_fd = accept(g_sock_fd, NULL, NULL);

  if (g_accept_fd < 0)
    {
      printf("accept() error: %d\n", errno);
      goto errout_close;
    }

  return;

errout_close:
  g_accept_fd = INVALID_FD;
  close(g_sock_fd);
  g_sock_fd = INVALID_FD;
}

/****************************************************************************
 * setup_close
 ****************************************************************************/

int get_fd(int table_num)
{
  switch (g_apitest_close_param[table_num].fd)
    {
    case SOCKET_FD:
      return g_sock_fd;
    case ACCEPT_FD:
      return g_accept_fd;
    default:
      return INVALID_FD;
    }
}

#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * apitest_close_main
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_close_main(char* host, char *port_no)
#else
int apitest_close_main(int port_no)
#endif
{
  int i;
  int ret;
  int err;
  int result = 0;

  printf("+++++++++++++++++++++close test start+++++++++++++++++++++\n");

#ifdef HOST_MAKE

  setup_close_host(host, port_no);

  if (g_sock_fd < 0)
    {
      printf("setup_close_host() failed: %d\n", g_sock_fd);
    }
  else
    {
      do_connect_host();
    }

  teardown_close_host();

#else

  setup_close(port_no);

  for (i = 0; i < TABLE_NUM(g_apitest_close_param); i++)
    {
      printf("[%s] test start\n", g_apitest_close_param[i].test_num);
  
      ret = close(get_fd(i));
      err = errno;

      /* Check return code */

      switch(g_apitest_close_param[i].verify_ret.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(ret, &g_apitest_close_param[i].verify_ret);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_close_param[i].verify_ret.verify_flag);
            result = -1;
            break;
        }

      if ((result == 0) && (g_apitest_close_param[i].verify_err_flag))
        {
          /* Check errno */

          switch(g_apitest_close_param[i].verify_err.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(err, &g_apitest_close_param[i].verify_err);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_apitest_close_param[i].verify_err.verify_flag);
                result = -1;
                break;
            }
        }

      if (-1 == result)
        {
          printf("[%s] error\n", g_apitest_close_param[i].test_num);
        }
    }

#endif

  printf("+++++++++++++++++++++close test end+++++++++++++++++++++\n");

  return result;
}
