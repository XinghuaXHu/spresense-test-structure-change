/****************************************************************************
 * test/lte_socket/api/accept_test.c
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
#define INVALID_FD        -1
#define IPV4              0
#define IPV6              1

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_accept_s
{
  FAR const char *test_num;

  /* Input parameters */

  int fd;
  int addr_ptn;
  int addrlen;
  int ipver;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;
  bool                          verify_addrlen_flag;
  struct apitest_verify_param_s verify_addrlen;

  /* Host behavior */

  bool do_connect;

};

/****************************************************************************
 * Private Data
 ****************************************************************************/

struct apitest_accept_s g_apitest_accept_param[] =
{
  { "01-01", SOCKET_FD,  ADDR_PTN_NOT_NULL, sizeof(struct sockaddr_in),     IPV4, { VERIFY_GE, 0}, false, { VERIFY_EQ, DONTCARE_VAL}, true,  { VERIFY_EQ, sizeof(struct sockaddr_in)},  true},
  { "01-02", INVALID_FD, ADDR_PTN_NOT_NULL, sizeof(struct sockaddr_in),     IPV4, { VERIFY_EQ, -1}, true, { VERIFY_EQ, EBADF},        false, { VERIFY_EQ, sizeof(struct sockaddr_in)},  false},
  /* 01-03 : same as 01-01 */
  { "01-04", SOCKET_FD,  ADDR_PTN_NOT_NULL, ADDRLEN_NULL,                   IPV4, { VERIFY_EQ, -1}, true, { VERIFY_EQ, EINVAL},       false, { VERIFY_EQ, sizeof(struct sockaddr_in)},  false},
  { "01-05", SOCKET_FD,  ADDR_PTN_NULL,     ADDRLEN_NULL,                   IPV4, { VERIFY_GE, 0}, false, { VERIFY_EQ, DONTCARE_VAL}, true,  { VERIFY_EQ, sizeof(struct sockaddr_in)},  true},
  /* 01-06 : same as 01-01 */
  { "01-07", SOCKET_FD,  ADDR_PTN_NOT_NULL, sizeof(struct sockaddr_in) -1,  IPV4, { VERIFY_GE, 0}, false, { VERIFY_EQ, DONTCARE_VAL}, true,  { VERIFY_EQ, sizeof(struct sockaddr_in)},  true},
  { "01-08", SOCKET_FD,  ADDR_PTN_NOT_NULL, sizeof(struct sockaddr_in6),    IPV6, { VERIFY_GE, 0}, false, { VERIFY_EQ, DONTCARE_VAL}, true,  { VERIFY_EQ, sizeof(struct sockaddr_in6)}, true},
  { "01-09", SOCKET_FD,  ADDR_PTN_NOT_NULL, sizeof(struct sockaddr_in6) -1, IPV6, { VERIFY_GE, 0}, false, { VERIFY_EQ, DONTCARE_VAL}, true,  { VERIFY_EQ, sizeof(struct sockaddr_in6)}, true},
};

static int g_accept_fd = -1;
static struct sockaddr_in6 g_accept_addr;
static socklen_t g_accept_addrlen;
#ifdef HOST_MAKE
static FAR struct addrinfo *g_ainfo = NULL;
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * setup_accept
 ****************************************************************************/

