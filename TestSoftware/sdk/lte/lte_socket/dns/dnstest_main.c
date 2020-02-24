/****************************************************************************
 * test/lte_socket/dns/dnstest_main.c
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
#include <arpa/inet.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if defined(CONFIG_NSH_MAXARGUMENTS)
#  if (CONFIG_NSH_MAXARGUMENTS < 4)
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

#define WORK_AREA_LEN 256

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct getaddrinfo_param_s
{
  int ai_socktype;
  int ai_family;
  int ai_flags;
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: show_usage
 ****************************************************************************/

static void show_usage(FAR const char *progname, int errcode)
{
  fprintf(stderr, "USAGE: %s [OPTIONS] <host>\n", progname);
  fprintf(stderr, "\nWhere:\n");
  fprintf(stderr, "\nOPTIONS include the following:\n");
  fprintf(stderr, "\t-p <port>: port number\n");
  fprintf(stderr, "\t-h: Show this text and exit\n");
  exit(errcode);
}

/****************************************************************************
 * Name: show_test_option
 ****************************************************************************/

static int show_test_option(FAR char *host, FAR char *port)
{
#ifdef SHOW_OPTIONS
  printf("options\n");
  printf("  host: %s\n", host);
  if (port)
    {
      printf("  port: %s\n", port);
    }
  else
    {
      printf("  port: NULL\n");
    }
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
  printf("ai_addr->sin_family: %d\n", ai_addr->sin_family);
  printf("ai_addr->sin_port  : %d\n", ntohs(ai_addr->sin_port));
  addr = inet_ntop(AF_INET, (FAR const void*)&ai_addr->sin_addr, dst,
                   INET_ADDRSTRLEN);
  if (addr)
    {
      printf("ai_addr->sin_addr  : %s\n", addr);
    }
  else
    {
      printf("ai_addr->sin_addr  : fail : %x\n", ai_addr->sin_addr.s_addr);
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
  printf("ai_addr->sin6_family: %d\n", ai_addr->sin6_family);
  printf("ai_addr->sin6_port  : %d\n", ntohs(ai_addr->sin6_port));
  addr = inet_ntop(AF_INET6, (FAR const void*)&ai_addr->sin6_addr, dst,
                   INET6_ADDRSTRLEN);
  if (addr)
    {
      printf("ai_addr->sin6_addr  : %s\n", addr);
    }
  else
    {
      printf("ai_addr->sin6_addr  : fail : %x\n", ai_addr->sin6_addr);
    }
}

/****************************************************************************
 * Name: show_ainfo
 ****************************************************************************/

static void show_ainfo(FAR struct addrinfo *ainfo)
{
  FAR struct addrinfo* ai = ainfo;

  while(ai)
    {
      printf("ai_flags   : %d\n", ai->ai_flags);
      printf("ai_family  : %d\n", ai->ai_family);
      printf("ai_socktype: %d\n", ai->ai_socktype);
      printf("ai_protocol: %d\n", ai->ai_protocol);
      printf("ai_addrlen : %d\n", ai->ai_addrlen);
      if (ai->ai_family == AF_INET)
        {
          show_addrin4((FAR struct sockaddr_in*)ai->ai_addr);
        }
      else
        {
          show_addrin6((FAR struct sockaddr_in6*)ai->ai_addr);
        }
      ai = ai->ai_next;
    }
}

/****************************************************************************
 * Name: show_hostent
 ****************************************************************************/

static void show_hostent(FAR struct hostent *h)
{
  int             i = 0;
  char            dst4[INET_ADDRSTRLEN];
  char            dst6[INET6_ADDRSTRLEN];
  FAR const char *addr;

  printf("h_name        : %s\n", h->h_name);
  if (h->h_aliases){
    while(h->h_aliases[i])
      {
        printf("h_aliases[%d]  : %s\n", i, h->h_aliases[i]);
        i++;
      }
  }
  printf("h_addrtype    : %d\n", h->h_addrtype);
  printf("h_length      : %d\n", h->h_length);
  i = 0;
  while(h->h_addr_list[i])
    {
      if (h->h_addrtype == AF_INET)
        {
          memset(dst4, 0, sizeof(dst4));
          addr = inet_ntop(AF_INET, (FAR const void*)h->h_addr_list[i], dst4,
                           INET_ADDRSTRLEN);
          if (addr)
            {
              printf("h_addr_list[%d]: %s\n", i, addr);
            }
          else
            {
              printf("h_addr_list[%d]: fail : %x\n",
                      i, *(int*)(h->h_addr_list[i]));
            }
        }
      else
        {
          memset(dst6, 0, sizeof(dst6));
          addr = inet_ntop(AF_INET6, (FAR const void*)h->h_addr_list[i], dst6,
                           INET6_ADDRSTRLEN);
          if (addr)
            {
              printf("h_addr_list[%d]: %s\n", i, addr);
            }
          else
            {
              printf("h_addr_list[%d]: fail : %x\n",
                      i, *(int*)(h->h_addr_list[i]));
            }
        }
      i++;
    }

}

/****************************************************************************
 * Name: do_getaddrinfo_test
 ****************************************************************************/

static int do_getaddrinfo_test(FAR char *host, FAR char *port,
                               FAR struct getaddrinfo_param_s* param)
{
  int                  ret;
  struct addrinfo      hints;
  FAR struct addrinfo *ainfo = NULL;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = param->ai_socktype;
  hints.ai_family   = param->ai_family;
  hints.ai_flags    = param->ai_flags;

  ret = getaddrinfo(host, port, &hints, &ainfo);
  if (ret != 0)
    {
      printf("getaddrinfo error = %d\n",ret);
      return -1;
    }

  show_ainfo(ainfo);

  if (ainfo)
    {
      freeaddrinfo(ainfo);
    }

  return 0;
}

/****************************************************************************
 * Name: getaddrinfo_test
 ****************************************************************************/

static int getaddrinfo_test(FAR char *host, FAR char *port)
{
  int                        ret = 0;
  int                        i;
  struct getaddrinfo_param_s test_param[] =
    {
      {SOCK_STREAM, AF_INET,   AI_PASSIVE},
      {SOCK_STREAM, AF_INET6,  AI_PASSIVE},
      {SOCK_STREAM, AF_UNSPEC, AI_PASSIVE},
      {SOCK_DGRAM,  AF_INET,   AI_PASSIVE},
      {SOCK_DGRAM,  AF_INET6,  AI_PASSIVE},
      {SOCK_DGRAM,  AF_UNSPEC, AI_PASSIVE},
    };

  for(i = 0; i < sizeof(test_param)/sizeof(struct getaddrinfo_param_s); i++)
    {
      ret |= do_getaddrinfo_test(host, port, &test_param[i]);
    }

  return ret;
}

/****************************************************************************
 * Name: gethostbyname_test
 ****************************************************************************/

static int gethostbyname_test(FAR char *host)
{
  int                 ret = -1;
  FAR struct hostent *h;

  h = gethostbyname(host);
  if (!h)
    {
      printf("gethostbyname error = %d\n", h_errno);
    }
  else
    {
      show_hostent(h);
      ret = 0;
    }

  return ret;
}

/****************************************************************************
 * Name: gethostbyname_r_test
 ****************************************************************************/

static int gethostbyname_r_test(FAR char *host)
{
  int                 ret;
  int                 herrno;
  FAR struct hostent *h;
  FAR struct hostent  h_work;
  FAR char           *buf;

  buf = (FAR char*)malloc(WORK_AREA_LEN);
  if (!buf)
    {
      printf("Failed to allocate memory\n");
      return -1;
    }

  ret = gethostbyname_r(host, &h_work, buf, WORK_AREA_LEN, &h, &herrno);
  if (ret == 0)
    {
      show_hostent(h);
    }
  else
    {
      printf("gethostbyname_r error = %d\n", herrno);
      ret = -1;
    }

  free(buf);

  return ret;
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
int dnstest_main(int argc, char *argv[])
#  endif
#endif
{
  int              ret;
  int              option;
  char            *host            = NULL;
  char            *port            = NULL;

  /* Parse input parameters */

  while ((option = getopt(argc, argv, ":p:h")) != -1)
    {
      switch (option)
        {
          case 'p':
            port = optarg;
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

  if (optind >= argc)
    {
      printf("ERROR: Missing required 'host' argument\n");
      show_usage(argv[0], EXIT_FAILURE);
    }

  host = argv[optind++];

  show_test_option(host, port);

  /* Start dns test */

  ret = getaddrinfo_test(host, port);
  if (ret < 0)
    {
      printf("failed to getaddrinfo_test()\n");
    }

  ret = gethostbyname_test(host);
  if (ret < 0)
    {
      printf("failed to getaddrinfo_test()\n");
    }

  ret = gethostbyname_r_test(host);
  if (ret < 0)
    {
      printf("failed to getaddrinfo_test()\n");
    }


  return 0;
}
