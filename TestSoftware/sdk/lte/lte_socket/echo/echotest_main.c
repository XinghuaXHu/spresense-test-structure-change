/****************************************************************************
 * test/lte_socket/echo/echotest_main.c
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

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if defined(CONFIG_NSH_MAXARGUMENTS)
#  if (CONFIG_NSH_MAXARGUMENTS < 6)
#    error CONFIG_NSH_MAXARGUMENTS too small
#  endif
#endif

#ifndef FAR
#  define FAR
#endif

#define SHOW_OPTIONS
#ifdef SHOW_OPTIONS
#  undef SHOW_OPTIONS_ONLY
#endif

#define MAX_XFER_LEN  1500
#define RECV_BUFF_LEN 1500

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct server_param_s
{
  bool    use_tcp;
  bool    verbose;

  char   *port;
  char   *recv_buf;
  size_t  recv_len;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: show_usage
 ****************************************************************************/

static void show_usage(FAR const char *progname, int errcode)
{
  fprintf(stderr, "USAGE: %s [OPTIONS]\n", progname);
  fprintf(stderr, "\nWhere:\n");
  fprintf(stderr, "\nOPTIONS include the following:\n");
  fprintf(stderr, "\t-p <port>: port number. Default: 80\n");
  fprintf(stderr, "\t-u: UDP server. Default: TCP server\n");
  fprintf(stderr, "\t-v: verbose mode\n");
  fprintf(stderr, "\t-h: Show this text and exit\n");
  exit(errcode);
}

/****************************************************************************
 * Name: server_test
 ****************************************************************************/

static int show_server_test_option(FAR struct server_param_s *params)
{
#ifdef SHOW_OPTIONS
  printf("server options\n");
  printf("  use_tcp           : %d\n", params->use_tcp);
  printf("  port              : %s\n", params->port);
#endif
#ifdef SHOW_OPTIONS_ONLY
  exit(EXIT_SUCCESS);
#else
  return 0;
#endif
}

/****************************************************************************
 * Name: show_addrin4
 ****************************************************************************/

static void show_addrin4(FAR struct sockaddr_in *ai_addr)
{
  char            dst[INET_ADDRSTRLEN];
  FAR const char *addr;

  memset(dst, 0, sizeof(dst));
  addr = inet_ntop(AF_INET, (FAR const void*)&ai_addr->sin_addr, dst,
                   INET_ADDRSTRLEN);
  if (addr)
    {
      printf("%s\n", addr);
    }
  else
    {
      printf("n/a\n");
    }
}

/****************************************************************************
 * Name: show_addrin6
 ****************************************************************************/

static void show_addrin6(FAR struct sockaddr_in6 *ai_addr)
{
  char            dst[INET6_ADDRSTRLEN];
  FAR char const *addr;

  memset(dst, 0, sizeof(dst));
  addr = inet_ntop(AF_INET6, (FAR const void*)&ai_addr->sin6_addr, dst,
                   INET6_ADDRSTRLEN);
  if (addr)
    {
      printf("%s\n", addr);
    }
  else
    {
      printf("n/a\n");
    }
}

/****************************************************************************
 * Name: server_test
 ****************************************************************************/

