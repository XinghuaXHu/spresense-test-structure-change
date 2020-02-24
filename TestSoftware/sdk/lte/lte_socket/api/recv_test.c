/****************************************************************************
 * test/lte_socket/api/recv_test.c
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

#define SEND_MAX_SIZE 100
#define SEND_SIZE SEND_MAX_SIZE

#define RECV_MAX_SIZE 100
#define RECV_SIZE RECV_MAX_SIZE

#define SOCKET_FD  0
#define ACCEPT_FD  1
#define INVALID_FD -1

#define BUFF_NOTNULL 1
#define BUFF_NULL    -1

#define BUFFLEN_VALID 0
#define BUFFLEN_ZERO 1

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_recv_s
{
  /* Input parameters */

  FAR const char *test_num;
  int fd;
  int buf;
  int len;
  int flags;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/


struct apitest_recv_s g_apitest_recv_param[] =
{
  {"07-01", SOCKET_FD, BUFF_NOTNULL,  BUFFLEN_VALID, 0,           { VERIFY_EQ, -1},  false, { VERIFY_EQ, DONTCARE_VAL}},
  {"07-02", ACCEPT_FD, BUFF_NOTNULL,  BUFFLEN_VALID, 0,           { VERIFY_EQ, RECV_SIZE},  false, { VERIFY_EQ, DONTCARE_VAL}},
  {"07-03", INVALID_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0,           { VERIFY_EQ, -1},  true, { VERIFY_EQ, EBADF}},

  {"07-04", ACCEPT_FD, BUFF_NULL,    BUFFLEN_VALID, 0,            { VERIFY_EQ, -1},  false, { VERIFY_EQ, EINVAL}},
  {"07-05", ACCEPT_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0,            { VERIFY_EQ, RECV_SIZE},  false, { VERIFY_EQ, DONTCARE_VAL}},

  {"07-06", ACCEPT_FD, BUFF_NOTNULL, BUFFLEN_ZERO,  0,            { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL}},
  {"07-07", ACCEPT_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0,            { VERIFY_EQ, RECV_SIZE},  false, { VERIFY_EQ, DONTCARE_VAL}},

  {"07-08", ACCEPT_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0,            { VERIFY_EQ, RECV_SIZE},  false, { VERIFY_EQ, DONTCARE_VAL}},
  {"07-09", ACCEPT_FD, BUFF_NOTNULL, BUFFLEN_VALID, MSG_DONTWAIT, { VERIFY_EQ, -1},  true, { VERIFY_EQ, EAGAIN}},
  {"07-10", ACCEPT_FD, BUFF_NOTNULL, BUFFLEN_VALID, MSG_PEEK,     { VERIFY_EQ, RECV_SIZE},  false, { VERIFY_EQ, DONTCARE_VAL}},
  {"07-11", ACCEPT_FD, BUFF_NOTNULL, BUFFLEN_VALID, MSG_WAITALL,  { VERIFY_EQ, RECV_SIZE},  false, { VERIFY_EQ, DONTCARE_VAL}},
  {"07-12", ACCEPT_FD, BUFF_NOTNULL, BUFFLEN_VALID, MSG_OOB,      { VERIFY_EQ, RECV_SIZE},  false, { VERIFY_EQ, DONTCARE_VAL}},
  {"07-13", ACCEPT_FD, BUFF_NOTNULL, BUFFLEN_VALID, MSG_MORE,     { VERIFY_EQ, RECV_SIZE},  false, { VERIFY_EQ, DONTCARE_VAL}},
};


#ifdef HOST_MAKE
static int g_clt_fd = -1;
static char g_cltbuf[SEND_MAX_SIZE] = {0};
static FAR struct addrinfo *g_ainfo = NULL;
#else
static int g_sock_fd = -1;
static int g_acpt_fd = -1;
static char g_buf[RECV_MAX_SIZE] = {0};
#endif
/****************************************************************************
 * Private Functions
 ****************************************************************************/
