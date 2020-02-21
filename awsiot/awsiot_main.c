/****************************************************************************
 * test/awsiot/awsiot_main.c
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
#include "awsiot_sub.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define AWSTEST_CLIENT_ID           "testfw"
#define AWSTEST_KEEP_ALIVE          60
#define AWSTEST_KEEP_WILL_SUB       240
#define AWSTEST_KEEP_WILL_PUB       2
#define AWSTEST_CLIENT_PUB          "testfw_pub"
#define AWSTEST_CLIENT_SUB          "testfw_sub"
#define AWSTEST_CLIENT_WILL         "testfw_will"
#define AWSTEST_TOPIC_DEF           "/testfw/MQTT/topic/DEF"
#define AWSTEST_TOPIC_WILL          "/testfw/MQTT/topic/WILL"
#define AWSTEST_TOPIC_RETAIN        "/testfw/MQTT/topic/RETAIN"
#define AWSTEST_MESSAGE_LEN         32
#define AWSTEST_QOS_DEF             0
#define AWSTEST_RETAIN_DEF          0
#define AWSTEST_DUP_DEF             0
#define AWSTEST_YIELD_DEF           2000
#define AWSTEST_WAIT_WILL           4000
#define AWSTEST_KEEP_RECONNECT      2
#define AWSTEST_WAIT_RECONNECT      10000
#define AWSTEST_JSON_LEN            256
#define AWSTEST_SHADOW_THING        "testfw_thing"
#define AWSTEST_SHADOW_GET_THING    "testfw_get_thing"
#define AWSTEST_SHADOW_DELETE_THING "testfw_delete_thing"
#define AWSTEST_CLIENT_DELTA        "testfw_det"
#define AWSTEST_CLIENT_UPDATE       "testfw_upd"
#define AWSTEST_CLIENT_GET          "testfw_get"
#define AWSTEST_CLIENT_DELETE       "testfw_del"
#define AWSTEST_YIELD_UPDATE        1000
#define AWSTEST_YIELD_DELTA         1000
#define AWSTEST_YIELD_LOOP          10000
#define AWSTEST_MQTT_SUB            0
#define AWSTEST_MQTT_PUB            1
#define AWSTEST_SHADOW_GET          0
#define AWSTEST_SHADOW_DELETE       0
#define AWSTEST_SHADOW_DELTA        0

#define AWSTEST_FLAG_FAIL           0	/* Need to fail test */
#define AWSTEST_FLAG_SUCCESS        1	/* Need to success test */
#define AWSTEST_MQTT_LIST_NUMBER    6
#define AWSTEST_SHADOW_LIST_NUMBER  10
#define AWSTEST_SHADOW_PARAM_NUMBER 5

