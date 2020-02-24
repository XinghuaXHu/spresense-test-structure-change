/****************************************************************************
 * test/lte_socket/protocol/apitest_host.c
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

#define TRANS_SIZE 20
#define SEND_SIZE TRANS_SIZE
#define RECV_SIZE TRANS_SIZE

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int                  g_sock_fd = -1;
static int                  g_acpt_fd = -1;
static struct sockaddr  *g_addr = NULL;
static socklen_t            g_addrlen = 0;
static struct sockaddr_in   g_addr4;
static char                 g_buf[TRANS_SIZE] = {0};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int host_close(void)
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

  if (g_addr)
    {
      g_addr = NULL;
    }

  return 0;
}

static int host_socket(int f, int t, int p)
{
  if (g_sock_fd != -1) return 0;

  g_sock_fd = socket(f, t, p);
  return g_sock_fd;
}

static int host_accept(char *port_no)
{
  int                ret;
  struct sockaddr    *addr;
  socklen_t          addrlen;
  struct sockaddr    *cli_addr;
  socklen_t          cli_addrlen;
  struct sockaddr_in src_addr4;
  struct sockaddr_in cli_addr4;
  int                val = 1;

  memset(&src_addr4, 0, sizeof(struct sockaddr_in));
  src_addr4.sin_family      = AF_INET;
  src_addr4.sin_port        = htons(atoi(port_no));
  src_addr4.sin_addr.s_addr = htonl(INADDR_ANY);

  addr     = (struct sockaddr*)&src_addr4;
  addrlen  = sizeof(struct sockaddr_in);

  memset(&cli_addr4, 0, sizeof(struct sockaddr_in));

  cli_addr     = (struct sockaddr*)&cli_addr4;
  cli_addrlen  = sizeof(struct sockaddr_in);

  ret = host_socket(AF_INET, SOCK_STREAM, 0);
  if (ret < 0)
    {
      printf("socket err %d\n", errno);
      return -1;
    }

  ret = setsockopt(g_sock_fd, SOL_SOCKET, SO_REUSEADDR,
                   (const int *)&val, sizeof(val));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
    }
  printf("setsockopt(%d, %d, %d, %d) success: ret = %d\n",
         g_sock_fd, SOL_SOCKET, SO_REUSEADDR, val, ret);

  ret = bind(g_sock_fd, addr, addrlen);
  if (ret < 0)
    {
      printf("bind error:%d\n", errno);
      return -1;
    }

  ret = listen(g_sock_fd, 1);
  if (ret < 0)
    {
      printf("listen error:%d\n", errno);
      return -1;
    }
  printf("listen success: ret = %d\n", ret);

  g_acpt_fd = accept(g_sock_fd, cli_addr, &cli_addrlen);
  if (g_acpt_fd < 0)
    {
      printf("accept error:%d\n", g_acpt_fd);
      return -1;
    }

  printf("accept success\n");
  return 0;
}

static int host_connect(char *host, char *port)
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
  g_sock_fd = socket(family, socktype, protocol);

  if (g_sock_fd < 0)
    {
      printf("socket(%d, %d, %d) error:%d\n",
             family, socktype, protocol, errno);
      return -1;
    }

  ret = connect(g_sock_fd, addr, addrlen);
  if (ret < 0)
    {
      printf("connect error %d\n", errno);
      return -1;
    }

  printf("connect success\n");
  return 0;
}

static int host_send(void)
{
  int ret = -1;
  ret = send(g_sock_fd, g_buf, SEND_SIZE, 0);

  printf("send ret %d\n", ret);
  return ret;
}

static int host_sendto(char *host, char *port_no)
{
  int             ret;
  int             sockfd = -1;
  int             port = atoi(port_no);
  struct in_addr  addr4;
  static struct sockaddr_in addr;


  if(inet_pton(AF_INET, host, &addr4))
    {
      memset(&addr, 0, sizeof(struct sockaddr_in));
      addr.sin_family = AF_INET;
      addr.sin_port   = htons(port);
      memcpy(&addr.sin_addr, &addr4, sizeof(struct sockaddr_in));
    }
  else
    {
      // error
      return -1;
    }

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    {
      printf("socket() error: %d\n", errno);
      return -1;
    }

  printf("setup success\n");

  ret = sendto(sockfd, g_buf, SEND_SIZE, 0, &addr, sizeof(struct sockaddr_in));
  printf("sendto ret %d\n", ret);

  return ret;
}

static int host_recv(void)
{
  int ret = -1;
  ret = recv(g_acpt_fd, g_buf, RECV_SIZE, 0);

  printf("recv ret %d\n", ret);
  return ret;
}

static int host_recvfrom(char *host, char *port_no)
{
  int                ret;
  struct sockaddr    *addr;
  socklen_t          addrlen;
  struct sockaddr    *cli_addr;
  socklen_t          cli_addrlen;
  struct sockaddr_in src_addr4;
  struct sockaddr_in cli_addr4;
  int                val = 1;

  memset(&src_addr4, 0, sizeof(struct sockaddr_in));
  src_addr4.sin_family      = AF_INET;
  src_addr4.sin_port        = htons(atoi(port_no));
  src_addr4.sin_addr.s_addr = htonl(INADDR_ANY);

  addr     = (struct sockaddr*)&src_addr4;
  addrlen  = sizeof(struct sockaddr_in);

  memset(&cli_addr4, 0, sizeof(struct sockaddr_in));

  cli_addr     = (struct sockaddr*)&cli_addr4;
  cli_addrlen  = sizeof(struct sockaddr_in);

  g_sock_fd = host_socket(AF_INET, SOCK_DGRAM, 0);
  if (ret < 0)
    {
      printf("socket err %d\n", errno);
      return -1;
    }

  ret = setsockopt(g_sock_fd, SOL_SOCKET, SO_REUSEADDR,
                   (const int *)&val, sizeof(val));
  if (ret < 0)
    {
      printf("setsockopt error:%d\n", errno);
    }
  printf("setsockopt(%d, %d, %d, %d) success: ret = %d\n",
         g_sock_fd, SOL_SOCKET, SO_REUSEADDR, val, ret);

  ret = bind(g_sock_fd, addr, addrlen);
  if (ret < 0)
    {
      printf("bind error:%d\n", errno);
      return -1;
    }

  printf("setup success\n");

  ret = recvfrom(sockfd, g_buf, RECV_SIZE, 0, &addr, sizeof(struct sockaddr_in));
  printf("sendto ret %d\n", ret);

  return ret;
}

/****************************************************************************
 * Name: show_usage
 ****************************************************************************/

