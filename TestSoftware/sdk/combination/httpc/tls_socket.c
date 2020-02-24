/****************************************************************************
 * apps/netutils/httpc/tls_socket.c
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

#include "netutils/mbedtls/config.h"
#include "netutils/mbedtls/ctr_drbg.h"
#include "netutils/mbedtls/entropy.h"
#include "netutils/mbedtls/net.h"
#include "netutils/mbedtls/platform.h"
#include "netutils/mbedtls/ssl.h"

#include "netutils/httpc/tls_socket.h"
#include "tls_internal.h"
#include "rootca_certs.h"

#include "net/lwip/net_lwip/socket.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define TLS_MAX_SOCKETS       2
#define TLS_MAX_SESSIONS      3


#ifndef CONFIG_NETUTILS_HTTPC_ROOTCA_USE_FILE
#define CONFIG_NETUTILS_HTTPC_ROOTCA_USE_FILE 0
#endif

#ifndef CONFIG_NETUTILS_MBEDTLS
#define CONFIG_NETUTILS_MBEDTLS 0
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/
#if !CONFIG_NETUTILS_MBEDTLS
void
NT_tls_socket_init(void)
{
  dbg("mbedTLS is not configured. Check CONFIG_NETUTILS_MBEDTLS.\n");
  return;
}

void NT_tls_socket_session_cache_enable(int enable)
{
  dbg("mbedTLS is not configured. Check CONFIG_NETUTILS_MBEDTLS.\n");
}

int
tls_socket_create(int domain, int type, int protocol)
{
  dbg("mbedTLS is not configured. Check CONFIG_NETUTILS_MBEDTLS.\n");
  return -1;
}

void
tls_socket_close(int s)
{
  dbg("mbedTLS is not configured. Check CONFIG_NETUTILS_MBEDTLS.\n");
  return;
}

int
tls_socket_connect(int s, const char *hostname, const struct sockaddr *addr)
{
  dbg("mbedTLS is not configured. Check CONFIG_NETUTILS_MBEDTLS.\n");
  return -1;
}

int
tls_socket_read(int s, char *buf, size_t len)
{
  dbg("mbedTLS is not configured. Check CONFIG_NETUTILS_MBEDTLS.\n");
  return -1;
}

int tls_socket_write(int s, const char *buf, size_t len)
{
  dbg("mbedTLS is not configured. Check CONFIG_NETUTILS_MBEDTLS.\n");
  return -1;
}

void
NT_tls_set_crt_info(mbedtls_x509_crt * tmp_crt)
{
  dbg("mbedTLS is not configured. Check CONFIG_NETUTILS_MBEDTLS.\n");
  return;
}
#else  /* CONFIG_NETUTILS_MBEDTLS */

typedef struct {
  int tcp_socket;
  mbedtls_ssl_context *tls_context;
  mbedtls_net_context tls_net_context;
} tls_socket_t;

typedef struct {
  mbedtls_ssl_session *session;
  struct sockaddr address;
} tls_session_t;

static tls_socket_t g_tls_sockets[TLS_MAX_SOCKETS];
static tls_session_t g_tls_sessions[TLS_MAX_SESSIONS];
static int g_tls_session_cache_enabled = 1;
static int g_tls_initialized = 0;

/* Static TLS data that is common to all TLS connections */
static mbedtls_entropy_context g_entropy;
static mbedtls_ctr_drbg_context g_ctr_drbg;
static mbedtls_ssl_config g_ssl_conf;
static mbedtls_x509_crt g_ssl_ca;

static mbedtls_ssl_session *tls_session_find(const struct sockaddr *addr);
static void tls_session_update(mbedtls_ssl_context *ctx, const struct sockaddr *addr);

