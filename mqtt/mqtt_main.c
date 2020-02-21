/****************************************************************************
 * test/mqtt/mqtt_main.c
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
#include "mqtt_sub.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MQTTTEST_FLAG_FAIL	0	/* Need to fail test */
#define MQTTTEST_FLAG_SUCCESS	1	/* Need to success test */
#define MQTTTEST_HOST           "iot.eclipse.org"
#define MQTTTEST_PORT           1883
#define MQTTTEST_PORT_SSL       8883
#define MQTTTEST_MESSAGE_LEN    32
#define MQTTTEST_CLIENT_DEF     "testfw"
#define MQTTTEST_CLIENT_PUB     "testfw_pub"
#define MQTTTEST_CLIENT_SUB     "testfw_sub"
#define MQTTTEST_CLIENT_WILL    "testfw_will"
#define MQTTTEST_KEEP_DEF       60
#define MQTTTEST_KEEP_WILL_SUB  240
#define MQTTTEST_KEEP_WILL_PUB  2
#define MQTTTEST_TOPIC_DEF      "/testfw/MQTT/topic/DEF"
#define MQTTTEST_TOPIC_QOS      "/testfw/MQTT/topic/QOS"
#define MQTTTEST_TOPIC_RETAIN   "/testfw/MQTT/topic/RETAIN"
#define MQTTTEST_TOPIC_WILL     "/testfw/MQTT/topic/WILL"
#define MQTTTEST_QOS_DEF        0
#define MQTTTEST_RETAIN_DEF     0
#define MQTTTEST_DUP_DEF        0
#define MQTTTEST_TIME_DEF       2000
#define MQTTTEST_WAIT_WILL      4000
#define MQTTTEST_SOCKET_SUB     0
#define MQTTTEST_SOCKET_PUB     1
#define MQTTTEST_SOCKET_WILL    2
#define MQTTTEST_LIST_NUMBER    6

