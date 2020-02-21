/****************************************************************************
 * test/lte_socket/api/connect_test.c
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
#define SOCKET_FD         0
#define INVALID_FD        -1
#define IPV4              0
#define IPV6              1

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_connect_s
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

  /* Host behavior */

  bool do_accept;

};

/****************************************************************************
 * Private Data
 ****************************************************************************/

struct apitest_connect_s g_apitest_connect_param[] =
{
  { "03-01", SOCKET_FD,  ADDR_PTN_NOT_NULL, sizeof(struct sockaddr_in),     IPV4, { VERIFY_EQ, 0}, false, { VERIFY_EQ, DONTCARE_VAL},  true},
  { "03-02", INVALID_FD, ADDR_PTN_NOT_NULL, sizeof(struct sockaddr_in),     IPV4, { VERIFY_EQ, -1}, true, { VERIFY_EQ, EBADF},         false},
  { "03-03", SOCKET_FD,  ADDR_PTN_NULL,     sizeof(struct sockaddr_in),     IPV4, { VERIFY_EQ, -1}, true, { VERIFY_EQ, EINVAL},        false},
  // TODO: IPv6 test case
  /* 03-04 : same as 03-01 */
  //{ "03-05", SOCKET_FD,  ADDR_PTN_NOT_NULL, sizeof(struct sockaddr_in6),    IPV6, { VERIFY_EQ, 0}, false, { VERIFY_EQ, DONTCARE_VAL},  true},

  { "03-06", SOCKET_FD,  ADDR_PTN_NOT_NULL, sizeof(struct sockaddr_in) -1,  IPV4, { VERIFY_EQ, -1}, true, { VERIFY_EQ, EINVAL},        false},
  /* 03-07 : same as 03-01 */
  //{ "03-08", SOCKET_FD,  ADDR_PTN_NOT_NULL, sizeof(struct sockaddr_in6) -1, IPV6, { VERIFY_EQ, -1}, true, { VERIFY_EQ, EINVAL},        false},
  /* 03-09 : same as 03-05 */
};

static int g_connect_fd = -1;
static FAR struct addrinfo *g_ainfo = NULL;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * setup_connect
 ****************************************************************************/

int setup_connect(int ipver, char *host, char *port_no)
{
  int             ret;
  struct addrinfo hints;

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

  g_connect_fd = socket(g_ainfo->ai_family, g_ainfo->ai_socktype,
                        g_ainfo->ai_protocol);
  if (g_connect_fd < 0)
    {
      printf("socket(%d, %d, %d) error:%d\n",
             g_ainfo->ai_family, g_ainfo->ai_socktype,
             g_ainfo->ai_protocol, errno);
      return -1;
    }
  printf("socket success: sockfd = %d\n", g_connect_fd);

  return 0;
}

/****************************************************************************
 * teardown_connect
 ****************************************************************************/

int teardown_connect(void)
{
  if (g_connect_fd != -1)
    {
      close(g_connect_fd);
      g_connect_fd = -1;
    }

  if (g_ainfo)
    {
      freeaddrinfo(g_ainfo);
    }

  return 0;
}

#ifdef HOST_MAKE

/****************************************************************************
 * setup_connect_host
 ****************************************************************************/

int setup_connect_host(int ipver, int port_no)
{
  int                  ret;
  int                  val = 1;
  int                  sockfd;
  struct sockaddr     *addr;
  struct sockaddr_in   src_addr4;
  struct sockaddr_in6  src_addr6;
  socklen_t            addrlen;

  if (ipver == IPV4)
    {
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
    }
  else
    {
      sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    }

  if (sockfd < 0)
    {
      printf("socket() error: %d\n", errno);
      return -1;
    }
  printf("socket success: sockfd = %d\n", sockfd);

  ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                   (const int *)&val, sizeof(val));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
      goto errout_close;
    }
  printf("setsockopt(%d, %d, %d, %d) success: ret = %d\n",
         sockfd, SOL_SOCKET, SO_REUSEADDR, val, ret);

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

  ret = bind(sockfd, addr, addrlen);
  if (ret < 0)
    {
      printf("bind error:%d\n", errno);
      goto errout_close;
    }
  printf("bind success: ret = %d\n", ret);

  ret = listen(sockfd, 1);
  if (ret < 0)
    {
      printf("listen error:%d\n", errno);
      goto errout_close;
    }
  printf("listen success: ret = %d\n", ret);

  return sockfd;

errout_close:
  close(sockfd);

  return -1;
}

