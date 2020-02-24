/****************************************************************************
 * test/lte_socket/protocol/protocoltest_main.c
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
#  if (CONFIG_NSH_MAXARGUMENTS < 8)
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

#define MAX_XFER_LEN 1500

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct client_param_s
{
  bool    use_getaddrinfo;
  bool    use_tcp;

  char   *host;
  char   *port;
  char   *send_buf;
  size_t  send_len;
};

struct server_param_s
{
  bool    use_getaddrinfo;
  bool    use_ipv6;
  bool    use_tcp;
  bool    use_select;

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
  fprintf(stderr, "\t-c <host>: Client test.  Default: Server Test\n");
  fprintf(stderr, "\t-p <port>: port number\n");
  fprintf(stderr, "\t-u: UDP test. Default: TCP Test\n");
  fprintf(stderr, "\t-g: Use getaddrinfo. Default: Not use\n");
  fprintf(stderr, "\t-s: Use select for server test. Default: Not use\n");
  fprintf(stderr, "\t-6: Use IPv6 address for server test. Default: IPv4\n");
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
  printf("  use_getaddrinfo: %d\n", params->use_getaddrinfo);
  printf("  use_ipv6       : %d\n", params->use_ipv6);
  printf("  use_tcp        : %d\n", params->use_tcp);
  printf("  use_select     : %d\n", params->use_select);
  printf("  port           : %s\n", params->port);
#endif
#ifdef SHOW_OPTIONS_ONLY
  exit(EXIT_SUCCESS);
#else
  return 0;
#endif
}

/****************************************************************************
 * Name: show_client_test_option
 ****************************************************************************/

static int show_client_test_option(FAR struct client_param_s *params)
{
#ifdef SHOW_OPTIONS
  printf("client options\n");
  printf("  use_getaddrinfo: %d\n", params->use_getaddrinfo);
  printf("  use_tcp        : %d\n", params->use_tcp);
  printf("  host           : %s\n", params->host);
  printf("  port           : %s\n", params->port);
#endif
#ifdef SHOW_OPTIONS_ONLY
  exit(EXIT_SUCCESS);
#else
  return 0;
#endif
}

/****************************************************************************
 * Name: server_test
 ****************************************************************************/

