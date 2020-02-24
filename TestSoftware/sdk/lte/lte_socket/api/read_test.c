/****************************************************************************
 * test/lte_socket/api/read_test.c
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

#define READ_MAX_SIZE 100
#define READ_SIZE READ_MAX_SIZE

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

struct apitest_read_s
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


struct apitest_read_s g_apitest_read_param[] =
{
  {"30-01", SOCKET_FD, BUFF_NOTNULL,  BUFFLEN_VALID, 0,           { VERIFY_EQ, READ_SIZE},  false, { VERIFY_EQ, DONTCARE_VAL}},
  {"30-02", ACCEPT_FD, BUFF_NOTNULL,  BUFFLEN_VALID, 0,           { VERIFY_EQ, READ_SIZE},  false, { VERIFY_EQ, DONTCARE_VAL}},
  {"30-03", INVALID_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0,           { VERIFY_EQ, -1},  true, { VERIFY_EQ, EBADF}},

  {"30-04", ACCEPT_FD, BUFF_NULL,    BUFFLEN_VALID, 0,            { VERIFY_EQ, -1},  false, { VERIFY_EQ, EINVAL}},
  {"30-05", ACCEPT_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0,            { VERIFY_EQ, READ_SIZE},  false, { VERIFY_EQ, DONTCARE_VAL}},

  {"30-06", ACCEPT_FD, BUFF_NOTNULL, BUFFLEN_ZERO,  0,            { VERIFY_EQ, 0},  false, { VERIFY_EQ, DONTCARE_VAL}},
  {"30-07", ACCEPT_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0,            { VERIFY_EQ, READ_SIZE},  false, { VERIFY_EQ, DONTCARE_VAL}},
};


#ifdef HOST_MAKE
static int g_clt_fd = -1;
static char g_cltbuf[SEND_MAX_SIZE] = {0};
static FAR struct addrinfo *g_ainfo = NULL;
#else
static int g_sock_fd = -1;
static int g_acpt_fd = -1;
static char g_buf[READ_MAX_SIZE] = {0};
#endif
/****************************************************************************
 * Private Functions
 ****************************************************************************/
#ifdef HOST_MAKE
static void teardown_read_host(void)
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

static int read_setup_host(char *host, char *port)
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

static bool apitest_read_chk_result(int retcode, int errcode)
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

static int apitest_read_getfd(int flg)
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

static FAR void *apitest_read_getbuf(int flg)
{
  if (BUFF_NULL == flg)
    {
      return NULL;
    }

  return (FAR void *)g_buf;
}

static int apitest_read_getrcvlen(int flg)
{
  if (flg == BUFFLEN_ZERO)
  {
    return 0;
  }

  return READ_SIZE;
}

static void teardown_read(void)
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

static int read_setup(int port_no)
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
  teardown_read();
  return -1;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/
#ifdef HOST_MAKE
int apitest_read_main(char *host, char *port_no)
{
  int ret;
  int i;
  int cnt;
  char *str = "Everything that one thinks about a lot becomes problematic.";
  ret = read_setup_host(host, port_no);
  if (ret < 0)
    {
      printf("setup err\n");
      return -1;
    }
  strncpy(g_cltbuf, str, strlen(str));
  for (i = 0; i < TABLE_NUM(g_apitest_read_param); i++)
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
  teardown_read_host();
  return 0;
}

int apitest_read_to(char *host, char *port_no)
{
  int ret;
  ret = read_setup_host(host, port_no);
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

  teardown_read_host();
}

int apitest_read_nb(char *host, char *port_no)
{
  int ret;
  ret = read_setup_host(host, port_no);
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

  teardown_read_host();
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

int apitest_read_to(int port_no)
{
  int ret;
  int result;
  int err;
  struct timeval val;

  printf("+----------- START READ TIMEOUT TEST-.- -----------+\n");

  val.tv_sec = 5;
  val.tv_usec = 0;
  ret = read_setup(port_no);
  if (ret < 0)
    {
      printf("setup err %d\n", errno);
      return -1;
    }
  ret = setsockopt(apitest_read_getfd(SOCKET_FD),SOL_SOCKET, SO_RCVTIMEO, &val,
                   sizeof(struct timeval));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
      goto errout;
    }

  printf("Wait for  5 seconds\n");

  printf("start 30-11\n");
  ret = read(
    apitest_read_getfd(SOCKET_FD),
    apitest_read_getbuf(BUFF_NOTNULL),
    apitest_read_getrcvlen(BUFFLEN_VALID));
  err = errno;

  if (ret < 0)
    {
      result = check_timeout_result(ret, err);
      printf("[30-11] %s.\n", 0 == result ? "success" : "error");
    }
  else
    {
      printf("[30-12] success read size %d\n", ret);
    }

  teardown_read();
  printf("+----------- END READ TIMEOUT TEST^v^ -----------+\n");
  return 0;

errout:
  teardown_read();
  return -1;
}

