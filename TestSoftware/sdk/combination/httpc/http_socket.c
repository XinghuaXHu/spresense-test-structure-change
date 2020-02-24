/****************************************************************************
 * apps/netutils/httpc/http_socket.c
 *
 *   Copyright (C) 2016 Sony Corporation. All rights reserved.
 *   Author: Mitsuo Hiragane <Mitsuo.Hiragane@sony.com>
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
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
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
#include <nuttx/config.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sdk/debug.h>

//#include "net/lwip/net_lwip/lwip/netdb.h"
#include "netdb.h"
//#include "net/lwip/net_lwip/socket.h"
#include "sys/socket.h"
#include "../httpc/include/http_socket.h"
#include "tls_internal.h"


/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define HTTP_MAX_SOCKETS 2
#define HTTP_BUF_SIZE    512

#define HTTP_BODY_CODING_NOT_CHUNK 0
#define HTTP_BODY_CODING_CHUNK     1

#ifndef CONFIG_NETUTILS_MBEDTLS
#define CONFIG_NETUTILS_MBEDTLS 0
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/
typedef enum {
  HTTP_SOCKET_STATE_CLOSED,
  HTTP_SOCKET_STATE_CREATED,
  HTTP_SOCKET_STATE_CONNECTED
} http_socket_state_t;

typedef struct {
  http_socket_state_t state;
  char *hostname;
  int is_chunk_sending;
  int is_chunked;
  int content_length_op;
  size_t bytes_remaining;
  void (*callback)(struct http_chunk_client *arg);
  int sd;
  int use_https;
} http_socket_t;

/* structure for HTTP client configuration */
typedef struct {
  bool cookie;      /* use HTTP cookie       */
  bool expect100;   /* use HTTP 100 Continue */
} http_config_t;

/* structure for HTTP cookie information */
typedef struct {
  char *hostname;
  char *data;
} http_cookie_t;

static http_socket_t g_http_sockets[HTTP_MAX_SOCKETS];
static http_config_t g_http_config;
static http_cookie_t g_http_cookie;
static char g_http_buf[HTTP_BUF_SIZE];
static int g_start_pos;
static int g_end_pos;

static char *http_socket_readline(int s);
static int http_socket_readdata(int s, char *buf, size_t len);
static int http_socket_readheader(int s);
static int http_socket_request(int post, int s, const char* path,
                               const char** headers, int num_headers,
                               int body_coding, const char *data, size_t data_len,
                               int *status_code);
static int http_save_cookie(int s, char *cookie);
static int http_set_cookie(int s, char **cookie);

/********************************************************************************
 * Public HTTP functions
 ********************************************************************************/

void NT_http_init(void)
{
  int i;
  for (i = 0; i < HTTP_MAX_SOCKETS; i++)
    {
      g_http_sockets[i].state = HTTP_SOCKET_STATE_CLOSED;
    }
  g_start_pos = 0;
  g_end_pos = 0;

  memset(&g_http_config, 0, sizeof(g_http_config));
  memset(&g_http_cookie, 0, sizeof(g_http_cookie));
}

int NT_http_create(int use_https)
{
  int i;
  int s = -1;
  http_socket_t *socket = NULL;

#if !CONFIG_NETUTILS_MBEDTLS
  if( use_https )
    {
//      dbg("mbedTLS is not configured. Check CONFIG_NETUTILS_MBEDTLS.\n");
      logdebug("mbedTLS is not configured. Check CONFIG_NETUTILS_MBEDTLS.\n");
      return -1;
    }
#endif

  for (i = 0; i < HTTP_MAX_SOCKETS; i++)
    {
      if (g_http_sockets[i].state == HTTP_SOCKET_STATE_CLOSED)
        {
          s = i;
          socket = &g_http_sockets[i];
          break;
        }
    }
  if (socket == NULL)
    {
      return -1;
    }
  socket->use_https = use_https;

  socket->state = HTTP_SOCKET_STATE_CREATED;
  socket->hostname = NULL;
  socket->is_chunked = 0;
  socket->content_length_op = 0;
  socket->bytes_remaining = 0;

  return s;
}