#define AWSTEST_FUNC_MQTT_CONNECT     0
#define AWSTEST_FUNC_MQTT_RECONNECT   1
#define AWSTEST_FUNC_SHADOW_CONNECT   2
#define AWSTEST_FUNC_SHADOW_RECONNECT 3
#define AWSTEST_FUNC_QOS              4
#define AWSTEST_FUNC_WILL             5
#define AWSTEST_FUNC_WILDCARD         6
#define AWSTEST_FUNC_GET              7
#define AWSTEST_FUNC_DELETE           8
#define AWSTEST_FUNC_DELTA            9
#define AWSTEST_FUNC_WAIT            10

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const char* topiclist[AWSTEST_MQTT_LIST_NUMBER] = {
  "/testfw/MQTT/topic/DATA/number/1",
  "/testfw/MQTT/topic/DATA/number/2",
  "/testfw/MQTT/topic/DATA/number/3",
  "/testfw/MQTT/topic/DATA/test/1",
  "/testfw/MQTT/topic/DATA/test/2",
  "/testfw/MQTT/topic/DUMMY/number/2"
};
static int16_t int_list[AWSTEST_SHADOW_LIST_NUMBER+1] = {
  1, 16, -128, -256, 4096, 64, -1024, 0, 4096, 32768, 4096
};
static uint16_t uint_list[AWSTEST_SHADOW_LIST_NUMBER+1] = {
  1, 16, 128, 256, 8, 64, 128, 2048, 0, 32768, 128
};
static float float_list[AWSTEST_SHADOW_LIST_NUMBER+1] = {
  1.0f, -1.0f, 1.414f, -1.414f, 0.0f, 3.14f, 0.0f, -3.14f, 1.0e3f, 1.0e-3f, 0.0f
};
static bool bool_list[AWSTEST_SHADOW_LIST_NUMBER+1] = {
  true, false, true, false, true, false, true, false, true, false, false
};
static char string_list[AWSTEST_SHADOW_LIST_NUMBER+1][AWSIOT_TEST_SHADOW_STRING_LEN] = {
  "start", "next 2", "next 3", "option", "next 5", "next 6", "next 7", "option", "next 9", "last", "option"
};
static char* key_list[] = {
  "INT32",  "INT16",  "INT8",
  "UINT32", "UINT16", "UINT8",
  "FLOAT",  "DOUBLE", "BOOL",
  "STRING", "OBJECT"
};
static int delta_list[AWSTEST_SHADOW_PARAM_NUMBER] = {
  SHADOW_JSON_INT16, SHADOW_JSON_UINT16, SHADOW_JSON_FLOAT, SHADOW_JSON_BOOL, SHADOW_JSON_STRING
};
struct awsiotwild_t
{
  int  answer[6];
  char *topic;
};
static struct awsiotwild_t awsiottest_wild[] =
{
  { { 1, 1, 1, 0, 0, 0 }, "/testfw/MQTT/topic/DATA/number/+" },
  { { 0, 1, 0, 0, 1, 0 }, "/testfw/MQTT/topic/DATA/+/2" },
  { { 1, 1, 1, 1, 1, 0 }, "/testfw/MQTT/topic/DATA/#" },
};
static int g_status =  0;
struct awsiottest_t
{
  char* code;
  int   func;
  int   loop;
  int   param;
  int   flag;
};
static struct awsiottest_t awsiottest_site[] = 
{
  { "04", AWSTEST_FUNC_MQTT_CONNECT    , 10, 0, AWSTEST_FLAG_SUCCESS },
  { "05", AWSTEST_FUNC_SHADOW_CONNECT  , 10, 0, AWSTEST_FLAG_SUCCESS },
  { "06", AWSTEST_FUNC_MQTT_RECONNECT  , 10, 0, AWSTEST_FLAG_SUCCESS },
  { "07", AWSTEST_FUNC_SHADOW_RECONNECT, 10, 0, AWSTEST_FLAG_SUCCESS },
  { "10", AWSTEST_FUNC_QOS             ,  3, 0, AWSTEST_FLAG_SUCCESS },
  { "11", AWSTEST_FUNC_QOS             ,  3, 1, AWSTEST_FLAG_SUCCESS },
  { "12", AWSTEST_FUNC_WILL            ,  3, 0, AWSTEST_FLAG_SUCCESS },
  { "13", AWSTEST_FUNC_WILDCARD        ,  1, 0, AWSTEST_FLAG_SUCCESS },
  { "14", AWSTEST_FUNC_WILDCARD        ,  1, 1, AWSTEST_FLAG_SUCCESS },
  { "15", AWSTEST_FUNC_WILDCARD        ,  1, 2, AWSTEST_FLAG_SUCCESS },
  { "20", AWSTEST_FUNC_GET             ,  3, 0, AWSTEST_FLAG_SUCCESS },
  { "21", AWSTEST_FUNC_DELETE          ,  3, 0, AWSTEST_FLAG_SUCCESS },
  { "22", AWSTEST_FUNC_DELTA           ,  3, SHADOW_JSON_INT16 , AWSTEST_FLAG_SUCCESS },
  { "23", AWSTEST_FUNC_DELTA           ,  3, SHADOW_JSON_UINT16, AWSTEST_FLAG_SUCCESS },
  { "24", AWSTEST_FUNC_DELTA           ,  3, SHADOW_JSON_FLOAT , AWSTEST_FLAG_SUCCESS },
  { "25", AWSTEST_FUNC_DELTA           ,  3, SHADOW_JSON_BOOL  , AWSTEST_FLAG_SUCCESS },
  { "26", AWSTEST_FUNC_DELTA           ,  3, SHADOW_JSON_STRING, AWSTEST_FLAG_SUCCESS },
  { "27", AWSTEST_FUNC_WAIT            ,  0, 100000, AWSTEST_FLAG_SUCCESS },
  { NULL, 0, 0, 0, AWSTEST_FLAG_FAIL },
};


/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void random_message(char* message, int length)
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


static int awstest_mqtt_connect(int loop)
{
  int result = 0;
  int cntt = 0;

  /* Initialize */
  awsiot_test_initialize();

  /* Main loop */
  for (cntt=0; cntt<loop; cntt++)
    {
      if ((result = awsiot_test_mqtt_connect(CONFIG_TEST_AWSIOT_ROOTCA,
          CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
          CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
          AWSTEST_CLIENT_ID, AWSTEST_KEEP_ALIVE, 0, NULL, NULL)) < 0)
        {
          break;
        }

      if ((result = awsiot_test_mqtt_disconnect(0)) < 0)
        {
          break;
        }
    }

  /* Finalize */
  awsiot_test_finalize();

  return result;
}


