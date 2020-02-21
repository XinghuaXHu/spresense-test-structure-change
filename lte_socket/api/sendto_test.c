/****************************************************************************
 * test/lte_socket/api/sendto_test.c
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

#define SEND_MAX_SIZE 1500
#define RECV_MAX_SIZE 1500

#define SOCKET_FD  0
#define ACCEPT_FD  1
#define INVALID_FD -1

#define BUFF_NOTNULL 1
#define BUFF_NULL    -1

#define BUFFLEN_VALID 0
#define BUFFLEN_ZERO 1

#define ADDR_V4   0
#define ADDR_V6   1
#define ADDR_NULL -1
#define ADDR_LEN_V4_VALID   0
#define ADDR_LEN_V4_INVALID 1
#define ADDR_LEN_V6_VALID   2
#define ADDR_LEN_V6_INVALID 3
#define ADDR_LEN_SMALL      4


#define BLOCK_BUFF_FULL     1
#define BLOCK_BUFF_SELECT   2
#define BLOCK_BUFF_NOT_FULL 3

#define SNDTIMEOUT_TIMEOUT    1
#define SNDTIMEOUT_NOTTIMEOUT 2

#define SEND_SIZE SEND_MAX_SIZE
#define RECV_SIZE RECV_MAX_SIZE

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_sendto_s
{
  FAR const char *test_num;

  /* Input parameters */

  int fd;
  int buff;
  int len;
  int flags;
  int addr_ptn;
  int addrlen;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;

};

/****************************************************************************
 * Private Data
 ****************************************************************************/

struct apitest_sendto_s g_apitest_sendto_param[] =
{
  {"10-01", SOCKET_FD, BUFF_NOTNULL,  BUFFLEN_VALID, 0, ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL} },
  //{"10-02", ACCEPT_FD, BUFF_NOTNULL,  BUFFLEN_VALID, 0, ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL} },
  {"10-03", INVALID_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0, ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, -1 },       true,  { VERIFY_EQ, EBADF}        },
  
  {"10-04", SOCKET_FD, BUFF_NULL,    BUFFLEN_VALID, 0, ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, -1 },       true,  { VERIFY_EQ, EINVAL}        },
  {"10-05", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0, ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL}  },

  {"10-06", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_ZERO, 0, ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, 0 },        false, { VERIFY_EQ, DONTCARE_VAL}  },
  {"10-07", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID,  0, ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL}  },

  {"10-08", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID,  0,            ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL} },
  {"10-09", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID,  MSG_DONTWAIT, ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, -1},        true,  { VERIFY_EQ, DONTCARE_VAL} }, // TODO: ŒŸØŠÖ”
  {"10-10", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID,  MSG_PEEK,     ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL} },
  {"10-11", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID,  MSG_WAITALL,  ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL} },
  {"10-12", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID,  MSG_OOB,      ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL} },
  {"10-13", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID,  MSG_MORE,     ADDR_V4, ADDR_LEN_V4_VALID, { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL} },

  {"10-14", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0, ADDR_NULL, ADDR_LEN_V4_VALID, { VERIFY_EQ, -1},        true, { VERIFY_EQ, EINVAL} },
  {"10-15", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0, ADDR_V4,   ADDR_LEN_V4_VALID, { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL} },
  // TODO:IPv6 test case
  //{"10-16", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0, ADDR_V6,   ADDR_LEN_V6_VALID, { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL} },

  {"10-17", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0, ADDR_V4, ADDR_LEN_V4_INVALID, { VERIFY_EQ, -1},        true,  { VERIFY_EQ, EINVAL}       },
  {"10-18", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0, ADDR_V4, ADDR_LEN_V4_VALID,   { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL} },
  // TODO: IPv6 test case
  //{"10-19", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0, ADDR_V6, ADDR_LEN_V6_INVALID, { VERIFY_EQ, -1},        true,  { VERIFY_EQ, EINVAL}       },
  //{"10-20", SOCKET_FD, BUFF_NOTNULL, BUFFLEN_VALID, 0, ADDR_V6, ADDR_LEN_V6_VALID,   { VERIFY_EQ, SEND_SIZE}, false, { VERIFY_EQ, DONTCARE_VAL} },
};


static int g_cl_fd = -1;

#ifdef HOST_MAKE

static char rcvbuf[SEND_MAX_SIZE] = {0};

#else

static struct sockaddr_in  g_v4_addr;
static struct sockaddr_in6 g_v6_addr;

static char sndbuf[SEND_MAX_SIZE] = {0};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/
#ifdef HOST_MAKE

