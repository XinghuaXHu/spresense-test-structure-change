/****************************************************************************
 * test/websocket/websocket_main.c
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

#include <sdk/config.h>

#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "lte_connection.h"
#include "websocket_sub.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define WSTEST_FLAG_FAIL          0       /* Need to fail test */
#define WSTEST_FLAG_SUCCESS       1       /* Need to success test */
#define WSTEST_ECHO_HOST_WS       "ws://echo.websocket.org:80/"
#define WSTEST_ECHO_HOST_WSS      "wss://echo.websocket.org:443/"
#define WSTEST_CHAT_HOST_WS       "ws://ruby-websockets-chat.herokuapp.com:80/"
#define WSTEST_CHAT_HOST_WSS      "wss://ruby-websockets-chat.herokuapp.com:443/"
#define WSTEST_MAX_ECHO_LENGTH    768
#define WSTEST_MAX_CHAT_LENGTH    512

#define WSTEST_FUNC_CONNECT       0
#define WSTEST_FUNC_ECHO          1
#define WSTEST_FUNC_PROTO         2
#define WSTEST_FUNC_CHAT          3

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int g_status =  0;
struct wstest_t
{
  char* code;
  int   func;
  int   ssl;
  int   loop;
  int   param;
  int   flag;
};
static struct wstest_t wstest_site[] = 
{
  { "03", WSTEST_FUNC_CONNECT, 0, 10, 0, WSTEST_FLAG_SUCCESS },
  { "04", WSTEST_FUNC_CONNECT, 1, 10, 0, WSTEST_FLAG_SUCCESS },
  { "10", WSTEST_FUNC_ECHO   , 0,  3, 1, WSTEST_FLAG_SUCCESS },
  { "11", WSTEST_FUNC_ECHO   , 1,  3, 1, WSTEST_FLAG_SUCCESS },
  { "12", WSTEST_FUNC_ECHO   , 0,  3, 2, WSTEST_FLAG_SUCCESS },
  { "13", WSTEST_FUNC_ECHO   , 1,  3, 2, WSTEST_FLAG_SUCCESS },
  { "14", WSTEST_FUNC_PROTO  , 0,  3, 0, WSTEST_FLAG_SUCCESS },
  { "15", WSTEST_FUNC_PROTO  , 1,  3, 0, WSTEST_FLAG_SUCCESS },
  { "20", WSTEST_FUNC_CHAT   , 0,  3, 2, WSTEST_FLAG_SUCCESS },
  { "21", WSTEST_FUNC_CHAT   , 1,  3, 2, WSTEST_FLAG_SUCCESS },
  { NULL, 0, 0, 0, 0, WSTEST_FLAG_FAIL },
};
static const int echolength[] = {
  1, 128,
  255, 256, 257,
  511, 512, 513,
  768,
  0
};
static const int chatlength[] = {
  32, 128,
  255, 256, 257,
  512,
  0
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

void random_message(char* message, int length)
{
  int cnt;
  char cx;

  time_t rtcTime = time(NULL);
  srand(rtcTime);

  for (cnt=0; cnt<length; cnt++)
    {
      do
        {
          cx = rand() & 0xff;
        } while ((cx < '0') || (cx > 'z'));
      message[cnt] = cx;
    }
  message[length] = '\0';
}


void random_chat(char* message, int length)
{
    int cnt;
    char cx;
    int pos;

    time_t rtcTime = time(NULL);
    srand(rtcTime);

    strncpy(message, "{\"handle\":\"wstest\",\"text\":\"", length);
    pos = strlen(message);

    for (cnt=pos; cnt<length-2; cnt++) {
        do {
            cx = rand() & 0xff;
        } while ((cx < 'a') || (cx > 'z'));
        message[cnt] = cx;
    }
    message[cnt] = '\0';
    strncat(message, "\"}", length);
}


int wstest_connect(int ssl, int loop)
{
  int result = 0;
  int cntt = 0;

  /* Initialize */
  ws_test_initialize();

  /* Main loop */
  for (cntt=0; cntt<loop; cntt++)
    {
      if ((result = ws_test_connect(ssl, CONFIG_TEST_WEBSOCKET_ROOTCA_ECHO,
              (ssl)?WSTEST_ECHO_HOST_WSS:WSTEST_ECHO_HOST_WS, WS_TEST_PROTOCOL_ECHO)) < 0)
        {
          break;
        }
      if ((result = ws_test_disconnect(0)) < 0)
        {
          break;
        }
    }

  /* Finalize */
  ws_test_finalize();

  return result;
}


int wstest_echo(int ssl, int loop, int ch)
{
  int result = 0;
  int cntt = 0;
  int cntc = 0;
  char randmes[WSTEST_MAX_ECHO_LENGTH+1];
  int *length;

  /* Initialize */
  ws_test_initialize();

  /* Main loop */
  for (cntt=0; cntt<loop; cntt++)
    {
      for (cntc=0; cntc<ch; cntc++)
        {
          if ((result = ws_test_connect(ssl, CONFIG_TEST_WEBSOCKET_ROOTCA_ECHO,
                  (ssl)?WSTEST_ECHO_HOST_WSS:WSTEST_ECHO_HOST_WS, WS_TEST_PROTOCOL_ECHO)) < 0)
            {
              break;
            }
        }
      if (result < 0)
        {
          break;
        }

      length = (int *)echolength;
      while (*length != 0)
        {
          for (cntc=0; cntc<ch; cntc++)
            {
              ws_test_subprotocol_clear(cntc);
              random_message(randmes, *length);

              if ((result = ws_test_send(cntc, randmes, *length, WS_TEST_OPCODE_TEXT)) < 0)
                {
                  break;
                }
              if ((result = ws_test_recv(cntc)) < 0)
                {
                  break;
                }
              if ((result = ws_test_subprotocol_check(cntc, WS_TEST_STATUS_MESSAGE, randmes)) < 0)
                {
                  break;
                }
            }
          if (result < 0)
            {
              break;
            }
          length++;
        }
      if (result < 0)
        {
          break;
        }

      for (cntc=0; cntc<ch; cntc++)
        {
          if ((result = ws_test_disconnect(cntc)) < 0)
            {
              break;
            }
        }
    }

  /* Finalize */
  ws_test_finalize();

  return result;
}


int wstest_chat(int ssl, int loop, int ch)
{
  int result = 0;
  int cntt = 0;
  int cntc = 0;
  char randmes[WSTEST_MAX_CHAT_LENGTH+1];
  int *length = (int *)chatlength;

  /* Initialize */
  ws_test_initialize();

  /* Main loop */
  for (cntt=0; cntt<loop; cntt++)
    {
      for (cntc=0; cntc<ch; cntc++)
        {
          if ((result = ws_test_connect(ssl, CONFIG_TEST_WEBSOCKET_ROOTCA_CHAT,
                  (ssl)?WSTEST_CHAT_HOST_WSS:WSTEST_CHAT_HOST_WS, WS_TEST_PROTOCOL_CHAT)) < 0)
            {
              break;
            }
        }
      if (result < 0)
        {
          break;
        }

      length = (int *)chatlength;
      while (*length != 0)
        {
          for (cntc=0; cntc<ch; cntc++)
            {
              ws_test_subprotocol_clear(cntc);
            }
          random_chat(randmes, *length);
//          printf("JSON : %s\n", randmes);

          if ((result = ws_test_send(0, randmes, *length, WS_TEST_OPCODE_TEXT)) < 0)
            {
              break;
            }
          for (cntc=0; cntc<ch; cntc++)
            {
              if ((result = ws_test_recv(cntc)) < 0)
                {
                  break;
                }
            }
          if (result < 0)
            {
              break;
            }
          for (cntc=0; cntc<2; cntc++)
            {
              if ((result = ws_test_subprotocol_check(cntc, WS_TEST_STATUS_MESSAGE, randmes)) < 0)
                {
                   break;
                }
            }
          if (result < 0)
            {
              break;
            }
          length++;
        }

      for (cntc=0; cntc<ch; cntc++)
        {
          ws_test_disconnect(cntc);
        }
      if (result < 0)
        {
            break;
        }
    }

  /* Finalize */
  ws_test_finalize();

  return result;
}


int wstest_subprotocol(int ssl, int loop)
{
  int result = 0;
  int cntt = 0;
  char randmes[WSTEST_MAX_ECHO_LENGTH+1];
  int *length;

  /* Initialize */
  ws_test_initialize();

  /* Main loop */
  for (cntt=0; cntt<loop; cntt++)
    {
      if ((result = ws_test_connect_second(ssl, CONFIG_TEST_WEBSOCKET_ROOTCA_ECHO,
             (ssl)?WSTEST_ECHO_HOST_WSS:WSTEST_ECHO_HOST_WS,
             WS_TEST_PROTOCOL_CHAT, WS_TEST_PROTOCOL_ECHO)) < 0)
        {
          break;
        }

      /* Force change to second subprotocol (chat -> echo) */
      ws_test_change_subprotocol(0, 1);

      length = (int *)echolength;
      while (*length != 0)
        {
          ws_test_subprotocol_clear(0);
          random_message(randmes, *length);

          if ((result = ws_test_send(0, randmes, *length, WS_TEST_OPCODE_TEXT)) < 0)
            {
              break;
            }
          if ((result = ws_test_recv(0)) < 0)
            {
              break;
            }
          if ((result = ws_test_subprotocol_check(0, WS_TEST_STATUS_MESSAGE, randmes)) < 0)
            {
              break;
            }
          length++;
        }
      if (result < 0)
        {
          break;
        }

      if ((result = ws_test_disconnect(0)) < 0)
        {
          break;
        }
    }

  /* Finalize */
  ws_test_finalize();

  return result;
}


/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int websocket_main(int argc, char *argv[])
#endif
{
  int ret;
  int exe = 0;
  struct wstest_t *site;

  if ((g_status == 1) && (argc > 1) && (strncmp(argv[1], "off", 3) == 0))
    {
      /* LTE finish only */
      g_status = 0;
      app_disconnect_from_lte();
      return 0;
    }

  else if (g_status == 0)
    {
      /* LTE connect */ 
      printf("Start to LTE connect\n");
      ret = app_connect_to_lte();
      if (ret < 0)
        {
          return -1;
        }
      g_status = 1;
    }

  site = &wstest_site[0];
  while (site->code)
    {
      /* Main TLS tests */
      if ((argc > 1) && (strncmp(argv[1], site->code, 2) == 0))
        {
          ret = 0;
          switch(site->func)
            {
              case WSTEST_FUNC_CONNECT:
                  ret = wstest_connect(site->ssl, site->loop);
                  exe++;
                  break;
              case WSTEST_FUNC_ECHO:
                  ret = wstest_echo(site->ssl, site->loop, site->param);
                  exe++;
                  break;
              case WSTEST_FUNC_PROTO:
                  ret = wstest_subprotocol(site->ssl, site->loop);
                  exe++;
                  break;
              case WSTEST_FUNC_CHAT:
                  ret = wstest_chat(site->ssl, site->loop, site->param);
                  exe++;
                  break;
              default:
                  break;
            }
          if ((site->flag == WSTEST_FLAG_SUCCESS) && (ret < 0))
            {
              printf("[ERR] Error : ret = %d\n", ret);
            }
          else if ((site->flag == WSTEST_FLAG_FAIL) && (ret == 0))
            {
              printf("[ERR] Illegal success\n");
            }
          else
            {
              printf("[OK] Success\n");
            }
        }
      site++;
    }
 
  if (exe == 0)
    {
      printf("Not exist test funcion : %s\n", argv[1]);
      printf("Funcion list :\n");
      printf("[Library] : 03 - 04\n");
      printf("[Echo]    : 10 - 15\n");
      printf("[Chat]    : 20 - 21\n");
    }

  return 0;
}