static int awstest_mqtt_reconnect(int loop)
{
  int result = 0;
  int cntc = 0;

  /* Initialize */
  awsiot_test_initialize();

  do
    {
      if ((result = awsiot_test_mqtt_connect(CONFIG_TEST_AWSIOT_ROOTCA,
           CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
           CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
           AWSTEST_CLIENT_ID, AWSTEST_KEEP_RECONNECT, 1, NULL, NULL)) < 0)
        {
          break;
        }

      /* Main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          usleep(AWSTEST_WAIT_RECONNECT*1000);

          if ((result = awsiot_test_yield(AWSTEST_MQTT_SUB, AWSTEST_YIELD_DEF)) < 0)
            {
              break;
            }
        }

      awsiot_test_mqtt_disconnect(AWSTEST_MQTT_SUB);
    } while (0);

  /* Finalize */
  awsiot_test_finalize();

  return result;
}


static int awstest_mqtt_pubsub(int loop, int qos)
{
  int result = 0;
  int result2 = 0;
  int cntc = 0;
  char randmes[AWSTEST_MESSAGE_LEN+1];

  /* Initialize */
  awsiot_test_initialize();

  do
    {
      /* Setup subscribe */
      if ((result = awsiot_test_mqtt_connect(CONFIG_TEST_AWSIOT_ROOTCA,
           CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
           CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
           AWSTEST_CLIENT_SUB, AWSTEST_KEEP_ALIVE, 0, NULL, NULL)) < 0)
        {
          break;
        }
      if ((result = awsiot_test_mqtt_subscribe(AWSTEST_MQTT_SUB, AWSTEST_TOPIC_DEF, qos)) < 0)
        {
          break;
        }

      /* Setup publish */
      if ((result = awsiot_test_mqtt_connect(CONFIG_TEST_AWSIOT_ROOTCA,
           CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
           CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
           AWSTEST_CLIENT_PUB, AWSTEST_KEEP_ALIVE, 0, NULL, NULL)) < 0)
        {
          break;
        }

      /* Main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          awsiot_test_reset_last();
          random_message(randmes, AWSTEST_MESSAGE_LEN);

          if ((result = awsiot_test_mqtt_publish(AWSTEST_MQTT_PUB, AWSTEST_TOPIC_DEF,
               qos, randmes)) < 0)
            {
              break;
            }

          if ((result = awsiot_test_yield(AWSTEST_MQTT_SUB, AWSTEST_YIELD_DEF)) < 0)
            {
              break;
            }

          if ((result = awsiot_test_compare_last(qos, AWSTEST_RETAIN_DEF, AWSTEST_DUP_DEF,
               AWSTEST_TOPIC_DEF, randmes)) < 0)
            {
              break;
            }
        }

      /* Reset publish */
      if ((result2 = awsiot_test_mqtt_disconnect(AWSTEST_MQTT_PUB)) < 0)
        {
          break;
        }

      /* Reset subscribe */
      if ((result2 = awsiot_test_mqtt_unsubscribe(AWSTEST_MQTT_SUB, AWSTEST_TOPIC_DEF)) < 0)
        {
          break;
        }
      if ((result2 = awsiot_test_mqtt_disconnect(AWSTEST_MQTT_SUB)) < 0)
        {
          break;
        }
    } while (0);

  /* Finalize */
  awsiot_test_finalize();

  if ((result == 0) && (result2 < 0))
    {
      result = result2;
    }
  return result;
}


static int awstest_mqtt_pubsub_will(int loop)
{
  int result = 0;
  int result2 = 0;
  int cntc = 0;
  char randmes[AWSTEST_MESSAGE_LEN+1];
  char willmes[AWSTEST_MESSAGE_LEN+1];

  /* Initialize */
  awsiot_test_initialize();

  do
    {
      /* Setup will */
      if ((result = awsiot_test_mqtt_connect(CONFIG_TEST_AWSIOT_ROOTCA,
           CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
           CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
           AWSTEST_CLIENT_SUB, AWSTEST_KEEP_WILL_SUB, 0, NULL, NULL)) < 0)
        {
          break;
        }
      if ((result = awsiot_test_mqtt_subscribe(AWSTEST_MQTT_SUB, AWSTEST_TOPIC_DEF,
           AWSTEST_QOS_DEF)) < 0)
        {
          break;
        }
      if ((result = awsiot_test_mqtt_subscribe(AWSTEST_MQTT_SUB, AWSTEST_TOPIC_WILL,
           AWSTEST_QOS_DEF)) < 0)
        {
          break;
        }

      /* Main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          /* Setup publish */
          random_message(willmes, AWSTEST_MESSAGE_LEN);
          if ((result = awsiot_test_mqtt_connect(CONFIG_TEST_AWSIOT_ROOTCA,
               CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
               CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
               AWSTEST_CLIENT_PUB, AWSTEST_KEEP_WILL_PUB, 0, AWSTEST_TOPIC_WILL, willmes)) < 0)
            {
              break;
            }

          /* Normal publish - subscribe */
          awsiot_test_reset_last();
          random_message(randmes, AWSTEST_MESSAGE_LEN);

          if ((result = awsiot_test_mqtt_publish(AWSTEST_MQTT_PUB, AWSTEST_TOPIC_DEF,
               AWSTEST_QOS_DEF, randmes)) < 0)
            {
              break;
            }

          if ((result = awsiot_test_yield(AWSTEST_MQTT_SUB, AWSTEST_YIELD_DEF)) < 0)
            {
              break;
            }

          if ((result = awsiot_test_compare_last(AWSTEST_QOS_DEF, AWSTEST_RETAIN_DEF,
               AWSTEST_DUP_DEF, AWSTEST_TOPIC_DEF, randmes)) < 0)
            {
              break;
            }

          /* Activate will */
          awsiot_test_reset_last();
          usleep(AWSTEST_WAIT_WILL*1000);

          /* Receive will */
          if ((result = awsiot_test_yield(AWSTEST_MQTT_SUB, AWSTEST_YIELD_DEF)) < 0)
            {
              break;
            }

          if ((result = awsiot_test_compare_last(AWSTEST_QOS_DEF, AWSTEST_RETAIN_DEF,
               AWSTEST_DUP_DEF, AWSTEST_TOPIC_WILL, willmes)) < 0)
            {
              break;
            }

          /* Reset publish */
          awsiot_test_mqtt_disconnect(AWSTEST_MQTT_PUB);
        }

      /* Reset subscribe */
      if ((result2 = awsiot_test_mqtt_unsubscribe(AWSTEST_MQTT_SUB, AWSTEST_TOPIC_WILL)) < 0)
        {
          break;
        }
      if ((result2 = awsiot_test_mqtt_unsubscribe(AWSTEST_MQTT_SUB, AWSTEST_TOPIC_DEF)) < 0)
        {
          break;
        }
      if ((result2 = awsiot_test_mqtt_disconnect(AWSTEST_MQTT_SUB)) < 0)
        {
          break;
        }
    } while (0);

  /* Finalize */
  awsiot_test_finalize();

  if ((result == 0) && (result2 < 0))
    {
      result = result2;
    }
  return result;
}


