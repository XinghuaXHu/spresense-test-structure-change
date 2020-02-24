/****************************************************************************
 * apps/include/netutils/httpc/tls_socket.h
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

#ifndef netutils_httpc_tls_socket_h
#define netutils_httpc_tls_socket_h

#include <stdint.h>
#include <string.h>

#include "net/lwip/net_lwip/socket.h"
#include "netutils/mbedtls/ssl.h"

/**
 * @ingroup lteiftop
 * @defgroup HTTPC HTTP client interface
 * HTTP client interface
 * @{
 */

/**
 * \defgroup TLS_socket_if TLS socket interface
 * TLS socket interface
 * \{
 */
/**
 * Initialize the TLS socket module
 */
void NT_tls_socket_init(void);

/**
 * Enable or disable the TLS session cache.
 * By default, the session cache is enabled.
 * @param enable 0:disable / 1:enable
 */
void NT_tls_socket_session_cache_enable(int enable);

/**
 * Set root CA certificate information for mbedTLS.
 * @param g_ssl_ca root CA certificate information
 */
void NT_tls_set_crt_info(mbedtls_x509_crt *g_ssl_ca);

/* \} */

/**@}*/
#endif // netutils_httpc_tls_socket_h
