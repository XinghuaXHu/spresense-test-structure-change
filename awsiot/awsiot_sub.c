/****************************************************************************
 * test/awsiot/awsiot_sub.c
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

#include "awsiot_sub.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static AWS_IoT_Client awsClient[AWSIOT_TEST_SOCKET+1];
static int aws_status[AWSIOT_TEST_SOCKET+1];
static int aws_recv = 0;
static int aws_qos = -1;
static int aws_retain = -1;
static int aws_dup = -1;
static char aws_topic[AWSIOT_TEST_RECV_BUF_SIZE];
static char aws_message[AWSIOT_TEST_RECV_BUF_SIZE];
static IoT_Client_Init_Params awsMqttInitParams[AWSIOT_TEST_SOCKET+1];
static IoT_Client_Connect_Params awsMqttConnectParams[AWSIOT_TEST_SOCKET+1];
static ShadowInitParameters_t awsShadowInitParams[AWSIOT_TEST_SOCKET+1];
static ShadowConnectParameters_t awsShadowConnectParams[AWSIOT_TEST_SOCKET+1];
static char awsShadowCallbackData[AWSIOT_TEST_CALLBACK_NUMBER][AWSIOT_TEST_SHADOW_STRING_LEN];
static int awsShadowJsonIndex = 0;
static jsonStruct_t awsShadowJson[AWSIOT_TEST_DESIRED_ITEM];
static bool awsShadowBool[AWSIOT_TEST_SOCKET];
static float awsShadowFloat[AWSIOT_TEST_SOCKET];
static double awsShadowDouble[AWSIOT_TEST_SOCKET];
static int8_t awsShadowInt8[AWSIOT_TEST_SOCKET];
static int16_t awsShadowInt16[AWSIOT_TEST_SOCKET];
static int32_t awsShadowInt32[AWSIOT_TEST_SOCKET];
static uint8_t awsShadowUint8[AWSIOT_TEST_SOCKET];
static uint16_t awsShadowUint16[AWSIOT_TEST_SOCKET];
static uint32_t awsShadowUint32[AWSIOT_TEST_SOCKET];
static char awsShadowString[AWSIOT_TEST_SOCKET][AWSIOT_TEST_SHADOW_STRING_LEN];
static char* awsShadowText[] = {
  "INT32",  "INT16",  "INT8", "UINT32", "UINT16", "UINT8", "FLOAT",  "DOUBLE", "BOOL", "STRING", "OBJECT"
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void awsiot_test_disconnect_callback(AWS_IoT_Client *pClient, void *data)
{
  if (aws_iot_is_autoreconnect_enabled(pClient))
    {
      printf("Callback : disconnected and auto reconnect\n");
    }
  else
    {
      printf("Callback : disconnected\n");
    }
}


static void awsiot_test_subscribe_callback(AWS_IoT_Client *pClient, char *topicName,
                                    uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData)
{
  aws_recv++;
  aws_qos = params->qos;
  aws_retain = params->isRetained;
  aws_dup = params->isDup;
  strncpy(aws_topic, topicName, AWSIOT_TEST_RECV_BUF_SIZE);
  aws_topic[topicNameLen] = '\0';
  strncpy(aws_message, params->payload, AWSIOT_TEST_RECV_BUF_SIZE);
  aws_message[params->payloadLen] = '\0';

  printf("Topic(%d):[%s]\n", topicNameLen, aws_topic);
  printf("Message(%d):[%s]\n", params->payloadLen, aws_message);
}


static void awsiot_test_get_callback(const char *pThingName, ShadowActions_t action,
                              Shadow_Ack_Status_t status,
                              const char *pReceivedJsonDocument, void *pContextData)
{
  printf("Get thing:[%s]\n", pThingName);
  printf("Json:[%s]\n", pReceivedJsonDocument);

  memset(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_GET], 0, AWSIOT_TEST_SHADOW_STRING_LEN);
  strncpy(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_GET], pReceivedJsonDocument,
          AWSIOT_TEST_SHADOW_STRING_LEN);
}


static void awsiot_test_delete_callback(const char *pThingName, ShadowActions_t action,
                                 Shadow_Ack_Status_t status,
                                 const char *pReceivedJsonDocument, void *pContextData)
{
  printf("Delete thing:[%s]\n", pThingName);
  printf("Json:[%s]\n", pReceivedJsonDocument);

  memset(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_DELETE], 0, AWSIOT_TEST_SHADOW_STRING_LEN);
  strncpy(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_DELETE], pReceivedJsonDocument,
          AWSIOT_TEST_SHADOW_STRING_LEN);
}


static void awsiot_test_update_callback(const char *pThingName, ShadowActions_t action,
                                 Shadow_Ack_Status_t status,
                                 const char *pReceivedJsonDocument, void *pContextData)
{
  if (SHADOW_ACK_TIMEOUT == status)
    {
      printf("Update callback : timeout : [%s]\n", pReceivedJsonDocument);
    }
  else if (SHADOW_ACK_REJECTED == status)
    {
      printf("Update callback : rejected : [%s]\n", pReceivedJsonDocument);
    }
  else if (SHADOW_ACK_ACCEPTED == status)
    {
      printf("Update callback : accepted : [%s]\n", pReceivedJsonDocument);
    }

  memset(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_UPDATE], 0, AWSIOT_TEST_SHADOW_STRING_LEN);
  strncpy(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_UPDATE], pReceivedJsonDocument,
          AWSIOT_TEST_SHADOW_STRING_LEN);
}


static void awsiot_test_delta_callback(const char *pJsonString,
                                       uint32_t JsonStringDataLen, jsonStruct_t *pContext)
{
  char message[AWSIOT_TEST_SHADOW_STRING_LEN];
  int length;

  memset(message, 0, AWSIOT_TEST_SHADOW_STRING_LEN);
  strncpy(message, "Delta received (", AWSIOT_TEST_SHADOW_STRING_LEN);
  strncat(message, awsShadowText[pContext->type], AWSIOT_TEST_SHADOW_STRING_LEN);
  strncat(message, "):", AWSIOT_TEST_SHADOW_STRING_LEN);
  length = AWSIOT_TEST_SHADOW_STRING_LEN - strlen(message);
  if (length > JsonStringDataLen)
    {
      length = JsonStringDataLen;
    }
  strncat(message, pJsonString, length);
  printf("%s\n", message);

  memset(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_DELTA], 0, AWSIOT_TEST_SHADOW_STRING_LEN);
  strncpy(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_DELTA],
          awsShadowText[pContext->type], AWSIOT_TEST_SHADOW_STRING_LEN);
  strncat(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_DELTA], ":", AWSIOT_TEST_SHADOW_STRING_LEN);
  length = AWSIOT_TEST_SHADOW_STRING_LEN - strlen(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_DELTA])-2;
  if (length > JsonStringDataLen)
    {
      length = JsonStringDataLen;
    }
  if (pContext->type == SHADOW_JSON_STRING)
    {
      strncat(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_DELTA], "\"", AWSIOT_TEST_SHADOW_STRING_LEN);
      strncat(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_DELTA], pJsonString, length);
      strncat(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_DELTA], "\"", AWSIOT_TEST_SHADOW_STRING_LEN);
    }
  else
    {
      strncat(awsShadowCallbackData[AWSIOT_TEST_CALLBACK_DELTA], pJsonString, length);
    }
}


static void awsiot_test_delete_thing_callback(AWS_IoT_Client *pClient, char *topicName,
                                    uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData)
{
  printf("Deleted thing : [%s]\n", topicName);
}


/****************************************************************************
 * Public Functions
 ****************************************************************************/

