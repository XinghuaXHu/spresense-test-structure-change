/****************************************************************************
 * test/lte_socket/api/send_test.c
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
#ifdef HOST_MAKE
#  define RECV_COUNT 9
#else
#  define FD_PTN_IPV4_SOCKET  0
#  define FD_PTN_IPV4_ACCEPT  1
#  define FD_PTN_INVALID      2
#endif

#define BUF_SIZE            50
#define RECV_SIZE           BUF_SIZE
#define SEND_SIZE           BUF_SIZE
#define KEY_WORD            "This APT has Super Cow Powers."

/****************************************************************************
 * Private Types
 ****************************************************************************/

#ifndef HOST_MAKE
struct apitest_send_s
{
  FAR const char *test_num;

  /* Input parameters */

  int      fd;
  FAR void *buf;
  size_t   len;
  int      flags;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;
  bool                          sp_check_flag;

};
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int  g_fd = INVALID_FD;
static char g_buf[BUF_SIZE];

#ifndef HOST_MAKE
static struct apitest_send_s g_send_param[] =
{
  { "09-01", FD_PTN_IPV4_SOCKET, g_buf, SEND_SIZE, 0,            { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  /* When invalid FD use Assert nuttx. */
  //{ "09-03", FD_PTN_INVALID,     g_buf, SEND_SIZE, 0,            { VERIFY_EQ, -1        }, true,  { VERIFY_EQ, EBADF        }, false },

  { "09-04", FD_PTN_IPV4_SOCKET, NULL,  SEND_SIZE, 0,            { VERIFY_EQ, -1        }, true,  { VERIFY_EQ, EINVAL       }, false },
  { "09-05", FD_PTN_IPV4_SOCKET, g_buf, SEND_SIZE, 0,            { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },

  { "09-06", FD_PTN_IPV4_SOCKET, g_buf, 0,         0,            { VERIFY_EQ, 0         }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "09-07", FD_PTN_IPV4_SOCKET, g_buf, SEND_SIZE, 0,            { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },

  { "09-08", FD_PTN_IPV4_SOCKET, g_buf, SEND_SIZE, 0,            { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "09-09", FD_PTN_IPV4_SOCKET, g_buf, SEND_SIZE, MSG_DONTWAIT, { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, true  },
  { "09-10", FD_PTN_IPV4_SOCKET, g_buf, SEND_SIZE, MSG_PEEK,     { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "09-11", FD_PTN_IPV4_SOCKET, g_buf, SEND_SIZE, MSG_WAITALL,  { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "09-12", FD_PTN_IPV4_SOCKET, g_buf, SEND_SIZE, MSG_OOB,      { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
  { "09-13", FD_PTN_IPV4_SOCKET, g_buf, SEND_SIZE, MSG_MORE,     { VERIFY_EQ, SEND_SIZE }, false, { VERIFY_EQ, DONTCARE_VAL }, false },
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * teardown_send
 ****************************************************************************/

static void teardown_send(void)
{
  close(g_fd);
}

/****************************************************************************
 * setup_send
 ****************************************************************************/

static bool setup_send(FAR char *host, FAR char *port_no)
{
  bool                result = false;
  int                 fd     = INVALID_FD;
  int                 ret;
  struct addrinfo     hints;
  FAR struct addrinfo *ainfo = NULL;

  memset(&hints, 0, sizeof(hints));

  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family   = AF_INET;
  hints.ai_flags    = AI_PASSIVE;

  ret = getaddrinfo(host, port_no, &hints, &ainfo);
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
      goto exit;
    }

  ret = connect(
    fd, (FAR struct sockaddr*)ainfo->ai_addr, sizeof(struct sockaddr_in));
  if (0 != ret)
    {
      printf("connect() error = %d\n", errno);
      teardown_send();
      goto exit;
    }
  else
    {
      g_fd = fd;
      result = true;
    }

  strcpy(g_buf, KEY_WORD);

exit:
  freeaddrinfo(ainfo);
  printf("%s() %s.\n", __func__, result ? "success" : "failure");
  return result;
}

/****************************************************************************
 * setup_send_host
 ****************************************************************************/

static bool setup_send_host(int port_no)
{
  int                 fd = INVALID_FD;
  int                 ret;
  int                 val = 1;
  struct sockaddr_in  addr;
  socklen_t           addrlen;

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    {
      printf("socket() error: %d\n", errno);
      return false;
    }

  ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                   (const int *)&val, sizeof(val));
  if (ret < 0)
    {
      printf("setsockopt() error:%d\n", errno);
      goto errout_close;
    }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(port_no);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addrlen  = sizeof(struct sockaddr_in);

  ret = bind(fd, (FAR struct sockaddr *)&addr, addrlen);
  if (ret < 0)
    {
      printf("bind() error:%d\n", errno);
      goto errout_close;
    }

  ret = listen(fd, 1);
  if (ret < 0)
    {
      printf("listen() error:%d\n", errno);
      goto errout_close;
    }

  ret = accept(fd, (FAR struct sockaddr *)&addr, &addrlen);
  if (ret < 0)
    {
      printf("accept() error: %d\n", errno);
      goto errout_close;
    }

  g_fd = ret;
  printf("%s() success.\n", __func__);
  return true;

errout_close:
  printf("%s() failure.\n", __func__);
  close(fd);
  return false;
}

#ifndef HOST_MAKE
/****************************************************************************
 * get_fd
 ****************************************************************************/

static int get_fd(int table_num)
{
  if (FD_PTN_INVALID == g_send_param[table_num].fd)
    {
      return INVALID_FD;
    }

  return g_fd;
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
 * apitest_send_main
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_send_main(int port_no)
{
  int i;
  int ret;

  printf("+++++++++++++++++++++send test start+++++++++++++++++++++\n");

  if (!setup_send_host(port_no))
    {
      return -1;
    }

  for (i = 0; i < RECV_COUNT; i++)
    {
      memset(g_buf, 0, sizeof(g_buf));
      ret = recv(g_fd,
        g_buf,
        RECV_SIZE,
        0);

      printf("recv count:%d, %s\n", i, RECV_SIZE == ret ? "success" : "error");
      printf("%s\n", g_buf);
    }

  printf("+++++++++++++++++++++send test end+++++++++++++++++++++\n");
  teardown_send();
  return 0;
}
#else
int apitest_send_main(FAR char *host, FAR char *port_no)
{
  int ret;
  int err;
  int result = 0;
  int i;

  printf("+++++++++++++++++++++send test start+++++++++++++++++++++\n");

  if (!setup_send(host, port_no))
    {
      return -1;
    }

  for (i = 0; i < TABLE_NUM(g_send_param); i++)
    {
      printf("[%s] test start\n", g_send_param[i].test_num);

      ret = send(
        get_fd(i),
        g_send_param[i].buf,
        g_send_param[i].len,
        g_send_param[i].flags);
      err = errno;

      if (g_send_param[i].sp_check_flag)
        {
          if (SEND_SIZE != ret)
            {
              result = check_timeout_result(ret, err);
            }
          else
            {
              printf("Case %s return value is SEND_SIZE.\n", g_send_param[i].test_num);
              result = 0;
            }
        }
      else
        {
          /* Check return code */

          switch(g_send_param[i].verify_ret.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(ret, &g_send_param[i].verify_ret);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_send_param[i].verify_ret.verify_flag);
                result = -1;
                break;
            }
        }

      if ((result == 0) && (g_send_param[i].verify_err_flag))
        {
          /* Check errno */

          switch(g_send_param[i].verify_err.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(err, &g_send_param[i].verify_err);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_send_param[i].verify_err.verify_flag);
                result = -1;
                break;
            }
        }

      if (-1 == result)
        {
          printf("[%s] error\n", g_send_param[i].test_num);
        }
    }

  printf("+++++++++++++++++++++send test end+++++++++++++++++++++\n");

  teardown_send();
  return result;
}
#endif

/****************************************************************************
 * apitest_send_main_host
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_send_main_host(FAR char *host, FAR char *port_no)
{
  int ret;

  printf("+++++++++++++++++++++send accept FD test start+++++++++++++++++++++\n");

  if (!setup_send(host, port_no))
    {
      return -1;
    }

  memset(g_buf, 0, sizeof(g_buf));
  ret = recv(g_fd,
    g_buf,
    RECV_SIZE,
    0);

  printf("recv %s\n", RECV_SIZE == ret ? "success" : "error");
  printf("ret = %d / strlen =%d\n", ret, (int)strlen(g_buf));
  printf("%s\n", g_buf);

  printf("+++++++++++++++++++++send accept FD test end+++++++++++++++++++++\n");
  teardown_send();
  return 0;
}
#else
int apitest_send_main_host(int port_no)
{
  int ret;
  int err;
  int result = 0;

  printf("+++++++++++++++++++++send accept FD test start+++++++++++++++++++++\n");

  if (!setup_send_host(port_no))
    {
      return -1;
    }

  printf("[09-02] test start\n");

  ret = send(
    g_fd,
    g_buf,
    SEND_SIZE,
    0);
  err = errno;

  if (SEND_SIZE != ret)
    {
      printf("[09-02] error. ret:%d, errno:%d\n", ret, err);
      result = -1;
    }
  else
    {
      printf("[09-02] success.\n");
    }

  printf("+++++++++++++++++++++send accept FD test end+++++++++++++++++++++\n");

  teardown_send();
  return result;
}
#endif

/****************************************************************************
 * apitest_send_nb
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_send_nb(int port_no)
{
  int cnt = 0;
  int ret;
  char buf[BUF_SIZE];

  strcpy(buf, KEY_WORD);
  printf("+++++++++++++++++++++send non-blocking test start+++++++++++++++++++++\n");

  if (!setup_send_host(port_no))
    {
      return -1;
    }

  recv(g_fd,
    g_buf,
    RECV_SIZE,
    0);
  printf("recv %s\n", RECV_SIZE == ret ? "success" : "error");
  printf("%s\n", g_buf);

  while (1)
    {
      memset(g_buf, 0, sizeof(g_buf));
      ret = recv(g_fd,
        g_buf,
        RECV_SIZE,
        0);

      if (!memcmp(g_buf, buf, RECV_SIZE))
        {
          printf("Discover super cow power!! count:%d\n", cnt);
          break;
        }

      cnt++;
    }

  printf("+++++++++++++++++++++send non-blocking test end+++++++++++++++++++++\n");
  teardown_send();
  return 0;
}
#else
int apitest_send_nb(FAR char *host, FAR char *port_no)
{
//  int    cnt = 0;
  int    ret = 0;
//  fd_set wfd;
  int    err;
  int    result = 0;
  int    flags = 0;
//  char buf[BUF_SIZE];

  printf("+++++++++++++++++++++send non-blocking test start+++++++++++++++++++++\n");

  if (!setup_send(host, port_no))
    {
      return -1;
    }

    ret = fcntl(g_fd, F_GETFL, 0);
  if (ret < 0)
    {
      printf("failed to fcntl(F_GETFL): %d\n", errno);
    }
  else
    {
      flags = ret;
    }

  ret = fcntl(g_fd, F_SETFL, (flags | O_NONBLOCK));
  if (ret < 0)
    {
      printf("failed to fcntl(F_SETFL): %d\n", errno);
    }

  /* Can't test case of send buff full */
#if 0
  printf("[09-16] test start\n");

  ret = send(
    g_fd,
    g_buf,
    SEND_SIZE,
    MSG_DONTWAIT);
  err = errno;

  if (SEND_SIZE == ret)
    {
      printf("[09-16] success.\n");
    }
  else
    {
      printf("[09-16] error. ret:%d, err:%d\n", ret, err);
    }

  memset(buf, 0xff, sizeof(buf));
  printf("[09-14] test start\n");

  while(1)
    {
      ret = send(
        g_fd,
        buf,
        SEND_SIZE,
        MSG_DONTWAIT);
      err = errno;

      if (ret < 0)
        {
          break;
        }

      cnt++;
    }

  printf("send count:%d\n", cnt);
  if (SEND_SIZE != ret)
    {
      result = check_timeout_result(ret, err);
    }
  else
    {
      printf("Case [09-14] return value is SEND_SIZE.\n");
      result = 0;
    }
  printf("[09-14] %s.\n", 0 == result ? "success" : "error");

  printf("[09-15] test start\n");
  printf("select start.\n");
  while (1)
    {
      FD_ZERO(&wfd);
      FD_SET(g_fd, &wfd);
      select(g_fd + 1, NULL, &wfd, NULL, NULL);
      if (FD_ISSET(g_fd, &wfd))
        {
          printf("select success \n");
          break;
        }
    }
#endif

  ret = send(
    g_fd,
    g_buf,
    SEND_SIZE,
    0);
  err = errno;

  if (ret == SEND_SIZE)
    {
      printf("[09-16] success. ret=%d\n", ret);
    }
  else
    {
      printf("[09-16] error. ret=%d, errno=%d\n", ret, err);
    }

  printf("+++++++++++++++++++++send non-blocking test end+++++++++++++++++++++\n");

  teardown_send();
  return result;
}
#endif

/****************************************************************************
 * apitest_send_to
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_send_to(int port_no)
{
  int ret;

  printf("+++++++++++++++++++++send timeout test start+++++++++++++++++++++\n");

  if (!setup_send_host(port_no))
    {
      return -1;
    }

  memset(g_buf, 0, sizeof(g_buf));
  ret = recv(g_fd,
    g_buf,
    RECV_SIZE,
    0);

  printf("recv %s\n", RECV_SIZE == ret ? "success" : "error");
  printf("%s\n", g_buf);

  printf("+++++++++++++++++++++send timeout test end+++++++++++++++++++++\n");
  teardown_send();
  return 0;
}
#else
int apitest_send_to(FAR char *host, FAR char *port_no)
{
  int    i   = 0;
  int    ret = 0;
  struct timeval val;
  int    err;
  int    result = 0;

  printf("+++++++++++++++++++++send timeout test start+++++++++++++++++++++\n");

  val.tv_sec = 5;
  val.tv_usec = 0;
  if (!setup_send(host, port_no))
    {
      return -1;
    }

  ret = setsockopt(
    g_fd, SOL_SOCKET, SO_SNDTIMEO, &val, sizeof(val));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
      teardown_send();
      return -1;
    }

  printf("[09-17] test start\n");

  ret = send(
    g_fd,
    g_buf,
    SEND_SIZE,
    0);
  err = errno;

  if (SEND_SIZE != ret)
    {
      result = check_timeout_result(ret, err);
    }
  else
    {
      printf("Case [09-17] return value is SEND_SIZE.\n");
      result = 0;
    }
  printf("[09-17] %s.\n", 0 == result ? "success" : "error");

  printf("[09-18] test start\n");

  for (i = 5; 0 < i; i--)
    {
      printf("Execute send after %d seconds\n", i);
      sleep(1);
    }

  ret = send(
    g_fd,
    g_buf,
    SEND_SIZE,
    0);
  err = errno;

  if (SEND_SIZE == ret)
    {
      printf("[09-18] success.\n");
    }
  else
    {
      printf("[09-18] error. ret:%d, err:%d\n", ret, err);
    }

  teardown_send();
  return result;
}
#endif
