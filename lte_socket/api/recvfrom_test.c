/****************************************************************************
 * test/lte_socket/api/recvfrom_test.c
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

#define INVALID_FD          -1
#define FD_PTN_IPV4_SOCKET  0
#define FD_PTN_IPV6_SOCKET  1
#define FD_PTN_IPV4_ACCEPT  2
#define FD_PTN_INVALID      3
#define ADDRLEN_PTN_NULL    -1
#define BUF_SIZE            50
#define RECV_SIZE           BUF_SIZE
#define SEND_SIZE           BUF_SIZE

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_recvfrom_s
{
  FAR const char *test_num;

  /* Input parameters */

  int             fd;
  FAR void        *buff;
  size_t          len;
  int             flags;
  struct sockaddr *addr;
  int             addrlen;
  

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;
  bool                          sp_check_flag;

};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct sockaddr_in  g_ipv4_addr;
static struct sockaddr_in6 g_ipv6_addr;
static socklen_t           g_addrlen;
static int    g_fd_table[] = { INVALID_FD, INVALID_FD, INVALID_FD };
static char g_buf[BUF_SIZE];
static struct apitest_recvfrom_s g_param_table[] =
{
  { "08-01", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  //{ "08-02", FD_PTN_IPV4_ACCEPT, g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "08-03", FD_PTN_INVALID,     g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, -1        }, true,  { VERIFY_EQ, EBADF        }, false },

  { "08-04", FD_PTN_IPV4_SOCKET, NULL,  RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, -1        }, true,  { VERIFY_EQ, EINVAL       }, false },
  { "08-05", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },

  { "08-06", FD_PTN_IPV4_SOCKET, g_buf, 0,         0,            (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, 0         }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "08-07", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },

  { "08-08", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "08-09", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, MSG_DONTWAIT, (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, true  },
  { "08-10", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, MSG_PEEK,     (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "08-11", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, MSG_WAITALL,  (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "08-12", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, MSG_OOB,      (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "08-13", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, MSG_MORE,     (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },

  { "08-14", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "08-15", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv4_addr, ADDRLEN_PTN_NULL,                { VERIFY_EQ, -1        }, true,  { VERIFY_EQ, EINVAL       }, false },
  { "08-16", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)NULL,         sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },

  { "08-17", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in),      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "08-18", FD_PTN_IPV4_SOCKET, g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in) - 1,  { VERIFY_EQ, -1        }, true,  { VERIFY_EQ, DONTCARE_VAL }, false },
  // TODO:IPv6 test case
  //{ "08-19", FD_PTN_IPV6_SOCKET, g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv6_addr, sizeof(struct sockaddr_in6),     { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  //{ "08-20", FD_PTN_IPV6_SOCKET, g_buf, RECV_SIZE, 0,            (FAR struct sockaddr *)&g_ipv6_addr, sizeof(struct sockaddr_in6) - 1, { VERIFY_EQ, -1        }, true,  { VERIFY_EQ, DONTCARE_VAL }, false },
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#if 0
/****************************************************************************
 * setup_recvfrom_host
 ****************************************************************************/

static bool setup_recvfrom_host(FAR char *hv4, FAR char *port_no)
{
  int                 fd     = INVALID_FD;
  int                 ret;
  struct addrinfo     hints;
  FAR struct addrinfo *ainfo = NULL;

  memset(&hints, 0, sizeof(hints));

  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_family   = AF_INET;
  hints.ai_flags    = AI_PASSIVE;

  ret = getaddrinfo(hv4, port_no, &hints, &ainfo);
  if (ret != 0)
    {
      printf("getaddrinfo() error = %d\n", ret);
      return false;
    }

  fd = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
  if (fd < 0)
    {
      printf(
        "socket(%d, %d, %d) error:%d\n",
        ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol, errno);
      freeaddrinfo(ainfo);
      return false;
    }

  memcpy(&g_ipv4_addr, ainfo->ai_addr, sizeof(struct sockaddr_in));
  ret = connect(
    fd, (FAR struct sockaddr*)ainfo->ai_addr, sizeof(struct sockaddr_in));
  freeaddrinfo(ainfo);
  if (0 != ret)
    {
      printf("connect() error = %d\n", errno);
      close(fd);
      return false;
    }

  g_fd_table[FD_PTN_IPV4_SOCKET] = fd;

  snprintf(g_buf, sizeof(g_buf), "This APT has Super Cow Powers.");

  printf("%s() success.\n", __func__);
  return true;
}
#endif

/****************************************************************************
 * setup_recvfrom
 ****************************************************************************/

static bool setup_recvfrom(FAR char *hv4, FAR char *hv6, FAR char *port_no)
{
  int             ret;
  int             sockfd = -1;
  int             family = AF_INET;
  int             socktype = SOCK_DGRAM;
  int             protocol = 0;
  int             port = atoi(port_no);
  struct in_addr  addr4;
  struct in6_addr addr6;

  memset(&g_ipv4_addr, 0, sizeof(struct sockaddr_in));
  g_ipv4_addr.sin_family      = AF_INET;
  g_ipv4_addr.sin_port        = htons(port);
  g_ipv4_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  sockfd = socket(family, socktype, protocol);
  if (sockfd < 0)
    {
      printf("socket() error: %d\n", errno);
      return -1;
    }

#ifndef HOST_MAKE
  ret = bind(sockfd, (FAR struct sockaddr *)&g_ipv4_addr, sizeof(struct sockaddr_in));
  if (ret < 0)
    {
      printf("bind() error:%d\n", errno);
      close(sockfd);
      return -1;
    }
  printf("bind() success: port:%d\n", port);
#endif

  g_fd_table[FD_PTN_IPV4_SOCKET] = sockfd;

  if (hv6)
    {
      if (inet_pton(AF_INET6, hv6, &addr6))
        {
          family = AF_INET6;
          memset(&g_ipv6_addr, 0, sizeof(struct sockaddr_in6));
          g_ipv6_addr.sin6_family = AF_INET6;
          g_ipv6_addr.sin6_port   = htons(port);
          memcpy(&g_ipv6_addr.sin6_addr, &addr6, sizeof(struct in6_addr));
        }
      else
        {
          // error
          close(g_fd_table[FD_PTN_IPV6_SOCKET]);
          return -1;
        }

      sockfd = socket(family, socktype, protocol);
      if (sockfd < 0)
        {
          printf("socket() error: %d\n", errno);
          close(g_fd_table[FD_PTN_IPV6_SOCKET]);
          return -1;
        }

      g_fd_table[FD_PTN_IPV6_SOCKET] = sockfd;
    }

  snprintf(g_buf, sizeof(g_buf), "This APT has Super Cow Powers.");

  printf("%s() success.\n", __func__);
  return true;
}

/****************************************************************************
 * get_fd
 ****************************************************************************/

static int get_fd(int table_num)
{
  return g_fd_table[g_param_table[table_num].fd];
}
/****************************************************************************
 * teardown_recvfrom
 ****************************************************************************/

static void teardown_recvfrom(void)
{
  int i;

  for (i = 0; i < FD_PTN_INVALID; i++)
    {
      if (INVALID_FD != g_fd_table[i])
        {
          close(g_fd_table[i]);
          g_fd_table[i] = INVALID_FD;
        }
    }
}

#ifdef HOST_MAKE

/****************************************************************************
 * exec_sendto
 ****************************************************************************/

static void exec_sendto(int table_num)
{
  int ret;
  int len;

  len = g_param_table[table_num].fd == FD_PTN_IPV4_SOCKET ?
    sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
  ret = sendto(
    get_fd(table_num),
    g_buf,
    SEND_SIZE,
    0,
    g_param_table[table_num].addr,
    len);

  if (SEND_SIZE != ret)
    {
      printf("sendto() error. ret:%d\n", ret);
    }
}

#else

/****************************************************************************
 * get_addrlen
 ****************************************************************************/

static FAR socklen_t *get_addrlen(int table_num)
{
  if (ADDRLEN_PTN_NULL == g_param_table[table_num].addrlen)
    {
      return NULL;
    }

  g_addrlen = g_param_table[table_num].addrlen;
  return &g_addrlen;
}

/****************************************************************************
 * check_timeout_result
 ****************************************************************************/

static int check_timeout_result(int ret, int err)
{
  if (-1 != ret)
    {
      printf("Failed to return value. Expect:-1 or %d, val:%d\n", SEND_SIZE, ret);
      return -1;
    }
  else
    {
      if (EAGAIN != err && EWOULDBLOCK != err)
        {
          printf("Failed to errno. Expect:%d or %d, val:%d\n", EAGAIN, EWOULDBLOCK, err);
          return -1;
        }
    }

  return 0;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * apitest_recvfrom_main
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_recvfrom_main(FAR char *hv4, FAR char *hv6, FAR char *port_no)
{
  int i;

  if (!setup_recvfrom(hv4, hv6, port_no))
    {
      return -1;
    }

  for (i = 0; i < TABLE_NUM(g_param_table); i++)
    {
      printf("[%s] test start\n", g_param_table[i].test_num);
      exec_sendto(i);
      usleep(5000);
    }

  teardown_recvfrom();

  return 0;
}
#else
int apitest_recvfrom_main(FAR char *hv4, FAR char *hv6, FAR char *port_no)
{
  int i;
  int ret;
  int err;
  int result = 0;

  printf("+++++++++++++++++++++recvfrom test start+++++++++++++++++++++\n");

  if (!setup_recvfrom(hv4, hv6, port_no))
    {
      return -1;
    }

  for (i = 0; i < TABLE_NUM(g_param_table); i++)
    {
      memset(g_buf, 0xFF, BUF_SIZE);
      printf("[%s] test start\n", g_param_table[i].test_num);

      ret = recvfrom(
        get_fd(i),
        g_param_table[i].buff,
        g_param_table[i].len,
        g_param_table[i].flags,
        g_param_table[i].addr,
        get_addrlen(i));
      err = errno;

      if (0 <= ret)
        {
          printf("recv size:%d\n%s\n", ret, (FAR char *)g_param_table[i].buff);
        }
      if (g_param_table[i].sp_check_flag)
        {
          if (SEND_SIZE != ret)
            {
              result = check_timeout_result(ret, err);
            }
          else
            {
              printf("Case %s return value is SEND_SIZE.\n", g_param_table[i].test_num);
              result = 0;
            }
        }
      else
        {
          /* Check return code */

          switch(g_param_table[i].verify_ret.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(ret, &g_param_table[i].verify_ret);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_param_table[i].verify_ret.verify_flag);
                result = -1;
                break;
            }
        }

      if ((result == 0) && (g_param_table[i].verify_err_flag))
        {
          /* Check errno */

          switch(g_param_table[i].verify_err.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(err, &g_param_table[i].verify_err);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_param_table[i].verify_err.verify_flag);
                result = -1;
                break;
            }
        }

      if (-1 == result)
        {
          printf("[%s] error. err:%d\n", g_param_table[i].test_num, err);
        }
    }

  printf("+++++++++++++++++++++recvfrom test end+++++++++++++++++++++\n");

  return result;
}
#endif

#ifdef HOST_MAKE
int apitest_recvfrom_nonblock(FAR char *host, FAR char *port_no)
{
  if (!setup_recvfrom(host, NULL, port_no))
    {
      return -1;
    }

  exec_sendto(0); /* 08-22 */
  exec_sendto(0); /* 08-23 */

  teardown_recvfrom();

  return 0;
}
#else
int apitest_recvfrom_nonblock(FAR char *host, FAR char *port_no)
{
  int    fd;
  int    ret;
  int    err;
  int    result = 0;
  fd_set readset;
  int flags = 0;

  printf("+++++++++++++++++++++recvfrom non-blocking test start+++++++++++++++++++++\n");

  if (!setup_recvfrom(host, NULL, port_no))
    {
      return -1;
    }

  fd = g_fd_table[FD_PTN_IPV4_SOCKET];
  g_addrlen = sizeof(struct sockaddr_in);
  ret = fcntl(fd, F_GETFL, 0);
  if (ret < 0)
    {
      printf("failed to fcntl(F_GETFL): %d\n", errno);
    }
  else
    {
      flags = ret;
    }

  ret = fcntl(fd, F_SETFL, (flags | O_NONBLOCK));
  if (ret < 0)
    {
      printf("failed to fcntl(F_SETFL): %d\n", errno);
    }
  printf("[08-21] test start\n");

  ret = recvfrom(
    fd,
    g_buf,
    RECV_SIZE,
    0,
    (FAR struct sockaddr *)&g_ipv4_addr,
    &g_addrlen);
  err = errno;

  if ((ret == -1) && ((err == EAGAIN) || (err == EWOULDBLOCK)))
    {
      printf("[08-21] success. ret=%d, errno=%d\n", ret, errno);
    }
  else
    {
      printf("[08-21] error. ret=%d, errno=%d\n", ret, errno);
    }

  printf("[08-22] test start\n");

  printf("please send data\n");

  FD_ZERO(&readset);
  FD_SET(fd, &readset);

  ret = select(fd + 1, &readset, NULL, NULL, NULL);
  if (ret < 0)
    {
      printf("select error:%d\n", errno);
      result = -1;
    }
  printf("select success: ret = %d\n", ret);

  if (!FD_ISSET(fd, &readset))
    {
      printf("Unexpected FD_ISSET\n");
      return -1;
    }

  ret = recvfrom(
    fd,
    g_buf,
    RECV_SIZE,
    0,
    (FAR struct sockaddr *)&g_ipv4_addr,
    &g_addrlen);
  err = errno;

  if (SEND_SIZE == ret)
    {
      printf("[08-22] success\n");
    }
  else
    {
      printf("[08-22] test error. ret:%d, errno:%d\n", ret, err);
      result = -1;
    }

  printf("[08-23] test start\n");
  printf("please send data\n");
  sleep(10);

  ret = recvfrom(
    fd,
    g_buf,
    RECV_SIZE,
    0,
    (FAR struct sockaddr *)&g_ipv4_addr,
    &g_addrlen);
  err = errno;

  if(ret < 0)
    {
      printf("08-23 failed\n");
    }

  printf("08-23 success\n");

  printf("+++++++++++++++++++++recvfrom non-blocking test end+++++++++++++++++++++\n");

  teardown_recvfrom();

  return result;
}
#endif

#ifdef HOST_MAKE
int apitest_recvfrom_timeout(FAR char *host, FAR char *port_no)
{
  if (!setup_recvfrom(host, NULL, port_no))
    {
      return -1;
    }

  exec_sendto(0); /* 08-25 */

  teardown_recvfrom();

  return 0;
}
#else
int apitest_recvfrom_timeout(FAR char *host, FAR char *port_no)
{
  int            fd;
  int            ret;
  int            err;
  int            result = 0;
  struct timeval tval;

  printf("+++++++++++++++++++++recvfrom timeout test start+++++++++++++++++++++\n");

  if (!setup_recvfrom(host, NULL, port_no))
    {
      return -1;
    }

  fd = g_fd_table[FD_PTN_IPV4_SOCKET];
  g_addrlen = sizeof(struct sockaddr_in);

  tval.tv_sec  = 5;
  tval.tv_usec = 0;
  ret = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tval,
                   sizeof(struct timeval));

  printf("[08-24] test start\n");
  printf("Wait for a timeout of 5 seconds\n");

  ret = recvfrom(
    fd,
    g_buf,
    RECV_SIZE,
    0,
    (FAR struct sockaddr *)&g_ipv4_addr,
    &g_addrlen);
  err = errno;

  if (ret < 0)
    {
      result = check_timeout_result(ret, err);
      printf("[08-24] %s.\n", 0 == result ? "success" : "error");
    }
  else
    {
      printf("recvfrom success. recv size is %d\n", ret);
      printf("[08-25] success.\n");
    }

/*
  printf("[08-25] test start\n");

  for (i = 5; 0 < i; i--)
    {
      printf("Execute recvfrom after %d seconds\n", i);
      sleep(1);
    }

  ret = recvfrom(
    fd,
    g_buf,
    RECV_SIZE,
    0,
    (FAR struct sockaddr *)&g_ipv4_addr,
    &g_addrlen);
  err = errno;

  if (SEND_SIZE != ret)
    {
      printf("[08-25] test error. ret:%d, errno:%d\n", ret, err);
      result = -1;
    }
*/
  printf("+++++++++++++++++++++recvfrom timeout test end+++++++++++++++++++++\n");

  teardown_recvfrom();

  return result;
}
#endif