static void show_usage(const char *progname, int errcode)
{
  fprintf(stderr, "USAGE: %s [OPTIONS] \n", progname);
  fprintf(stderr, "\nWhere:\n");
  //fprintf(stderr, "\t<api> is the target api for test\n");
  fprintf(stderr, "\nOPTIONS include the following:\n");
  fprintf(stderr, "\t-c <host>: Client option\n");
  fprintf(stderr, "\t-p <port>: port number\n");
  fprintf(stderr, "\t-h: Show this text and exit\n");
  exit(errcode);
}

static void show_usage_command(void)
{
  //fprintf(stderr, "USAGE: %s [OPTIONS] \n", progname);
  fprintf(stderr, "\nWhere:\n");
  //fprintf(stderr, "\t<api> is the target api for test\n");
  fprintf(stderr, "\naccept: Wait accept to open and binding socket.\n");
  fprintf(stderr, "\tconnect: Try connect to open socket.\n");
  fprintf(stderr, "\tsend: Only send. Use after connect.\n");
  fprintf(stderr, "\trecv: Only recv. Use after accept.\n");
  fprintf(stderr, "\tsendto: Sendto for input addr.\n");
  fprintf(stderr, "\trecvfrom: Wait recv for input addr binding socket.\n");
  fprintf(stderr, "\tclose: Close sockets.\n");
  fprintf(stderr, "\tend: Close sockets and end Application.\n");
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * main
 ****************************************************************************/

int main(int argc, char *argv[])
{
  int option;
  int port   = -1;
  int no = -1;
  char *port_str = NULL;
  char api[50];
  char *host = NULL;
  char *host_v6 = NULL;

  /* Parse input parameters */
  

  while ((option = getopt(argc, argv, ":c:p:h")) != -1)
    {
      switch (option)
        {
          case 'c':
            host = optarg;
            break;

          case 'p':
            port_str = optarg;
            port = atoi(optarg);
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

  while(1)
    {
      /*
      if (optind >= argc)
        {
          printf("ERROR: Missing required 'api' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      */

      printf("input api\n");
      //api = argv[optind++];
      scanf("%50s", api);
      /* accept */

      if (0 == strncmp(api, "accept", strlen(api)))
        {
          if (port == -1)
            {
              printf("ERROR: Missing required '-p <port>' argument\n");
              show_usage(argv[0], EXIT_FAILURE);
            }
          host_accept(port_str);
        }

      /* connect */

      else if (0 == strncmp(api, "connect", strlen(api)))
        {
          if (port == -1)
            {
              printf("ERROR: Missing required '-p <port>' argument\n");
              show_usage(argv[0], EXIT_FAILURE);
            }
          if (host == NULL)
            {
              printf("ERROR: Missing required '-c <host>' argument\n");
              show_usage(argv[0], EXIT_FAILURE);
            }
          host_connect(host, port_str);
        }

      /* recv */

      else if (0 == strncmp(api, "recv", strlen(api)))
        {
          host_recv();
        }

      /* recvfrom */

      else if (0 == strncmp(api, "recvfrom", strlen(api)))
        {
          if (port == -1)
            {
              printf("ERROR: Missing required '-p <port>' argument\n");
              show_usage(argv[0], EXIT_FAILURE);
            }
          if (host == NULL)
            {
              printf("ERROR: Missing required '-c <host>' argument\n");
              show_usage(argv[0], EXIT_FAILURE);
            }
          host_recvfrom(host, port_str);
        }

      /* send */

      else if (0 == strncmp(api, "send", strlen(api)))
        {
          host_send();
        }

      /* sendto */

      else if (0 == strncmp(api, "sendto", strlen(api)))
        {
          if (port == -1)
            {
              printf("ERROR: Missing required '-p <port>' argument\n");
              show_usage(argv[0], EXIT_FAILURE);
            }
          if (host == NULL)
            {
              printf("ERROR: Missing required '-c <host>' argument\n");
              show_usage(argv[0], EXIT_FAILURE);
            }

          host_sendto(host, port_str);
        }

      else if (0 == strncmp(api, "close", strlen(api)))
        {
          host_close();
          printf("close success\n");
        }

      else if (0 == strncmp(api, "end", strlen(api)))
        {
          host_close();
          break;
        }

      else
        {
          printf("Check your command.\n");
        }
    }

  return 0;
}