static void apitest_sendto_host(char *port_no)
{
  int                 ret;
  int                 sockfd = -1;
  int                 clsockfd = -1;
  int                 family = AF_INET;
  int                 socktype = SOCK_STREAM;
  int                 protocol = 0;
  struct addrinfo     hints;
  struct addrinfo     *ainfo = NULL;
  struct sockaddr_in  cli_addr4;
  struct sockaddr_in6 cli_addr6;
  struct sockaddr     *cli_addr;
  socklen_t           cli_addrlen;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags  = AI_PASSIVE;
  ret = getaddrinfo(NULL, port_no, &hints, &ainfo);
  if (ret != 0)
    {
      printf("getaddrinfo error = %d\n",ret);
    }
  family   = ainfo->ai_family;
  socktype = ainfo->ai_socktype;
  protocol = ainfo->ai_protocol;

  if (family == AF_INET)
    {
      memset(&cli_addr4, 0, sizeof(struct sockaddr_in));

      cli_addr     = (struct sockaddr*)&cli_addr4;
      cli_addrlen  = sizeof(struct sockaddr_in);
    }
  else
    {
      memset(&cli_addr6, 0, sizeof(struct sockaddr_in6));

      cli_addr     = (struct sockaddr*)&cli_addr6;
      cli_addrlen  = sizeof(struct sockaddr_in6);
    }
  sockfd = socket(family, socktype, protocol);
  if (sockfd < 0)
    {
      printf("socket(%d, %d, %d) error:%d\n",
             family, socktype, protocol, errno);
      goto errout_free;
    }

  ret = bind(sockfd, ainfo->ai_addr, ainfo->ai_addrlen);
  if (ret < 0)
    {
      printf("bind err %d \n", errno);
      goto errout_close;
    }

  while(1)
  {
    ret = recvfrom(sockfd, (FAR void*)rcvbuf, RECV_MAX_SIZE, 0, cli_addr, &cli_addrlen);
    if (ret < 0)
      {
        printf("recvfrom error = %d\n", errno);
        goto errout_close;
      }
    else if(ret == 0)
      {
        printf("recv EOF\n");
        //break;
      }
    else
      {
        printf("rcv :%s\n", rcvbuf);
      }
  }


  close(sockfd);
  freeaddrinfo(ainfo);

  return;

errout_close:
  if (clsockfd != -1)
    {
      close(clsockfd);
      clsockfd = -1;
    }
  if (sockfd != -1)
    {
      close(sockfd);
      sockfd = -1;
    }
  
errout_free:
  if (ainfo)
    {
      freeaddrinfo(ainfo);
      ainfo = NULL;
    }
}


#else

static int apitest_sendto_getfd(int flg)
{
  if (flg == SOCKET_FD)
  {
    return g_cl_fd;
  }
  else
  {
    return -1;
  }
}

static int apitest_sendto_getaddrlen(int flg)
{
  switch(flg)
    {
      case ADDR_LEN_V4_VALID:
        return sizeof(struct sockaddr_in);
      case ADDR_LEN_V4_INVALID:
        return (sizeof(struct sockaddr_in) - 1);
      case  ADDR_LEN_V6_INVALID:
        return sizeof(struct sockaddr_in6);
      case ADDR_LEN_V6_VALID:
        return (sizeof(struct sockaddr_in6) - 1);
      default:
        return -1;
    }
}

static int apitest_sendto_getsendlen(int flg)
{
  if (flg == BUFFLEN_ZERO)
  {
    return 0;
  }

  return SEND_SIZE;
}

static FAR struct sockaddr *apitest_sendto_getaddr(int flg)
{
  if (ADDR_V4 == flg)
    {
      return ((FAR struct sockaddr*)&(g_v4_addr));
    }
  else if (ADDR_V6 == flg)
    {
      return ((FAR struct sockaddr*)&(g_v6_addr));
    }
  else
    {
      return NULL;
    }
}

static FAR void *apitest_sendto_getbuf(int flg)
{
  char *str = "I'm so happy because today I've found my friends, They're in my head";
  if (BUFF_NULL == flg)
    {
      return NULL;
    }

  memset(sndbuf, 0, SEND_MAX_SIZE);
  strncpy(sndbuf, str, strlen(str));
  return (FAR void *)sndbuf;
}

static bool apitest_sendto_chk_result(int retcode, int errcode)
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