void awsiot_test_reset_last()
{
  aws_recv = 0;
  aws_qos = -1;
  aws_retain = -1;
  aws_dup = -1;
  memset(aws_topic, 0, AWSIOT_TEST_RECV_BUF_SIZE);
  memset(aws_message, 0, AWSIOT_TEST_RECV_BUF_SIZE);

  printf("AWSIoT socket reset last packet\n");
}


int awsiot_test_compare_last(int qos, int retain, int dup, const char* topic, const char* message)
{
  char *ptr = NULL;

  if (aws_recv == 0)
    {
      printf("awsiot_test_compare error [not received]\n");
      return AWSIOT_TEST_ERROR_COUNT;
    }
  if (qos != aws_qos)
    {
      printf("awsiot_test_compare error [qos] : [%d]/[%d]\n", qos, aws_qos);
      return AWSIOT_TEST_ERROR_QOS;
    }
  if (retain != aws_retain)
    {
      printf("awsiot_test_compare error [retain] : [%d]/[%d]\n", retain, aws_retain);
      return AWSIOT_TEST_ERROR_RETAIN;
    }
  if (dup != aws_dup)
    {
      printf("awsiot_test_compare error [dup] : [%d]/[%d]\n", dup, aws_dup);
      return AWSIOT_TEST_ERROR_DUP;
    }
  if (strncmp(topic, aws_topic, AWSIOT_TEST_RECV_BUF_SIZE) != 0)
    {
      if (((ptr = strstr(topic, "+")) != NULL) || ((ptr = strstr(topic, "#")) != NULL))
        {
          if (strncmp(topic, aws_topic, (ptr-topic)) != 0)
            {
              printf("awsiot_test_compare error [topic wildard] : [%s]/[%s]\n", topic, aws_topic);
              return AWSIOT_TEST_ERROR_TOPIC;
            }
        }
      else
        {
          printf("awsiot_test_compare error [topic] : [%s]/[%s]\n", topic, aws_topic);
          return AWSIOT_TEST_ERROR_TOPIC;
        }
    }
  if (strncmp(message, aws_message, AWSIOT_TEST_RECV_BUF_SIZE) != 0)
    {
      printf("awsiot_test_compare error [message] : [%s]/[%s]\n", message, aws_message);
      return AWSIOT_TEST_ERROR_MESSAGE;
    }
  printf("awsiot_test_compare success : [recv:%d]\n", aws_recv);
  return AWSIOT_TEST_ERROR_NONE;
}


