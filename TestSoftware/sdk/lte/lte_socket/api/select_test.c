/****************************************************************************
 * test/lte_socket/api/select_test.c
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
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "apitest.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define NORMAL_FD      0
#define INVALID_FD    -1
#define FDSET_NULL     0
#define FDSET_NOT_NULL 1
#define TO_INFINI      0
#define TO_IMIDIATE    1
#define TO_ONE_SEC     2
#define ACCEPT_MODE    0
#define CONNECT_MODE   1

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_select_s
{
  FAR const char *test_num;

  /* Input parameters */

  int  maxfd;
  int  read_fd_ptn;
  int  write_fd_ptn;
  int  except_fd_ptn;
  int  timeout_ptn;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

struct apitest_select_s g_apitest_select_param[] =
{
  { "27-01", NORMAL_FD,  FDSET_NOT_NULL, FDSET_NULL,     FDSET_NULL, TO_INFINI,   { VERIFY_EQ, 1}, false,  { VERIFY_EQ, DONTCARE_VAL}},
  { "27-02", INVALID_FD, FDSET_NOT_NULL, FDSET_NULL,     FDSET_NULL, TO_INFINI,   { VERIFY_EQ, -1}, true,  { VERIFY_EQ, EINVAL}},
  { "27-03", NORMAL_FD,  FDSET_NULL,     FDSET_NOT_NULL, FDSET_NULL, TO_INFINI,   { VERIFY_EQ, 1}, false,  { VERIFY_EQ, DONTCARE_VAL}},
  { "27-04", NORMAL_FD,  FDSET_NOT_NULL, FDSET_NULL,     FDSET_NULL, TO_INFINI,   { VERIFY_EQ, 1}, false,  { VERIFY_EQ, DONTCARE_VAL}},
  { "27-05", NORMAL_FD,  FDSET_NOT_NULL, FDSET_NULL,     FDSET_NULL, TO_INFINI,   { VERIFY_EQ, 1}, false,  { VERIFY_EQ, DONTCARE_VAL}},
  { "27-06", NORMAL_FD,  FDSET_NULL,     FDSET_NOT_NULL, FDSET_NULL, TO_INFINI,   { VERIFY_EQ, 1}, false,  { VERIFY_EQ, DONTCARE_VAL}},
  { "27-07", NORMAL_FD,  FDSET_NOT_NULL, FDSET_NULL,     FDSET_NULL, TO_INFINI,   { VERIFY_EQ, 1}, false,  { VERIFY_EQ, DONTCARE_VAL}},
  /* 27-08 : NT */
  /* 27-09 : NT */
  { "27-10", NORMAL_FD,  FDSET_NOT_NULL, FDSET_NULL,     FDSET_NULL, TO_IMIDIATE, { VERIFY_EQ, 0}, false,  { VERIFY_EQ, DONTCARE_VAL}},
  { "27-11", NORMAL_FD,  FDSET_NOT_NULL, FDSET_NULL,     FDSET_NULL, TO_ONE_SEC,  { VERIFY_EQ, 0}, false,  { VERIFY_EQ, DONTCARE_VAL}},

};

static int                  g_accept_fd = -1;
static int                  g_connect_fd = -1;
static fd_set               g_read_fdset;
static fd_set               g_write_fdset;
static fd_set               g_except_fdset;
static struct timeval       g_timeval;
static FAR struct addrinfo  *g_ainfo = NULL;
static FAR struct sockaddr  *g_addr = NULL;
static socklen_t            g_addrlen = 0;
static struct sockaddr_in   g_addr4;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int setup_accept_mode(int port)
{
  int                  ret;
  int                  val = 1;
  struct sockaddr     *addr;
  struct sockaddr_in   src_addr4;
  socklen_t            addrlen;

  g_accept_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (g_accept_fd < 0)
    {
      printf("socket() error: %d\n", errno);
      return -1;
    }
  printf("socket success: sockfd = %d\n", g_accept_fd);

  ret = setsockopt(g_accept_fd, SOL_SOCKET, SO_REUSEADDR,
                   (const int *)&val, sizeof(val));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
      goto errout_close;
    }
  printf("setsockopt(%d, %d, %d, %d) success: ret = %d\n",
         g_accept_fd, SOL_SOCKET, SO_REUSEADDR, val, ret);

  memset(&src_addr4, 0, sizeof(struct sockaddr_in));
  src_addr4.sin_family      = AF_INET;
  src_addr4.sin_port        = htons(port);
  src_addr4.sin_addr.s_addr = htonl(INADDR_ANY);

  addr    = (struct sockaddr*)&src_addr4;
  addrlen = sizeof(struct sockaddr_in);

  ret = bind(g_accept_fd, addr, addrlen);
  if (ret < 0)
    {
      printf("bind error:%d\n", errno);
      goto errout_close;
    }
  printf("bind success: ret = %d\n", ret);

  ret = listen(g_accept_fd, 1);
  if (ret < 0)
    {
      printf("listen error:%d\n", errno);
      goto errout_close;
    }
  printf("listen success: ret = %d\n", ret);

  return 0;