static int apitest_sendto_setup(char *host, char *port_no)
{
  int                 sockfd = -1;
  int                 family = AF_INET;
  int                 socktype = SOCK_DGRAM;
  int                 protocol = 0;
  int                 port = atoi(port_no);
  struct in_addr     addr4;
  struct in6_addr    addr6;

  if(inet_pton(AF_INET, host, &addr4))
    {
      memset(&g_v4_addr, 0, sizeof(struct sockaddr_in));
      g_v4_addr.sin_family = AF_INET;
      g_v4_addr.sin_port   = htons(port);
      memcpy(&g_v4_addr.sin_addr, &addr4, sizeof(struct in_addr));
    }
  else if (inet_pton(AF_INET6, host, &addr6))
    {
      family = AF_INET6;
      memset(&g_v6_addr, 0, sizeof(struct sockaddr_in6));
      g_v6_addr.sin6_family = AF_INET6;
      g_v6_addr.sin6_port   = htons(port);
      memcpy(&g_v6_addr.sin6_addr, &addr6, sizeof(struct in6_addr));
    }
  else
    {
      // error
      return -1;
    }

  sockfd = socket(family, socktype, protocol);
  if (sockfd < 0)
    {
      printf("socket() error: %d\n", errno);
      return -1;
    }

  g_cl_fd = sockfd;
  return 0;
}