int awsiot_test_callback_clear(int param)
{
  int cnt;
  if ((param < 0) || (param >= AWSIOT_TEST_CALLBACK_NUMBER))
    {
      for (cnt=0; cnt<AWSIOT_TEST_CALLBACK_NUMBER; cnt++)
        {
          memset(awsShadowCallbackData[cnt], 0, AWSIOT_TEST_SHADOW_STRING_LEN);
        }
      return 0;
    }
  memset(awsShadowCallbackData[param], 0, AWSIOT_TEST_SHADOW_STRING_LEN);
  return 0;
}


int awsiot_test_callback_check(int param, const char* key, int type, void* data)
{
  char message[AWSIOT_TEST_SHADOW_STRING_LEN];
  char value[AWSIOT_TEST_SHADOW_STRING_LEN];

  memset(value, 0, AWSIOT_TEST_SHADOW_STRING_LEN);
  switch (type)
    {
      case SHADOW_JSON_BOOL:
          snprintf(value, AWSIOT_TEST_SHADOW_STRING_LEN, "%s", (*(bool*)data)?"true":"false");
          break;
      case SHADOW_JSON_INT32:
          snprintf(value, AWSIOT_TEST_SHADOW_STRING_LEN, "%u", *(int32_t*)data);
          break;
      case SHADOW_JSON_INT16:
          snprintf(value, AWSIOT_TEST_SHADOW_STRING_LEN, "%d", *(int16_t*)data);
          break;
      case SHADOW_JSON_INT8:
          snprintf(value, AWSIOT_TEST_SHADOW_STRING_LEN, "%d", *(int8_t*)data);
          break;
      case SHADOW_JSON_UINT32:
          snprintf(value, AWSIOT_TEST_SHADOW_STRING_LEN, "%u", *(uint32_t*)data);
          break;
      case SHADOW_JSON_UINT16:
          snprintf(value, AWSIOT_TEST_SHADOW_STRING_LEN, "%d", *(uint16_t*)data);
          break;
      case SHADOW_JSON_UINT8:
          snprintf(value, AWSIOT_TEST_SHADOW_STRING_LEN, "%d", *(uint8_t*)data);
          break;
      case SHADOW_JSON_FLOAT:
          snprintf(value, AWSIOT_TEST_SHADOW_STRING_LEN, "%.3f", *(float*)data);
          while (value[strlen(value)-1] == '0')
            {
              value[strlen(value)-1] = '\0';
            }
          if (value[strlen(value)-1] == '.')
            {
              value[strlen(value)] = '0';
              value[strlen(value)+1] = '\0';
            }
          if ((strstr(value, ".") == NULL) && (strstr(value, "E") == NULL))
            {
              strncat(value, ".0", AWSIOT_TEST_SHADOW_STRING_LEN);
            }
          break;
      case SHADOW_JSON_DOUBLE:
          snprintf(value, AWSIOT_TEST_SHADOW_STRING_LEN, "%.3f", *(double*)data);
          while (value[strlen(value)-1] == '0')
            {
              value[strlen(value)-1] = '\0';
            }
          if (value[strlen(value)-1] == '.')
            {
              value[strlen(value)] = '0';
              value[strlen(value)+1] = '\0';
            }
          if ((strstr(value, ".") == NULL) && (strstr(value, "E") == NULL))
            {
              strncat(value, ".0", AWSIOT_TEST_SHADOW_STRING_LEN);
            }
          break;
      case SHADOW_JSON_STRING:
          snprintf(value, AWSIOT_TEST_SHADOW_STRING_LEN, "\"%s\"", (char*)data);
          break;
      case SHADOW_JSON_OBJECT:
      default:
          break;
    }

  memset(message, 0, AWSIOT_TEST_SHADOW_STRING_LEN);
  switch (param)
    {
      case AWSIOT_TEST_CALLBACK_GET:
          snprintf(message, AWSIOT_TEST_SHADOW_STRING_LEN,
                   "\"state\":{\"desired\":{\"%s\":%s}", key, value);
          break;
      case AWSIOT_TEST_CALLBACK_DELETE:
          snprintf(message, AWSIOT_TEST_SHADOW_STRING_LEN, "{\"version\":");
          break;
      case AWSIOT_TEST_CALLBACK_UPDATE:
          snprintf(message, AWSIOT_TEST_SHADOW_STRING_LEN,
                   "\"state\":{\"reported\":{\"%s\":%s}}", key, value);
          break;
      case AWSIOT_TEST_CALLBACK_DELTA:
          snprintf(message, AWSIOT_TEST_SHADOW_STRING_LEN, "%s:%s", key, value);
          break;
      default:
          break;
    }

  if (strlen(awsShadowCallbackData[param]) == 0)
    {
      printf("Don't receive message : [%s]:[%s]\n", key, message);
      return AWSIOT_TEST_ERROR_COUNT;
    }
  else if (strstr(awsShadowCallbackData[param], message) == NULL)
    {
      printf("Don't find message : [%s]:[%s]<->[%s]\n",
             key, message, awsShadowCallbackData[param]);
      return AWSIOT_TEST_ERROR_MESSAGE;
    }

  if (param == AWSIOT_TEST_CALLBACK_DELETE)
    {
      printf("Find message : [%s]\n",  message);
    }
  else
    {
      printf("Find message : [%s][%s]\n", key, message);
    }
  return AWSIOT_TEST_ERROR_NONE;
}


