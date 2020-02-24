/****************************************************************************
 * demo/collet_box/tracker/tracker_net_client.c
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
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

//#include "lte/lte_client.h"

#include "../httpc/include/http_socket.h"
//#include "netutils/httpc/tls_socket.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BUF_SIZE               2048

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static char buf[BUF_SIZE];
static char line[BUF_SIZE + 1];

static int http_socket = -1;
static struct http_chunk_client chunk_data_static;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static size_t print_lines(const char *buffer)
{
  const char *p = buffer;
  const char *s;
  size_t n;

  while(1)
    {
      s = strchr(p, '\n');
      if (s == NULL)
        {
          n = strnlen(p, BUF_SIZE);
          memcpy(line, p, n);
          line[n] = '\0';
          printf("%s", line);
          break;
        }
      n = s - p;
      memcpy(line, p, n);
      line[n] = '\0';
      p = s + 1;
      printf("%s", line);
    }
  return (p - buffer);
}

static void chunked_recv(struct http_chunk_client *chunk_data)
{
  if (chunk_data != NULL)
    {
      chunk_data->chunk_buf[chunk_data->chunk_readlen] = '\0';

      printf("%s", chunk_data->chunk_buf);
      if(chunk_data->chunk_size == 0)
        {
          NT_http_close(http_socket);
          http_socket = -1;
        }
    }
}

static void http_post(int show_content, int use_https, char *domain, int port,
  const char *path, const char** headers, int num_headers,
  const char *data, size_t data_len)
{
  int r;
  int status;
  size_t n = 0;

  if (http_socket < 0)
    {
      http_socket = NT_http_create(use_https);
      printf("HTTP socket created: %d\n", http_socket);
      if (http_socket < 0)
        {
          return;
        }

      if (NT_http_connect(http_socket, domain, port) < 0)
        {
          printf("HTTP socket connect failed\n");
          NT_http_close(http_socket);
          http_socket = -1;
          return;
        }
    }
  else
    {
      printf("HTTP running process: %d\n", http_socket);
      return;
    }

  NT_http_socket_recvtimeout(http_socket, 10);

  r = NT_http_post(http_socket, path, headers, num_headers, data, data_len, &status);
  printf("HTTP socket post: %d\n", r);
  if (r < 0)
    {
      NT_http_close(http_socket);
      http_socket = -1;
      return;
    }

  printf("Response status code = %d\n", status);
  if(show_content)
    {
      chunk_data_static.chunk_buf = buf;
      chunk_data_static.chunk_buflen = BUF_SIZE;
      NT_http_add_listener(http_socket, &chunk_data_static, chunked_recv);
    }
  else
    {
      char *p = buf;
      for (;;) {
          r = NT_http_read_response(http_socket, p, BUF_SIZE);
          if (r <= 0) {
        break;
      }

      p[r] = '\0';
      if ((status >= 200) && (status < 300)) {
        printf("Received %4d/%d bytes\n", r, n);
      } else {
        print_lines(buf);
      }
      n += r;
    }
  }
  if (show_content)
    {
      printf("Received %d bytes\n", n);
    }
  else
    {
       NT_http_close(http_socket);
       http_socket = -1;
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int http_post_request(int use_https, char *domain, int port, const char *path,
  const char** headers, int num_headers, const char *data, size_t data_len)
{
  int ret = 0;

  printf("URL = %s://%s:%d%s\n", (use_https ? "https" : "http"), domain, port, path);

  http_post(0, use_https, domain, port, path, headers, num_headers, data, data_len);

  return ret;
}