/****************************************************************************
 * teardown_connect_host
 ****************************************************************************/

int teardown_connect_host(int sockfd)
{
  close(sockfd);

  return 0;
}

/****************************************************************************
 * do_accept_host
 ****************************************************************************/

int do_accept_host(int ipver, int sockfd)
{
  int ret;
  struct sockaddr     *addr;
  struct sockaddr_in   addr4;
  struct sockaddr_in6  addr6;
  socklen_t            addrlen;

  if (ipver == IPV4)
    {
      memset(&addr4, 0, sizeof(struct sockaddr_in));
      addr    = (struct sockaddr*)&addr4;
      addrlen = sizeof(struct sockaddr_in);
    }
  else
    {
      memset(&addr6, 0, sizeof(struct sockaddr_in6));
      addr    = (struct sockaddr*)&addr6;
      addrlen = sizeof(struct sockaddr_in6);
    }

  ret = accept(sockfd, addr, &addrlen);
  if (ret < 0)
    {
      printf("accept error:%d\n", errno);
    }
  else
    {
      close(ret);
      printf("accept success: ret = %d\n", ret);
    }

  return 0;
}


#endif

/****************************************************************************
 * get_connect_fd
 ****************************************************************************/

static int get_connect_fd(int fd)
{
  int ret = -1;

  if (fd != INVALID_FD)
    {
      ret = g_connect_fd;
    }

  return ret;
}

/****************************************************************************
 * get_connect_addr
 ****************************************************************************/

static FAR struct sockaddr *get_connect_addr(int addr_ptn)
{
  if (addr_ptn == ADDR_PTN_NULL)
    {
      return NULL;
    }

  return (FAR struct sockaddr*)g_ainfo->ai_addr;
}

/****************************************************************************
 * get_connect_addr
 ****************************************************************************/

static FAR socklen_t get_connect_addrlen(int addrlen)
{
  return addrlen;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * apitest_connect_main
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_connect_main(int port_no)
#else
int apitest_connect_main(char *host, char *port_no)
#endif
{
  int i;
  int ret;
  int result = 0;
#ifdef HOST_MAKE
  int fd;
#endif

  printf("+++++++++++++++++++++connect test start+++++++++++++++++++++\n");

  for (i = 0; i < TABLE_NUM(g_apitest_connect_param); i++)
    {
      printf("[%s] test start\n", g_apitest_connect_param[i].test_num);

#ifdef HOST_MAKE

      if (g_apitest_connect_param[i].do_accept)
        {
          fd = setup_connect_host(g_apitest_connect_param[i].ipver, port_no);

          if (fd < 0)
            {
              printf("setup_connect_host() failed: %d\n", fd);
            }
          else
            {
              do_accept_host(g_apitest_connect_param[i].ipver, fd);
            }

          teardown_connect_host(fd);
        }

    
#else
      setup_connect(g_apitest_connect_param[i].ipver, host, port_no);

      ret = connect(get_connect_fd(g_apitest_connect_param[i].fd),
                    get_connect_addr(g_apitest_connect_param[i].addr_ptn),
                    get_connect_addrlen(g_apitest_connect_param[i].addrlen));

      /* Check return code */

      switch(g_apitest_connect_param[i].verify_ret.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(ret, &g_apitest_connect_param[i].verify_ret);
            break;

          case VERIFY_NE:
            result = apitest_verify_ne(ret, &g_apitest_connect_param[i].verify_ret);
            break;

          case VERIFY_LT:
            result = apitest_verify_lt(ret, &g_apitest_connect_param[i].verify_ret);
            break;

          case VERIFY_LE:
            result = apitest_verify_le(ret, &g_apitest_connect_param[i].verify_ret);
            break;

          case VERIFY_GT:
            result = apitest_verify_gt(ret, &g_apitest_connect_param[i].verify_ret);
            break;

          case VERIFY_GE:
            result = apitest_verify_ge(ret, &g_apitest_connect_param[i].verify_ret);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_connect_param[i].verify_ret.verify_flag);
            result = -1;
            break;
        }

      if ((result == 0) && (g_apitest_connect_param[i].verify_err_flag))
        {
          /* Check errno */

          switch(g_apitest_connect_param[i].verify_err.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(errno, &g_apitest_connect_param[i].verify_err);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_apitest_connect_param[i].verify_err.verify_flag);
                result = -1;
                break;
            }
        }

      if (-1 == result)
        {
          printf("[%s] error\n", g_apitest_connect_param[i].test_num);
        }

      teardown_connect();
#endif
    }

  printf("+++++++++++++++++++++connect test end+++++++++++++++++++++\n");

  return result;
}