int NT_http_connect(int s, char *hostname, uint16_t port)
{
  http_socket_t *sockets;
  int r;
  struct addrinfo hints, *res = NULL;
  int ret = -1;

  if (s < 0 || s >= HTTP_MAX_SOCKETS || g_http_sockets[s].state != HTTP_SOCKET_STATE_CREATED)
    {
      return ret;
    }

  sockets = &g_http_sockets[s];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  r = getaddrinfo(hostname, NULL, &hints, &res);
  if (r == 0)
    {
      if (res->ai_family == AF_INET)
        {
//          ((struct sockaddr_in *)res->ai_addr)->sin_port = NT_Htons(port);
          ((struct sockaddr_in *)res->ai_addr)->sin_port = htons(port);
//          dbg("Got address from DNS %s: %s\n", (r == 1 ? "cache" : "server"), inet_ntoa(((struct sockaddr_in *)res->ai_addr)->sin_addr));
          logdebug("Got address from DNS %s: %s\n", (r == 1 ? "cache" : "server"), inet_ntoa(((struct sockaddr_in *)res->ai_addr)->sin_addr));
#if LWIP_IPV6
        }
      else if (((struct sockaddr *)res->ai_addr)->sa_family == AF_INET6)
        {
          ((struct sockaddr_in6 *)res->ai_addr)->sin6_port = NT_Htons(port);
          dbg("Got address from DNSv6 %s: %s\n", (r == 1 ? "cache" : "server"), inet6_ntoa(((struct sockaddr_in6 *)res->ai_addr)->sin6_addr));
#endif
        }
    }
  else
    {
      goto connect_end;
    }

#if 0
  int sd = sockets->use_https ? tls_socket_create(res->ai_family, res->ai_socktype, res->ai_protocol)
         : socket(res->ai_family, res->ai_socktype, res->ai_protocol);
#else
  int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
#endif
  if (sd < 0)
    {
      goto connect_end;
    }
  sockets->sd = sd;

#if 0
  r = sockets->use_https ? tls_socket_connect(sockets->sd, hostname, res->ai_addr)
    : connect(sockets->sd, res->ai_addr, res->ai_addrlen);
#else
  r = connect(sockets->sd, res->ai_addr, res->ai_addrlen);
#endif
  if (r < 0)
    {
      goto connect_end;
    }
  sockets->state = HTTP_SOCKET_STATE_CONNECTED;
  sockets->hostname = malloc(strlen(hostname) + 1);
  if(sockets->hostname != NULL)
    {
      strncpy(sockets->hostname, hostname, strlen(hostname));
      sockets->hostname[strlen(hostname)] = '\0';
    }
  ret = 0;
connect_end:
  if (res != NULL)
    {
      freeaddrinfo(res);
    }

  return ret;
}

void NT_http_close(int s)
{
  http_socket_t *socket;

  if (s < 0 || s >= HTTP_MAX_SOCKETS || g_http_sockets[s].state == HTTP_SOCKET_STATE_CLOSED)
    return;
  socket = &g_http_sockets[s];

  socket->state = HTTP_SOCKET_STATE_CLOSED;
  if (socket->hostname != NULL)
    {
      free(socket->hostname);
      socket->hostname = NULL;
    }
#if 0
  if (socket->use_https)
    {
      tls_socket_close(socket->sd);
    }
  else
#endif
    {
//      NT_Close(socket->sd);
      close(socket->sd);
    }
  socket->sd = -1;
}

int NT_http_get(int s, const char* path,
                const char** headers, int num_headers,
                int *status_code)
{
  return http_socket_request(0, s, path, headers, num_headers,
                             HTTP_BODY_CODING_NOT_CHUNK, NULL, 0,
                             status_code);
}