void awsiot_test_initialize()
{
  int cnt;

  for (cnt=0; cnt<AWSIOT_TEST_SOCKET+1; cnt++)
    {
      aws_status[cnt] = AWSIOT_TEST_STATUS_NONE;
    }
  printf("AWSIoT initialize\n");
}


void awsiot_test_finalize()
{
  int cnt;

  for (cnt=0; cnt<AWSIOT_TEST_SOCKET+1; cnt++)
    {
      if (aws_status[cnt] == AWSIOT_TEST_STATUS_MQTT)
        {
          awsiot_test_mqtt_disconnect(cnt);
        }
      if (aws_status[cnt] == AWSIOT_TEST_STATUS_SHADOW)
        {
          awsiot_test_shadow_disconnect(cnt);
        }
      aws_status[cnt] = AWSIOT_TEST_STATUS_NONE;
    }
  printf("AWSIoT finalize\n");
}


int awsiot_test_yield(int sock, int msec)
{
  int ret;

  if (aws_status[sock] == AWSIOT_TEST_STATUS_MQTT)
    {
      if ((ret = aws_iot_mqtt_yield(&awsClient[sock], msec)) < 0)
        {
          printf("aws_iot_mqtt_yield failed : %d\n", ret);
          return ret;
        }
      printf("aws_iot_mqtt_yield success : %d\n", sock);
    }
  else if (aws_status[sock] == AWSIOT_TEST_STATUS_SHADOW)
    {
      if ((ret = aws_iot_shadow_yield(&awsClient[sock], msec)) < 0)
        {
          printf("aws_iot_shadow_yield failed : %d\n", ret);
          return ret;
        }
      printf("aws_iot_shadow_yield success : %d\n", sock);
    }
  else
    {
      return -1;
    }
  return 0;
}