/****************************************************************************
 * apitest_connect_noblock
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_connect_noblock(int port_no)
{
  int fd;

  fd = setup_connect_host(IPV4, port_no);

  if (fd < 0)
    {
      do_accept_host(IPV4, fd);
    }
  else
    {
      printf("setup_connect_host() failed: %d\n", fd);
    }

  teardown_connect_host(fd);
}
#else
int apitest_connect_noblock(char* host, char *port_no)
{
  int    ret;
  int    result = 0;
  int    conn_result;
  int    val_len;
  int    flags = 0;
  fd_set writeset;

  printf("+++++++++++++++++++++connect non-blocking test start+++++++++++++++++++++\n");

  setup_connect(IPV4, host, port_no);

  ret = fcntl(g_connect_fd, F_GETFL, 0);
  if (ret < 0)
    {
      printf("failed to fcntl(F_GETFL): %d\n", errno);
    }
  else
    {
      flags = ret;
    }

  ret = fcntl(g_connect_fd, F_SETFL, (flags | O_NONBLOCK));
  if (ret < 0)
    {
      printf("failed to fcntl(F_SETFL): %d\n", errno);
    }

  ret = connect(g_connect_fd, get_connect_addr(ADDR_PTN_NOT_NULL),
                get_connect_addrlen(sizeof(struct sockaddr_in)));
  if ((ret < 0) && (errno == EINPROGRESS))
    {
      printf("connect with non-block(%d)\n", errno);

      FD_ZERO(&writeset);
      FD_SET(g_connect_fd, &writeset);

      ret = select(g_connect_fd + 1, NULL, &writeset, NULL, NULL);
      if (ret == 1)
        {
          printf("select success\n");
          if (FD_ISSET(g_connect_fd, &writeset))
            {
              printf("FD_ISSET success\n");

              val_len = sizeof(conn_result);

              ret = getsockopt(g_connect_fd, SOL_SOCKET,
                               SO_ERROR, (FAR void*)&conn_result,
                               (socklen_t*)&val_len);
              if (ret == 0)
                {
                  if (conn_result == 0)
                    {
                      printf("getsockopt success\n");
                      result = 0;
                    }
                  else
                    {
                      printf("getsockopt success connect error: %d\n", conn_result);
                      result = -1;
                    }
                }
              else
                {
                  printf("getsockopt error: %d\n", errno);
                  result = -1;
                }
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
  else if (ret < 0)
    {
      printf("connect with non-block(immidiately) error:%d\n", errno);
      result = -1;
    }
  else
    {
      printf("connect with non-block(immidiately) success:%d\n", ret);
      result = 0;
    }

  teardown_connect();

  printf("+++++++++++++++++++++connect non-blocking test end+++++++++++++++++++++\n");

  return result;
}
#endif

/****************************************************************************
 * apitest_connect_timeout
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_connect_timeout(int port_no)
{
  int fd;

  fd = setup_connect_host(IPV4, port_no);

  if (fd < 0)
    {
      do_accept_host(IPV4, fd);
    }
  else
    {
      printf("setup_connect_host() failed: %d\n", fd);
    }

  teardown_connect_host(fd);
}
#else
int apitest_connect_timeout(char* host, char *port_no)
{
  int            ret;
  int            result = 0;
  struct timeval tval;

  printf("+++++++++++++++++++++connect timeout test start+++++++++++++++++++++\n");

  setup_connect(IPV4, host, port_no);

  tval.tv_sec  = 3;
  tval.tv_usec = 0;

  ret = setsockopt(g_connect_fd, SOL_SOCKET, SO_SNDTIMEO, &tval,
                   sizeof(struct timeval));
  if (ret < 0)
    {
      printf("failed to setsockopt(SO_SNDTIMEO): %d\n", errno);
    }

  ret = connect(g_connect_fd, get_connect_addr(ADDR_PTN_NOT_NULL),
                      get_connect_addrlen(sizeof(struct sockaddr_in)));
  if ((ret < 0) && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
    {
      printf("connect with timeout(%d)\n", errno);
      result = 0;
    }
  else if (ret < 0)
    {
      printf("connect with timeout error:%d\n", errno);
      result = -1;
    }
  else
    {
      printf("connect with timeout(timeout does not occur) success:%d\n", ret);
      result = 0;
    }

  teardown_connect();

  printf("+++++++++++++++++++++connect timeout test end+++++++++++++++++++++\n");

  return result;
}
#endif