#define MQTTTEST_FUNC_CONNECT   0
#define MQTTTEST_FUNC_QOS       1
#define MQTTTEST_FUNC_RETAIN    2
#define MQTTTEST_FUNC_WILL      3
#define MQTTTEST_FUNC_MULTI     4
#define MQTTTEST_FUNC_WILDCARD  5
#define MQTTTEST_FUNC_API_CON   6
#define MQTTTEST_FUNC_API_DIS   7

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const char* topiclist[MQTTTEST_LIST_NUMBER] = {
  "/testfw/MQTT/topic/DATA/number/1",
  "/testfw/MQTT/topic/DATA/number/2",
  "/testfw/MQTT/topic/DATA/number/3",
  "/testfw/MQTT/topic/DATA/test/1",
  "/testfw/MQTT/topic/DATA/test/2",
  "/testfw/MQTT/topic/DUMMY/number/2"
};
static int g_status =  0;
struct mqtttest_t
{
  char* code;
  int   func;
  int   ssl;
  int   loop;
  int   param;
  int   flag;
};
static struct mqtttest_t mqtttest_site[] = 
{
  { "03", MQTTTEST_FUNC_CONNECT , 0, 10, 0, MQTTTEST_FLAG_SUCCESS },
  { "04", MQTTTEST_FUNC_CONNECT , 1, 10, 0, MQTTTEST_FLAG_SUCCESS },
  { "10", MQTTTEST_FUNC_QOS     , 0,  3, 0, MQTTTEST_FLAG_SUCCESS },
  { "11", MQTTTEST_FUNC_QOS     , 1,  3, 0, MQTTTEST_FLAG_SUCCESS },
  { "12", MQTTTEST_FUNC_QOS     , 0,  3, 1, MQTTTEST_FLAG_SUCCESS },
  { "13", MQTTTEST_FUNC_QOS     , 1,  3, 1, MQTTTEST_FLAG_SUCCESS },
  { "14", MQTTTEST_FUNC_QOS     , 0,  3, 2, MQTTTEST_FLAG_SUCCESS },
  { "15", MQTTTEST_FUNC_QOS     , 1,  3, 2, MQTTTEST_FLAG_SUCCESS },
  { "16", MQTTTEST_FUNC_RETAIN  , 0,  3, 0, MQTTTEST_FLAG_SUCCESS },
  { "17", MQTTTEST_FUNC_RETAIN  , 1,  3, 0, MQTTTEST_FLAG_SUCCESS },
  { "18", MQTTTEST_FUNC_WILL    , 0,  3, 0, MQTTTEST_FLAG_SUCCESS },
  { "19", MQTTTEST_FUNC_WILL    , 1,  3, 0, MQTTTEST_FLAG_SUCCESS },
  { "20", MQTTTEST_FUNC_MULTI   , 0,  5, 0, MQTTTEST_FLAG_SUCCESS },
  { "21", MQTTTEST_FUNC_MULTI   , 1,  5, 0, MQTTTEST_FLAG_SUCCESS },
  { "22", MQTTTEST_FUNC_MULTI   , 0,  5, 1, MQTTTEST_FLAG_SUCCESS },
  { "23", MQTTTEST_FUNC_MULTI   , 1,  5, 1, MQTTTEST_FLAG_SUCCESS },
  { "30", MQTTTEST_FUNC_WILDCARD, 0,  1, 0, MQTTTEST_FLAG_SUCCESS },
  { "31", MQTTTEST_FUNC_WILDCARD, 1,  1, 0, MQTTTEST_FLAG_SUCCESS },
  { "32", MQTTTEST_FUNC_WILDCARD, 0,  1, 1, MQTTTEST_FLAG_SUCCESS },
  { "33", MQTTTEST_FUNC_WILDCARD, 1,  1, 1, MQTTTEST_FLAG_SUCCESS },
  { "34", MQTTTEST_FUNC_WILDCARD, 0,  1, 2, MQTTTEST_FLAG_SUCCESS },
  { "35", MQTTTEST_FUNC_WILDCARD, 1,  1, 2, MQTTTEST_FLAG_SUCCESS },
  { "40", MQTTTEST_FUNC_API_CON , 0,  6, 0, MQTTTEST_FLAG_FAIL },
  { "41", MQTTTEST_FUNC_API_CON , 0,  1, 1, MQTTTEST_FLAG_FAIL },
  { "42", MQTTTEST_FUNC_API_CON , 0,  1, 2, MQTTTEST_FLAG_FAIL },
  { "43", MQTTTEST_FUNC_API_DIS , 0,  1, 0, MQTTTEST_FLAG_FAIL },
  { "44", MQTTTEST_FUNC_API_DIS , 0,  1, 1, MQTTTEST_FLAG_FAIL },
  { "45", MQTTTEST_FUNC_API_DIS , 0,  1, 2, MQTTTEST_FLAG_FAIL },
  { NULL, 0, 0, 0,  0, MQTTTEST_FLAG_FAIL },
};
struct mqttwild_t
{
  int  answer[6];
  char *topic;
};
static struct mqttwild_t mqtttest_wild[] =
{
  { { 1, 1, 1, 0, 0, 0 }, "/testfw/MQTT/topic/DATA/number/+" },
  { { 0, 1, 0, 0, 1, 0 }, "/testfw/MQTT/topic/DATA/+/2" },
  { { 1, 1, 1, 1, 1, 0 }, "/testfw/MQTT/topic/DATA/#" },
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

static int mqtttest_connect(int ssl, int loop)
{
  int result = 0;
  int cnt = 0;

  /* Initialize */
  mqtt_test_initialize(ssl, CONFIG_TEST_MQTT_ROOTCA);

  /* Main loop */
  for (cnt=0; cnt<loop; cnt++)
    {
      if ((result = mqtt_test_connect(0, MQTTTEST_HOST,
                                      ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                      MQTTTEST_CLIENT_DEF, MQTTTEST_KEEP_DEF,
                                      MQTTTEST_QOS_DEF, NULL, NULL)) < 0)
        {
          break;
        }

      if ((result = mqtt_test_disconnect(0)) < 0)
        {
          break;
        }
    }

  /* Finalize */
  mqtt_test_finalize();

  return result;
}

static int mqtttest_qos(int ssl, int loop, int qos)
{
  int result = 0;
  int result2 = 0;
  int cntc = 0;
  int ytime = MQTTTEST_TIME_DEF * (qos + 1);
  char randmes[MQTTTEST_MESSAGE_LEN+1];

  /* Initialize */
  mqtt_test_initialize(ssl, CONFIG_TEST_MQTT_ROOTCA);

  do
    {
      /* Setup subscribe */
      if ((result = mqtt_test_connect(MQTTTEST_SOCKET_SUB, MQTTTEST_HOST,
                                      ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                      MQTTTEST_CLIENT_SUB, MQTTTEST_KEEP_DEF,
                                      qos, NULL, NULL)) < 0)
        {
          break;
        }
      if ((result = mqtt_test_subscribe(MQTTTEST_SOCKET_SUB,
                                        MQTTTEST_TOPIC_QOS, qos)) < 0)
        {
          break;
        }

      /* Setup publish */
      if ((result = mqtt_test_connect(MQTTTEST_SOCKET_PUB, MQTTTEST_HOST,
                                      ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                      MQTTTEST_CLIENT_PUB, MQTTTEST_KEEP_DEF,
                                      qos, NULL, NULL)) < 0)
        {
          break;
        }

      /* Main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          mqtt_test_reset_last();
          random_message(randmes, MQTTTEST_MESSAGE_LEN);

          if ((result = mqtt_test_publish(MQTTTEST_SOCKET_PUB, MQTTTEST_TOPIC_QOS,
                                          qos, MQTTTEST_RETAIN_DEF, randmes)) < 0)
            {
              break;
            }

          if ((result = mqtt_test_yield(MQTTTEST_SOCKET_SUB, ytime)) < 0)
            {
              break;
            }

          if ((result = mqtt_test_compare_last(qos, MQTTTEST_RETAIN_DEF,
                                               MQTTTEST_DUP_DEF, randmes)) < 0)
            {
                break;
            }
        }

      /* Reset publish */
      if ((result2 = mqtt_test_disconnect(MQTTTEST_SOCKET_PUB)) < 0)
        {
          break;
        }

      /* Reset subscribe */
      if ((result2 = mqtt_test_unsubscribe(MQTTTEST_SOCKET_SUB, MQTTTEST_TOPIC_QOS)) < 0)
        {
          break;
        }
      if ((result2 = mqtt_test_disconnect(MQTTTEST_SOCKET_SUB)) < 0)
        {
          break;
        }
    } while (0);

  /* Finalize */
  mqtt_test_finalize();

  if ((result == 0) && (result2 < 0))
    {
      result = result2;
    }
  return result;
}

static int mqtttest_retain(int ssl, int loop)
{
  int result = 0;
  int result2 = 0;
  int cntc = 0;
  char randmes[MQTTTEST_MESSAGE_LEN+1];

  /* Initialize */
  mqtt_test_initialize(ssl, CONFIG_TEST_MQTT_ROOTCA);

  do
    {
      /* Setup subscribe */
      if ((result = mqtt_test_connect(MQTTTEST_SOCKET_SUB, MQTTTEST_HOST,
                                      ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                      MQTTTEST_CLIENT_SUB, MQTTTEST_KEEP_DEF,
                                      MQTTTEST_QOS_DEF, NULL, NULL)) < 0)
        {
          break;
        }
      if ((result = mqtt_test_subscribe(MQTTTEST_SOCKET_SUB, MQTTTEST_TOPIC_RETAIN,
                                        MQTTTEST_QOS_DEF)) < 0)
        {
          break;
        }

      /* Setup publish */
      if ((result = mqtt_test_connect(MQTTTEST_SOCKET_PUB, MQTTTEST_HOST,
                                      ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                      MQTTTEST_CLIENT_PUB, MQTTTEST_KEEP_DEF,
                                      MQTTTEST_QOS_DEF, NULL, NULL)) < 0)
        {
          break;
        }

      /* Retain check */
      if ((result = mqtt_test_yield(MQTTTEST_SOCKET_SUB, MQTTTEST_TIME_DEF)) < 0)
        {
          break;
        }
      result = mqtt_test_compare_last(MQTTTEST_QOS_DEF, MQTT_TEST_RETAIN_ON,
                                      MQTTTEST_DUP_DEF, "");
      if ((result != MQTT_TEST_ERROR_COUNT) && (result != MQTT_TEST_ERROR_RETAIN)
          && (result != MQTT_TEST_ERROR_MESSAGE))
        {
          break;
        }

      /* 1st main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          mqtt_test_reset_last();
          random_message(randmes, MQTTTEST_MESSAGE_LEN);

          if ((result = mqtt_test_publish(MQTTTEST_SOCKET_PUB, MQTTTEST_TOPIC_RETAIN,
                                          MQTTTEST_QOS_DEF, MQTT_TEST_RETAIN_ON,
                                          randmes)) < 0)
            {
              break;
            }

          if ((result = mqtt_test_yield(MQTTTEST_SOCKET_SUB, MQTTTEST_TIME_DEF)) < 0)
            {
              break;
            }

          if ((result = mqtt_test_compare_last(MQTTTEST_QOS_DEF, MQTT_TEST_RETAIN_OFF,
                                               MQTTTEST_DUP_DEF, randmes)) < 0)
            {
              break;
            }
        }

      /* Reset publish */
      if ((result2 = mqtt_test_disconnect(MQTTTEST_SOCKET_PUB)) < 0)
        {
          break;
        }

      /* Reset subscribe */
      if ((result2 = mqtt_test_unsubscribe(MQTTTEST_SOCKET_SUB, MQTTTEST_TOPIC_RETAIN)) < 0)
        {
          break;
        }
      if ((result2 = mqtt_test_disconnect(MQTTTEST_SOCKET_SUB)) < 0)
        {
          break;
        }
      if (result < 0)
        {
          break;
        }

      /* 2nd main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          mqtt_test_reset_last();

          /* Setup subscribe */
          if ((result = mqtt_test_connect(MQTTTEST_SOCKET_SUB, MQTTTEST_HOST,
                                          ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                          MQTTTEST_CLIENT_SUB, MQTTTEST_KEEP_DEF,
                                          MQTTTEST_QOS_DEF, NULL, NULL)) < 0)
            {
              break;
            }
          if ((result = mqtt_test_subscribe(MQTTTEST_SOCKET_SUB, MQTTTEST_TOPIC_RETAIN,
                                            MQTTTEST_QOS_DEF)) < 0)
            {
              break;
            }

          /* Retain check */
          if ((result = mqtt_test_yield(MQTTTEST_SOCKET_SUB, MQTTTEST_TIME_DEF)) < 0)
            {
              break;
            }
          if ((result = mqtt_test_compare_last(MQTTTEST_QOS_DEF, MQTT_TEST_RETAIN_ON,
                                               MQTTTEST_DUP_DEF, randmes)) < 0)
            {
              break;
            }

          /* Reset subscribe */
          if ((result = mqtt_test_unsubscribe(MQTTTEST_SOCKET_SUB, MQTTTEST_TOPIC_RETAIN)) < 0)
            {
              break;
            }
          if ((result = mqtt_test_disconnect(MQTTTEST_SOCKET_SUB)) < 0)
            {
              break;
            }
        }
    } while (0);

