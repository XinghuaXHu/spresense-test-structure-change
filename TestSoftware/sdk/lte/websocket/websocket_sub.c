/****************************************************************************
 * test/websocket/websocket_sub.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "client.h"
#include "websocket_sub.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int ws_status[WS_TEST_SOCKET+1];
static cwebsocket_client ws_client[WS_TEST_SOCKET+1];
static cwebsocket_subprotocol ws_protocol[WS_TEST_SOCKET+1];
static int ws_callback[WS_TEST_SOCKET+1];
static char ws_payload[WS_TEST_SOCKET+1][WS_TEST_MESSAGE_SIZE];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

int ws_test_get_sock(cwebsocket_client *client)
{
  int cnt;

  for (cnt=0; cnt<WS_TEST_SOCKET+1; cnt++)
    {
      if (ws_client[cnt].fd == client->fd)
        {
          printf("get_sock : fd=%d/sock=%d\n", client->fd, cnt);
          return cnt;
        }
    }
  return -1;
}


void ws_test_subprotocol_onopen_echo(void *websocket)
{
  cwebsocket_client *client = (cwebsocket_client *)websocket;
  int sock = ws_test_get_sock(client);

  if (sock >= 0)
    {
      ws_callback[sock] = WS_TEST_STATUS_OPEN;
      memset(ws_payload[sock], 0, WS_TEST_MESSAGE_SIZE);
    }
  printf("onopen_echo : fd=%i \n", client->fd);
}


void ws_test_subprotocol_onopen_chat(void *websocket)
{
  cwebsocket_client *client = (cwebsocket_client *)websocket;
  int sock = ws_test_get_sock(client);

  if (sock >= 0)
    {
      ws_callback[sock] = WS_TEST_STATUS_OPEN;
      memset(ws_payload[sock], 0, WS_TEST_MESSAGE_SIZE);
    }
  printf("onopen_chat : fd=%i \n", client->fd);
}


void ws_test_subprotocol_onmessage_echo(void *websocket)
{
  cwebsocket_client *client = (cwebsocket_client *)websocket;
  int sock = ws_test_get_sock(client);

  if (sock >= 0)
    {
      ws_callback[sock] = WS_TEST_STATUS_MESSAGE;
      strncat(ws_payload[sock], client->message.payload, WS_TEST_MESSAGE_SIZE);
    }
//  printf("onmessage_echo : fd=%i, opcode=%#04x, payload_len=%d, payload=%s, chunk_len=%d, chunk_pos=%d\n", client->fd, client->message.opcode, (int)(client->message.payload_len & 0xFFFF), client->message.payload, (int)(client->message.chunk_len & 0xFFFF), (int)(client->message.chunk_pos & 0xFFFF));
  printf("onmessage_echo : fd=%i, opcode=%#04x, payload_len=%d, chunk_len=%d, chunk_pos=%d\n", client->fd, client->message.opcode, (int)(client->message.payload_len & 0xFFFF), (int)(client->message.chunk_len & 0xFFFF), (int)(client->message.chunk_pos & 0xFFFF));
}


void ws_test_subprotocol_onmessage_chat(void *websocket)
{
  cwebsocket_client *client = (cwebsocket_client *)websocket;
  int sock = ws_test_get_sock(client);

  if (sock >= 0)
    {
      ws_callback[sock] = WS_TEST_STATUS_MESSAGE;
      strncat(ws_payload[sock], client->message.payload, WS_TEST_MESSAGE_SIZE);
    }
//  printf("onmessage_chat : fd=%i, opcode=%#04x, payload_len=%d, payload=%s, chunk_len=%d, chunk_pos=%d\n", client->fd, client->message.opcode, (int)(client->message.payload_len & 0xFFFF), client->message.payload, (int)(client->message.chunk_len & 0xFFFF), (int)(client->message.chunk_pos & 0xFFFF));
  printf("onmessage_chat : fd=%i, opcode=%#04x, payload_len=%d, chunk_len=%d, chunk_pos=%d\n", client->fd, client->message.opcode, (int)(client->message.payload_len & 0xFFFF), (int)(client->message.chunk_len & 0xFFFF), (int)(client->message.chunk_pos & 0xFFFF));
}


void ws_test_subprotocol_onclose_echo(void *websocket)
{
  cwebsocket_client *client = (cwebsocket_client *)websocket;
  int sock = ws_test_get_sock(client);

  if (sock >= 0)
    {
      ws_callback[sock] = WS_TEST_STATUS_CLOSE;
      memset(ws_payload[sock], 0, WS_TEST_MESSAGE_SIZE);
    }
  printf("onclose_echo : fd=%i, code=%i, reason=%s\n", client->fd, client->code, client->message.payload);
}


void ws_test_subprotocol_onclose_chat(void *websocket)
{
  cwebsocket_client *client = (cwebsocket_client *)websocket;
  int sock = ws_test_get_sock(client);

  if (sock >= 0)
    {
      ws_callback[sock] = WS_TEST_STATUS_CLOSE;
      memset(ws_payload[sock], 0, WS_TEST_MESSAGE_SIZE);
    }
  printf("onclose_chat : fd=%i, code=%i, reason=%s\n", client->fd, client->code, client->message.payload);
}


void ws_test_subprotocol_onerror_echo(void *websocket)
{
  cwebsocket_client *client = (cwebsocket_client *)websocket;
  int sock = ws_test_get_sock(client);

  if (sock >= 0)
    {
      ws_callback[sock] = WS_TEST_STATUS_ERROR;
      memset(ws_payload[sock], 0, WS_TEST_MESSAGE_SIZE);
      strncpy(ws_payload[sock], client->message.payload, WS_TEST_MESSAGE_SIZE);
    }
  printf("onerror_echo : fd=%i, message=%s\n", client->fd, client->message.payload);
}


void ws_test_subprotocol_onerror_chat(void *websocket)
{
  cwebsocket_client *client = (cwebsocket_client *)websocket;
  int sock = ws_test_get_sock(client);

  if (sock >= 0)
    {
      ws_callback[sock] = WS_TEST_STATUS_ERROR;
      memset(ws_payload[sock], 0, WS_TEST_MESSAGE_SIZE);
      strncpy(ws_payload[sock], client->message.payload, WS_TEST_MESSAGE_SIZE);
    }
  printf("onerror_chat : fd=%i, message=%s\n", client->fd, client->message.payload);
}


void ws_test_subprotocol_clear(int sock)
{
  memset(ws_payload[sock], 0, WS_TEST_MESSAGE_SIZE);
}


int ws_test_subprotocol_check(int sock, int status, const char* message)
{
  if (status != ws_callback[sock])
    {
      printf("subprotocol_check : illegal status : [%d]/[%d]\n", status, ws_callback[sock]);
      return -status;
    }
  if ((ws_callback[sock] == WS_TEST_STATUS_MESSAGE) && (strncmp(ws_payload[sock], message, WS_TEST_MESSAGE_SIZE) != 0))
    {
      printf("subprotocol_check : illegal message : [%s]/[%s]\n", message, ws_payload[sock]);
      return -ws_callback[sock];
    }

  printf("ws_test_subprotocol_check success : %d (%dbytes)\n", sock, strlen(ws_payload[sock]));
  return 0;
}


int ws_test_subprotocol_new(cwebsocket_subprotocol *protocol, int code)
{
  memset(protocol, 0, sizeof(cwebsocket_subprotocol));
  switch (code)
    {
      case WS_TEST_PROTOCOL_ECHO:
        protocol->name = "echo";
        protocol->onopen = &ws_test_subprotocol_onopen_echo;
        protocol->onmessage = &ws_test_subprotocol_onmessage_echo;
        protocol->onclose = &ws_test_subprotocol_onclose_echo;
        protocol->onerror = &ws_test_subprotocol_onerror_echo;
        break;
      case WS_TEST_PROTOCOL_CHAT:
        protocol->name = "chat";
        protocol->onopen = &ws_test_subprotocol_onopen_chat;
        protocol->onmessage = &ws_test_subprotocol_onmessage_chat;
        protocol->onclose = &ws_test_subprotocol_onclose_chat;
        protocol->onerror = &ws_test_subprotocol_onerror_chat;
        break;
      default:
        return -1;
    }
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void ws_test_initialize()
{
  int cnt;

  for (cnt=0; cnt<WS_TEST_SOCKET+1; cnt++)
    {
      ws_status[cnt] = 0;
    }
  for (cnt=0; cnt<WS_TEST_PROTOCOL_NUMBER; cnt++)
    {
      ws_callback[cnt] = WS_TEST_STATUS_CLOSE;
    }
  printf("websocket initialize\n");
}


void ws_test_finalize()
{
  int cnt;

  for (cnt=0; cnt<WS_TEST_SOCKET+1; cnt++)
    {
      if (ws_status[cnt] != 0)
        {
          ws_test_disconnect(cnt);
        }
    }
  printf("websocket finalize\n");
}


int ws_test_connect(int ssl, const char* cafile, const char* host, int protocol)
{
  int sock = -1;
  int ret;
  int cnt;

  for (cnt=0; cnt<WS_TEST_SOCKET+1; cnt++)
    {
      if (ws_status[cnt] == 0)
        {
          break;
        }
    }
  if (cnt == WS_TEST_SOCKET+1)
    {
      printf("websocket memory failed\n");
      return -1;
    }
  sock = cnt;
  printf("set socket number : %d\n", sock);

  cwebsocket_subprotocol* ws_protocols = &ws_protocol[sock];
  if (ws_test_subprotocol_new(ws_protocols, protocol) < 0)
    {
      printf("websocket protocol failed\n");
      return -1;
    }
  cwebsocket_client_init(&ws_client[sock], &ws_protocols, 1);
  ws_status[sock] = 1;
  if (ssl)
    {
      if ((ret = cwebsocket_client_ssl_init(&ws_client[sock], (char*)cafile, NULL, NULL, "")) < 0)
        {
          printf("websocket initialize failed (%d) : [%d]\n", sock, ret);
          return ret;
        }
      if (cafile)
        {
          printf("Set SSL root CA     : [%s]\n", cafile);
        }
    }

  ws_client[sock].uri = (char*)host;
  if ((ret = cwebsocket_client_connect(&ws_client[sock])) < 0)
    {
      printf("cwebsocket_client_connect failed (%d) : %d\n", sock, ret);
      return ret;
    }

  printf("websocket connected : %d\n", sock);
  return sock;
}


int ws_test_connect_second(int ssl, const char* cafile, const char* host, int protocol1, int protocol2)
{
  int sock = -1;
  int ret;
  int cnt;
  cwebsocket_subprotocol* ws_protocols[2];

  for (cnt=0; cnt<WS_TEST_SOCKET; cnt++)
    {
      if ((ws_status[cnt] == 0) && (ws_status[cnt+1] == 0))
        {
          break;
        }
    }
  if (cnt == WS_TEST_SOCKET)
    {
      printf("websocket memory failed\n");
      return -1;
    }
  sock = cnt;
  printf("set socket number : %d\n", sock);

  ws_protocols[0] = &ws_protocol[sock];
  if (ws_test_subprotocol_new(ws_protocols[0], protocol1) < 0)
    {
      printf("websocket protocol failed\n");
      return -1;
    }
  ws_protocols[1] = &ws_protocol[sock+1];
  if (ws_test_subprotocol_new(ws_protocols[1], protocol2) < 0)
    {
      printf("websocket protocol failed\n");
      return -1;
    }
  cwebsocket_client_init(&ws_client[sock], ws_protocols, 2);
  ws_status[sock] = 1;

  if (ssl)
    {
      if ((ret = cwebsocket_client_ssl_init(&ws_client[sock], (char*)cafile, NULL, NULL, "")) < 0)
        {
          printf("websocket initialize failed (%d) : [%d]\n", sock, ret);
          return ret;
        }
      if (cafile)
        {
          printf("Set SSL root CA     : [%s]\n", cafile);
        }
    }
  ws_client[sock].uri = (char*)host;
  if ((ret = cwebsocket_client_connect(&ws_client[sock])) < 0)
    {
      printf("cwebsocket_client_connect failed (%d) : %d\n", sock, ret);
      return ret;
    }

  printf("websocket connected : %d\n", sock);
  return sock;
}


int ws_test_change_subprotocol(int sock, int number)
{
  if (sock >= WS_TEST_SOCKET+1)
    {
      return -1;
    }
  if ((number < 0) || (number >= WEBSOCKET_SUBPROTOCOL_MAX))
    {
      return -1;
    }

  ws_client[sock].subprotocol = ws_client[sock].subprotocols[number];
  return 0;
}


int ws_test_disconnect(int sock)
{
  if (sock >= WS_TEST_SOCKET+1)
    {
      return -1;
    }
  if (ws_status[sock] != 0)
    {
      cwebsocket_client_close(&ws_client[sock], 1000, WS_TEST_TEXT_DISCONNECT);
      ws_status[sock] = 0;

      printf("cwebsocket_client_close : %d\n", sock);
    }

  return 0;
}


int ws_test_send(int sock, const char* data, uint64_t length, int code)
{
  int ret;

  if ((sock >= WS_TEST_SOCKET) || (ws_status[sock] == 0))
    {
      return -1;
    }
  if ((ret = cwebsocket_client_write_data(&ws_client[sock], data, length, code)) < 0)
    {
      printf("cwebsocket_client_write_data failed (%d) : %d\n", sock, ret);
      return ret;
    }

  printf("cwebsocket_client_write_data : %d\n", sock);
  return ret;
}


int ws_test_recv(int sock)
{
  int ret;

  if ((sock >= WS_TEST_SOCKET) || (ws_status[sock] == 0))
    {
      return -1;
    }
  if ((ret = cwebsocket_client_read_data(&ws_client[sock])) < 0)
    {
      printf("cwebsocket_client_read_data failed (%d) : %d\n", sock, ret);
      return ret;
    }

  printf("cwebsocket_client_read_data : %d\n", sock);
  return ret;
}