static int awstest_mqtt_pubsub_wildcard(int loop, const char* topic, int *answer)
{
  int result = 0;
  int result2 = 0;
  int cntc = 0;
  int cntd = 0;
  char randmes[AWSTEST_MESSAGE_LEN+1];

  /* Initialize */
  awsiot_test_initialize();

  do
    {
      /* Setup subscribe */
      if ((result = awsiot_test_mqtt_connect(CONFIG_TEST_AWSIOT_ROOTCA,
           CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
           CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
           AWSTEST_CLIENT_SUB, AWSTEST_KEEP_ALIVE, 0, NULL, NULL)) < 0)
        {
          break;
        }
      if ((result = awsiot_test_mqtt_subscribe(AWSTEST_MQTT_SUB, topic, AWSTEST_QOS_DEF)) < 0)
        {
          break;
        }

      /* Setup publish */
      if ((result = awsiot_test_mqtt_connect(CONFIG_TEST_AWSIOT_ROOTCA,
           CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
           CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
           AWSTEST_CLIENT_PUB, AWSTEST_KEEP_ALIVE, 0, NULL, NULL)) < 0)
        {
          break;
        }

      /* Main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          for (cntd=0; cntd<AWSTEST_MQTT_LIST_NUMBER; cntd++)
            {
              /* Setup message */
              awsiot_test_reset_last();
              random_message(randmes, AWSTEST_MESSAGE_LEN);

              if ((result = awsiot_test_mqtt_publish(AWSTEST_MQTT_PUB, topiclist[cntd],
                   AWSTEST_QOS_DEF, randmes)) < 0)
                {
                  break;
                }

              if ((result = awsiot_test_yield(AWSTEST_MQTT_SUB, AWSTEST_YIELD_DEF)) < 0)
                {
                  break;
                }
              if (answer[cntd] == 1)
                {
                  if ((result = awsiot_test_compare_last(AWSTEST_QOS_DEF,
                       AWSTEST_RETAIN_DEF, AWSTEST_DUP_DEF, topic, randmes)) < 0)
                    {
                      break;
                    }
                }
              else
                {
                  if ((result = awsiot_test_compare_last(AWSTEST_QOS_DEF,
                       AWSTEST_RETAIN_DEF, AWSTEST_DUP_DEF, topic, randmes))
                       != AWSIOT_TEST_ERROR_COUNT)
                    {
                      result = AWSIOT_TEST_ERROR_COUNT;
                      break;
                    }
                  result = 0;
                }

              if ((result = awsiot_test_yield(AWSTEST_MQTT_PUB, AWSTEST_YIELD_DEF)) < 0)
                {
                  break;
                }
            }
          if (result < 0)
            {
              break;
            }
        }

      /* Reset publish */
      if ((result2 = awsiot_test_mqtt_disconnect(AWSTEST_MQTT_PUB)) < 0)
        {
          break;
        }

      /* Reset subscribe */
      if ((result2 = awsiot_test_mqtt_unsubscribe(AWSTEST_MQTT_SUB, topic)) < 0)
        {
          break;
        }
      if ((result2 = awsiot_test_mqtt_disconnect(AWSTEST_MQTT_SUB)) < 0)
        {
          break;
        }
    } while (0);

  /* Finalize */
  awsiot_test_finalize();

  if ((result == 0) && (result2 < 0))
    {
      result = result2;
    }
  return result;
}