#ifdef HOST_MAKE
static void teardown_recv_host(void)
{
  if (g_clt_fd != -1)
    {
      close(g_clt_fd);
      g_clt_fd = -1;
    }

  if (g_ainfo)
    {
      freeaddrinfo(g_ainfo);
    }
}

static int recv_setup_host(char *host, char *port)
{
  int                 ret;
  int                 family = AF_INET;
  int                 socktype = SOCK_STREAM;
  int                 protocol = 0;
  struct sockaddr     *addr;
  socklen_t           addrlen;
  struct addrinfo     hints;
  struct in_addr      addr4;
  int                 port_no = atoi(port);
  struct sockaddr_in   dst_addr4;

  inet_pton(AF_INET, host, &addr4);
  memset(&dst_addr4, 0, sizeof(struct sockaddr_in));
  dst_addr4.sin_family = AF_INET;
  dst_addr4.sin_port   = htons(port_no);
  memcpy(&dst_addr4.sin_addr, &addr4, sizeof(struct in_addr));

  addr     = (struct sockaddr*)&dst_addr4;
  addrlen  = sizeof(struct sockaddr_in);
  g_clt_fd = socket(family, socktype, protocol);

  if (g_clt_fd < 0)
    {
      printf("socket(%d, %d, %d) error:%d\n",
             family, socktype, protocol, errno);
      return -1;
    }

  ret = connect(g_clt_fd, addr, addrlen);
  if (ret < 0)
    {
      printf("connect error %d\n", errno);
      goto errout_free;
    }

  return 0;

errout_free:
  if (g_ainfo)
    {
      freeaddrinfo(g_ainfo);
      g_ainfo = NULL;
    }
errout_close:
  close(g_clt_fd);
  g_clt_fd = -1;
  return -1;
}

#else

static bool apitest_recv_chk_result(int retcode, int errcode)
{
  if (retcode == SEND_SIZE)
    {
      return true;
    }
  else
    {
      if (errcode == EAGAIN || errcode == EWOULDBLOCK)
        {
          return true;
        }
    }

  return false;
}

static int apitest_recv_getfd(int flg)
{
  if (flg == SOCKET_FD)
    {
      return g_sock_fd;
    }
  else if (flg == ACCEPT_FD)
    {
      return g_acpt_fd;
    }
  else
    {
      return -1;
    }
}

static FAR void *apitest_recv_getbuf(int flg)
{
  if (BUFF_NULL == flg)
    {
      return NULL;
    }

  return (FAR void *)g_buf;
}

static int apitest_recv_getrcvlen(int flg)
{
  if (flg == BUFFLEN_ZERO)
  {
    return 0;
  }

  return RECV_SIZE;
}

static void teardown_recv(void)
{
  if (g_sock_fd != -1)
    {
      close(g_sock_fd);
      g_sock_fd = -1;
    }

  if (g_acpt_fd != -1)
    {
      close(g_acpt_fd);
      g_acpt_fd = -1;
    }
}

static int recv_setup(int port_no)
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

  g_acpt_fd = accept(g_sock_fd, cli_addr, &cli_addrlen);
  if (g_acpt_fd < 0)
    {
      printf("accept error:%d\n", g_acpt_fd);
      goto errout_close;
    }

  return 0;
errout_close:
  teardown_recv();
  return -1;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/
#ifdef HOST_MAKE
int apitest_recv_main(char *host, char *port_no)
{
  int ret;
  int i;
  int cnt;
  char *str = "Everything that one thinks about a lot becomes problematic.";

  printf("+----------- START RECV TEST-.- -----------+\n");

  ret = recv_setup_host(host, port_no);
  if (ret < 0)
    {
      printf("setup err\n");
      return -1;
    }
  strncpy(g_cltbuf, str, strlen(str));
  for (i = 0; i < TABLE_NUM(g_apitest_recv_param); i++)
    {
      sleep(5);
      ret = send(g_clt_fd, g_cltbuf, SEND_SIZE, 0);
      if (ret < 0)
        {
          printf("send err %d\n", errno);
        }
      else
        {
          printf("send:%d\n", ret);
        }
    }
  teardown_recv_host();
  printf("+----------- END RECV TEST^v^ -----------+\n");

  return 0;
}