int NT_http_post(int s, const char* path,
                 const char** headers, int num_headers,
                 const char *data, size_t data_len,
                 int *status_code)
{
  return http_socket_request(1, s, path, headers, num_headers,
                             HTTP_BODY_CODING_NOT_CHUNK, data, data_len,
                             status_code);
}

int NT_http_post_chunk(int s, const char* path,
                       const char** headers, int num_headers,
                       const char *data, size_t data_len,
                       int *status_code)
{
  return http_socket_request(1, s, path, headers, num_headers,
                             HTTP_BODY_CODING_CHUNK, data, data_len,
                             status_code);
}

int NT_http_config(enum http_config_item_e item, bool value)
{
  switch (item)
    {
      case HTTP_CONFIG_ITEM_COOKIE:
        g_http_config.cookie = value;
        break;

      case HTTP_CONFIG_ITEM_EXPECT100:
        g_http_config.expect100 = value;
        break;

      default:
        printf("Invalid parameter: item=%d\n", item);
        return -1;
    }

  return 0;
}

int NT_http_del_cookie(char *hostname)
{
  if (hostname == NULL)
    {
      printf("NT_http_del_cookie: hostname is NULL pointer");
      return -1;
    }

  if (g_http_cookie.hostname == NULL)
    {
      /* HTTP client do not have any cookie information */
      return 0;
    }

  /* Delete cookie about IN parameter hostname */
  if (strcmp(g_http_cookie.hostname, hostname) == 0)
    {
      free(g_http_cookie.hostname);
      free(g_http_cookie.data);
      memset(&g_http_cookie, 0, sizeof(g_http_cookie));
    }

  return 0;
}

int http_read_launch(int s, struct http_chunk_client *chunk_data)
{
  int n;
  int r = -1;
  http_socket_t *socket;
  int len = chunk_data->chunk_buflen;
  char *buf = chunk_data->chunk_buf;

  socket = &g_http_sockets[s];

  if (socket->is_chunked)
    {
      while (len > 0)
        {
          if (socket->bytes_remaining > 0)
            {
              // There is data available in the current chunk
              n = MIN(len, socket->bytes_remaining);
              n = http_socket_readdata(s, buf, n);
              if (n < 0)
                {
                  return -1;
                }
              socket->bytes_remaining -= n;
              len -= n;
              buf += n;

              r = n + (r < 0 ? 0 : r);
              if (socket->bytes_remaining == 0)
                {
                  // Used up the whole chunk, read past the <CR><LF> at the end of the chunk
                  http_socket_readdata(s, NULL, 2);
                }
            }
          if (socket->bytes_remaining == 0 && len > 0)
            {
              // Need more data, so read beginning of next chunk
              int chunksize;
              char *line = http_socket_readline(s);
              if (line[0] == '\0')
                {
                  chunksize = 0;
                }
              else
                {
                  sscanf(line, "%x", &chunksize);
                  chunk_data->chunk_size = chunksize;
                }
              socket->bytes_remaining = chunksize;
              if (chunksize == 0)
                {
                  socket->is_chunked = 0;
                  if (r < 0)
                    {
                      r = 0;
                    }
                  break;
                }
            }
        }
    }
  else
    {
      while (len > 0 && socket->bytes_remaining > 0)
        {
          n = MIN(socket->bytes_remaining, len);
          r = http_socket_readdata(s, buf, n);
          if (r < 0)
            {
              return -1;
            }
          socket->bytes_remaining -= r;
          len -= r;
        }
    }
  if(r > 0 && socket->callback != NULL)
    {
      chunk_data->chunk_readlen = buf - chunk_data->chunk_buf;
      socket->callback(chunk_data);
    }

  return chunk_data->chunk_readlen;

}


int NT_http_add_listener(int s, struct http_chunk_client *chunk_data, void (*onmessage)(struct http_chunk_client *arg)){
  if (s < 0 || s >= HTTP_MAX_SOCKETS || g_http_sockets[s].state != HTTP_SOCKET_STATE_CONNECTED || onmessage == NULL)
    {
      return -1;
    }
  g_http_sockets[s].callback = onmessage;
  while(g_http_sockets[s].state == HTTP_SOCKET_STATE_CONNECTED)
    {
      http_read_launch(s, chunk_data);
    }
  return 0;
}