static int awstest_shadow_connect(int loop)
{
  int result = 0;
  int cntt = 0;

  /* Initialize */
  awsiot_test_initialize();

  /* Main loop */
  for (cntt=0; cntt<loop; cntt++)
    {
      if ((result = awsiot_test_shadow_connect(CONFIG_TEST_AWSIOT_ROOTCA,
           CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
           CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
           AWSTEST_CLIENT_ID, 0, AWSTEST_SHADOW_THING)) < 0)
        {
          break;
        }

      if ((result = awsiot_test_shadow_disconnect(0)) < 0)
        {
          break;
        }
    }

  /* Finalize */
  awsiot_test_finalize();

  return result;
}


static int awstest_shadow_reconnect(int loop)
{
  int result = 0;
  int cntc = 0;

  /* Initialize */
  awsiot_test_initialize();

  do
    {
      if ((result = awsiot_test_shadow_connect(CONFIG_TEST_AWSIOT_ROOTCA,
           CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
           CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
           AWSTEST_CLIENT_ID, 1, AWSTEST_SHADOW_THING)) < 0)
        {
          break;
        }

      /* Main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          usleep(AWSTEST_WAIT_RECONNECT*1000);

          if ((result = awsiot_test_yield(AWSTEST_SHADOW_DELTA, AWSTEST_YIELD_DEF)) < 0)
            {
              break;
            }
        }

      awsiot_test_shadow_disconnect(AWSTEST_SHADOW_DELTA);
    } while (0);

  /* Finalize */
  awsiot_test_finalize();

  return result;
}


static int awstest_shadow_get(int loop)
{
  int result = 0;
  int result2 = 0;
  int cntc = 0;
  char randmes[AWSTEST_JSON_LEN+1];

  /* Initialize */
  awsiot_test_initialize();

  do
    {
      if ((result = awsiot_test_shadow_connect(CONFIG_TEST_AWSIOT_ROOTCA,
           CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
           CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
           AWSTEST_CLIENT_GET, 0, AWSTEST_SHADOW_GET_THING)) < 0)
        {
          break;
        }

      /* Main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          awsiot_test_shadow_json_init(randmes, AWSTEST_JSON_LEN);
          awsiot_test_shadow_json_add_single(randmes, AWSTEST_JSON_LEN,
                key_list[SHADOW_JSON_STRING], SHADOW_JSON_STRING, &string_list[cntc]);
          awsiot_test_shadow_json_fin(randmes, AWSTEST_JSON_LEN);
          if ((result = awsiot_test_shadow_update(AWSTEST_SHADOW_GET,
               AWSTEST_SHADOW_GET_THING, randmes)) < 0)
            {
              break;
            }

          /* Update (not check) */
          awsiot_test_callback_clear(-1);
          if ((result = awsiot_test_yield(AWSTEST_SHADOW_GET, AWSTEST_YIELD_UPDATE)) < 0)
            {
              break;
            }

          /* Get check */
          if ((result = awsiot_test_shadow_get(AWSTEST_SHADOW_GET,
               AWSTEST_SHADOW_GET_THING)) < 0)
            {
              break;
            }
          if ((result = awsiot_test_yield(AWSTEST_SHADOW_GET, AWSTEST_YIELD_UPDATE)) < 0)
            {
              break;
            }
          if ((result = awsiot_test_callback_check(AWSIOT_TEST_CALLBACK_GET,
               key_list[SHADOW_JSON_STRING], SHADOW_JSON_STRING, &string_list[cntc])) < 0)
            {
              break;
            }
        }

      if ((result2 = awsiot_test_shadow_disconnect(AWSTEST_SHADOW_GET)) < 0)
        {
          break;
        }
    } while (0);

  /* Finalize */
  awsiot_test_finalize();

  if ((result == 0) && (result2 < 0))
    {
      result = result2;
    }
  return result;
}