static void teardown_sendto(void)
{
  if (g_cl_fd != -1)
    {
      close(g_cl_fd);
      g_cl_fd = -1;
    }
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef HOST_MAKE
int apitest_sendto_timeout(char *port_no)
{
  int i;
  int ret;
  int result = 0;

  printf("+----------- START SENDTO TIMEOUT TEST-.- -----------+\n");
  apitest_sendto_host(port_no);
  printf("+----------- END SENDTO TIMEOUT TEST^v^ -----------+\n");
}

int apitest_sendto_nb(char *port_no)
{
  int i;
  int ret;
  int result = 0;

  printf("+----------- START SENDTO NOBLOCK TEST-.- -----------+\n");
  apitest_sendto_host(port_no);
  printf("+----------- END SENDTO NOBLOCK TEST^v^ -----------+\n");
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

int apitest_sendto_timeout(char *host, char *port_no)
{
  int ret;
  int i;
  int result = 0;
  struct timeval val;
  int            err;
  printf("+----------- START SENDTO TIMEOUT TEST-.- -----------+\n");

  val.tv_sec = 5;
  val.tv_usec = 0;
  ret = apitest_sendto_setup(host, port_no);
  if (ret < 0)
    {
      printf("setup failed\n");
      return -1;
    }
  ret = setsockopt(g_cl_fd, SOL_SOCKET, SO_SNDTIMEO, &val,
                   sizeof(struct timeval));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
      goto errout;
    }

  printf("start 10-24 \n");
  ret = sendto(
    g_cl_fd,
    sndbuf,
    SEND_SIZE,
    0,
    apitest_sendto_getaddr(ADDR_V4),
    sizeof(struct sockaddr_in));

  err = errno;
  if (ret < 0)
    {
      result = check_timeout_result(ret, err);
    }
  else
    {
      if (ret == SEND_SIZE)
        {
          result = 0;
        }
      else
        {
          printf("sendto sccess. but send size is %d\n", ret);
          result = 0;
        }
    }

  if (0 != result)
    {
      printf("[10-24] test error\n");
    }
  
  printf("start 10-25 \n");

  for (i = 5; 0 < i; i--)
    {
      printf("Execute sendto after %d seconds\n", i);
      sleep(1);
    }

  ret = sendto(
    g_cl_fd,
    sndbuf,
    SEND_SIZE,
    0,
    apitest_sendto_getaddr(ADDR_V4),
    sizeof(struct sockaddr_in));

  if (RECV_SIZE != ret)
    {
      printf("[10-25] test error. ret:%d, errno:%d\n", ret, err);
      result = -1;
    }

  printf("[10-25] test success\n");
  printf("+----------- END SENDTO TIMEOUT TEST^v^ -----------+\n");
  teardown_sendto();

  return result;
errout:
  teardown_sendto();
  return -1;
}

int apitest_sendto_noblock(char *host, char *port_no)
{
  int ret;
  int    flags = 0;
#if 0
  fd_set wfd;
#endif
  printf("+----------- START SENDTO NOBLOCK TEST-.- -----------+\n");

  apitest_sendto_setup(host, port_no);
    ret = fcntl(g_cl_fd, F_GETFL, 0);
  if (ret < 0)
    {
      printf("failed to fcntl(F_GETFL): %d\n", errno);
    }
  else
    {
      flags = ret;
    }

  ret = fcntl(g_cl_fd, F_SETFL, (flags | O_NONBLOCK));
  if (ret < 0)
    {
      printf("failed to fcntl(F_SETFL): %d\n", errno);
    }

  /* Can't test case of send buff full */
#if 0
  while (1)
    {
      ret = sendto(
        g_cl_fd,
        sndbuf,
        SEND_SIZE,
        0,
        apitest_sendto_getaddr(ADDR_V4),
        sizeof(struct sockaddr_in));

      if (ret == EAGAIN || ret == EWOULDBLOCK)
        {
          printf("No.10-21 success \n");
          break;
        }
    }

  printf("start select!! fd=%d\n", g_cl_fd);
  while(1)
    {
      FD_ZERO(&wfd);
      FD_SET(g_cl_fd, &wfd);

      select((g_cl_fd+1), NULL, &wfd, NULL, NULL);
      if (FD_ISSET(g_cl_fd, &wfd))
        {
          printf("select success!! fd=%d\n", g_cl_fd);
          printf("No.10-22 success \n");
          break;
        }
    }
#endif

  ret = sendto(
    g_cl_fd,
    sndbuf,
    SEND_SIZE,
    0,
    apitest_sendto_getaddr(ADDR_V4),
    sizeof(struct sockaddr_in));

  if (ret == SEND_SIZE)
    {
      printf("[10-23] success. ret=%d\n", ret);
    }
  else
    {
      printf("[10-23] error. ret=%d, errno=%d\n", ret, errno);
    }

  teardown_sendto();
  printf("+----------- END SENDTO NOBLOCK TEST^v^ -----------+\n");

  return 0;
}

#endif

int apitest_sendto_main(char *host, char *host_v6, char *port_no)
{
  int i;
  int ret;
  int result = 0;

  printf("+----------- START SENDTO TEST-.- -----------+\n");
#ifdef HOST_MAKE
      apitest_sendto_host(port_no);
#else
  ret = apitest_sendto_setup(host, port_no);
  if (0 > ret)
    {
      printf("setup failed\n");
      return -1;
    }
  for (i = 0; i < TABLE_NUM(g_apitest_sendto_param); i++)
    {
      printf("[%s] test start\n", g_apitest_sendto_param[i].test_num);

      ret = sendto(
        apitest_sendto_getfd(g_apitest_sendto_param[i].fd),
        apitest_sendto_getbuf(g_apitest_sendto_param[i].buff),
        apitest_sendto_getsendlen(g_apitest_sendto_param[i].len),
        g_apitest_sendto_param[i].flags,
        apitest_sendto_getaddr(g_apitest_sendto_param[i].addr_ptn),
        apitest_sendto_getaddrlen(g_apitest_sendto_param[i].addrlen));

      /* Check return code */

      if (g_apitest_sendto_param[i].flags == MSG_DONTWAIT)
        {
          if(!apitest_sendto_chk_result(ret, g_apitest_sendto_param[i].verify_err.verify_val))
            {
              result = -1;
            }
        }
      else
        {
        switch(g_apitest_sendto_param[i].verify_ret.verify_flag)
          {
            case VERIFY_EQ:
              result = apitest_verify_eq(ret, &g_apitest_sendto_param[i].verify_ret);
              break;

            case VERIFY_NE:
              result = apitest_verify_ne(ret, &g_apitest_sendto_param[i].verify_ret);
              break;

            case VERIFY_LT:
              result = apitest_verify_lt(ret, &g_apitest_sendto_param[i].verify_ret);
              break;

            case VERIFY_LE:
              result = apitest_verify_le(ret, &g_apitest_sendto_param[i].verify_ret);
              break;

            case VERIFY_GT:
              result = apitest_verify_gt(ret, &g_apitest_sendto_param[i].verify_ret);
              break;

            case VERIFY_GE:
              result = apitest_verify_ge(ret, &g_apitest_sendto_param[i].verify_ret);
              break;

            default:
              printf("Unexpected verify_flag: %d\n",
                     g_apitest_sendto_param[i].verify_ret.verify_flag);
              result = -1;
              break;
          }

        if ((result == 0) && (g_apitest_sendto_param[i].verify_err_flag))
          {
            /* Check errno */

            switch(g_apitest_sendto_param[i].verify_err.verify_flag)
              {
                case VERIFY_EQ:
                  result = apitest_verify_eq(errno, &g_apitest_sendto_param[i].verify_err);
                  break;

                default:
                  printf("Unexpected verify_flag: %d\n",
                         g_apitest_sendto_param[i].verify_err.verify_flag);
                  result = -1;
                  break;
              }
          }
        }

      if (-1 == result)
        {
          printf("[%s] error\n", g_apitest_sendto_param[i].test_num);
        }

    }
  teardown_sendto();

#endif
  printf("+----------- END SENDTO TEST^v^ -----------+\n");

  return 0;
}