int awsiot_test_mqtt_connect(const char* cafile, const char* clifile, const char* keyfile,
                             const char* host, int port, const char* clientID, int interval,
                             int reconnect, const char* willtopic, const char* willmessage)
{
  int sock = -1;
  int ret;
  int cnt;

  for (cnt=0; cnt<AWSIOT_TEST_SOCKET+1; cnt++)
    {
      if (aws_status[cnt] == AWSIOT_TEST_STATUS_NONE)
        {
          break;
        }
    }
  if (cnt == AWSIOT_TEST_SOCKET+1)
    {
      printf("AWSIoT memory failed\n");
      return -1;
    }
  sock = cnt;
  aws_status[sock] = AWSIOT_TEST_STATUS_MQTT;
  printf("set socket number : %d\n", sock);

  awsMqttInitParams[sock].enableAutoReconnect = false;
  awsMqttInitParams[sock].pHostURL = (char*)host;
  awsMqttInitParams[sock].port = port;
  awsMqttInitParams[sock].pRootCALocation = (char*)cafile;
  awsMqttInitParams[sock].pDeviceCertLocation = (char*)clifile;
  awsMqttInitParams[sock].pDevicePrivateKeyLocation = (char*)keyfile;
  awsMqttInitParams[sock].mqttCommandTimeout_ms = 20000;
  awsMqttInitParams[sock].tlsHandshakeTimeout_ms = 5000;
  awsMqttInitParams[sock].mqttPacketTimeout_ms = 20000;
  awsMqttInitParams[sock].isSSLHostnameVerify = true;
  awsMqttInitParams[sock].disconnectHandler = awsiot_test_disconnect_callback;
  awsMqttInitParams[sock].disconnectHandlerData = NULL;

  if ((ret = aws_iot_mqtt_init(&awsClient[sock], &awsMqttInitParams[sock])) < 0)
    {
      printf("aws_iot_mqtt_init [%d] failed : %d\n", sock, ret);
      return ret;
    }

  awsMqttConnectParams[sock].keepAliveIntervalInSec = interval;
  awsMqttConnectParams[sock].isCleanSession = 1;      /* 0 is not supported at AWS IoT*/
  awsMqttConnectParams[sock].MQTTVersion = MQTT_3_1_1;
  awsMqttConnectParams[sock].pClientID = (char*)clientID;
  awsMqttConnectParams[sock].clientIDLen = (uint16_t) strlen(clientID);
  if ((willtopic == NULL) || (willmessage == NULL))
    {
      awsMqttConnectParams[sock].isWillMsgPresent = 0;
    }
  else
    {
      IoT_MQTT_Will_Options will = IoT_MQTT_Will_Options_Initializer;
      awsMqttConnectParams[sock].isWillMsgPresent = 1;
      memcpy(&awsMqttConnectParams[sock].will, &will, sizeof(IoT_MQTT_Will_Options));
      awsMqttConnectParams[sock].will.pTopicName = (char*)willtopic;
      awsMqttConnectParams[sock].will.topicNameLen = (uint16_t) strlen(willtopic);
      awsMqttConnectParams[sock].will.pMessage = (char*)willmessage;
      awsMqttConnectParams[sock].will.msgLen = (uint16_t) strlen(willmessage);
      printf("Set will : [%s]/[%s]\n", willtopic, willmessage);
    }
  awsMqttConnectParams[sock].pUsername = (char*)AWSIOT_TEST_MQTT_USERNAME;
  awsMqttConnectParams[sock].usernameLen = strlen(AWSIOT_TEST_MQTT_USERNAME);
  awsMqttConnectParams[sock].pPassword = (char*)AWSIOT_TEST_MQTT_PASSWORD;
  awsMqttConnectParams[sock].passwordLen = strlen(AWSIOT_TEST_MQTT_PASSWORD);

  if ((ret = aws_iot_mqtt_connect(&awsClient[sock], &awsMqttConnectParams[sock])) < 0)
    {
      printf("aws_iot_mqtt_connect [%d] failed : %d\n", sock, ret);
      return ret;
    }

  if (reconnect)
    {
      if ((ret = aws_iot_mqtt_autoreconnect_set_status(&awsClient[sock], true)) < 0)
        {
          printf("aws_iot_mqtt_autoreconnect_set_status [%d] failed : %d\n", sock, ret);
          return ret;
        }
      printf("Set reconnect state : %d\n", sock);
    }

  if (cafile)
    {
      printf("Set SSL root CA     : [%s]\n", cafile);
    }
  if (clifile)
    {
      printf("Set SSL client cert : [%s]\n", clifile);
    }
  if (keyfile)
    {
      printf("Set SSL client key  : [%s]\n", keyfile);
    }
  printf("aws_iot_mqtt_connect : %d\n", sock);
  return sock;
}