errout_close:
  close(g_accept_fd);
  g_accept_fd = -1;

  return -1;
}

static int teardown_accept_mode(void)
{
  if (g_accept_fd != -1)
    {
      close(g_accept_fd);
      g_accept_fd = -1;
    }

  return 0;
}

static int setup_connect_mode(bool do_connect, bool non_block, char *host, char *port)
{
  int                  ret;
  int                  flags;
  int                  family = AF_INET;
  int                  socktype = SOCK_STREAM;
  int                  protocol = 0;
  int                  port_no = atoi(port);
  struct in_addr       addr4;

  if(!inet_pton(AF_INET, host, &addr4))
    {
      return -1;
    }
  memset(&g_addr4, 0, sizeof(struct sockaddr_in));
  g_addr4.sin_family = AF_INET;
  g_addr4.sin_port   = htons(port_no);
  memcpy(&g_addr4.sin_addr, &addr4, sizeof(struct in_addr));
  g_addr     = (struct sockaddr*)&g_addr4;
  g_addrlen  = sizeof(struct sockaddr_in);

  g_connect_fd = socket(family, socktype, protocol);
  if (g_connect_fd < 0)
    {
      printf("socket(%d, %d, %d) error:%d\n",
             family, socktype, protocol, errno);
      return -1;
    }
  printf("socket success: sockfd = %d\n", g_connect_fd);

  if (non_block)
    {
      flags = fcntl(g_connect_fd, F_GETFL, 0);
      if (flags < 0)
        {
          printf("failed to fcntl(F_GETFL): %d\n", errno);
          goto errout_close;
        }

      ret = fcntl(g_connect_fd, F_SETFL, (flags | O_NONBLOCK));
      if (ret < 0)
        {
          printf("failed to fcntl(F_SETFL): %d\n", errno);
          goto errout_close;
        }
    }

  if (do_connect)
    {
      ret = connect(g_connect_fd, g_addr, g_addrlen);
      if ((ret < 0) && (errno != EINPROGRESS))
        {
          printf("conncet error:%d\n", errno);
        }
      else if (ret == 0)
        {
          printf("connect success: ret = %d\n", ret);
        }
      else
        {
          printf("conncet inprogress\n");
        }
    }
  return 0;

errout_close:
  close(g_connect_fd);
  g_connect_fd = -1;

  return -1;
}

static int teardown_connect_mode(void)
{
  if (g_connect_fd != -1)
    {
      close(g_connect_fd);
      g_connect_fd = -1;
    }

  if (g_ainfo)
    {
      freeaddrinfo(g_ainfo);
      g_ainfo = NULL;
    }

  return 0;
}

static int setup_select(int mode, char *host, char *port_no, int port)
{
  int ret;

  if (mode == ACCEPT_MODE)
    {
      ret = setup_accept_mode(port);
    }
  else
    {
      ret = setup_connect_mode(true, true, host, port_no);


    }

  return ret;
}

static int teardown_select(int mode)
{
  int ret;

  if (mode == ACCEPT_MODE)
    {
      ret = teardown_accept_mode();
    }
  else
    {
      ret = teardown_connect_mode();
    }

  return ret;
}

#ifdef HOST_MAKE
static int setup_select_host(int mode, char *host, char *port_no, int port)
{
  int ret;

  if (mode == ACCEPT_MODE)
    {
      ret = setup_accept_mode(port);
    }
  else
    {
      ret = setup_connect_mode(false, false, host, port_no);
    }

  return ret;
}

static int teardown_select_host(int mode)
{
  int ret;

  if (mode == ACCEPT_MODE)
    {
      ret = teardown_accept_mode();
    }
  else
    {
      ret = teardown_connect_mode();
    }

  return ret;
}

static int do_accept_host(void)
{
  int                ret = 0;
  int                accept_fd;
  struct sockaddr_in src_addr4;
  socklen_t          addrlen = sizeof(struct sockaddr_in);

  accept_fd = accept(g_accept_fd, (struct sockaddr*)&src_addr4, &addrlen);
  if (accept_fd < 0)
    {
      printf("accept error: %d\n", errno);
      ret = -1;
    }

  if (accept_fd != -1)
    {
      close(accept_fd);
    }

  return ret;
}

static int do_connect_host(void)
{
  int ret;

  ret = connect(g_connect_fd, g_addr, g_addrlen);
  if (ret < 0)
    {
      printf("conncet error:%d\n", errno);
    }
  else
    {
      printf("connect success: ret = %d\n", ret);
    }

  return ret;
}
#endif

static int get_select_maxfd(int ptn, int mode)
{
  if (ptn == INVALID_FD)
    {
      return -1;
    }

  if (mode == ACCEPT_MODE)
    {
      return g_accept_fd + 1;
    }

  return g_connect_fd + 1;
}

