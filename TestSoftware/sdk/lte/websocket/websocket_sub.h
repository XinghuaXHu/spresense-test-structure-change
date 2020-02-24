/****************************************************************************
 * test/websocket/websocket_sub.h
 *
 *   Copyright 2018 Sony Corporation
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

#ifndef __TEST_WEBSOCKET_WEBSOCKET_SUB_H
#define __TEST_WEBSOCKET_WEBSOCKET_SUB_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sdk/config.h>

#include <stdint.h>
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define WS_TEST_NORMAL             0
#define WS_TEST_SSL                1
#define WS_TEST_PROTOCOL_ECHO      0
#define WS_TEST_PROTOCOL_CHAT      1
#define WS_TEST_PROTOCOL_NUMBER    2
#define WS_TEST_OPCODE_TEXT        1
#define WS_TEST_OPCODE_BINARY      2
#define WS_TEST_SOCKET             4
#define WS_TEST_SSL_SOCKET         2
#define WS_TEST_TEXT_DISCONNECT    "Disconnect called"
#define WS_TEST_STATUS_CLOSE       0
#define WS_TEST_STATUS_OPEN        1
#define WS_TEST_STATUS_MESSAGE     2
#define WS_TEST_STATUS_ERROR       3
#define WS_TEST_MESSAGE_SIZE       1024

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void ws_test_initialize( void );
void ws_test_finalize( void );
int ws_test_connect( int ssl, const char* cafile, const char* host, int protocol );
int ws_test_connect_second( int ssl, const char* cafile, const char* host, int protocol1, int protocol2 );
int ws_test_disconnect( int sock );
int ws_test_change_subprotocol(int sock, int number);

int ws_test_send( int sock, const char* data, uint64_t length, int code );
int ws_test_recv( int sock );

void ws_test_subprotocol_clear(int sock);
int ws_test_subprotocol_check(int sock, int status, const char* message);

#endif /* __TEST_WEBSOCKET_WEBSOCKET_SUB_H */
