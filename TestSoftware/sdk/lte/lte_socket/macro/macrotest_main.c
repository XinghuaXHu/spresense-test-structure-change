/****************************************************************************
 * test/lte_socket/macro/macrotest_main.c
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

#ifndef FAR
#  define FAR
#endif

#define SHOW_OPTIONS
#ifdef SHOW_OPTIONS
#  undef SHOW_OPTIONS_ONLY
#endif

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
  fprintf(stderr, "\t-h: Show this text and exit\n");
  exit(errcode);
}

/****************************************************************************
 * Name: byteoder_test
 ****************************************************************************/

static int byteoder_test(void)
{
  int ret = 0;
  unsigned int   lval = 0x12345678;
  unsigned short sval = 0x9876;

  if (lval == htonl(0x78563412))
    {
      printf("htonl success\n");
    }
  else
    {
      printf("htonl error: %d\n", htonl(0x78563412));
      ret = -1;
    }

  if (sval == htons(0x7698))
    {
      printf("htons success\n");
    }
  else
    {
      printf("htons error: %d\n", htons(0x7698));
      ret = -1;
    }

  if (lval == ntohl(0x78563412))
    {
      printf("ntohl success\n");
    }
  else
    {
      printf("ntohl error: %d\n", ntohl(0x78563412));
      ret = -1;
    }

  if (sval == ntohs(0x7698))
    {
      printf("ntohs success\n");
    }
  else
    {
      printf("ntohs error: %d\n", ntohs(0x7698));
      ret = -1;
    }

  return ret;
}

/****************************************************************************
 * Name: inet_addr_test
 ****************************************************************************/

static int inet_addr_test(void)
{
  int       ret = 0;
  in_addr_t addr;
  in_addr_t cmp_addr = 0x0100007F;

  addr = inet_addr("127.0.0.1");
  if (addr == INADDR_NONE)
    {
      printf("inet_addr error\n");
      ret = -1;
    }
  else if (cmp_addr != addr)
    {
      printf("inet_addr error: 0x%x\n", addr);
      ret = -1;
    }
  else
    {
      printf("inet_addr success: 0x%x\n", addr);
    }

  return ret;
}

/****************************************************************************
 * Name: inet_aton_test
 ****************************************************************************/

static int inet_aton_test(void)
{
  int            ret = 0;
  struct in_addr addr;
  in_addr_t      cmp_addr = 0x0100007F;

  ret = inet_aton("127.0.0.1", &addr);
  if (ret == 0)
    {
      printf("inet_aton error: %d\n", ret);
      ret = -1;
    }
  else if (cmp_addr != addr.s_addr)
    {
      printf("inet_aton error: 0x%x\n", addr.s_addr);
      ret = -1;
    }
  else
    {
      printf("inet_aton success: 0x%x\n", addr.s_addr);
    }

  return ret;
}

/****************************************************************************
 * Name: inet_ntoa_test
 ****************************************************************************/

static int inet_ntoa_test(void)
{
  int             ret = 0;
  struct in_addr  addr;
  char           *ipaddr;

  addr.s_addr = 0x0100007F;

  ipaddr = inet_ntoa(addr);
  if (strncmp("127.0.0.1", ipaddr, strlen("127.0.0.1")))
    {
      printf("inet_ntoa error: %s\n", ipaddr);
      ret = -1;
    }
  else
    {
      printf("inet_ntoa success: %s\n", ipaddr);
    }

  return ret;
}

/****************************************************************************
 * Name: inet_ntop_test
 ****************************************************************************/

static int inet_ntop_test(void)
{
  int             ret = 0;
  char            ip4addr[] = {0x7f, 0x00, 0x00, 0x01};
  char            ip6addr[] = {0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x01};
  char           *ip6addr_str = "::1";
  char            dst4[INET_ADDRSTRLEN];
  char            dst6[INET6_ADDRSTRLEN];
  FAR const char *addr;

  memset(dst4, 0, sizeof(dst4));
  addr = inet_ntop(AF_INET, (FAR const void*)ip4addr, dst4,
                   INET_ADDRSTRLEN);
  if (!addr)
    {
      printf("inet_ntop(v4) error\n");
      ret = -1;
    }
  else if (strncmp("127.0.0.1", addr, strlen("127.0.0.1")))
    {
      printf("inet_ntop(v4) error: %s\n", addr);
      ret = -1;
    }
  else
    {
      printf("inet_ntop(v4) success: %s\n", addr);
    }

  memset(dst6, 0, sizeof(dst6));
  addr = inet_ntop(AF_INET6, (FAR const void*)ip6addr, dst6,
                   INET6_ADDRSTRLEN);
  if (!addr)
    {
      printf("inet_ntop(v6) error\n");
      ret = -1;
    }
  else if (strncmp(ip6addr_str, addr, strlen(ip6addr_str)))
    {
      printf("inet_ntop(v6) error: %s\n", addr);
      ret = -1;
    }
  else
    {
      printf("inet_ntop(v6) success: %s\n", addr);
    }

  return ret;
}