static fd_set *get_select_readfd(int ptn, int mode)
{
  if (ptn == FDSET_NULL)
    {
      return NULL;
    }

  FD_ZERO(&g_read_fdset);

  if (mode == ACCEPT_MODE)
    {
      FD_SET(g_accept_fd, &g_read_fdset);
    }
  else
    {
      FD_SET(g_connect_fd, &g_read_fdset);
    }
  
  return &g_read_fdset;
}

static fd_set *get_select_writefd(int ptn, int mode)
{
  if (ptn == FDSET_NULL)
    {
      return NULL;
    }

  FD_ZERO(&g_write_fdset);

  if (mode == ACCEPT_MODE)
    {
      FD_SET(g_accept_fd, &g_write_fdset);
    }
  else
    {
      FD_SET(g_connect_fd, &g_write_fdset);
    }

  return &g_write_fdset;
}

static fd_set *get_select_exceptfd(int ptn, int mode)
{
  if (ptn == FDSET_NULL)
    {
      return NULL;
    }

  FD_ZERO(&g_except_fdset);

  if (mode == ACCEPT_MODE)
    {
      FD_SET(g_accept_fd, &g_except_fdset);
    }
  else
    {
      FD_SET(g_connect_fd, &g_except_fdset);
    }

  return &g_except_fdset;
}

static struct timeval *get_select_timeout(int ptn)
{
  if (ptn == TO_INFINI)
    {
      return NULL;
    }
  else if (ptn == TO_IMIDIATE)
    {
      g_timeval.tv_sec  = 0;
      g_timeval.tv_usec = 0;
    }
  else
    {
      g_timeval.tv_sec  = 1;
      g_timeval.tv_usec = 0;
    }

  return &g_timeval;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int apitest_select_main(char *host, char *port_no, int port, int n)
{
  int i;
  int ret;
  int result = 0;
  int mode;

  printf("+++++++++++++++++++++select test start+++++++++++++++++++++\n");
  i = n;
//  for (i = 0; i < TABLE_NUM(g_apitest_select_param); i++)
//    {
      printf("[%s] test start\n", g_apitest_select_param[i].test_num);

#ifdef HOST_MAKE

      if (g_apitest_select_param[i].read_fd_ptn == FDSET_NOT_NULL)
        {
          mode = CONNECT_MODE;
          if (setup_select_host(mode, NULL, port_no, port) < 0)
            {
              printf("setup select host failed\n");
              return -1;
            }
          do_connect_host();
        }
      else
        {
          mode = ACCEPT_MODE;
          if (setup_select_host(mode, host, port_no, 0) < 0)
            {
              printf("setup select host failed\n");
              return -1;
            }
          do_accept_host();
        }
      teardown_select_host(mode);
#else

      if (g_apitest_select_param[i].read_fd_ptn == FDSET_NOT_NULL)
        {
          mode = ACCEPT_MODE;
          ret = setup_select(mode, NULL, NULL, port);
        }
      else
        {
          mode = CONNECT_MODE;
          ret = setup_select(mode, host, port_no, 0);
        }

      if (ret < 0)
        {
          printf("setup_select failed. ret = %d err = %d\n", ret, errno);
          teardown_select(mode);
          return -1;
        }

      ret = select(get_select_maxfd(g_apitest_select_param[i].maxfd, mode),
                   get_select_readfd(g_apitest_select_param[i].read_fd_ptn, mode),
                   get_select_writefd(g_apitest_select_param[i].write_fd_ptn, mode),
                   get_select_exceptfd(g_apitest_select_param[i].except_fd_ptn, mode),
                   get_select_timeout(g_apitest_select_param[i].timeout_ptn));

      /* Check return code */

      switch(g_apitest_select_param[i].verify_ret.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(ret, &g_apitest_select_param[i].verify_ret);
            break;

          case VERIFY_NE:
            result = apitest_verify_ne(ret, &g_apitest_select_param[i].verify_ret);
            break;

          case VERIFY_LT:
            result = apitest_verify_lt(ret, &g_apitest_select_param[i].verify_ret);
            break;

          case VERIFY_LE:
            result = apitest_verify_le(ret, &g_apitest_select_param[i].verify_ret);
            break;

          case VERIFY_GT:
            result = apitest_verify_gt(ret, &g_apitest_select_param[i].verify_ret);
            break;

          case VERIFY_GE:
            result = apitest_verify_ge(ret, &g_apitest_select_param[i].verify_ret);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_select_param[i].verify_ret.verify_flag);
            result = -1;
            break;
        }

      if ((result == 0) && (g_apitest_select_param[i].verify_err_flag))
        {
          /* Check errno */

          switch(g_apitest_select_param[i].verify_err.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(errno, &g_apitest_select_param[i].verify_err);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_apitest_select_param[i].verify_err.verify_flag);
                result = -1;
                break;
            }
        }
      if (-1 == result)
        {
          printf("No.%d error\n", i);
        }

      teardown_select(mode);
#endif
//    }

  printf("+++++++++++++++++++++select test end+++++++++++++++++++++\n");

  return result;
}