int awsiot_test_mqtt_disconnect(int sock)
{
  int ret;

  if (aws_status[sock] != AWSIOT_TEST_STATUS_MQTT)
    {
      return -1;
    }
  aws_status[sock] = AWSIOT_TEST_STATUS_NONE;

  if ((ret = aws_iot_mqtt_disconnect(&awsClient[sock])) < 0)
    {
      printf("aws_iot_mqtt_disconnect [%d] failed : %d\n", sock, ret);
      return ret;
    }

  printf("aws_iot_mqtt_disconnect : %d\n", sock);
  return ret;
}


int awsiot_test_mqtt_subscribe(int sock, const char* topic, int qos)
{
  int ret;

  if (aws_status[sock] != AWSIOT_TEST_STATUS_MQTT)
    {
      return -1;
    }
  if ((ret = aws_iot_mqtt_subscribe(&awsClient[sock], topic, strlen(topic), qos,
                                    awsiot_test_subscribe_callback, NULL)) < 0)
    {
      printf("aws_iot_mqtt_subscribe failed : %d\n", ret);
      return ret;
    }
  printf("aws_iot_mqtt_subscribe success : %d\n", sock);
  return 0;
}


int awsiot_test_mqtt_unsubscribe(int sock, const char* topic)
{
  int ret;

  if (aws_status[sock] != AWSIOT_TEST_STATUS_MQTT)
    {
      return -1;
    }
  if ((ret = aws_iot_mqtt_unsubscribe(&awsClient[sock], topic, strlen(topic))) < 0)
    {
      printf("aws_iot_mqtt_unsubscribe failed : %d\n", ret);
      return ret;
    }
  printf("aws_iot_mqtt_unsubscribe success : %d\n", sock);
  return 0;
}


int awsiot_test_mqtt_publish(int sock, const char* topic, int qos, const char* message)
{
  int ret;
  IoT_Publish_Message_Params msg;

  if (aws_status[sock] != AWSIOT_TEST_STATUS_MQTT)
    {
      return -1;
    }
  msg.qos = qos;
  msg.isRetained = 0;         /* Retain is not supported at AWS IoT */
  msg.payload = (void *)message;
  msg.payloadLen = strlen(message);
  if ((ret = aws_iot_mqtt_publish(&awsClient[sock], topic, strlen(topic), &msg)) < 0)
    {
      printf("aws_iot_mqtt_publish failed : %d\n", ret);
      return ret;
    }
  printf("aws_iot_mqtt_publish success : %d\n", sock);
  return 0;
}


int awsiot_test_shadow_connect(const char* cafile, const char* clifile, const char* keyfile,
                               const char* host, int port, const char* clientID,
                               int reconnect, const char* thing)
{
  int sock = -1;
  int ret;
  int cnt;

  for (cnt=0; cnt<AWSIOT_TEST_SOCKET+1; cnt++)
    {
      if (aws_status[cnt] == AWSIOT_TEST_STATUS_NONE)
        {
          break;
        }
    }
  if (cnt == AWSIOT_TEST_SOCKET+1)
    {
      printf("AWSIoT memory failed\n");
      return -1;
    }
  sock = cnt;
  aws_status[sock] = AWSIOT_TEST_STATUS_SHADOW;
  awsShadowJsonIndex = 0;
  printf("set socket number : %d\n", sock);

  awsShadowInitParams[sock].enableAutoReconnect = false;
  awsShadowInitParams[sock].pHost = (char*)host;
  awsShadowInitParams[sock].port = port;
  awsShadowInitParams[sock].pRootCA = (char*)cafile;
  awsShadowInitParams[sock].pClientCRT = (char*)clifile;
  awsShadowInitParams[sock].pClientKey = (char*)keyfile;
  awsShadowInitParams[sock].disconnectHandler = awsiot_test_disconnect_callback;

  if ((ret = aws_iot_shadow_init(&awsClient[sock], &awsShadowInitParams[sock])) < 0)
    {
      printf("aws_iot_shadow_init [%d] failed : %d\n", sock, ret);
      return ret;
    }

  awsShadowConnectParams[sock].pMyThingName = (char *)thing;
  awsShadowConnectParams[sock].pMqttClientId = (char *)clientID;
  awsShadowConnectParams[sock].mqttClientIdLen = (uint16_t) strlen(clientID);
  awsShadowConnectParams[sock].deleteActionHandler = awsiot_test_delete_thing_callback;

  if ((ret = aws_iot_shadow_connect(&awsClient[sock], &awsShadowConnectParams[sock])) < 0)
    {
      printf("aws_iot_shadow_connect [%d] failed : %d\n", sock, ret);
      return ret;
    }

  if (reconnect)
    {
      if ((ret = aws_iot_shadow_set_autoreconnect_status(&awsClient[sock], true)) < 0)
        {
          printf("aws_iot_shadow_set_autoreconnect_status [%d] failed : %d\n", sock, ret);
          return ret;
        }
    }
  if (cafile)
    {
      printf("Set SSL root CA     : [%s]\n", cafile);
    }
  if (clifile)
    {
      printf("Set SSL client cert : [%s]\n", clifile);
    }
  if (keyfile)
    {
      printf("Set SSL client key  : [%s]\n", keyfile);
    }
  printf("aws_iot_shadow_connect : %d\n", sock);
  return sock;
}