static int echo_server(FAR struct server_param_s *params)
{
  int                  ret;
  int                  i;
  int                  sockfd = -1;
  int                  clsockfd = -1;
  int                  val = 1;
  struct addrinfo      hints;
  struct addrinfo     *ainfo = NULL;
  struct sockaddr_in   cli_addr4;
  struct sockaddr_in6  cli_addr6;
  struct sockaddr     *cli_addr;
  socklen_t            cli_addrlen;
  size_t               recv_len;
  size_t               send_len;
  size_t               rsize = 0;

  show_server_test_option(params);

  memset(&hints, 0, sizeof(hints));
  if (params->use_tcp)
    {
      hints.ai_socktype = SOCK_STREAM;
    }
  else
    {
      hints.ai_socktype = SOCK_DGRAM;
    }
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags  = AI_PASSIVE;

  ret = getaddrinfo(NULL, params->port, &hints, &ainfo);
  if (ret != 0)
    {
      printf("getaddrinfo error = %d\n",ret);
      return -1;
    }

  sockfd = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
  if (sockfd < 0)
    {
      printf("socket(%d, %d, %d) error:%d\n",
             ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol, errno);
      goto errout_free;
    }
  printf("socket success: sockfd = %d\n", sockfd);

  ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                   (const int *)&val, sizeof(val));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
      return -1;
    }
  printf("setsockopt(%d, %d, %d, %d) success: ret = %d\n",
         sockfd, SOL_SOCKET, SO_REUSEADDR, val, ret);

  ret = bind(sockfd, ainfo->ai_addr, ainfo->ai_addrlen);
  if (ret < 0)
    {
      printf("bind error:%d\n", errno);
      goto errout_close;
    }
  printf("bind success: ret = %d\n", ret);

  if (ainfo->ai_family == AF_INET)
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

  /* TCP */

  if (params->use_tcp)
    {
      ret = listen(sockfd, 1);
      if (ret < 0)
        {
          printf("listen error:%d\n", errno);
          goto errout_close;
        }
      printf("listen success: ret = %d\n", ret);

      clsockfd = accept(sockfd, cli_addr, &cli_addrlen);
      if (clsockfd < 0)
        {
          printf("accept error:%d\n", clsockfd);
          goto errout_close;
        }
      printf("accept success: ret = %d\n", clsockfd);
      printf("connect from: ");
      if (ainfo->ai_family == AF_INET)
        {
          show_addrin4((FAR struct sockaddr_in*)cli_addr);
        }
      else
        {
          show_addrin6((FAR struct sockaddr_in6*)cli_addr);
        }

      while(1)
        {
          recv_len = params->recv_len;

          ret = recv(clsockfd, (FAR void*)params->recv_buf, recv_len, 0);
          if (ret < 0)
            {
              printf("recv error:%d\n", errno);
              goto errout_close;
            }
          else if (ret == 0)
            {
              printf("recv ret = %d\n", ret);
              break;
            }
          printf("recv ret = %d\n", ret);

          if (params->verbose)
            {
              for (i = 0; i < ret; i++)
                {
                  printf("%02x ", params->recv_buf[i]);
                }
              printf("\n");
            }

          send_len = ret;

          ret = send(clsockfd, (FAR void*)params->recv_buf, send_len, 0);
          if (ret < 0)
            {
              printf("send error:%d\n", errno);
              goto errout_close;
            }
          printf("send ret = %d\n", ret);
        }

      ret = close(clsockfd);
      if (ret < 0)
      {
        printf("close(%d) error:%d\n", clsockfd, errno);
        return -1;
      }
    }

  /* UDP */

  else
    {
      while(1)
        {
          recv_len = params->recv_len;

          ret = recvfrom(sockfd, (FAR void*)params->recv_buf,
                         recv_len, 0, cli_addr, &cli_addrlen);
          if (ret < 0)
            {
              printf("recvfrom error:%d\n", errno);
              goto errout_close;
            }
          else if (ret == 0)
            {
              printf("recvfrom ret = %d\n", ret);
              break;
            }
          printf("recvfrom ret = %d\n", ret);

          printf("connect from: ");

          if (ainfo->ai_family == AF_INET)
            {
              show_addrin4((FAR struct sockaddr_in*)cli_addr);
            }
          else
            {
              show_addrin6((FAR struct sockaddr_in6*)cli_addr);
            }

          if (params->verbose)
            {
              for (i = 0; i < ret; i++)
                {
                  printf("%02x ", params->recv_buf[i]);
                }
              printf("\n");
            }

          send_len = ret;

          ret = sendto(sockfd, (FAR void*)params->recv_buf, send_len, 0,
                       cli_addr, cli_addrlen);
          if (ret < 0)
            {
              printf("sendto error:%d\n", errno);
              goto errout_close;
            }
          printf("sendto ret = %d\n", ret);
        }

      for (i = 0; i < rsize; i++)
        {
          printf("%02x ", params->recv_buf[i]);
        }
      printf("\n");
    }

  ret = close(sockfd);
  if (ret < 0)
  {
    printf("close(%d) error:%d\n", sockfd, errno);
    return -1;
  }

  return 0;

errout_close:
  if (clsockfd != -1)
    {
      close(clsockfd);
    }
  if (sockfd != -1)
    {
      close(sockfd);
    }

errout_free:
  if (ainfo)
    {
      freeaddrinfo(ainfo);
    }
  return -1;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef HOST_MAKE
int main(int argc, FAR char *argv[])
#else
#  ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#  else
int echotest_main(int argc, char *argv[])
#  endif
#endif
{
  int   ret;
  int   option;
  char *port            = "80";  /* default port */
  bool  use_tcp         = true;
  bool  verbose         = false;
  int   recv_buff_len   = RECV_BUFF_LEN;  /* default length */

  /* Parse input parameters */

  while ((option = getopt(argc, argv, ":p:uh")) != -1)
    {
      switch (option)
        {
          case 'p':
            port = optarg;
            break;

          case 'u':
            use_tcp = false;
            break;

          case 'v':
            verbose = true;
            break;

          case 'h':
            show_usage(argv[0], EXIT_SUCCESS);
            break;

          case ':':
            fprintf(stderr, "ERROR: Missing required argument\n");
            show_usage(argv[0], EXIT_FAILURE);
            break;

          default:
          case '?':
            fprintf(stderr, "ERROR: Unrecognized option\n");
            show_usage(argv[0], EXIT_FAILURE);
            break;
        }
    }

  /* Start echo server test */

  struct server_param_s sparams;
  memset(&sparams, 0, sizeof(struct server_param_s));

  sparams.use_tcp  = use_tcp;
  sparams.verbose  = verbose;
  sparams.port     = port;
  sparams.recv_len = recv_buff_len;
  sparams.recv_buf = (char *)malloc(recv_buff_len);
  if (sparams.recv_buf == NULL)
    {
      printf("Failed to allocate recv buffer\n");
      return -1;
    }

  ret = echo_server(&sparams);
  if (ret != 0)
    {
      printf("Failed to echo_server\n");
    }

  if (sparams.recv_buf)
    {
      free(sparams.recv_buf);
    }

  return 0;
}