static int server_test(FAR struct server_param_s *params)
{
  int                  ret;
  int                  i;
  int                  sockfd = -1;
  int                  clsockfd = -1;
  int                  val = 1;
  int                  family = AF_INET;
  int                  socktype = SOCK_STREAM;
  int                  protocol = 0;
  int                  port_no = atoi(params->port);
  struct addrinfo      hints;
  struct addrinfo     *ainfo = NULL;
  struct sockaddr_in   src_addr4;
  struct sockaddr_in6  src_addr6;
  struct sockaddr_in   cli_addr4;
  struct sockaddr_in6  cli_addr6;
  struct sockaddr     *addr;
  struct sockaddr     *cli_addr;
  socklen_t            addrlen;
  socklen_t            cli_addrlen;
  fd_set               readset;
  size_t               recv_len;
  size_t               remain_len;
  size_t               rsize = 0;

  show_server_test_option(params);

  if (params->use_getaddrinfo)
    {
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
      family   = ainfo->ai_family;
      socktype = ainfo->ai_socktype;
      protocol = ainfo->ai_protocol;
      addr     = ainfo->ai_addr;
      addrlen  = ainfo->ai_addrlen;

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
    }
  else
    {
      if (params->use_ipv6)
        {
          family = AF_INET6;
        }
      if (!params->use_tcp)
        {
          socktype = SOCK_DGRAM;
        }

      if (params->use_ipv6)
        {
          memset(&src_addr6, 0, sizeof(struct sockaddr_in6));
          src_addr6.sin6_family = AF_INET6;
          src_addr6.sin6_port   = htons(port_no);

          addr     = (struct sockaddr*)&src_addr6;
          addrlen  = sizeof(struct sockaddr_in6);

          memset(&cli_addr6, 0, sizeof(struct sockaddr_in6));

          cli_addr     = (struct sockaddr*)&cli_addr6;
          cli_addrlen  = sizeof(struct sockaddr_in6);
        }
      else
        {
          memset(&src_addr4, 0, sizeof(struct sockaddr_in));
          src_addr4.sin_family      = AF_INET;
          src_addr4.sin_port        = htons(port_no);
          src_addr4.sin_addr.s_addr = htonl(INADDR_ANY);

          addr     = (struct sockaddr*)&src_addr4;
          addrlen  = sizeof(struct sockaddr_in);

          memset(&cli_addr4, 0, sizeof(struct sockaddr_in));

          cli_addr     = (struct sockaddr*)&cli_addr4;
          cli_addrlen  = sizeof(struct sockaddr_in);
        }
    }

  sockfd = socket(family, socktype, protocol);
  if (sockfd < 0)
    {
      printf("socket(%d, %d, %d) error:%d\n",
             family, socktype, protocol, errno);
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

  ret = bind(sockfd, addr, addrlen);
  if (ret < 0)
    {
      printf("bind error:%d\n", errno);
      goto errout_close;
    }
  printf("bind success: ret = %d\n", ret);

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

      remain_len = params->recv_len;

      while(remain_len > 0)
        {
          if (params->use_select)
            {
              FD_ZERO(&readset);
              FD_SET(clsockfd, &readset);

              ret = select((clsockfd + 1), &readset, NULL, NULL, NULL);
              if (ret < 0)
                {
                  printf("select error:%d\n", errno);
                  goto errout_close;
                }
              printf("select success: ret = %d\n", ret);

              if (!FD_ISSET(clsockfd, &readset))
                {
                  printf("Unexpected FD_ISSET\n");
                  goto errout_close;
                }
            }

          recv_len = (remain_len < MAX_XFER_LEN) ? remain_len : MAX_XFER_LEN;

          ret = recv(clsockfd, (FAR void*)&params->recv_buf[params->recv_len - remain_len],
                     recv_len, 0);
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
          rsize += ret;
          remain_len -= ret;
        }

      for (i = 0; i < rsize; i++)
        {
          printf("%02x ", params->recv_buf[i]);
        }
      printf("\n");

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
      remain_len = params->recv_len;

      while(remain_len > 0)
        {
          if (params->use_select)
            {
              FD_ZERO(&readset);
              FD_SET(sockfd, &readset);

              ret = select((sockfd + 1), &readset, NULL, NULL, NULL);
              if (ret < 0)
                {
                  printf("select error:%d\n", errno);
                  goto errout_close;
                }
              printf("select success: ret = %d\n", ret);

              if (!FD_ISSET(sockfd, &readset))
                {
                  printf("Unexpected FD_ISSET\n");
                  goto errout_close;
                }
            }

          recv_len = (remain_len < MAX_XFER_LEN) ? remain_len : MAX_XFER_LEN;

          ret = recvfrom(sockfd, (FAR void*)&params->recv_buf[params->recv_len - remain_len],
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
          rsize += ret;
          remain_len -= ret;
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
 * Name: client_test
 ****************************************************************************/

static int client_test(FAR struct client_param_s *params)
{
  int                  ret;
  int                  sockfd = -1;
  int                  family = AF_INET;
  int                  socktype = SOCK_STREAM;
  int                  protocol = 0;
  int                  port_no = atoi(params->port);
  struct addrinfo      hints;
  FAR struct addrinfo *ainfo = NULL;
  struct sockaddr_in   dst_addr4;
  struct sockaddr_in6  dst_addr6;
  FAR struct sockaddr *addr;
  socklen_t            addrlen;
  struct in_addr       addr4;
  struct in6_addr      addr6;
  bool                 use_ipv6 = false;
  size_t               send_len;
  size_t               remain_len;

  show_client_test_option(params);

  if (params->use_getaddrinfo)
    {
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

      ret = getaddrinfo(params->host, params->port, &hints, &ainfo);
      if (ret != 0)
        {
          printf("getaddrinfo error = %d\n",ret);
          return -1;
        }
      family   = ainfo->ai_family;
      socktype = ainfo->ai_socktype;
      protocol = ainfo->ai_protocol;
      addr     = ainfo->ai_addr;
      addrlen  = ainfo->ai_addrlen;
    }
  else
    {
      if(inet_pton(AF_INET, params->host, &addr4))
        {
          use_ipv6 = false;
        }
      else if (inet_pton(AF_INET6, params->host, &addr6))
        {
          use_ipv6 = true;
        }
      else
        {
          printf("invalid host address = %s\n", params->host);
          return -1;
        }

      if (use_ipv6)
        {
          family = AF_INET6;
        }
      if (!params->use_tcp)
        {
          socktype = SOCK_DGRAM;
        }

      if (use_ipv6)
        {
          memset(&dst_addr6, 0, sizeof(struct sockaddr_in6));
          dst_addr6.sin6_family = AF_INET6;
          dst_addr6.sin6_port   = htons(port_no);
          memcpy(&dst_addr6.sin6_addr, &addr6, sizeof(struct in6_addr));

          addr     = (struct sockaddr*)&dst_addr6;
          addrlen  = sizeof(struct sockaddr_in6);
        }
      else
        {
          memset(&dst_addr4, 0, sizeof(struct sockaddr_in));
          dst_addr4.sin_family = AF_INET;
          dst_addr4.sin_port   = htons(port_no);
          memcpy(&dst_addr4.sin_addr, &addr4, sizeof(struct in_addr));

          addr     = (struct sockaddr*)&dst_addr4;
          addrlen  = sizeof(struct sockaddr_in);
        }
    }

  sockfd = socket(family, socktype, protocol);
  if (sockfd < 0)
    {
      printf("socket(%d, %d, %d) error:%d\n",
             family, socktype, protocol, errno);
      goto errout_free;
    }
  printf("socket success: sockfd = %d\n", sockfd);

  if (params->use_tcp)
    {
      ret = connect(sockfd, addr, addrlen);
      if (ret < 0)
        {
          printf("conncet error:%d\n", errno);
          goto errout_close;
        }
      printf("connect success: ret = %d\n", ret);

      remain_len = params->send_len;

      while(remain_len > 0)
        {
          send_len = (remain_len < MAX_XFER_LEN) ? remain_len : MAX_XFER_LEN;

          ret = send(sockfd, (FAR const void*)&params->send_buf[params->send_len - remain_len],
                     send_len, 0);
          if (ret < 0)
            {
              printf("send error:%d\n", errno);
              goto errout_close;
            }
          printf("send success: ret = %d\n", ret);
          remain_len -= ret;
        }
    }
  else
    {
      remain_len = params->send_len;

      while(remain_len > 0)
        {
          send_len = (remain_len < MAX_XFER_LEN) ? remain_len : MAX_XFER_LEN;

          ret = sendto(sockfd, (FAR const void*)&params->send_buf[params->send_len - remain_len],
                       send_len, 0, addr, addrlen);
          if (ret < 0)
            {
              printf("sendto error:%d\n", errno);
              goto errout_close;
            }
          printf("sendto success: ret = %d\n", ret);
          remain_len -= ret;
        }
    }

  ret = close(sockfd);
  if (ret < 0)
  {
    printf("close error:%d\n", errno);
    return -1;
  }

  return 0;

errout_close:
  close(sockfd);

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
int protocoltest_main(int argc, char *argv[])
#  endif
#endif
{
  int   ret;
  int   i;
  int   option;
  char *host            = NULL;
  char *port            = "80";  /* default port */
  bool  use_ipv6        = false;
  bool  use_tcp         = true;
  bool  use_getaddrinfo = false;
  bool  use_select      = false;
  int   use_length      = 4096;

  /* Parse input parameters */

  while ((option = getopt(argc, argv, ":c:p:ugs6hl:")) != -1)
    {
      switch (option)
        {
          case 'c':
            host = optarg;
            break;

          case 'p':
            port = optarg;
            break;

          case 'u':
            use_tcp = false;
            break;

          case 'g':
            use_getaddrinfo = true;
            break;

          case 's':
            use_select = true;
            break;

          case '6':
            use_ipv6 = true;
            break;

          case 'h':
            show_usage(argv[0], EXIT_SUCCESS);
            break;

          case 'l':
            use_length = atoi(optarg);
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

  /* Start socket test */

  if (host)
    {
      /* do client test */

      struct client_param_s cparams;
      memset(&cparams, 0, sizeof(struct client_param_s));

      cparams.use_getaddrinfo = use_getaddrinfo;
      cparams.use_tcp         = use_tcp;

      cparams.host     = host;
      cparams.port     = port;
      cparams.send_len = use_length;
      cparams.send_buf = (char *)malloc(cparams.send_len);
      if (cparams.send_buf == NULL)
        {
          printf("Failed to allocate send buffer\n");
          return -1;
        }
      for (i = 0; i < cparams.send_len; i++)
        {
          cparams.send_buf[i] = (char)i;
        }

      ret = client_test(&cparams);
      if (ret != 0)
        {
          printf("Failed to client test\n");
        }

      if (cparams.send_buf)
        {
          free(cparams.send_buf);
        }
    }
  else
    {
      /* do server test */

      struct server_param_s sparams;
      memset(&sparams, 0, sizeof(struct server_param_s));

      sparams.use_getaddrinfo = use_getaddrinfo;
      sparams.use_ipv6        = use_ipv6;
      sparams.use_tcp         = use_tcp;
      sparams.use_select      = use_select;

      sparams.port     = port;
      sparams.recv_len = use_length;
      sparams.recv_buf = (char *)malloc(sparams.recv_len);
      if (sparams.recv_buf == NULL)
        {
          printf("Failed to allocate recv buffer\n");
          return -1;
        }

      ret = server_test(&sparams);
      if (ret != 0)
        {
          printf("Failed to server test\n");
        }

      if (sparams.recv_buf)
        {
          free(sparams.recv_buf);
        }
    }

  return 0;
}