static int awstest_shadow_delete(int loop)
{
  int result = 0;
  int result2 = 0;
  int cntc = 0;
  char randmes[AWSTEST_JSON_LEN+1];

  /* Initialize */
  awsiot_test_initialize();

  do
    {
      if ((result = awsiot_test_shadow_connect(CONFIG_TEST_AWSIOT_ROOTCA,
           CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
           CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
           AWSTEST_CLIENT_DELETE, 0, AWSTEST_SHADOW_DELETE_THING)) < 0)
        {
          break;
        }

      /* Main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          awsiot_test_shadow_json_init(randmes, AWSTEST_JSON_LEN);
          awsiot_test_shadow_json_add_single(randmes, AWSTEST_JSON_LEN,
                key_list[SHADOW_JSON_STRING], SHADOW_JSON_STRING, &string_list[cntc]);
          awsiot_test_shadow_json_fin(randmes, AWSTEST_JSON_LEN);
          if ((result = awsiot_test_shadow_update(AWSTEST_SHADOW_DELETE,
               AWSTEST_SHADOW_DELETE_THING, randmes)) < 0)
            {
              break;
            }

          /* Update (not check) */
          awsiot_test_callback_clear(-1);
          if ((result = awsiot_test_yield(AWSTEST_SHADOW_DELETE, AWSTEST_YIELD_UPDATE)) < 0)
            {
              break;
            }

          /* Delete check */
          if ((result = awsiot_test_shadow_delete(AWSTEST_SHADOW_DELETE,
               AWSTEST_SHADOW_DELETE_THING)) < 0)
            {
              break;
            }
          if ((result = awsiot_test_yield(AWSTEST_SHADOW_DELETE, AWSTEST_YIELD_UPDATE)) < 0)
            {
              break;
            }
          if ((result = awsiot_test_callback_check(AWSIOT_TEST_CALLBACK_DELETE,
               key_list[SHADOW_JSON_STRING], SHADOW_JSON_STRING, NULL)) < 0)
            {
              break;
            }

          /* Get check (error required) */
          if ((result = awsiot_test_shadow_get(AWSTEST_SHADOW_DELETE,
               AWSTEST_SHADOW_DELETE_THING)) == 0)
            {
              break;
            }
          result = 0;
        }

      if ((result2 = awsiot_test_shadow_disconnect(AWSTEST_SHADOW_DELETE)) < 0)
        {
          break;
        }
    } while (0);

  /* Finalize */
  awsiot_test_finalize();

  if ((result == 0) && (result2 < 0))
    {
      result = result2;
    }
  return result;
}