  /* Finalize */
  mqtt_test_finalize();

  if ((result == 0) && (result2 < 0))
    {
      result = result2;
    }
  return result;
}

static int mqtttest_will(int ssl, int loop)
{
  int result = 0;
  int result2 = 0;
  int cntc = 0;
  char randmes[MQTTTEST_MESSAGE_LEN+1];
  char willmes[MQTTTEST_MESSAGE_LEN+1];

  /* Initialize */
  mqtt_test_initialize(ssl, CONFIG_TEST_MQTT_ROOTCA);

  do
    {
      /* Setup subscribe */
      if ((result = mqtt_test_connect(MQTTTEST_SOCKET_SUB, MQTTTEST_HOST,
                                      ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                      MQTTTEST_CLIENT_SUB, MQTTTEST_KEEP_WILL_SUB,
                                      MQTTTEST_QOS_DEF, NULL, NULL)) < 0)
        {
          break;
        }
      if ((result = mqtt_test_subscribe(MQTTTEST_SOCKET_SUB, MQTTTEST_TOPIC_DEF,
                                        MQTTTEST_QOS_DEF)) < 0)
        {
          break;
        }

      /* Setup will */
      if ((result = mqtt_test_connect(MQTTTEST_SOCKET_WILL, MQTTTEST_HOST,
                                      ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                      MQTTTEST_CLIENT_WILL, MQTTTEST_KEEP_WILL_SUB,
                                      MQTTTEST_QOS_DEF, NULL, NULL)) < 0)
        {
          break;
        }
      if ((result = mqtt_test_subscribe(MQTTTEST_SOCKET_WILL, MQTTTEST_TOPIC_WILL,
                                        MQTTTEST_QOS_DEF)) < 0)
        {
          break;
        }

      /* Main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          /* Setup publish */
          random_message(willmes, MQTTTEST_MESSAGE_LEN);
          if ((result = mqtt_test_connect(MQTTTEST_SOCKET_PUB, MQTTTEST_HOST,
                                          ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                          MQTTTEST_CLIENT_PUB, MQTTTEST_KEEP_WILL_PUB,
                                          MQTTTEST_QOS_DEF,
                                          MQTTTEST_TOPIC_WILL, willmes)) < 0)
            {
              break;
            }

          /* Normal publish - subscribe */
          mqtt_test_reset_last();
          random_message(randmes, MQTTTEST_MESSAGE_LEN);

          if ((result = mqtt_test_publish(MQTTTEST_SOCKET_PUB, MQTTTEST_TOPIC_DEF,
                                          MQTTTEST_QOS_DEF, MQTTTEST_RETAIN_DEF,
                                          randmes)) < 0)
            {
              break;
            }
          if ((result = mqtt_test_yield(MQTTTEST_SOCKET_SUB, MQTTTEST_TIME_DEF)) < 0)
            {
              break;
            }

          if ((result = mqtt_test_compare_last(MQTTTEST_QOS_DEF, MQTTTEST_RETAIN_DEF,
                                               MQTTTEST_DUP_DEF, randmes)) < 0)
            {
              break;
            }

          /* Activate will */
          mqtt_test_reset_last();
          usleep(MQTTTEST_WAIT_WILL*1000);

          /* Receive will */
          if ((result = mqtt_test_yield(MQTTTEST_SOCKET_WILL, MQTTTEST_TIME_DEF)) < 0)
            {
              break;
            }

          if ((result = mqtt_test_compare_last(MQTTTEST_QOS_DEF, MQTTTEST_RETAIN_DEF,
                                               MQTTTEST_DUP_DEF, willmes)) < 0)
            {
              break;
            }

          /* Reset publish */
          mqtt_test_disconnect(MQTTTEST_SOCKET_PUB);
        }

      /* Reset subscribe */
      if ((result2 = mqtt_test_unsubscribe(MQTTTEST_SOCKET_WILL, MQTTTEST_TOPIC_WILL)) < 0)
        {
          break;
        }
      if ((result2 = mqtt_test_disconnect(MQTTTEST_SOCKET_WILL)) < 0)
        {
          break;
        }

      /* Reset subscribe */
      if ((result2 = mqtt_test_unsubscribe(MQTTTEST_SOCKET_SUB, MQTTTEST_TOPIC_DEF)) < 0)
        {
          break;
        }
      if ((result2 = mqtt_test_disconnect(MQTTTEST_SOCKET_SUB)) < 0)
        {
          break;
        }
    } while (0);

  /* Finalize */
  mqtt_test_finalize();

  if ((result == 0) && (result2 < 0))
    {
      result = result2;
    }
  return result;
}


