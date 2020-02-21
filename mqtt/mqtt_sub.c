/****************************************************************************
 * test/mqtt/mqtt_sub.c
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

#include "mqtt_sub.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int mqtt_state[MQTT_TEST_SOCKET+1];
static MQTTSocket mqtt_sock[MQTT_TEST_SOCKET+1];
static MQTTClient mqtt_client[MQTT_TEST_SOCKET+1];
static MQTTPacket_connectData mqtt_option[MQTT_TEST_SOCKET+1];
static unsigned char mqtt_sendbuf[MQTT_TEST_SOCKET+1][MQTT_TEST_SEND_BUF_SIZE];
static unsigned char mqtt_recvbuf[MQTT_TEST_SOCKET+1][MQTT_TEST_RECV_BUF_SIZE];

static int last_recv = 0;
static int last_qos = -1;
static int last_retain = -1;
static int last_dup = -1;
static char last_message[MQTT_TEST_RECV_BUF_SIZE];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

void mqtt_test_arrived(MessageData* md)
{
  MQTTMessage* message = md->message;
  MQTTString* topicName = md->topicName;

  last_recv++;
  last_qos = message->qos;
  last_retain = message->retained;
  last_dup = message->dup;
  printf("Qos:[%d], Retained:[%d], Dup:[%d]\n", last_qos, last_retain, last_dup);

  if (topicName->lenstring.len >= MQTT_TEST_RECV_BUF_SIZE)
    {
      printf("mqtt_test_arrived topic length error : %d\n", topicName->lenstring.len);
    }
  else
    {
      memcpy(last_message, topicName->lenstring.data, topicName->lenstring.len);
      last_message[topicName->lenstring.len] = '\0';
      printf("Topic:[%s]\n", last_message);
    }
  if (message->payloadlen >= MQTT_TEST_RECV_BUF_SIZE)
    {
      printf("mqtt_test_arrived message length error : %d\n", message->payloadlen);
    }
  else
    {
      memcpy(last_message, message->payload, message->payloadlen);
      last_message[message->payloadlen] = '\0';
      printf("Message:[%s]\n", last_message);
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void mqtt_test_initialize(int ssl, const char* cafile)
{
  int cnt;

  for (cnt=0; cnt<MQTT_TEST_SOCKET+1; cnt++)
    {
      mqtt_state[cnt] = 0;
      mqtt_sock[cnt].pRootCALocation = (char*)cafile;
      mqtt_sock[cnt].pDeviceCertLocation = NULL;
      mqtt_sock[cnt].pDevicePrivateKeyLocation = NULL;
      MQTTSocketInit(&mqtt_sock[cnt], ssl);

      MQTTClientInit(&mqtt_client[cnt], &mqtt_sock[cnt], MQTT_TEST_COMMAND_TIMEOUT,
          mqtt_sendbuf[cnt], MQTT_TEST_SEND_BUF_SIZE,
          mqtt_recvbuf[cnt], MQTT_TEST_RECV_BUF_SIZE);
    }
  printf("MQTT socket initialize\n");
  if (cafile)
    {
      printf("Set SSL root CA : [%s]\n", cafile);
    }
}


void mqtt_test_finalize(void)
{
  int cnt;

  for (cnt=0; cnt<MQTT_TEST_SOCKET+1; cnt++)
   {
      if (mqtt_state[cnt] > 0)
        {
         mqtt_test_disconnect(cnt);
        }
      MQTTSocketFin(&mqtt_sock[cnt]);
    }
  printf("MQTT socket finalize\n");
}


int mqtt_test_disconnect(int sock)
{
  int ret = 0;

  if (mqtt_state[sock] >= 2)
    {
      ret = MQTTDisconnect(&mqtt_client[sock]);
      if (ret < 0)
        {
          printf("MQTTDisconnect [%d] failed : %d\n", sock, ret);
        }
      else
        {
          printf("MQTTDisconnect : %d\n", sock);
        }
    }

  if (mqtt_state[sock] >= 1)
    {
      MQTTSocketDisconnect(&mqtt_sock[sock]);
    }

  mqtt_state[sock] = 0;
  printf("MQTTSocketDisconnect : %d\n", sock);

  return ret;
}


int mqtt_test_connect(int sock, const char* host, int port,
                      const char* clientID, int interval, int qos,
                      const char* willtopic, const char* willmessage)
{
  int ret;
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  MQTTPacket_willOptions will = MQTTPacket_willOptions_initializer;

  mqtt_state[sock] = 1;
  if ((ret = MQTTSocketConnect(&mqtt_sock[sock], (char*)host, port)) < 0)
    {
      printf("MQTTSocketConnect error [%s:%d] : %d\n", host, port, ret);
      return ret;
    }
  mqtt_state[sock] = 2;
  printf("MQTTSocket connected : %d\n", sock);

  memcpy(&mqtt_option[sock], &data, sizeof(MQTTPacket_connectData));
  mqtt_option[sock].MQTTVersion = 3;
  mqtt_option[sock].clientID.cstring = (char*)clientID;
  mqtt_option[sock].keepAliveInterval = interval;
  mqtt_option[sock].cleansession = 1;
  if ((willtopic == NULL) || (willmessage == NULL))
    {
      mqtt_option[sock].willFlag = 0;
    }
  else
    {
      mqtt_option[sock].willFlag = 1;
      memcpy(&mqtt_option[sock].will, &will, sizeof(MQTTPacket_willOptions));
      mqtt_option[sock].will.qos = qos;
      mqtt_option[sock].will.topicName.cstring = (char*)willtopic;
      mqtt_option[sock].will.message.cstring = (char*)willmessage;
    }

  if ((ret = MQTTConnect(&mqtt_client[sock], &mqtt_option[sock])) < 0)
    {
      printf("MQTTConnect error : %d\n", ret);
      return ret;
    }
  mqtt_state[sock] = 3;

  printf("MQTT connected : %d\n", sock);
  return sock;
}


void mqtt_test_reset_last(void)
{
  last_recv = 0;
  last_qos = -1;
  last_retain = -1;
  last_dup = -1;
  memset(last_message, 0, MQTT_TEST_RECV_BUF_SIZE);

  printf("MQTT socket reset last packet\n");
}


int mqtt_test_compare_last(int qos, int retain, int dup, const char* message)
{
  if (last_recv == 0)
    {
      printf("mqtt_test_compare error [not received]\n");
      return MQTT_TEST_ERROR_COUNT;
    }
  if (qos != last_qos)
    {
      printf("mqtt_test_compare error [qos] : [%d]/[%d]\n", qos, last_qos);
      return MQTT_TEST_ERROR_QOS;
    }
  if (retain != last_retain)
    {
      printf("mqtt_test_compare error [retain] : [%d]/[%d]\n", retain, last_retain);
      return MQTT_TEST_ERROR_RETAIN;
    }
  if (dup != last_dup)
    {
      printf("mqtt_test_compare error [dup] : [%d]/[%d]\n", dup, last_dup);
      return MQTT_TEST_ERROR_DUP;
    }
  if (strncmp(message, last_message, MQTT_TEST_RECV_BUF_SIZE) != 0)
    {
      printf("mqtt_test_compare error [message] : [%s]/[%s]\n", message, last_message);
      return MQTT_TEST_ERROR_MESSAGE;
    }
  printf("mqtt_test_compare success : [recv:%d]\n", last_recv);
  return MQTT_TEST_ERROR_NONE;
}


int mqtt_test_subscribe(int sock, const char* topic, int qos)
{
  int ret = 0;

  if ((ret = MQTTSubscribe(&mqtt_client[sock], topic, qos, mqtt_test_arrived)) < 0)
    {
      printf("MQTTSubscribe failed : %d\n", ret);
      return ret;
    }
  printf("MQTTSubscribe success : %d\n", sock);
  return ret;
}


int mqtt_test_unsubscribe(int sock, const char* topic)
{
  int ret = 0;

  if ((ret = MQTTUnsubscribe(&mqtt_client[sock], topic)) < 0)
    {
      printf("MQTTUnsubscribe failed : %d\n", ret);
      return ret;
    }
  printf("MQTTUnsubscribe success : %d\n", sock);
  return ret;
}


int mqtt_test_publish(int sock, const char* topic, int qos, int retain, const char* message)
{
  int ret = 0;
  MQTTMessage msg;

  msg.qos = qos;
  msg.retained = retain;
  msg.payload = (void *)message;
  msg.payloadlen = strlen(message);
  if ((ret = MQTTPublish(&mqtt_client[sock], topic, &msg)) < 0)
    {
      printf("MQTTPublish failed : %d\n", ret);
      return ret;
    }
  printf("MQTTPublish success : %d\n", sock);
  return ret;
}


int mqtt_test_yield(int sock, int msec)
{
  int ret = 0;

  if ((ret = MQTTYield(&mqtt_client[sock], msec)) < 0)
    {
      printf("MQTTYield failed : %d\n", ret);
      return ret;
    }
  printf("MQTTYield success : %d\n", sock);
  return ret;
}