int apitest_read_nb(int port_no)
{
  int ret;
  fd_set rfd;
  int flags = 0;

  printf("+----------- START READ NON-BLOCK TEST-.- -----------+\n");
  ret = read_setup(port_no);
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

  ret = read(
    g_acpt_fd,
    apitest_read_getbuf(BUFF_NOTNULL),
    READ_SIZE);

  if ((ret == -1) && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
    {
      printf("[30-08] success. ret=%d, errno=%d\n", ret, errno);
    }
  else
    {
      printf("[30-08] error. ret=%d, errno=%d\n", ret, errno);
    }

  printf("please send data\n");

  FD_ZERO(&rfd);
  FD_SET(g_acpt_fd, &rfd);

  ret = select((g_acpt_fd+1), &rfd, NULL , NULL, NULL);
  if (0 < ret)
    {
      if (FD_ISSET(g_acpt_fd, &rfd))
        {
          printf("select success!! fd=%d\n", g_acpt_fd);
        }
    }
  else
    {
      printf("select failed %d\n", ret);
      return -1;
    }

  ret = read(
    g_acpt_fd,
    apitest_read_getbuf(BUFF_NOTNULL),
    apitest_read_getrcvlen(BUFFLEN_VALID));
  if (ret < 0)
    {
      printf("30-09 failed err=%d\n", ret);
      goto errout;
    }
  printf("30-09 success\n");

  printf("please send data\n");
  sleep(10);

  ret = read(
    g_acpt_fd,
    apitest_read_getbuf(BUFF_NOTNULL),
    apitest_read_getrcvlen(BUFFLEN_VALID));

  if(ret < 0)
    {
      printf("30-10 failed\n");
      goto errout;
    }

  printf("30-10 success\n");

  teardown_read();
  printf("+----------- END READ NON-BLOCK TEST^v^ -----------+\n");

  return 0;

errout:
  teardown_read();
  printf("+----------- END READ NON-BLOCK TEST^v^ -----------+\n");
  return -1;

}

int apitest_read_main(int port_no)
{
  int ret;
  int i;
  int result = 0;

  printf("+----------- START READ TEST-.- -----------+\n");
  ret = read_setup(port_no);
  if (ret < 0)
    {
      printf("setup_read failed\n");
      return -1;
    }
  else
    {
      printf("setup_read success\n");
    }

  for (i = 0; i < TABLE_NUM(g_apitest_read_param); i++)
    {
      printf("[%s] test start\n", g_apitest_read_param[i].test_num);

      ret = read(
        apitest_read_getfd(g_apitest_read_param[i].fd),
        apitest_read_getbuf(g_apitest_read_param[i].buf),
        apitest_read_getrcvlen(g_apitest_read_param[i].len));

      /* Check return code */
      if (g_apitest_read_param[i].flags == MSG_DONTWAIT)
        {
          if (!apitest_read_chk_result(ret, g_apitest_read_param[i].verify_err.verify_val))
            {
              result = -1;
            }
        }
      else
        {
          switch(g_apitest_read_param[i].verify_ret.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(ret, &g_apitest_read_param[i].verify_ret);
                break;

              case VERIFY_NE:
                result = apitest_verify_ne(ret, &g_apitest_read_param[i].verify_ret);
                break;

              case VERIFY_LT:
                result = apitest_verify_lt(ret, &g_apitest_read_param[i].verify_ret);
                break;

              case VERIFY_LE:
                result = apitest_verify_le(ret, &g_apitest_read_param[i].verify_ret);
                break;

              case VERIFY_GT:
                result = apitest_verify_gt(ret, &g_apitest_read_param[i].verify_ret);
                break;

              case VERIFY_GE:
                result = apitest_verify_ge(ret, &g_apitest_read_param[i].verify_ret);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_apitest_read_param[i].verify_ret.verify_flag);
                result = -1;
                break;
            }

          if ((result == 0) && (g_apitest_read_param[i].verify_err_flag))
            {
              /* Check errno */

              switch(g_apitest_read_param[i].verify_err.verify_flag)
                {
                  case VERIFY_EQ:
                    result = apitest_verify_eq(errno, &g_apitest_read_param[i].verify_err);
                    break;

                  default:
                    printf("Unexpected verify_flag: %d\n",
                           g_apitest_read_param[i].verify_err.verify_flag);
                    result = -1;
                    break;
                }
            }

          if (-1 == result)
            {
              printf("No.%s error\n", g_apitest_read_param[i].test_num);
            }
        }
    }
  teardown_read();
  printf("+----------- END READ TEST^v^ -----------+\n");
  return result;
}
#endif