int NT_http_read_response(int s, char *buf, size_t len)
{
  int n;
  int r = -1;
  http_socket_t *socket;

  if (s < 0 || s >= HTTP_MAX_SOCKETS || g_http_sockets[s].state != HTTP_SOCKET_STATE_CONNECTED)
    {
      return -1;
    }

  socket = &g_http_sockets[s];
  if (socket->is_chunked)
    {
      while (len > 0)
        {
          if (socket->bytes_remaining > 0)
            {
              // There is data available in the current chunk
              n = MIN(len, socket->bytes_remaining);
              n = http_socket_readdata(s, buf, n);
              if (n < 0)
                {
                  return -1;
                }
              socket->bytes_remaining -= n;
              len -= n;
              buf += n;
              r = n + (r < 0 ? 0 : r);
              if (socket->bytes_remaining == 0)
                {
                  // Used up the whole chunk, read past the <CR><LF> at the end of the chunk
                  http_socket_readdata(s, NULL, 2);
                }
            }
          if (socket->bytes_remaining == 0 && len > 0)
            {
              // Need more data, so read beginning of next chunk
              int chunksize;
              char *line = http_socket_readline(s);
              if (line[0] == '\0')
                {
                  chunksize = 0;
                }
              else
                {
                  sscanf(line, "%x", &chunksize);
                }
              socket->bytes_remaining = chunksize;
              if (chunksize == 0)
                {
                  if (r < 0)
                    {
                      r = 0;
                    }
                  break;
                }
            }
        }
    }
  else if (socket->bytes_remaining > 0)
    {
      n = MIN(socket->bytes_remaining, len);
      r = http_socket_readdata(s, buf, n);
      if (r < 0)
        {
          return -1;
        }
      socket->bytes_remaining -= r;
    }
  else if (socket->content_length_op == 0)
    {
      /* When Neither "Content-Length" header nor "Transfer-Encoding: chunked" header exist, */
      /* HTTP server send HTTP content which has unknown length. */
      r = http_socket_readdata(s, buf, len);
      if (r < 0)
        {
          return -1;
        }
    }

  return r;
}


/********************************************************************************
 * Local HTTP functions for reading and writing data
 ********************************************************************************/

/**
 * Read len bytes of data into buf from socket s.
 */
static int http_socket_read(int s, char *buf, size_t len)
{
  http_socket_t *socket;

  if (s < 0 || s >= HTTP_MAX_SOCKETS || g_http_sockets[s].state != HTTP_SOCKET_STATE_CONNECTED)
    {
      return -1;
    }
  socket = &g_http_sockets[s];

#if 0
  return socket->use_https ? tls_socket_read(socket->sd, buf, len) : NT_Recv(socket->sd, buf, len, 0);
#else
//  return NT_Recv(socket->sd, buf, len, 0);
  return recv(socket->sd, buf, len, 0);
#endif
}

/**
 * Write len bytes of data from buf into socket s.
 */
static int http_socket_write(int s, const char *buf, size_t len)
{
  http_socket_t *socket;

  if (s < 0 || s >= HTTP_MAX_SOCKETS || g_http_sockets[s].state != HTTP_SOCKET_STATE_CONNECTED)
    {
      return -1;
    }
  socket = &g_http_sockets[s];

#if 0
  return socket->use_https ? tls_socket_write(socket->sd, buf, len) : NT_Send(socket->sd, buf, len, 0);
#else
//  return NT_Send(socket->sd, buf, len, 0);
  return send(socket->sd, buf, len, 0);
#endif
}

/**
 * Read a line of text from socket s into a static buffer.
 * Returns a pointer to the static buffer.
 * Lines are delimited by '<CR><LF>'
 */