static int mqtttest_wildcard(int ssl, int loop, const char* topic, int *answer)
{
  int result = 0;
  int result2 = 0;
  int cntc = 0;
  int cntd = 0;
  char randmes[MQTTTEST_MESSAGE_LEN+1];

  /* Initialize */
  mqtt_test_initialize(ssl, CONFIG_TEST_MQTT_ROOTCA);

  do
    {
      /* Setup subscribe */
      if ((result = mqtt_test_connect(MQTTTEST_SOCKET_SUB, MQTTTEST_HOST,
                                      ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                      MQTTTEST_CLIENT_SUB, MQTTTEST_KEEP_DEF,
                                      MQTTTEST_QOS_DEF, NULL, NULL)) < 0)
        {
          break;
        }
      if ((result = mqtt_test_subscribe(MQTTTEST_SOCKET_SUB, topic,
                                        MQTTTEST_QOS_DEF)) < 0)
        {
          break;
        }

      /* Setup publish */
      if ((result = mqtt_test_connect(MQTTTEST_SOCKET_PUB, MQTTTEST_HOST,
                                      ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                      MQTTTEST_CLIENT_PUB, MQTTTEST_KEEP_DEF,
                                      MQTTTEST_QOS_DEF, NULL, NULL)) < 0)
        {
          break;
        }

      /* Main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          for (cntd=0; cntd<MQTTTEST_LIST_NUMBER; cntd++)
            {
              /* Setup message */
              mqtt_test_reset_last();
              random_message(randmes, MQTTTEST_MESSAGE_LEN);

              if ((result = mqtt_test_publish(MQTTTEST_SOCKET_PUB, topiclist[cntd],
                                              MQTTTEST_QOS_DEF, MQTTTEST_RETAIN_DEF,
                                              randmes)) < 0)
                {
                  break;
                }

              if ((result = mqtt_test_yield(MQTTTEST_SOCKET_SUB, MQTTTEST_TIME_DEF)) < 0)
                {
                  break;
                }
              if (answer[cntd] == 1)
                {
                  if ((result = mqtt_test_compare_last(MQTTTEST_QOS_DEF, MQTTTEST_RETAIN_DEF,
                                                     MQTTTEST_DUP_DEF, randmes)) < 0)
                    {
                      break;
                    }
                }
              else
                {
                  if ((result = mqtt_test_compare_last(MQTTTEST_QOS_DEF, MQTTTEST_RETAIN_DEF,
                                                       MQTTTEST_DUP_DEF, randmes))
                                                       != MQTT_TEST_ERROR_COUNT)
                    {
                      result = MQTT_TEST_ERROR_COUNT;
                      break;
                    }
                  result = 0;
                }

              if ((result = mqtt_test_yield(MQTTTEST_SOCKET_PUB, MQTTTEST_TIME_DEF)) < 0)
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
      if ((result2 = mqtt_test_disconnect(MQTTTEST_SOCKET_PUB)) < 0)
        {
          break;
        }

      /* Reset subscribe */
      if ((result2 = mqtt_test_unsubscribe(MQTTTEST_SOCKET_SUB, topic)) < 0)
        {
          break;
        }
      if ((result2 = mqtt_test_disconnect(MQTTTEST_SOCKET_SUB)) < 0)
        {
          break;
        }
    } while (0);

  /* Finalize */
  mqtt_test_finalize();

  if ((result == 0) && (result2 < 0))
    {
      result = result2;
    }
  return result;
}