int apitest_recv_to(char *host, char *port_no)
{
  int ret;
  ret = recv_setup_host(host, port_no);
  if (ret < 0)
    {
      printf("setup err\n");
      return -1;
    }

  ret = send(g_clt_fd, g_cltbuf, SEND_SIZE, 0);
  if (ret < 0)
    {
      printf("send err %d\n", errno);
    }

  teardown_recv_host();
}

int apitest_recv_nb(char *host, char *port_no)
{
  int ret;
  ret = recv_setup_host(host, port_no);
  if (ret < 0)
    {
      printf("setup err\n");
      return -1;
    }

  printf("send fst data\n");
  ret = send(g_clt_fd, g_cltbuf, SEND_SIZE, 0);
  if (ret < 0)
    {
      printf("send err %d\n", errno);
    }

  printf("wait interval\n");
  sleep(5);

  printf("send snd data\n");
  ret = send(g_clt_fd, g_cltbuf, SEND_SIZE, 0);
  if (ret < 0)
    {
      printf("send err %d\n", errno);
    }

  teardown_recv_host();
}

#else

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

int apitest_recv_to(int port_no)
{
  int ret;
  int result;
  int i;
  int err;
  struct timeval val;

  printf("+----------- START RECV TIMEOUT TEST-.- -----------+\n");

  val.tv_sec = 5;
  val.tv_usec = 0;
  ret = recv_setup(port_no);
  if (ret < 0)
    {
      printf("setup err %d\n", errno);
      return -1;
    }
  ret = setsockopt(g_acpt_fd,SOL_SOCKET, SO_RCVTIMEO, &val,
                   sizeof(struct timeval));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
      goto errout;
    }

  printf("start 07-17\n");
  ret = recv(
    g_acpt_fd,
    apitest_recv_getbuf(BUFF_NOTNULL),
    apitest_recv_getrcvlen(BUFFLEN_VALID),
    0);
  err = errno;
  result = check_timeout_result(ret, err);
  printf("[07-17] %s.\n", 0 == result ? "success" : "error");

  printf("start 07-18\n");
  for (i = 5; 0 < i; i--)
    {
      printf("Execute recvfrom after %d seconds\n", i);
      sleep(1);
    }

  ret = recv(
    g_acpt_fd,
    apitest_recv_getbuf(BUFF_NOTNULL),
    apitest_recv_getrcvlen(BUFFLEN_VALID),
    0);
  if (ret != -1)
    {
      printf("success 07-18\n");
    }
  else
    {
      printf("error 07-18 %d\n", ret);
      goto errout;

    }
  
  teardown_recv();
  printf("+----------- END RECV TIMEOUT TEST^v^ -----------+\n");
  return 0;

errout:
  teardown_recv();
  return -1;
}

