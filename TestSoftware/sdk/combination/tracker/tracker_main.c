/****************************************************************************
 * demo/collet_box/tracker_main.c
 *
 *   Copyright (C) 2017 Sony Corporation. All rights reserved.
 *   Author: Yutaka Miyajima <Yutaka.Miyajima@sony.com>
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
 * 3. Neither the name NuttX nor Sony nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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

#include <sdk/config.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <arch/chip/gnss.h>
//#include <netutils/httpc/http_socket.h>
#include "../httpc/include/http_socket.h"
//#include <netutils/httpc/tls_socket.h>
//#include "../httpc/include/tls_socket.h"

#include "netutils/cJSON.h"

//#include "tracker_tram.h"
#include "tracker_net_client.h"
#include "tracker_location.h"
#ifdef CONFIG_DEMO_COLLET_TRACKER_USING_NXFB

//#include "tracker_nxfb.h"

#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define DOUBLE_STR_LEN         64

#ifdef CONFIG_DEMO_COLLET_TRACKER_USING_NXFB
#  define write_display(format, ...) \
  do \
    { \
        int r; \
        r = tracker_nxfb_initialize(); \
        if (r < 0) \
          { \
            printf("nxfb initialzie error:%d,%d\n", r, errno); \
          } \
        else \
          { \
            tracker_nxfb_printf(format, ##__VA_ARGS__); \
            tracker_nxfb_flash(); \
            tracker_nxfb_terminate(); \
          } \
    } \
  while(0)
#else
#  define write_display(format, ...)
#endif /* CONFIG_DEMO_COLLET_TRACKER_USING_NXFB */

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int g_use_https;
static int g_http_header_num = 0;
static const char *g_http_headers[CONFIG_DEMO_COLLET_TRACKER_HTTP_CUSTOM_HEADER_NUM];
static struct cxd56_gnss_positiondata_s g_pos_data = {0};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void show_usage(FAR const char *progname, int errcode)
{
  fprintf(stderr, "USAGE: %s [OPTIONS] <host> <port> <path>\n", progname);
  fprintf(stderr, "\nWhere:\n");
  fprintf(stderr, "\t<host> is the destination server address\n");
  fprintf(stderr, "\t<port> is the destination port number\n");
  fprintf(stderr, "\t<path> is the file path\n");
  fprintf(stderr, "\nand OPTIONS include the following:\n");
  fprintf(stderr, "\t-s: Using HTTPS.  Default: HTTP\n");
  fprintf(stderr, "\t-H <LINE>: Pass custom header LINE to server\n");
  fprintf(stderr, "\t-h: Show this text and exit\n");
  exit(errcode);
}

static void do_tracker(char* domain, int port, char* path)
{
#if 1
  cJSON *root;
  cJSON *gnss;
  cJSON *gpstime;
  cJSON *tram;
#endif
  char *out = NULL;
  char double_str[DOUBLE_STR_LEN] = {0};
  const char *tram_data = "0000";

//  tracker_tram_init();

//  write_display("Waiting for\nConnect LTE");

  /* Connect to the LTE network */

//  lte_setup();
  if(0 != lte_start())
    {
      printf("lte_start failed.\n");
      return;
    }
//  write_display("\nLTE Connected");

//  NT_tls_socket_init();
  NT_http_init();

//  write_display("Waiting for\nfix position");

  /* Location start */

  tracker_location_start();

  for(;;)
    {
      sleep(10);

      tracker_get_location_data(&g_pos_data);
      
//      tram_data = tracker_get_tram_state();

      if (g_pos_data.receiver.pos_fixmode == CXD56_GNSS_PVT_POSFIX_INVALID)
        {
          continue;
        }

//      write_display("LAT:%.06f\nLONG:%.06f\nMODE:%s"
//                    ,g_pos_data.receiver.latitude
//                    ,g_pos_data.receiver.longitude
//                    ,tram_data);

      /* Construct data using JSON format */
#if 1
      root = cJSON_CreateObject();
      cJSON_AddStringToObject(root, "host", "spritzer");
      cJSON_AddStringToObject(root, "ts_period", "10");
      cJSON_AddStringToObject(root, "ts_unit", "ns");
      cJSON_AddItemToObject(root, "gnss", gnss = cJSON_CreateObject());
      cJSON_AddNumberToObject(gnss, "ts", g_pos_data.data_timestamp);
      memset(double_str, 0x00, DOUBLE_STR_LEN);
      snprintf(double_str, DOUBLE_STR_LEN, "%.06f", g_pos_data.receiver.latitude);
      cJSON_AddStringToObject(gnss, "lat", double_str);
      memset(double_str, 0x00, DOUBLE_STR_LEN);
      snprintf(double_str, DOUBLE_STR_LEN, "%.06f", g_pos_data.receiver.longitude);
      cJSON_AddStringToObject(gnss, "lng", double_str);
      memset(double_str, 0x00, DOUBLE_STR_LEN);
      snprintf(double_str, DOUBLE_STR_LEN, "%.06f", g_pos_data.receiver.altitude);
      cJSON_AddStringToObject(gnss, "alt", double_str);
      cJSON_AddItemToObject(gnss, "gpstime", gpstime = cJSON_CreateObject());
      cJSON_AddNumberToObject(gpstime, "year", g_pos_data.receiver.gpsdate.year);
      cJSON_AddNumberToObject(gpstime, "month", g_pos_data.receiver.gpsdate.month);
      cJSON_AddNumberToObject(gpstime, "day", g_pos_data.receiver.gpsdate.day);
      cJSON_AddNumberToObject(gpstime, "hour", g_pos_data.receiver.gpstime.hour);
      cJSON_AddNumberToObject(gpstime, "minute", g_pos_data.receiver.gpstime.minute);
      cJSON_AddNumberToObject(gpstime, "sec", g_pos_data.receiver.gpstime.sec);
      cJSON_AddNumberToObject(gpstime, "usec", g_pos_data.receiver.gpstime.usec);
      cJSON_AddItemToObject(root, "tram", tram = cJSON_CreateObject());
      cJSON_AddStringToObject(tram, "mode", tram_data);
      out = cJSON_Print(root);
#endif
      printf("%s\n", out);

      /* Send HTTP request to server using POST method */

      http_post_request(g_use_https, domain, port, path,
                        g_http_headers, g_http_header_num,
                        out, strlen(out));

      /* Delete JSON object */

      cJSON_Delete(root);
      free(out);
    }
  lte_end();
  return;
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int tracker_main(int argc, char *argv[])
#endif
{
  char *host;
  char *path;
  int port;
  int option;

  /* Parse input parameters */

  while ((option = getopt(argc, argv, "shH:")) != ERROR)
    {
      switch (option)
        {
          case 's':
            g_use_https = 1;
            break;

          case 'h':
            show_usage(argv[0], EXIT_SUCCESS);
            break;

          case 'H':
            if ((g_http_header_num + 1) >= CONFIG_DEMO_COLLET_TRACKER_HTTP_CUSTOM_HEADER_NUM)
              {
                fprintf(stderr, "ERROR: Too many headers\n");
                show_usage(argv[0], EXIT_FAILURE);
              }
            g_http_headers[g_http_header_num] = optarg;
            g_http_header_num++;
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

  if ((optind + 2) >= argc)
    {
      printf("ERROR: Missing required 'host' or 'port' or 'path' argument\n");
      show_usage(argv[0], EXIT_FAILURE);
    }

  host = argv[optind++];
  port = atoi(argv[optind++]);
  path = argv[optind++];

  /* Start tracker */

  do_tracker(host, port, path);

  return 0;
}