int awsiot_test_shadow_disconnect(int sock)
{
  int ret;

  if (aws_status[sock] != AWSIOT_TEST_STATUS_SHADOW)
    {
      return -1;
    }
  aws_status[sock] = AWSIOT_TEST_STATUS_NONE;

  if ((ret = aws_iot_shadow_disconnect(&awsClient[sock])) < 0)
    {
      printf("aws_iot_shadow_disconnect [%d] failed : %d\n", sock, ret);
      return ret;
    }

  printf("aws_iot_shadow_disconnect : %d\n", sock);
  return ret;
}


int awsiot_test_shadow_delta(int sock, const char* key, int type)
{
  int ret;

  if (aws_status[sock] != AWSIOT_TEST_STATUS_SHADOW)
    {
      return -1;
    }
  if (awsShadowJsonIndex >= AWSIOT_TEST_DESIRED_ITEM)
    {
      return -1;
    }

  awsShadowJson[awsShadowJsonIndex].cb = awsiot_test_delta_callback;
  awsShadowJson[awsShadowJsonIndex].pKey = key;
  awsShadowJson[awsShadowJsonIndex].type = type;
  switch (type)
    {
      case SHADOW_JSON_BOOL:
          awsShadowJson[awsShadowJsonIndex].pData = &awsShadowBool[sock];
          break;
      case SHADOW_JSON_INT32:
          awsShadowJson[awsShadowJsonIndex].pData = &awsShadowInt32[sock];
          break;
      case SHADOW_JSON_INT16:
          awsShadowJson[awsShadowJsonIndex].pData = &awsShadowInt16[sock];
          break;
      case SHADOW_JSON_INT8:
          awsShadowJson[awsShadowJsonIndex].pData = &awsShadowInt8[sock];
          break;
      case SHADOW_JSON_UINT32:
          awsShadowJson[awsShadowJsonIndex].pData = &awsShadowUint32[sock];
          break;
      case SHADOW_JSON_UINT16:
          awsShadowJson[awsShadowJsonIndex].pData = &awsShadowUint16[sock];
          break;
      case SHADOW_JSON_UINT8:
          awsShadowJson[awsShadowJsonIndex].pData = &awsShadowUint8[sock];
          break;
      case SHADOW_JSON_FLOAT:
          awsShadowJson[awsShadowJsonIndex].pData = &awsShadowFloat[sock];
          break;
      case SHADOW_JSON_DOUBLE:
          awsShadowJson[awsShadowJsonIndex].pData = &awsShadowDouble[sock];
          break;
      case SHADOW_JSON_STRING:
          awsShadowJson[awsShadowJsonIndex].pData = &awsShadowString[sock];
          break;
      case SHADOW_JSON_OBJECT:
      default:
          printf("Unsupported type : %d\n", type);
          return -1;
    }
  if ((ret = aws_iot_shadow_register_delta(&awsClient[sock],
                                           &awsShadowJson[awsShadowJsonIndex])) < 0)
    {
      printf("aws_iot_shadow_register_delta [%d]/[%d] failed : %d\n",
             sock, awsShadowJsonIndex, ret);
      return ret;
    }

  printf("aws_iot_shadow_register_delta (%d/%d) : %s\n", sock, awsShadowJsonIndex, key);
  awsShadowJsonIndex++;

  return ret;
}


int awsiot_test_shadow_update(int sock, const char* thing, char* buffer)
{
  int ret;

  if (aws_status[sock] != AWSIOT_TEST_STATUS_SHADOW)
    {
      return -1;
    }

  if ((ret = aws_iot_shadow_update(&awsClient[sock], thing, buffer,
                                   awsiot_test_update_callback, NULL,
                                   AWSIOT_TEST_SHADOW_TIMEOUT_SEC, true)) < 0)
    {
      printf("aws_iot_shadow_update [%d] failed : %d\n", sock, ret);
      return ret;
    }

  printf("aws_iot_shadow_update [%d]\n", sock);
  return ret;
}