/****************************************************************************
 * Name: inet_pton_test
 ****************************************************************************/

static int inet_pton_test(void)
{
  int             ret;
  int             i;
  struct in_addr  in4addr;
  struct in6_addr in6addr;
  char            ip6addr[] = {0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x01};
  char           *ip4addr_str = "127.0.0.1";
  char           *ip6addr_str = "0000:0000:0000:0000:0000:0000:0000:0001";

  ret = inet_pton(AF_INET, (FAR const void*)ip4addr_str, &in4addr.s_addr);
  if (ret < 0)
    {
      printf("inet_pton(v4) error: %d\n", errno);
      ret = -1;
    }
  else if (ret == 0)
    {
      printf("inet_pton(v4) error: %s\n", ip4addr_str);
      ret = -1;
    }
  else
    {
      if (in4addr.s_addr != 0x0100007f)
        {
          printf("inet_pton(v4) error: 0x%x\n", in4addr.s_addr);
          ret = -1;
        }
      else
        {
          printf("inet_ntop(v4) success: 0x%x\n", in4addr.s_addr);
        }
    }

  ret = inet_pton(AF_INET6, (FAR const void*)ip6addr_str, &in6addr);
  if (ret < 0)
    {
      printf("inet_pton(v6) error: %d\n", errno);
      ret = -1;
    }
  else if (ret == 0)
    {
      printf("inet_pton(v6) error: %s\n", ip6addr_str);
      ret = -1;
    }
  else
    {
      if (memcmp(&in6addr.s6_addr, ip6addr, sizeof(struct in6_addr)))
        {
          printf("inet_pton(v4) error: ");
          for (i = 0; i < sizeof(struct in6_addr); i++)
            {
              printf("0x%x ", in6addr.s6_addr[i]);
            }
          printf("\n");
          ret = -1;
        }
      else
        {
          printf("inet_pton(v4) success: ");
          for (i = 0; i < sizeof(struct in6_addr); i++)
            {
              printf("0x%x ", in6addr.s6_addr[i]);
            }
          printf("\n");
          ret = 0;
        }
    }

  return ret;
}

/****************************************************************************
 * Name: inet_xxx_test
 ****************************************************************************/

static int inet_xxx_test(void)
{
  int ret    = 0;
  int result = 0;

  ret = inet_addr_test();
  if (ret < 0)
    {
      printf("failed to inet_addr_test()\n");
      result = -1;
    }

  ret = inet_aton_test();
  if (ret < 0)
    {
      printf("failed to inet_aton_test()\n");
      result = -1;
    }

  ret = inet_ntoa_test();
  if (ret < 0)
    {
      printf("failed to inet_ntoa_test()\n");
      result = -1;
    }

  ret = inet_ntop_test();
  if (ret < 0)
    {
      printf("failed to inet_ntop_test()\n");
      result = -1;
    }

  ret = inet_pton_test();
  if (ret < 0)
    {
      printf("failed to inet_pton_test()\n");
      result = -1;
    }

  return result;
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
int macrotest_main(int argc, char *argv[])
#  endif
#endif
{
  int ret;
  int option;

  /* Parse input parameters */

  while ((option = getopt(argc, argv, "h")) != -1)
    {
      switch (option)
        {
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

  /* Start byteoder test */

  ret = byteoder_test();
  if (ret < 0)
    {
      printf("failed to byteoder_test()\n");
    }

  /* Start inet_xxx test */

  ret = inet_xxx_test();
  if (ret < 0)
    {
      printf("failed to inet_xxx_test()\n");
    }

  return ret;
}
