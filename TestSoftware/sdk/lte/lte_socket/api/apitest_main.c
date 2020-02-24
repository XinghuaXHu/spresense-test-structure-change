/****************************************************************************
 * test/lte_socket/api/apitest_main.c
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

#include "apitest.h"


/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: show_usage
 ****************************************************************************/

static void show_usage(FAR const char *progname, int errcode)
{
  fprintf(stderr, "USAGE: %s [OPTIONS] <api>\n", progname);
  fprintf(stderr, "\nWhere:\n");
  fprintf(stderr, "\t<api> is the target api for test\n");
  fprintf(stderr, "\nOPTIONS include the following:\n");
  fprintf(stderr, "\t-c <host>: Client option\n");
  fprintf(stderr, "\t-p <port>: port number\n");
  fprintf(stderr, "\t-h: Show this text and exit\n");
  exit(errcode);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * apitest_verify_eq
 ****************************************************************************/

int apitest_verify_eq(int val, FAR struct apitest_verify_param_s *vparam)
{
  int result = -1;

  if (val == vparam->verify_val)
    {
      result = 0;
    }
  else
    {
      printf("Failed to verify EQ val: %d expected(%d)\n",
             val, vparam->verify_val);
      result = -1;
    }

  return result;
}

/****************************************************************************
 * apitest_verify_ne
 ****************************************************************************/

int apitest_verify_ne(int val, FAR struct apitest_verify_param_s *vparam)
{
  int result = -1;

  if (val != vparam->verify_val)
    {
      result = 0;
    }
  else
    {
      printf("Failed to verify NE val: %d expected(%d)\n",
             val, vparam->verify_val);
      result = -1;
    }

  return result;
}

/****************************************************************************
 * apitest_verify_lt
 ****************************************************************************/

int apitest_verify_lt(int val, FAR struct apitest_verify_param_s *vparam)
{
  int result = -1;

  if (val < vparam->verify_val)
    {
      result = 0;
    }
  else
    {
      printf("Failed to verify LT val: %d expected(%d)\n",
             val, vparam->verify_val);
      result = -1;
    }

  return result;
}

/****************************************************************************
 * apitest_verify_le
 ****************************************************************************/

int apitest_verify_le(int val, FAR struct apitest_verify_param_s *vparam)
{
  int result = -1;

  if (val <= vparam->verify_val)
    {
      result = 0;
    }
  else
    {
      printf("Failed to verify LE val: %d expected(%d)\n",
             val, vparam->verify_val);
      result = -1;
    }

  return result;
}

/****************************************************************************
 * apitest_verify_gt
 ****************************************************************************/

int apitest_verify_gt(int val, FAR struct apitest_verify_param_s *vparam)
{
  int result = -1;

  if (val > vparam->verify_val)
    {
      result = 0;
    }
  else
    {
      printf("Failed to verify GT val: %d expected(%d)\n",
             val, vparam->verify_val);
      result = -1;
    }

  return result;
}

/****************************************************************************
 * apitest_verify_ge
 ****************************************************************************/

int apitest_verify_ge(int val, FAR struct apitest_verify_param_s *vparam)
{
  int result = -1;

  if (val >= vparam->verify_val)
    {
      result = 0;
    }
  else
    {
      printf("Failed to verify GE val: %d expected(%d)\n",
             val, vparam->verify_val);
      result = -1;
    }

  return result;
}

/****************************************************************************
 * apitest_main
 ****************************************************************************/

#ifdef HOST_MAKE
int main(int argc, FAR char *argv[])
#else
#  ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#  else
int apitest_main(int argc, char *argv[])
#  endif
#endif
{
  int option;
  int port   = -1;
  int no = -1;
  char *port_str = NULL;
  char *api  = NULL;
  char *host = NULL;
  char *host_v6 = NULL;

  /* Parse input parameters */

  while ((option = getopt(argc, argv, ":c:p:6:n:h")) != -1)
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

          case '6':
            host_v6 = optarg;
            break;

          case 'n':
            no = atoi(optarg);
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

  /* There should be three final parameters remaining on the command line */

  if (optind >= argc)
    {
      printf("ERROR: Missing required 'api' argument\n");
      show_usage(argv[0], EXIT_FAILURE);
    }

  api = argv[optind++];

  /* accept */

  if (0 == strncmp(api, "accept4", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_accept_main_v4(host, port_str);
#else
      apitest_accept_main_v4(port);
#endif
    }

//  if (0 == strncmp(api, "accept6", strlen(api)))
//    {
//      if (port == -1)
//        {
//          printf("ERROR: Missing required '-p <port>' argument\n");
//          show_usage(argv[0], EXIT_FAILURE);
//        }
//#ifdef HOST_MAKE
//      if (host == NULL)
//        {
//          printf("ERROR: Missing required '-c <host>' argument\n");
//          show_usage(argv[0], EXIT_FAILURE);
//        }
//      apitest_accept_main_v6(host, port_str);
//#else
//      apitest_accept_main_v6(port);
//#endif
//    }

  else if (0 == strncmp(api, "acceptnb", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_accept_noblock(host, port_str);
#else
      apitest_accept_noblock(port);
#endif
    }

  else if (0 == strncmp(api, "acceptto", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_accept_timeout(host, port_str);
#else
      apitest_accept_timeout(port);
#endif
    }

  /* bind */

#ifndef HOST_MAKE
  else if (0 == strncmp(api, "bind", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_bind_main(port);
    }
#endif

  /* connect */

  else if (0 == strncmp(api, "connect", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifndef HOST_MAKE
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_connect_main(host, port_str);
#else
      apitest_connect_main(port);
#endif
    }

  else if (0 == strncmp(api, "connectnb", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifndef HOST_MAKE
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_connect_noblock(host, port_str);
#else
      apitest_connect_noblock(port);
#endif
    }

  else if (0 == strncmp(api, "connectto", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifndef HOST_MAKE
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_connect_timeout(host, port_str);
#else
      apitest_connect_timeout(port);
#endif
    }

  /* getsockname */

#ifndef HOST_MAKE
  else if (0 == strncmp(api, "getsn", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_getsockname_main(port);
    }
#endif

  /* getsockopt */


  else if (0 == strncmp(api, "getso", strlen(api)))
    {
#ifndef HOST_MAKE
      apitest_getsockopt_main();
#endif
    }


  /* listen */


  else if (0 == strncmp(api, "listen", strlen(api)))
    {
#ifndef HOST_MAKE
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_listen_main(port);
#endif

    }

  /* recv */

  else if (0 == strncmp(api, "recv", strlen(api)))
    {
#ifndef HOST_MAKE
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_recv_main(port);
#else
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
      apitest_recv_main(host, port_str);
#endif
    }
  else if (0 == strncmp(api, "recvnb", strlen(api)))
    {
#ifndef HOST_MAKE
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_recv_nb(port);
#else
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
      apitest_recv_nb(host, port_str);
#endif
    }
    else if (0 == strncmp(api, "recv_to", strlen(api)))
    {
#ifndef HOST_MAKE
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_recv_to(port);
#else
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
      apitest_recv_to(host, port_str);
#endif
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
      if (host_v6 == NULL)
        {
          printf("ERROR: Missing required '-6 <host_v6>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_recvfrom_main(host, host_v6, port_str);
    }

  else if (0 == strncmp(api, "recvfromnb", strlen(api)))
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
      apitest_recvfrom_nonblock(host, port_str);
    }

  else if (0 == strncmp(api, "recvfromto", strlen(api)))
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
      apitest_recvfrom_timeout(host, port_str);
    }

  /* send */

  else if (0 == strncmp(api, "send", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      apitest_send_main(port);
#else
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_send_main(host, port_str);
#endif
    }

  else if (0 == strncmp(api, "sendh", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_send_main_host(host, port_str);
#else
      apitest_send_main_host(port);
#endif
    }

  else if (0 == strncmp(api, "sendnb", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      apitest_send_nb(port);
#else
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_send_nb(host, port_str);
#endif
    }

  else if (0 == strncmp(api, "send_to", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      apitest_send_to(port);
#else
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_send_to(host, port_str);
#endif
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
      if (host_v6 == NULL)
        {
          printf("ERROR: Missing required '-6 <v6 host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_sendto_main(host, host_v6, port_str);
    }
  else if (0 == strncmp(api, "sendtonb", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
    // nop
    apitest_sendto_nb(port_str);
#else
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_sendto_noblock(host, port_str);
#endif
  }

  else if (0 == strncmp(api, "sendtoto", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      int apitest_sendto_timeout(char *port_no);
#else
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
     apitest_sendto_timeout(host, port_str);
#endif
    }

  /* setsockopt */

#ifndef HOST_MAKE
  else if (0 == strncmp(api, "setso", strlen(api)))
    {
      apitest_setsockopt_main();
    }
#endif

  /* shutdown */

  else if (0 == strncmp(api, "shutdown", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }

      apitest_shutdown_main(host, port_str);
#else
      apitest_shutdown_main(port);
#endif
    }

  /* socket */

#ifndef HOST_MAKE
  else if (0 == strncmp(api, "socket", strlen(api)))
    {
      apitest_socket_main();
    }
#endif

  /* select */

  else if (0 == strncmp(api, "select", strlen(api)))
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
      if (no == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_select_main(host, port_str, port, no);
    }

  /* close */

  else if (0 == strncmp(api, "close", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_close_main(host, port_str);
#else
      apitest_close_main(port);
#endif
    }
  /* fcntl */

  else if (0 == strncmp(api, "fcntl", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifndef HOST_MAKE
      apitest_fcntl_main(port);
#else
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }

      apitest_fcntl_main(host, port_str);
#endif
    }

  /* read */

  else if (0 == strncmp(api, "read", strlen(api)))
    {
#ifndef HOST_MAKE
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_read_main(port);
#else
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
      apitest_read_main(host, port_str);
#endif
    }
  else if (0 == strncmp(api, "readnb", strlen(api)))
    {
#ifndef HOST_MAKE
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_read_nb(port);
#else
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
      apitest_read_nb(host, port_str);
#endif
    }
    else if (0 == strncmp(api, "read_to", strlen(api)))
    {
#ifndef HOST_MAKE
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_read_to(port);
#else
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
      apitest_read_to(host, port_str);
#endif
    }

  /* write */

  else if (0 == strncmp(api, "write", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      apitest_write_main(port);
#else
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_write_main(host, port_str);
#endif
    }

  else if (0 == strncmp(api, "writenb", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      apitest_write_nb(port);
#else
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_write_nb(host, port_str);
#endif
    }

  else if (0 == strncmp(api, "write_to", strlen(api)))
    {
      if (port == -1)
        {
          printf("ERROR: Missing required '-p <port>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
#ifdef HOST_MAKE
      apitest_write_to(port);
#else
      if (host == NULL)
        {
          printf("ERROR: Missing required '-c <host>' argument\n");
          show_usage(argv[0], EXIT_FAILURE);
        }
      apitest_write_to(host, port_str);
#endif
    }

  /* fdset */

  else if (0 == strncmp(api, "fdset", strlen(api)))
    {
      //apitest_fdset_main();
    }

  return 0;
}