int setup_accept(int ipver, int port_no)
{
  int                  ret;
  int                  val = 1;
  struct sockaddr     *addr;
  struct sockaddr_in   src_addr4;
  struct sockaddr_in6  src_addr6;
  socklen_t            addrlen;

  if (ipver == IPV4)
    {
  g_accept_fd = socket(AF_INET, SOCK_STREAM, 0);
    }
  else
    {
      g_accept_fd = socket(AF_INET6, SOCK_STREAM, 0);
    }

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

  if (ipver == IPV4)
    {
      memset(&src_addr4, 0, sizeof(struct sockaddr_in));
      src_addr4.sin_family      = AF_INET;
      src_addr4.sin_port        = htons(port_no);
      src_addr4.sin_addr.s_addr = htonl(INADDR_ANY);

      addr    = (struct sockaddr*)&src_addr4;
      addrlen = sizeof(struct sockaddr_in);
    }
  else
    {
      memset(&src_addr6, 0, sizeof(struct sockaddr_in6));
      src_addr6.sin6_family = AF_INET6;
      src_addr6.sin6_port   = htons(port_no);

      addr     = (struct sockaddr*)&src_addr6;
      addrlen  = sizeof(struct sockaddr_in6);
    }

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

/****************************************************************************
 * teardown_accept
 ****************************************************************************/

int teardown_accept(int fd)
{
  if (fd != -1)
    {
      close(fd);
    }

  if (g_accept_fd != -1)
    {
      close(g_accept_fd);
      g_accept_fd = -1;
    }

  return 0;
}

#ifdef HOST_MAKE

/****************************************************************************
 * setup_accept_host
 ****************************************************************************/

int setup_accept_host(int ipver, char *host, char *port_no)
{
  int                  ret;
  int                  sockfd;
  int                  val = 1;
  struct addrinfo      hints;

  memset(&hints, 0, sizeof(hints));

  hints.ai_socktype = SOCK_STREAM;
  if (ipver == IPV4)
    {
      hints.ai_family = AF_INET;
    }
  else
    {
      hints.ai_family = AF_INET6;
    }
  hints.ai_flags  = AI_PASSIVE;

  ret = getaddrinfo(host, port_no, &hints, &g_ainfo);
  if (ret != 0)
    {
      printf("getaddrinfo error = %d\n",ret);
      return -1;
    }

  sockfd = socket(g_ainfo->ai_family, g_ainfo->ai_socktype,
                  g_ainfo->ai_protocol);
  if (sockfd < 0)
    {
      printf("socket(%d, %d, %d) error:%d\n",
             g_ainfo->ai_family, g_ainfo->ai_socktype,
             g_ainfo->ai_protocol, errno);
      return -1;
    }
  printf("socket success: sockfd = %d\n", sockfd);

  return sockfd;
}

/****************************************************************************
 * teardown_accept_host
 ****************************************************************************/

int teardown_accept_host(int sockfd)
{
  close(sockfd);

  if (g_ainfo)
    {
      freeaddrinfo(g_ainfo);
    }

  return 0;
}

/****************************************************************************
 * do_connect_host
 ****************************************************************************/

int do_connect_host(int sockfd)
{
  int ret;
  char buf[4];

  ret = connect(sockfd, g_ainfo->ai_addr, g_ainfo->ai_addrlen);
  if (ret < 0)
    {
      printf("conncet ret:%d, error:%d\n", ret, errno);
    }
  else
    {
      printf("connect success.\n");
    }

  ret = recv(sockfd, buf, sizeof(buf), 0);
  if (ret == 0)
    {
      printf("shutdown from server.\n");
    }

  return 0;
}

#endif

/****************************************************************************
 * get_accept_fd
 ****************************************************************************/

static int get_accept_fd(int fd)
{
  int ret = -1;

  if (fd != INVALID_FD)
    {
      ret = g_accept_fd;
    }

  return ret;
}

/****************************************************************************
 * get_accept_addr
 ****************************************************************************/

static FAR struct sockaddr *get_accept_addr(int addr_ptn)
{
  if (addr_ptn == ADDR_PTN_NULL)
    {
      return NULL;
    }

  return (FAR struct sockaddr*)&g_accept_addr;
}

/****************************************************************************
 * get_accept_addr
 ****************************************************************************/

static FAR socklen_t *get_accept_addrlen(int addrlen)
{
  if (addrlen == ADDRLEN_NULL)
    {
      return NULL;
    }

  g_accept_addrlen = addrlen;

  return &g_accept_addrlen;
}

/****************************************************************************
 * main_proc
 ****************************************************************************/

#ifdef HOST_MAKE
static int main_proc(int table_num, FAR char *host, FAR char *port_no)
#else
static int main_proc(int table_num, int port_no)
#endif
{
  int ret;
  int result;
#ifdef HOST_MAKE
  int fd;
#endif

  printf("[%s] test start\n", g_apitest_accept_param[table_num].test_num);

#ifdef HOST_MAKE

  if (g_apitest_accept_param[table_num].do_connect)
    {
      fd = setup_accept_host(g_apitest_accept_param[table_num].ipver, host, port_no);

      if (fd < 0)
        {
          printf("setup_accept_host() failed: %d\n", fd);
        }
      else
        {
          do_connect_host(fd);
        }

      teardown_accept_host(fd);
    }
  else
    {
      printf("Do not connect case.\n");
    }

#else
  setup_accept(g_apitest_accept_param[table_num].ipver, port_no);

  ret = accept(get_accept_fd(g_apitest_accept_param[table_num].fd),
               get_accept_addr(g_apitest_accept_param[table_num].addr_ptn),
               get_accept_addrlen(g_apitest_accept_param[table_num].addrlen));

  /* Check return code */

  switch(g_apitest_accept_param[table_num].verify_ret.verify_flag)
    {
      case VERIFY_EQ:
        result = apitest_verify_eq(ret, &g_apitest_accept_param[table_num].verify_ret);
        break;

      case VERIFY_NE:
        result = apitest_verify_ne(ret, &g_apitest_accept_param[table_num].verify_ret);
        break;

      case VERIFY_LT:
        result = apitest_verify_lt(ret, &g_apitest_accept_param[table_num].verify_ret);
        break;

      case VERIFY_LE:
        result = apitest_verify_le(ret, &g_apitest_accept_param[table_num].verify_ret);
        break;

      case VERIFY_GT:
        result = apitest_verify_gt(ret, &g_apitest_accept_param[table_num].verify_ret);
        break;

      case VERIFY_GE:
        result = apitest_verify_ge(ret, &g_apitest_accept_param[table_num].verify_ret);
        break;

      default:
        printf("Unexpected verify_flag: %d\n",
               g_apitest_accept_param[table_num].verify_ret.verify_flag);
        result = -1;
        break;
    }

  if ((result == 0) && (g_apitest_accept_param[table_num].verify_err_flag))
    {
      /* Check errno */

      switch(g_apitest_accept_param[table_num].verify_err.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(errno, &g_apitest_accept_param[table_num].verify_err);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_accept_param[table_num].verify_err.verify_flag);
            result = -1;
            break;
        }
    }

  if ((result == 0) && (g_apitest_accept_param[table_num].verify_addrlen_flag))
    {
      /* Check addrlen */

      switch(g_apitest_accept_param[table_num].verify_addrlen.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(g_accept_addrlen, &g_apitest_accept_param[table_num].verify_addrlen);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_accept_param[table_num].verify_addrlen.verify_flag);
            result = -1;
            break;
        }
    }

  if (-1 == result)
    {
      printf("[%s] error\n", g_apitest_accept_param[table_num].test_num);
    }

  teardown_accept(ret);