int apitest_recv_nb(int port_no)
{
  int ret;
  fd_set rfd;
  int flags = 0;

  printf("+----------- START RECV NON-BLOCK TEST-.- -----------+\n");
  ret = recv_setup(port_no);
  if (ret < 0)
    {
      printf("setup failed\n");
      return -1;
    }

  ret = fcntl(g_acpt_fd, F_GETFL, 0);
  if (ret < 0)
    {
      printf("failed to fcntl(F_GETFL): %d\n", errno);
    }
  else
    {
      flags = ret;
    }

  ret = fcntl(g_acpt_fd, F_SETFL, (flags | O_NONBLOCK));
  if (ret < 0)
    {
      printf("failed to fcntl(F_SETFL): %d\n", errno);
    }

  ret = recv(
    g_acpt_fd,
    apitest_recv_getbuf(BUFF_NOTNULL),
    apitest_recv_getrcvlen(BUFFLEN_VALID),
    0);

  if ((ret == -1) && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
    {
      printf("[07-14] success. ret=%d, errno=%d\n", ret, errno);
    }
  else
    {
      printf("[07-14] error. ret=%d, errno=%d\n", ret, errno);
    }

  printf("please send data\n");

  FD_ZERO(&rfd);
  FD_SET(g_acpt_fd, &rfd);

  ret = select((g_acpt_fd+1),&rfd, NULL, NULL, NULL);
  if (ret > 0)
    {
      if (FD_ISSET(g_acpt_fd, &rfd))
        {
          printf("select success!! fd=%d\n", apitest_recv_getfd(SOCKET_FD));
        }
    }
  else
    {
      printf("select failed %d\n", ret);
      return -1;
    }

  ret = recv(
    g_acpt_fd,
    apitest_recv_getbuf(BUFF_NOTNULL),
    apitest_recv_getrcvlen(BUFFLEN_VALID),
    0);
  if (ret < 0)
    {
      printf("07-15 failed err=%d, errno=%d\n", ret, errno);
      goto errout;
    }
  printf("07-15 success\n");

  printf("please send data\n");
  sleep(10);

  ret = recv(
    g_acpt_fd,
    apitest_recv_getbuf(BUFF_NOTNULL),
    apitest_recv_getrcvlen(BUFFLEN_VALID),
    0);

  if(ret < 0)
    {
      printf("07-16 failed err=%d, errno=%d\n", ret, errno);
      goto errout;
    }

  printf("07-16 success\n");

  teardown_recv();
  printf("+----------- END RECV NON-BLOCK TEST^v^ -----------+\n");

  return 0;

errout:
  teardown_recv();
  return -1;

}

int apitest_recv_main(int port_no)
{
  int ret;
  int i;
  int result = 0;

  printf("+----------- START RECV TEST-.- -----------+\n");
  ret = recv_setup(port_no);
  if (ret < 0)
    {
      printf("setup_recv failed\n");
      return -1;
    }
  else
    {
      printf("setup_recv success\n");
    }

  for (i = 0; i < TABLE_NUM(g_apitest_recv_param); i++)
    {
      printf("[%s] test start\n", g_apitest_recv_param[i].test_num);

      ret = recv(
        apitest_recv_getfd(g_apitest_recv_param[i].fd),
        apitest_recv_getbuf(g_apitest_recv_param[i].buf),
        apitest_recv_getrcvlen(g_apitest_recv_param[i].len),
        g_apitest_recv_param[i].flags);

      /* Check return code */
      if (g_apitest_recv_param[i].flags == MSG_DONTWAIT)
        {
          if (!apitest_recv_chk_result(ret, g_apitest_recv_param[i].verify_err.verify_val))
            {
              result = -1;
            }
        }
      else
        {
          switch(g_apitest_recv_param[i].verify_ret.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(ret, &g_apitest_recv_param[i].verify_ret);
                break;

              case VERIFY_NE:
                result = apitest_verify_ne(ret, &g_apitest_recv_param[i].verify_ret);
                break;

              case VERIFY_LT:
                result = apitest_verify_lt(ret, &g_apitest_recv_param[i].verify_ret);
                break;

              case VERIFY_LE:
                result = apitest_verify_le(ret, &g_apitest_recv_param[i].verify_ret);
                break;

              case VERIFY_GT:
                result = apitest_verify_gt(ret, &g_apitest_recv_param[i].verify_ret);
                break;

              case VERIFY_GE:
                result = apitest_verify_ge(ret, &g_apitest_recv_param[i].verify_ret);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_apitest_recv_param[i].verify_ret.verify_flag);
                result = -1;
                break;
            }

          if ((result == 0) && (g_apitest_recv_param[i].verify_err_flag))
            {
              /* Check errno */

              switch(g_apitest_recv_param[i].verify_err.verify_flag)
                {
                  case VERIFY_EQ:
                    result = apitest_verify_eq(errno, &g_apitest_recv_param[i].verify_err);
                    break;

                  default:
                    printf("Unexpected verify_flag: %d\n",
                           g_apitest_recv_param[i].verify_err.verify_flag);
                    result = -1;
                    break;
                }
            }

          if (-1 == result)
            {
              printf("No.%s error\n", g_apitest_recv_param[i].test_num);
            }
        }
    }
  teardown_recv();
  printf("+----------- END RECV TEST^v^ -----------+\n");
  return result;
}
#endif