static char *http_socket_readline(int s)
{
  if (g_start_pos >= g_end_pos)
    {
      int r = http_socket_read(s, g_http_buf, HTTP_BUF_SIZE - 1);
      if (r < 0)
        {
          return NULL;
        }
      g_start_pos = 0;
      g_end_pos = r;
      g_http_buf[r] = '\0';
    }

  char *crlf = strstr(g_http_buf + g_start_pos, "\r\n");
  if (crlf == NULL)
    {
      int len = g_end_pos - g_start_pos;
      memmove(g_http_buf, g_http_buf + g_start_pos, len);
      g_start_pos = 0;
      g_end_pos = len;
      int d = HTTP_BUF_SIZE - len - 1;
      int r = http_socket_read(s, g_http_buf + g_end_pos, d);
      if (r < 0)
        {
          return NULL;
        }
      g_end_pos += r;
      g_http_buf[g_end_pos] = '\0';
      crlf = strstr(g_http_buf, "\r\n");
    }
  if (crlf == NULL)
    {
      return NULL;
    }

  *crlf = '\0';
  char *ret = g_http_buf + g_start_pos;
  g_start_pos = crlf - g_http_buf + 2;

  return ret;
}

/**
 * Read len bytes from the socket s into buf, if buf != NULL;
 * otherwise just flush len bytes from the input.
 */
static int http_socket_readdata(int s, char *buf, size_t len)
{
  int buflen = g_end_pos - g_start_pos;
  int n = MIN(buflen, len);
  // If there are unused bytes present in g_http_buf, those bytes
  // are used first
  if (buf != NULL && n > 0)
    {
      memcpy(buf, g_http_buf + g_start_pos, n);
      buf += n;
      g_start_pos += n;
      if (g_start_pos >= g_end_pos)
        {
          g_start_pos = g_end_pos = 0;
        }
    }
  len -= n;

  if (len > 0)
    {
      if (buf != NULL) {
          int r = http_socket_read(s, buf, len);
          if (r < 0)
            {
              return -1;
            }
          n += r;
        }
      else
        {
          while (len > 0)
            {
              int m = MIN(len, HTTP_BUF_SIZE);
              int r = http_socket_read(s, g_http_buf, m);
              if (r < 0)
                {
                  return -1;
                }
              len -= r;
              n += r;
            }
        }
    }

  return n;
}

/* Wait HTTP response and read header */
static int http_socket_readheader(int s)
{
  int    status;                /* HTTP status code(return value)        */
  char   *line;                 /* pointer to each line of HTTP response */
  char   *value;                /* pointer to value of each header line  */
  size_t len;                   /* length of each header field name      */
  int    content_length_op = 0; /* Content-Length option presence        */
  size_t content_length = 0;    /* Content-Length value                  */
  int    chunked = 0;           /* chunked reception                     */

  /* Reset the input buffer */
  g_start_pos = 0;
  g_end_pos  = 0;

  /* Wait for response */
  line = http_socket_readline(s);
  if (line == NULL)
    {
       printf("Failed to read header field\n");
       return -1;
    }

  /* Skip over HTTP version field and get status code */
  value = strchr(line, ' ');
  if (value == NULL)
    {
      printf("The message format of received response is not HTTP/1.1\n");
      return -1;
    }

  status = atoi(value+1);

  /* Loop until reaching to the end of header field */
  for (;;)
    {
      /* read one line */
      line = http_socket_readline(s);
      if (line == NULL)
        {
           printf("Failed to read header field\n");
           return -1;
        }
      if (line[0] == '\0')
        {
          /* Reach to the end of header field */
          break;
        }

      /* Search the end of header field name */
      value  = strchr(line, ':');
      if (value == NULL)
        {
          printf("Invalid header field\n");
          return -1;
        }

      /* Save length of header field name */
      len = value - line;

      /* Get value of header */
      value++;

      while (value[0] == ' ' || value[0] == '\t')
        {
          /* Ignore blank or tab */
          value++;
        }

      if (strncasecmp(line, "Content-Length", len) == 0)
        {
          /* If "Content-Length" exists, save it */
          content_length_op = 1;
          content_length    = atoi(value);
        }
      else if (strncasecmp(line, "Transfer-Encoding", len) == 0)
        {
          /* If "Transfer-Encoding: chunked" exists, save it */
          if ((strncasecmp(value, "chunked", 7) == 0) &&
              (value[7] == '\0' || value[7] == ' ' || value[7] == '\t'))
            {
              chunked = 1;
            }
        }
      else if (strncasecmp(line, "Set-Cookie", len) == 0)
        {
          /* If "Set-Cookie" exists, save it */
          if (http_save_cookie(s, value) != 0)
            {
              return -1;
            }
        }
    }

  /* Update HTTP connection information */
  g_http_sockets[s].is_chunked        = chunked;
  g_http_sockets[s].content_length_op = content_length_op;
  g_http_sockets[s].bytes_remaining   = content_length;

  /* Return HTTP status code */
  return status;
}