static int awstest_shadow_delta(int loop, int type)
{
  int result = 0;
  int result2 = 0;
  int cntc = 0;
  int cntd = 0;
  char randmes[AWSTEST_JSON_LEN+1];

  /* Initialize */
  awsiot_test_initialize();

  do
    {
      /* Setup delta */
      if ((result = awsiot_test_shadow_connect(CONFIG_TEST_AWSIOT_ROOTCA,
           CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
           CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
           AWSTEST_CLIENT_DELTA, 0, AWSTEST_SHADOW_THING)) < 0)
        {
          break;
        }
      if ((result = awsiot_test_shadow_delta(AWSTEST_SHADOW_DELTA, key_list[type], type)) < 0)
        {
          break;
        }
      awsiot_test_shadow_enable_delta_recv();

      /* Main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          awsiot_test_shadow_json_init(randmes, AWSTEST_JSON_LEN);
          awsiot_test_shadow_json_add_desired(randmes, AWSTEST_JSON_LEN,
              key_list[SHADOW_JSON_INT16], SHADOW_JSON_INT16,
              &int_list[AWSTEST_SHADOW_LIST_NUMBER],
              key_list[SHADOW_JSON_UINT16], SHADOW_JSON_UINT16,
              &uint_list[AWSTEST_SHADOW_LIST_NUMBER],
              key_list[SHADOW_JSON_FLOAT], SHADOW_JSON_FLOAT,
              &float_list[AWSTEST_SHADOW_LIST_NUMBER],
              key_list[SHADOW_JSON_BOOL], SHADOW_JSON_BOOL,
              &bool_list[AWSTEST_SHADOW_LIST_NUMBER],
              key_list[SHADOW_JSON_STRING], SHADOW_JSON_STRING,
              &string_list[AWSTEST_SHADOW_LIST_NUMBER]);
          awsiot_test_shadow_json_fin(randmes, AWSTEST_JSON_LEN);
          if ((result = awsiot_test_shadow_update(AWSTEST_SHADOW_DELTA,
               AWSTEST_SHADOW_THING, randmes)) < 0)
            {
              break;
            }

          for (cntd=0; cntd<AWSTEST_SHADOW_LIST_NUMBER; cntd++)
            {
              /* Setup message */
              void* dataPtr = NULL;
              void* deltaPtr = NULL;
              bool sameFlag = false;
              awsiot_test_callback_clear(-1);
              awsiot_test_shadow_json_init(randmes, AWSTEST_JSON_LEN);
              switch(type)
                {
                  case SHADOW_JSON_INT16:
                      dataPtr = &int_list[cntd];
                      deltaPtr = &int_list[AWSTEST_SHADOW_LIST_NUMBER];
                      sameFlag = (*(int16_t*)dataPtr == *(int16_t*)deltaPtr)?true:false;
                      break;
                  case SHADOW_JSON_UINT16:
                      dataPtr = &uint_list[cntd];
                      deltaPtr = &uint_list[AWSTEST_SHADOW_LIST_NUMBER];
                      sameFlag = (*(uint16_t*)dataPtr == *(uint16_t*)deltaPtr)?true:false;
                      break;
                  case SHADOW_JSON_FLOAT:
                      dataPtr = &float_list[cntd];
                      deltaPtr = &float_list[AWSTEST_SHADOW_LIST_NUMBER];
                      sameFlag = (*(float*)dataPtr == *(float*)deltaPtr)?true:false;
                      break;
                  case SHADOW_JSON_BOOL:
                      dataPtr = &bool_list[cntd];
                      deltaPtr = &bool_list[AWSTEST_SHADOW_LIST_NUMBER];
                      sameFlag = (*(bool*)dataPtr == *(bool*)deltaPtr)?true:false;
                      break;
                  case SHADOW_JSON_STRING:
                      dataPtr = &string_list[cntd];
                      deltaPtr = &string_list[AWSTEST_SHADOW_LIST_NUMBER];
                      sameFlag = (strncmp(dataPtr, deltaPtr, AWSIOT_TEST_SHADOW_STRING_LEN) == 0)?true:false;
                      break;
                    default:
                      result = -1;
                      break;
                }
              awsiot_test_shadow_json_add_reported(randmes, AWSTEST_JSON_LEN,
                  key_list[type], type, dataPtr);
              awsiot_test_shadow_json_fin(randmes, AWSTEST_JSON_LEN);
              if (result < 0)
                {
                  break;
                }

              if ((result = awsiot_test_shadow_update(AWSTEST_SHADOW_DELTA,
                   AWSTEST_SHADOW_THING, randmes)) < 0)
                {
                  break;
                }

              /* Update check */
              if ((result = awsiot_test_yield(AWSTEST_SHADOW_DELTA, AWSTEST_YIELD_UPDATE)) < 0)
                {
                  break;
                }
              if ((result = awsiot_test_callback_check(AWSIOT_TEST_CALLBACK_UPDATE,
                   key_list[type], type, dataPtr)) < 0)
                {
                  break;
                }

              /* Delta check */
              if (sameFlag)
                {
                  if ((result = awsiot_test_callback_check(AWSIOT_TEST_CALLBACK_DELTA,
                       key_list[type], type, deltaPtr)) != AWSIOT_TEST_ERROR_COUNT)
                    {
                      break;
                    }
                  result = AWSIOT_TEST_ERROR_NONE;
                }
              else
                {
                  if ((result = awsiot_test_callback_check(AWSIOT_TEST_CALLBACK_DELTA,
                       key_list[type], type, deltaPtr)) < 0)
                    {
                      break;
                    }
                }
            }
          if (result < 0)
            {
              break;
            }
        }

      /* Reset delta */
      if ((result2 = awsiot_test_shadow_disconnect(AWSTEST_SHADOW_DELTA)) < 0)
        {
          break;
        }
    } while (0);

  /* Finalize */
  awsiot_test_finalize();

  if ((result == 0) && (result2 < 0))
    {
      result = result2;
    }
  return result;
}


