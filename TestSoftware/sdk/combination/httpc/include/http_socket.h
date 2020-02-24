/****************************************************************************
 * apps/include/netutils/httpc/http_socket.h
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

#ifndef netutils_httpc_http_socket_h
#define netutils_httpc_http_socket_h

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <stdbool.h>

/**
 * @ingroup lteiftop
 * @defgroup HTTPC HTTP client interface
 * HTTP client interface
 * @{
 */
/****************************************************************************
 * Public Types
 ****************************************************************************/
/**
 * \defgroup HTTP_config_item Items for HTTP client configuration
 * Items for HTTP client configuration.
 * @{
 */
enum http_config_item_e
{
  HTTP_CONFIG_ITEM_COOKIE = 0,     /**< Cookie configuration              */
  HTTP_CONFIG_ITEM_EXPECT100       /**< Expect 100 Continue configuration */
};
/**@}*/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
/**
 * \defgroup HTTP_socket_if HTTP socket interface
 * HTTP socket interface
 * \{
 */
/**
 * \brief           HTTP chunk structure
 */
typedef struct http_chunk_client{
  char *chunk_buf;   /**< Receiving buffer by application */
  int chunk_buflen;  /**< Receiving buffer size */

  int chunk_readlen; /**< Received size. */
  int chunk_size;    /**< Chunked one size. Returns 0 on chunked finish */
} http_chunk_client;
/**
 * Initialize the HTTP socket module
 */
void NT_http_init(void);

/**
 * Update HTTP client configuration.
 * @note About cookie, this client can only store one.
 *       If multiple Set-Cookie headers are set on an HTTP response message,
 *       this client only store the last one.
 * @param item  configuration item to be updated
 * @param value true means enable, false means disable. (default: false)
 * @return -1 on error, 0 otherwise.
 */
int NT_http_config(enum http_config_item_e item, bool value);

/**
 * Delete cookie.
 * @param hostname hostname of which cookie is deleted
 * @return -1 on error, 0 otherwise.
 */
int NT_http_del_cookie(char *hostname);

/**
 * Create a HTTP socket
 * @param use_https 0:not use mbedTLS(=HTTP) / 1:use mbedTLS(HTTPS)
 * @return -1 on error, and the socket number otherwise.
 */
int NT_http_create(int use_https);

/**
 * Close a HTTP socket.
 * @param s socket number to be closed(return value of NT_http_create)
 */
void NT_http_close(int s);

/**
 * Establish a HTTP socket connection to the given hostname and port number.
 * @param s socket number to be used(return value of NT_http_create)
 * @param hostname hostname to be connected
 * @param port port number to be connected
 * @return -1 on error, 0 otherwise.
 */
int NT_http_connect(int s, char *hostname, uint16_t port);

/**
 * Perform a HTTP GET request.
 * @param s socket number to be used(return value of NT_http_create)
 * @param path path to be gotten at HTTP server
 * @param headers header information string to be set in HTTP GET request
 * @param num_headers number of headers
 * @param status_code status code of response
 * @return -1 on error, 0 otherwise.
 */
int NT_http_get(int s, const char* path, const char** headers, int num_headers, int *status_code);

/**
 * Perform a HTTP POST request.
 * @param s socket number to be used(return value of NT_http_create)
 * @param path path to be posted at HTTP server
 * @param headers header information string to be set in HTTP GET request
 * @param num_headers number of headers
 * @param post_data data to be posted
 * @param post_data_len length of post_data
 * @param status_code status code of response
 * @return -1 on error, 0 otherwise.
 */
int NT_http_post(int s, const char* path, const char** headers, int num_headers,
                     const char *post_data, size_t post_data_len,
                     int *status_code);

/**
 * Perform a HTTP POST request with chunked transfer coding.
 * @param s socket number to be used(return value of NT_http_create)
 * @param path path to be posted at HTTP server
 * @param headers header information string to be set in HTTP POST request
 * @param num_headers number of headers
 * @param chunk_data data to be posted
 * @param chunk_size length of chunk_data
 * @param status_code status code of response
 * @return -1 on error, 0 otherwise.
 */
int NT_http_post_chunk(int s, const char* path,
                       const char** headers, int num_headers,
                       const char *chunk_data, size_t chunk_size,
                       int *status_code);

/**
 * Read the data returned in the response body, after a successful call to
 * http_socket_get or http_socket_post.
 * @param s socket number to be read(return value of NT_http_create)
 * @param buf buffer address that application allocate.
 * @param len length of buf
 * @return -1 on error, otherwise the number of bytes that were read.
 */
int NT_http_read_response(int s, char *buf, size_t len);

/**
 * Listen a HTTP socket.
 * Set chunk_buf and chunk_maxlen in chunk_client by application.
 * @param s socket number(return value of NT_http_create)
 * @param chunk_data chunk_data to be received. (chunk_buf and chunk_maxlen is set by application)
 * @param onmessage callback function called in receiving data.
 * @return -1 on error, otherwise the number of bytes that were read.
 */
int NT_http_add_listener(int s, struct http_chunk_client *chunk_data, void (*onmessage)(struct http_chunk_client *arg));

/**
 * Set timeout for http socket.
 * @param s socket number(return value of NT_http_create)
 * @param timer timer set(unit:second)
 * @return -1 on error, 0 otherwise.
 */
int NT_http_socket_recvtimeout(int s, int timer);
/* \} */

/**@}*/
#endif // netutils_httpc_http_socket_h