void
NT_tls_socket_init(void)
{
  int r;
  static const char *pers = "tls_test";
  size_t i;

  if (g_tls_initialized)
    {
      return;
    }
  // Initialize state variables
  mbedtls_ctr_drbg_init(&g_ctr_drbg);
  mbedtls_ssl_config_init(&g_ssl_conf);
  mbedtls_entropy_init(&g_entropy);
  mbedtls_x509_crt_init(&g_ssl_ca);

  // Set up for random bits generation
  if ((r = mbedtls_ctr_drbg_seed(&g_ctr_drbg, &g_entropy,
                                 (const unsigned char *)pers, strlen(pers))) != 0)
    {
      goto exit;
    }
  mbedtls_ssl_conf_rng(&g_ssl_conf, &g_ctr_drbg);

  // General configuration
  if ((r = mbedtls_ssl_config_defaults(&g_ssl_conf, MBEDTLS_SSL_IS_CLIENT,
                                       MBEDTLS_SSL_TRANSPORT_STREAM,
                                       MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
      goto exit;
    }

  // Install the root certificate(s), and set up for server authentication
#if CONFIG_NETUTILS_HTTPC_ROOTCA_USE_FILE
  mbedtls_x509_crt_parse_file( &g_ssl_ca, CONFIG_NETUTILS_HTTPC_ROOTCA_FILE );
#else
  size_t num_certs = rootca_number_of_certificates();
  for (i = 0; i < num_certs; i++) {
    if (mbedtls_x509_crt_parse_der(&g_ssl_ca, rootca_get_certificate(i), rootca_get_certificate_size(i)) != 0) {
      goto exit;
    }
  }
#endif
  mbedtls_ssl_conf_ca_chain(&g_ssl_conf, &g_ssl_ca, NULL);
  mbedtls_ssl_conf_authmode(&g_ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);

  for (i = 0; i < TLS_MAX_SOCKETS; i++)
    {
      g_tls_sockets[i].tcp_socket = -1;
      g_tls_sockets[i].tls_context = NULL;
    }
  for (i = 0; i < TLS_MAX_SESSIONS; i++)
    {
      g_tls_sessions[i].session = NULL;
    }

  g_tls_initialized = 1;

  return;

exit:
  mbedtls_ssl_config_free(&g_ssl_conf);
  mbedtls_ctr_drbg_free(&g_ctr_drbg);
  mbedtls_entropy_free(&g_entropy);
}

void NT_tls_socket_session_cache_enable(int enable)
{
  g_tls_session_cache_enabled = enable;
}

int
tls_socket_create(int domain, int type, int protocol)
{
  int i;
  for (i = 0; i < TLS_MAX_SOCKETS; i++)
    {
      tls_socket_t *socket = &g_tls_sockets[i];
      if (socket->tcp_socket < 0)
        {
          int tcp_socket = NT_Socket(domain, type, protocol);
          if (tcp_socket < 0)
            {
              return -1;
            }
          mbedtls_net_init( &socket->tls_net_context );
          socket->tcp_socket = tcp_socket;
          socket->tls_context = NULL;
          socket->tls_net_context.fd = tcp_socket;
          return i;
        }
    }

  return -1;
}

void
tls_socket_close(int s)
{
  if (s < 0 || s >= TLS_MAX_SOCKETS || g_tls_sockets[s].tcp_socket < 0)
    {
      return;
    }
  tls_socket_t *socket = &g_tls_sockets[s];

  if (socket->tls_context != NULL)
    {
      mbedtls_ssl_free(socket->tls_context);
      free(socket->tls_context);
      socket->tls_context = NULL;
      mbedtls_net_free( &socket->tls_net_context );
    }

  NT_Close(socket->tcp_socket);
  socket->tcp_socket = -1;
}

int
tls_socket_connect(int s, const char *hostname, const struct sockaddr *addr)
{
  int ret;
  if (s < 0 || s >= TLS_MAX_SOCKETS || g_tls_sockets[s].tcp_socket < 0 ||
      g_tls_sockets[s].tls_context != NULL)
    {
      return -1;
    }
  tls_socket_t *socket = &g_tls_sockets[s];

  if (NT_Connect(socket->tcp_socket, addr, addr->sa_len) < 0)
    {
      dbg("NT_Connect failed\n");
      return -1;
    }

  mbedtls_ssl_context *tls_context = calloc(1, sizeof(mbedtls_ssl_context));
  socket->tls_context = tls_context;

  mbedtls_ssl_init(tls_context);
  if (mbedtls_ssl_setup(tls_context, &g_ssl_conf) != 0)
    {
      return -1;
    }
  if (hostname != NULL)
    {
      mbedtls_ssl_set_hostname(tls_context, hostname);
    }
  mbedtls_ssl_set_bio(tls_context, &socket->tls_net_context);

  mbedtls_ssl_session *session = tls_session_find(addr);
  if (session != NULL)
    {
      mbedtls_ssl_set_session(tls_context, session);
      if (addr->sa_family == AF_INET)
        {
          dbg("TLS handshake starts, addr = %s, session = 0x%08x\n",
                 inet_ntoa(((struct sockaddr_in *)addr)->sin_addr), session);
        }
#if LWIP_IPV6
      else if(addr->sa_family == AF_INET6)
        {
          dbg("TLS handshake starts, addr = %s, session = 0x%08x\n",
                 inet6_ntoa(((struct sockaddr_in6 *)addr)->sin6_addr), session);
        }
#endif
    }

  if ((ret = mbedtls_ssl_handshake(tls_context)) != 0)
    {
      dbg("TLS handshake failed\n");
      mbedtls_printf( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret );
      return -1;
    }

  dbg("TLS handshake succeeded\n");
  tls_session_update(tls_context, addr);

  return 0;
}

int
tls_socket_read(int s, char *buf, size_t len)
{
  if (s < 0 || s >= TLS_MAX_SOCKETS || g_tls_sockets[s].tcp_socket < 0 ||
      g_tls_sockets[s].tls_context == NULL)
    {
      return -1;
    }

  return mbedtls_ssl_read(g_tls_sockets[s].tls_context, (unsigned char *)buf, len);
}

int tls_socket_write(int s, const char *buf, size_t len)
{
  if (s < 0 || s >= TLS_MAX_SOCKETS || g_tls_sockets[s].tcp_socket < 0 ||
      g_tls_sockets[s].tls_context == NULL)
    {
      return -1;
    }

  return mbedtls_ssl_write(g_tls_sockets[s].tls_context, (unsigned char *)buf, len);
}


static mbedtls_ssl_session *
tls_session_find(const struct sockaddr *addr)
{
  size_t i;
  if (g_tls_session_cache_enabled)
    {
      for (i = 0; i < TLS_MAX_SESSIONS; i++)
        {
          if (g_tls_sessions[i].session != NULL &&
              memcmp(addr, &g_tls_sessions[i].address, sizeof(struct sockaddr)) == 0)
            {
              return g_tls_sessions[i].session;
            }
        }
    }

  return NULL;
}

static void
tls_session_update(mbedtls_ssl_context *ctx, const struct sockaddr *addr)
{
  int idx = -1;
  mbedtls_ssl_session *session;
  size_t i;

  if (!g_tls_session_cache_enabled)
    return;

  // Find an empty slot or a matching entry
  for (i = 0; i < TLS_MAX_SESSIONS; i++)
    {
      if (g_tls_sessions[i].session == NULL)
        {
          if (idx < 0)
            {
              idx = i;
            }
        }
      else if (memcmp(addr, &g_tls_sessions[i].address, sizeof(struct sockaddr)) == 0)
        {
          idx = i;
          break;
        }
    }
  if (idx < 0)
    {
      // Table is full, and no entry matches. Delete the oldest entry (at index 0).
      session = g_tls_sessions[0].session;
      mbedtls_ssl_session_free(session);
      free(session);
      for (i = 0; i < TLS_MAX_SESSIONS - 1; i++)
        {
          g_tls_sessions[i] = g_tls_sessions[i + 1];
        }
      idx = TLS_MAX_SESSIONS - 1;
      g_tls_sessions[idx].session = NULL;
    }

  session = g_tls_sessions[idx].session;
  if (session == NULL)
    {
      session = calloc(1, sizeof(mbedtls_ssl_session));
    }
  else
    {
      mbedtls_ssl_session_free(session);
    }
  mbedtls_ssl_session_init(session);
  if (mbedtls_ssl_get_session(ctx, session))
    {
      mbedtls_ssl_session_free(session);
      free(session);
      g_tls_sessions[idx].session = NULL;
      return;
    }
  g_tls_sessions[idx].session = session;
  memcpy(&g_tls_sessions[idx].address, addr, sizeof(struct sockaddr));
}

void
NT_tls_set_crt_info(mbedtls_x509_crt * tmp_crt){
  mbedtls_ssl_conf_ca_chain(&g_ssl_conf, tmp_crt, NULL);
  return ;
}

#endif /* CONFIG_NETUTILS_MBEDTLS */