static int awstest_shadow_loop(int msec)
{
  int result = 0;
  int cntd = 0;
  int type;
  char randmes[AWSTEST_JSON_LEN+1];

  /* Initialize */
  awsiot_test_initialize();

  do
    {
      /* Setup */
      if ((result = awsiot_test_shadow_connect(CONFIG_TEST_AWSIOT_ROOTCA,
           CONFIG_TEST_AWSIOT_CLIENT_CERT, CONFIG_TEST_AWSIOT_CLIENT_KEY,
           CONFIG_TEST_AWSIOT_HOST, CONFIG_TEST_AWSIOT_PORT,
           AWSTEST_CLIENT_ID, 0, AWSTEST_SHADOW_THING)) < 0)
        {
          break;
        }

      awsiot_test_callback_clear(-1);
      awsiot_test_shadow_json_init(randmes, AWSTEST_JSON_LEN);
      awsiot_test_shadow_json_add_desired(randmes, AWSTEST_JSON_LEN,
          key_list[SHADOW_JSON_INT16], SHADOW_JSON_INT16,
          &int_list[AWSTEST_SHADOW_LIST_NUMBER],
          key_list[SHADOW_JSON_UINT16], SHADOW_JSON_UINT16,
          &uint_list[AWSTEST_SHADOW_LIST_NUMBER],
          key_list[SHADOW_JSON_FLOAT], SHADOW_JSON_FLOAT,
          &float_list[AWSTEST_SHADOW_LIST_NUMBER],
          key_list[SHADOW_JSON_BOOL], SHADOW_JSON_BOOL,
          &bool_list[AWSTEST_SHADOW_LIST_NUMBER],
          key_list[SHADOW_JSON_STRING], SHADOW_JSON_STRING,
          &string_list[AWSTEST_SHADOW_LIST_NUMBER]);
      awsiot_test_shadow_json_fin(randmes, AWSTEST_JSON_LEN);
      if ((result = awsiot_test_shadow_update(AWSTEST_SHADOW_DELTA,
           AWSTEST_SHADOW_THING, randmes)) < 0)
        {
          break;
        }

      for (cntd=0; cntd<AWSTEST_SHADOW_PARAM_NUMBER; cntd++)
        {
          type = delta_list[cntd];
          if ((result = awsiot_test_shadow_delta(AWSTEST_SHADOW_DELTA,
               key_list[type], type)) < 0)
            {
              break;
            }
        }
      if (result < 0)
        {
          break;
        }

      /* Wait loop */
      while (msec > 0)
        {
          if ((result = awsiot_test_yield(AWSTEST_SHADOW_DELTA, AWSTEST_YIELD_LOOP)) < 0)
           {
              break;
          }
          msec -= AWSTEST_YIELD_LOOP;

          /* Delete check */
          if ((result = awsiot_test_callback_check(AWSIOT_TEST_CALLBACK_DELETE,
               key_list[SHADOW_JSON_STRING], SHADOW_JSON_STRING, NULL))
               == AWSIOT_TEST_ERROR_MESSAGE)
            {
              break;
            }
          awsiot_test_callback_clear(AWSIOT_TEST_CALLBACK_DELETE);
        }

      if ((result = awsiot_test_shadow_disconnect(AWSTEST_SHADOW_DELTA)) < 0)
        {
          break;
        }
    } while (0);

  /* Finalize */
  awsiot_test_finalize();

  return result;
}


/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int awsiot_main(int argc, char *argv[])
#endif
{
  int ret;
  int exe = 0;
  struct awsiottest_t *site;

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

  site = &awsiottest_site[0];
  while (site->code)
    {
      /* Main TLS tests */
      if ((argc > 1) && (strncmp(argv[1], site->code, 2) == 0))
        {
          ret = 0;
          switch(site->func)
            {
              case AWSTEST_FUNC_MQTT_CONNECT:
                  ret = awstest_mqtt_connect(site->loop);
                  exe++;
                  break;
              case AWSTEST_FUNC_MQTT_RECONNECT:
                  ret = awstest_mqtt_reconnect(site->loop);
                  exe++;
                  break;
              case AWSTEST_FUNC_SHADOW_CONNECT:
                  ret = awstest_shadow_connect(site->loop);
                  exe++;
                  break;
              case AWSTEST_FUNC_SHADOW_RECONNECT:
                  ret = awstest_shadow_reconnect(site->loop);
                  exe++;
                  break;
              case AWSTEST_FUNC_QOS:
                  ret = awstest_mqtt_pubsub(site->loop, site->param);
                  exe++;
                  break;
              case AWSTEST_FUNC_WILL:
                  ret = awstest_mqtt_pubsub_will(site->loop);
                  exe++;
                  break;
              case AWSTEST_FUNC_WILDCARD:
                  ret = awstest_mqtt_pubsub_wildcard(site->loop,
                                          awsiottest_wild[site->param].topic,
                                          awsiottest_wild[site->param].answer);
                  exe++;
                  break;
              case AWSTEST_FUNC_GET:
                  ret = awstest_shadow_get(site->loop);
                  exe++;
                  break;
              case AWSTEST_FUNC_DELETE:
                  ret = awstest_shadow_delete(site->loop);
                  exe++;
                  break;
              case AWSTEST_FUNC_DELTA:
                  ret = awstest_shadow_delta(site->loop, site->param);
                  exe++;
                  break;
              case AWSTEST_FUNC_WAIT:
                  ret = awstest_shadow_loop(site->param);
                  exe++;
                  break;
              default:
                  break;
            }
          if ((site->flag == AWSTEST_FLAG_SUCCESS) && (ret < 0))
            {
              printf("[ERR] Error : ret = %d\n", ret);
            }
          else if ((site->flag == AWSTEST_FLAG_FAIL) && (ret == 0))
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
      printf("[Library]       : 04 - 07\n");
      printf("[MQTT QoS]      : 10 - 11\n");
      printf("[MQTT will]     : 12\n");
      printf("[MQTT wildcard] : 13 - 15\n");
      printf("[Shadow get]    : 20\n");
      printf("[Shadow delete] : 21\n");
      printf("[Shadow delta]  : 22 - 26\n");
      printf("[Shadow update] : 27\n");
    }

  return 0;
}