/**
 * Send a HTTP request over the open socket, and receive the response
 */
static int http_socket_request(int post, int s, const char* path,
                               const char** headers, int num_headers,
                               int body_coding, const char *data, size_t data_len,
                               int *status_code)
{
  int i;
  if (s < 0 || s >= HTTP_MAX_SOCKETS || g_http_sockets[s].state != HTTP_SOCKET_STATE_CONNECTED)
    {
      return -1;
    }
  char *p;

  /* In case of sending chunk, skip header setting */
  if (g_http_sockets[s].is_chunk_sending == 0)
    {
      // Assemble the request, with headers, in a text buffer and send the request
      sprintf(g_http_buf, "%s %s HTTP/1.1\r\nHost: %s\r\n", post ? "POST" : "GET", path, g_http_sockets[s].hostname);
      p = g_http_buf + strlen(g_http_buf);

      for (i = 0; i < num_headers; i++)
        {
          sprintf(p, "%s\r\n", headers[i]);
          p += strlen(p);
        }

      if (data_len)
        {
          /*
           * If app request chunked transfer coding,
           * set "Transfer-Encoding: chunked" header.
           * Otherwise, set "Content-Length" header.
           */
          if (body_coding == HTTP_BODY_CODING_CHUNK)
            {
              sprintf(p, "Transfer-Encoding: chunked\r\n");
              g_http_sockets[s].is_chunk_sending = 1;
            }
          else
            {
              sprintf(p, "Content-Length: %d\r\n", data_len);
            }

          p += strlen(p);
        }

      /* Set Cookie header */
      if (http_set_cookie(s, &p) != 0)
        {
          return -1;
        }

      /* p is incremented in http_set_cookie */

      /* Set Expect: 100 continue header */
      if ((g_http_config.expect100 == true) &&
          (data_len > 0))
        {
          sprintf(p, "Expect: 100-continue\r\n");
          p += strlen(p);
        }

      strcat(g_http_buf, "\r\n");

      if (http_socket_write(s, g_http_buf, strlen(g_http_buf)) < 0)
        {
          return -1;
        }

      /*
       * Wait HTTP 100 continue response and read header
       * in "Expect: 100-continue" setting case
       */
      if ((g_http_config.expect100 == true) &&
          (data_len > 0))
        {
          *status_code = http_socket_readheader(s);
          if (*status_code < 0)
            {
              printf("Failed to receive HTTP response\n");
              return -1;
            }
          if (*status_code != 100)
            {
              /* Notify status code to app without sending HTTP body */
              return 0;
            }
        }
    }

  /* In chunked transfer coding case, send chunk_size in hex string format */
  if (body_coding == HTTP_BODY_CODING_CHUNK)
    {
      sprintf( g_http_buf, "%x\r\n", (unsigned int)data_len);

      if (http_socket_write(s, g_http_buf, strlen(g_http_buf)) < 0)
        {
          return -1;
        }
    }

  // If we have data to send with the request
  if (post && data_len > 0)
    {
      if (http_socket_write(s, data, data_len) < 0)
        {
          return -1;
        }
    }

  if (body_coding == HTTP_BODY_CODING_CHUNK)
    {
      /* In chunked transfer coding case, send \r\n */
      sprintf( g_http_buf, "\r\n");
      if (http_socket_write(s, g_http_buf, strlen(g_http_buf)) < 0)
        {
          return -1;
        }

      if (data_len > 0)
        {
          /*
           * In case of chunk sending and not last chunk,
           * not wait response in this timing
           */
          g_http_sockets[s].is_chunk_sending = 1;
          return 0;
        }
      else
        {
          /* In last chunk case */
          g_http_sockets[s].is_chunk_sending = 0;

          /* Because sending completes, wait response */
        }
    }

  /* Wait HTTP response and read header */
  *status_code = http_socket_readheader(s);
  if (*status_code < 0)
    {
      printf("Failed to receive HTTP response\n");
      return -1;
    }

  return 0;
}