int awsiot_test_shadow_get(int sock, const char* thing)
{
  int ret;

  if (aws_status[sock] != AWSIOT_TEST_STATUS_SHADOW)
    {
      return -1;
    }

  if ((ret = aws_iot_shadow_get(&awsClient[sock], thing, awsiot_test_get_callback,
                                NULL, AWSIOT_TEST_SHADOW_TIMEOUT_SEC, true)) < 0)
    {
      printf("aws_iot_shadow_get [%d] failed : %d\n", sock, ret);
      return ret;
    }

  printf("aws_iot_shadow_get [%d]\n", sock);
  return ret;
}


int awsiot_test_shadow_delete(int sock, const char* thing)
{
  int ret;

  if (aws_status[sock] != AWSIOT_TEST_STATUS_SHADOW)
    {
      return -1;
    }

  if ((ret = aws_iot_shadow_delete(&awsClient[sock], thing,
                                   awsiot_test_delete_callback, NULL,
                                   AWSIOT_TEST_SHADOW_TIMEOUT_SEC, true)) < 0)
    {
      printf("aws_iot_shadow_delete [%d] failed : %d\n", sock, ret);
      return ret;
    }

  printf("aws_iot_shadow_delete [%d]\n", sock);
  return ret;
}


int awsiot_test_shadow_enable_delta_recv()
{
  aws_iot_shadow_disable_discard_old_delta_msgs();
  printf("aws_iot_shadow_disable_discard_old_delta_msgs\n");
  return 0;
}


int awsiot_test_shadow_json_init(char* buffer, int length)
{
  int ret;

  if ((ret = aws_iot_shadow_init_json_document(buffer, length)) < 0)
    {
      printf("aws_iot_shadow_init_json_document failed : %d\n", ret);
      return ret;
    }
  return ret;
}


int awsiot_test_shadow_json_fin(char* buffer, int length)
{
  int ret;

  if ((ret = aws_iot_finalize_json_document(buffer, length)) < 0)
    {
      printf("aws_iot_finalize_json_document failed : %d\n", ret);
      return ret;
    }
  return ret;
}


int awsiot_test_shadow_json_add_reported(char* buffer, int length, const char* key,
                                         int type, void* data)
{
  int ret;
  jsonStruct_t addHandler;

  addHandler.cb = NULL;
  addHandler.pKey = key;
  addHandler.type = type;
  addHandler.pData = data;

  if ((ret = aws_iot_shadow_add_reported(buffer, length, 1, &addHandler)) < 0)
    {
      printf("aws_iot_shadow_add_reported failed : %d\n", ret);
      return ret;
    }
  return ret;
}


int awsiot_test_shadow_json_add_desired(char* buffer, int length, ...)
{
  va_list ap;
  int ret;
  int cnt;
  jsonStruct_t addHandler[AWSIOT_TEST_DESIRED_ITEM];

  va_start(ap, length);
  for (cnt=0; cnt<AWSIOT_TEST_DESIRED_ITEM; cnt++)
    {
      addHandler[cnt].cb = NULL;
      addHandler[cnt].pKey = va_arg(ap, char*);
      addHandler[cnt].type = va_arg(ap, int);
      addHandler[cnt].pData = va_arg(ap, void*);
    }
  va_end(ap);

  if ((ret = aws_iot_shadow_add_desired(buffer, length, AWSIOT_TEST_DESIRED_ITEM,
    &addHandler[0], &addHandler[1], &addHandler[2], &addHandler[3], &addHandler[4])) < 0)
    {
      printf("aws_iot_shadow_add_desired failed : %d\n", ret);
      return ret;
    }
  return ret;
}


int awsiot_test_shadow_json_add_single(char* buffer, int length, ...)
{
  va_list ap;
  int ret;
  int cnt;
  jsonStruct_t addHandler[AWSIOT_TEST_DESIRED_ITEM];

  va_start(ap, length);
  for (cnt=0; cnt<1; cnt++)
    {
      addHandler[cnt].cb = NULL;
      addHandler[cnt].pKey = va_arg(ap, char*);
      addHandler[cnt].type = va_arg(ap, int);
      addHandler[cnt].pData = va_arg(ap, void*);
    }
  va_end(ap);

  if ((ret = aws_iot_shadow_add_desired(buffer, length, 1, &addHandler[0])) < 0)
    {
      printf("aws_iot_shadow_add_desired failed : %d\n", ret);
      return ret;
    }
  return ret;
}