#endif

  return result;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * apitest_accept_main_v4
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_accept_main_v4(char* host, char *port_no)
#else
int apitest_accept_main_v4(int port_no)
#endif
{
  int i;
  int result = 0;

  printf("+++++++++++++++++++++accept test start+++++++++++++++++++++\n");

  for (i = 0; i < TABLE_NUM(g_apitest_accept_param); i++)
    {
      if (IPV4 != g_apitest_accept_param[i].ipver)
        {
          continue;
        }

#ifdef HOST_MAKE
      result |= main_proc(i, host, port_no);
#else
      result |= main_proc(i, port_no);
#endif
    }

  printf("+++++++++++++++++++++accept test end+++++++++++++++++++++\n");

  return result;
}

/****************************************************************************
 * apitest_accept_main_v6
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_accept_main_v6(char* host, char *port_no)
#else
int apitest_accept_main_v6(int port_no)
#endif
{
  int i;
  int result = 0;

  printf("+++++++++++++++++++++accept test start+++++++++++++++++++++\n");

  for (i = 0; i < TABLE_NUM(g_apitest_accept_param); i++)
    {
      if (IPV6 != g_apitest_accept_param[i].ipver)
        {
          continue;
        }

#ifdef HOST_MAKE
      result |= main_proc(i, host, port_no);
#else
      result |= main_proc(i, port_no);
#endif
    }

  printf("+++++++++++++++++++++accept test end+++++++++++++++++++++\n");

  return result;
}

/****************************************************************************
 * apitest_accept_noblock
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_accept_noblock(char* host, char *port_no)
{
  int fd;

  fd = setup_accept_host(IPV4, host, port_no);

  if (fd < 0)
    {
      do_connect_host(fd);
    }
  else
    {
      printf("setup_accept_host() failed: %d\n", fd);
    }

  teardown_accept_host(fd);
}
#else
int apitest_accept_noblock(int port_no)
{
  int    ret;
  int    result = 0;
  int    acceptfd;
  int    flags = 0;
  fd_set readset;

  printf("+++++++++++++++++++++accept non-blocking test start+++++++++++++++++++++\n");

  setup_accept(IPV4, port_no);

  ret = fcntl(g_accept_fd, F_GETFL, 0);
  if (ret < 0)
    {
      printf("failed to fcntl(F_GETFL): %d\n", errno);
    }
  else
    {
      flags = ret;
    }

  ret = fcntl(g_accept_fd, F_SETFL, (flags | O_NONBLOCK));
  if (ret < 0)
    {
      printf("failed to fcntl(F_SETFL): %d\n", errno);
    }

  acceptfd = accept(g_accept_fd, get_accept_addr(ADDR_PTN_NOT_NULL),
                    get_accept_addrlen(sizeof(struct sockaddr_in)));
  if ((acceptfd < 0) && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
    {
      printf("accept with non-block(%d)\n", errno);

      FD_ZERO(&readset);
      FD_SET(g_accept_fd, &readset);

      ret = select(g_accept_fd + 1, &readset, NULL, NULL, NULL);
      if (ret == 1)
        {
          printf("select success\n");
          if (FD_ISSET(g_accept_fd, &readset))
            {
              printf("FD_ISSET success\n");
              result = 0;
            }
          else
            {
              printf("FD_ISSET error(unexpected!!)\n");
              result = -1;
            }
        }
      else
        {
          printf("select error:%d,%d\n", ret, errno);
          result = -1;
        }
    }
  else if (acceptfd < 0)
    {
      printf("accept with non-block(immidiately) error:%d\n", errno);
      result = -1;
    }
  else
    {
      printf("accept with non-block(immidiately) success:%d\n", acceptfd);
      result = 0;
    }

  teardown_accept(acceptfd);

  printf("+++++++++++++++++++++accept non-blocking test end+++++++++++++++++++++\n");

  return result;
}
#endif

/****************************************************************************
 * apitest_accept_timeout
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_accept_timeout(char* host, char *port_no)
{
  int fd;

  fd = setup_accept_host(IPV4, host, port_no);

  if (fd < 0)
    {
      do_connect_host(fd);
    }
  else
    {
      printf("setup_accept_host() failed: %d\n", fd);
    }

  teardown_accept_host(fd);
}
#else
int apitest_accept_timeout(int port_no)
{
  int            ret;
  int            result = 0;
  int            acceptfd;
  struct timeval tval;

  printf("+++++++++++++++++++++accept timeout test start+++++++++++++++++++++\n");

  setup_accept(IPV4, port_no);

  tval.tv_sec  = 3;
  tval.tv_usec = 0;

  ret = setsockopt(g_accept_fd, SOL_SOCKET, SO_RCVTIMEO, &tval,
                   sizeof(struct timeval));
  if (ret < 0)
    {
      printf("failed to setsockopt(SO_RCVTIMEO): %d\n", errno);
    }

  acceptfd = accept(g_accept_fd, get_accept_addr(ADDR_PTN_NOT_NULL),
                    get_accept_addrlen(sizeof(struct sockaddr_in)));
  if ((acceptfd < 0) && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
    {
      printf("accept with timeout(%d)\n", errno);
      result = 0;
    }
  else if (acceptfd < 0)
    {
      printf("accept with timeout error:%d\n", errno);
      result = -1;
    }
  else
    {
      printf("accept with timeout(timeout does not occur) success:%d\n", acceptfd);
      result = 0;
    }

  teardown_accept(acceptfd);

  printf("+++++++++++++++++++++accept timeout test end+++++++++++++++++++++\n");

  return result;
}
#endif