/* Set Cookie header */
static int http_set_cookie(int s, char **send_buf)
{
  if ((send_buf == NULL) || (*send_buf == NULL))
    {
      printf("http_set_cookie fail: send_buf is NULL address\n");
      return -1;
    }

  if (g_http_config.cookie != true)
    {
      /* If cookie is disable, not set Cookie header */
      return 0;
    }

  /* search cookie information for destination hostname */
  if (strncmp(g_http_cookie.hostname,
              g_http_sockets[s].hostname,
              sizeof(g_http_cookie.hostname))==0)
    {
      sprintf(*send_buf, "Cookie: %s\r\n", g_http_cookie.data);
      *send_buf += strlen(*send_buf);
    }

  return 0;
}

/* Save contents of Set-Cookie header */
static int http_save_cookie(int s, char *cookie)
{
  char *l_hostname;
  char *l_cookie;

  if (g_http_config.cookie != true)
    {
      /* If cookie is disable, ignore Set-Cookie header */
      return 0;
    }

  if ((cookie == NULL) || (strlen(cookie) == 0))
    {
      printf("http_save_cookie: Invalid cookie header\n");
      return -1;
    }

  /* Get address for saving cookie information */
  l_hostname = malloc(strlen(g_http_sockets[s].hostname) + 1);
  if (l_hostname == NULL)
    {
      printf("http_save_cookie: Failed to allocate memory for hostname\n");
      return -1;
    }

  l_cookie = malloc(strlen(cookie) + 1);
  if (l_cookie == NULL)
    {
      printf("http_save_cookie: Failed to allocate memory for cookie\n");
      free(l_hostname);
      return -1;
    }

  /* Delete old cookie(Currently, this library store only last cookie) */
  if (g_http_cookie.hostname != NULL)
    {
      free(g_http_cookie.hostname);
    }

  if (g_http_cookie.data != NULL)
    {
      free(g_http_cookie.data);
    }

  memset(&g_http_cookie, 0, sizeof(g_http_cookie));

  /* Save cookie information */
  strncpy(l_hostname,
          g_http_sockets[s].hostname,
          strlen(g_http_sockets[s].hostname) + 1);
  strncpy(l_cookie, cookie, strlen(cookie) + 1);

  /* Save address */
  g_http_cookie.hostname = l_hostname;
  g_http_cookie.data     = l_cookie;

  return 0;
}

/**
 * Set timeout for http socket.
 */
int NT_http_socket_recvtimeout(int s, int timer)
{
  http_socket_t *socket;
  struct timeval tv;

  if (s < 0 || s >= HTTP_MAX_SOCKETS || g_http_sockets[s].state != HTTP_SOCKET_STATE_CONNECTED)
    {
      return -1;
    }
  socket = &g_http_sockets[s];

  tv.tv_sec = timer;  /* second Timeout */
//  NT_SetSockOpt(socket->sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  setsockopt(socket->sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  return 0;
}