static int mqtttest_multi(int ssl, int loop, int same)
{
  int result = 0;
  int result2 = 0;
  int cntc = 0;
  int cntd = 0;
  int pubch, subch;
  char randmes[MQTTTEST_MESSAGE_LEN+1];

  /* Initialize */
  mqtt_test_initialize(ssl, CONFIG_TEST_MQTT_ROOTCA);
  if (same)
    {
      pubch = subch = MQTTTEST_SOCKET_SUB;
    }
  else
    {
      pubch = MQTTTEST_SOCKET_PUB;
      subch = MQTTTEST_SOCKET_SUB;
    }

  do {
      /* Setup subscribe */
      if ((result = mqtt_test_connect(subch, MQTTTEST_HOST,
                                      ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                      MQTTTEST_CLIENT_SUB, MQTTTEST_KEEP_DEF,
                                      MQTTTEST_QOS_DEF, NULL, NULL)) < 0)
        {
          break;
        }
      for (cntd=0; cntd<MQTT_TEST_LIST_MULTI; cntd++)
        {
          if ((result = mqtt_test_subscribe(subch, topiclist[cntd], MQTTTEST_QOS_DEF)) < 0)
            {
              break;
            }
        }

      if (!same)
        {
          /* Setup publish */
          if ((result = mqtt_test_connect(pubch, MQTTTEST_HOST,
                                          ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                                          MQTTTEST_CLIENT_PUB, MQTTTEST_KEEP_DEF,
                                          MQTTTEST_QOS_DEF, NULL, NULL)) < 0)
            {
              break;
            }
        }

      /* Main loop */
      for (cntc=0; cntc<loop; cntc++)
        {
          for (cntd=0; cntd<MQTT_TEST_LIST_MULTI; cntd++)
            {
              /* Setup message */
              mqtt_test_reset_last();
              random_message(randmes, MQTTTEST_MESSAGE_LEN);

              if ((result = mqtt_test_publish(pubch, topiclist[cntd],
                                              MQTTTEST_QOS_DEF, MQTTTEST_RETAIN_DEF,
                                              randmes)) < 0)
                {
                  break;
                }
              if ((result = mqtt_test_yield(subch, MQTTTEST_TIME_DEF)) < 0)
                {
                  break;
                }
              if ((result = mqtt_test_compare_last(MQTTTEST_QOS_DEF, MQTTTEST_RETAIN_DEF,
                                                   MQTTTEST_DUP_DEF, randmes)) < 0)
                {
                  break;
                }

              if (!same)
                {
                  if ((result = mqtt_test_yield(pubch, MQTTTEST_TIME_DEF)) < 0)
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

      if (!same)
        {
          /* Reset publish */
          if ((result2 = mqtt_test_disconnect(pubch)) < 0)
            {
              break;
            }
        }

      /* Reset subscribe */
      for (cntd=0; cntd<MQTT_TEST_LIST_MULTI; cntd++)
        {
          if ((result2 = mqtt_test_unsubscribe(subch, topiclist[cntd])) < 0)
            {
              break;
            }
        }
      if ((result2 = mqtt_test_disconnect(subch)) < 0)
        {
          break;
        }
    } while (0);

  /* Finalize */
  mqtt_test_finalize();

  if ((result == 0) && (result2 < 0))
    {
      result = result2;
    }
  return result;
}

static int mqtttest_error_connect(int ssl, int testcase)
{
  int result = 0;
  int cntc = 0;

  /* Initialize */
  mqtt_test_initialize(ssl, CONFIG_TEST_MQTT_ROOTCA);

  do
    {
      /* Setup subscribe */
      if (mqtt_test_connect(MQTTTEST_SOCKET_SUB, MQTTTEST_HOST,
                            ((ssl)?MQTTTEST_PORT_SSL:MQTTTEST_PORT),
                            MQTTTEST_CLIENT_SUB, MQTTTEST_KEEP_DEF,
                            MQTTTEST_QOS_DEF, NULL, NULL) < 0)
        {
          break;
        }

      if (testcase == 0)
        {
          /* Main loop */
          for (cntc=0; cntc<MQTTTEST_LIST_NUMBER; cntc++)
            {
              if ((result = mqtt_test_subscribe(MQTTTEST_SOCKET_SUB, topiclist[cntc],
                                                MQTTTEST_QOS_DEF)) < 0)
                {
                  break;
                }
            }

          /* Result check */
          if (cntc != MQTT_TEST_LIST_MULTI)
            {
              result = 0;
            }

          for (cntc=0; cntc<MQTTTEST_LIST_NUMBER; cntc++)
            {
              if (mqtt_test_unsubscribe(MQTTTEST_SOCKET_SUB, topiclist[cntc]) < 0)
                {
                  break;
                }
            }
        }

      else if (testcase == 1)
        {
          result = mqtt_test_unsubscribe(MQTTTEST_SOCKET_SUB, topiclist[0]);
        }

      else if (testcase == 2)
        {
          /* Main loop */
          for (cntc=0; cntc<2; cntc++)
            {
              if ((result = mqtt_test_subscribe(MQTTTEST_SOCKET_SUB, topiclist[0],
                                                MQTTTEST_QOS_DEF)) < 0)
                {
                  break;
                }
            }

          /* Result check */
          if (cntc != 1)
            {
              result = 0;
            }

          for (cntc=0; cntc<2; cntc++)
            {
              if (mqtt_test_unsubscribe(MQTTTEST_SOCKET_SUB, topiclist[0]) < 0)
                {
                  break;
                }
            }
        }

      if (mqtt_test_disconnect(MQTTTEST_SOCKET_SUB) < 0)
        {
          break;
        }
    } while (0);

  /* Finalize */
  mqtt_test_finalize();

  return result;
}

static int mqtttest_error_disconnect(int ssl, int testcase)
{
  int result = 0;
  char randmes[MQTTTEST_MESSAGE_LEN+1];

  /* Initialize */
  mqtt_test_initialize(ssl, CONFIG_TEST_MQTT_ROOTCA);
  random_message(randmes, MQTTTEST_MESSAGE_LEN);

  do
    {
      if (testcase == 0)
        {
          result = mqtt_test_publish(MQTTTEST_SOCKET_PUB, topiclist[0],
                                     MQTTTEST_QOS_DEF, MQTTTEST_RETAIN_DEF, randmes);
        }

      else if (testcase == 1)
        {
          result = mqtt_test_subscribe(MQTTTEST_SOCKET_SUB, topiclist[0],
                                       MQTTTEST_QOS_DEF);
        }

      else if (testcase == 2)
        {
          result = mqtt_test_unsubscribe(MQTTTEST_SOCKET_SUB, topiclist[0]);
        }
    } while (0);

  /* Finalize */
  mqtt_test_finalize();

  return result;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int mqtt_main(int argc, char *argv[])
#endif
{
  int ret;
  int exe = 0;
  struct mqtttest_t *site;

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

  site = &mqtttest_site[0];
  while (site->code)
    {
      /* Main TLS tests */
      if ((argc > 1) && (strncmp(argv[1], site->code, 2) == 0))
        {
          ret = 0;
          switch(site->func)
            {
              case MQTTTEST_FUNC_CONNECT:
                  ret = mqtttest_connect(site->ssl, site->loop);
                  exe++;
                  break;
              case MQTTTEST_FUNC_QOS:
                  ret = mqtttest_qos(site->ssl, site->loop, site->param);
                  exe++;
                  break;
              case MQTTTEST_FUNC_RETAIN:
                  ret = mqtttest_retain(site->ssl, site->loop);
                  exe++;
                  break;
              case MQTTTEST_FUNC_WILL:
                  ret = mqtttest_will(site->ssl, site->loop);
                  exe++;
                  break;
              case MQTTTEST_FUNC_MULTI:
                  ret = mqtttest_multi(site->ssl, site->loop, site->param);
                  exe++;
                  break;
              case MQTTTEST_FUNC_WILDCARD:
                  ret = mqtttest_wildcard(site->ssl, site->loop,
                                          mqtttest_wild[site->param].topic,
                                          mqtttest_wild[site->param].answer);
                  exe++;
                  break;
              case MQTTTEST_FUNC_API_CON:
                  ret = mqtttest_error_connect(site->ssl, site->param);
                  exe++;
                  break;
              case MQTTTEST_FUNC_API_DIS:
                  ret = mqtttest_error_disconnect(site->ssl, site->param);
                  exe++;
                  break;
              default:
                  break;
            }
          if ((site->flag == MQTTTEST_FLAG_SUCCESS) && (ret < 0))
            {
              printf("[ERR] Error : ret = %d\n", ret);
            }
          else if ((site->flag == MQTTTEST_FLAG_FAIL) && (ret == 0))
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
      printf("[Library]     : 03 - 04\n");
      printf("[QoS]         : 10 - 15\n");
      printf("[Retain]      : 16 - 17\n");
      printf("[Will]        : 18 - 19\n");
      printf("[Multi topic] : 20 - 23\n");
      printf("[Wildcard]    : 30 - 35\n");
      printf("[API check]   : 40 - 45\n");
    }

  return 0;
}
