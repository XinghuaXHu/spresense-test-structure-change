/****************************************************************************
 * test/mqtt/mqtt_sub.h
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

#ifndef __TEST_MQTT_MQTT_SUB_H
#define __TEST_MQTT_MQTT_SUB_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sdk/config.h>

#include <stdint.h>
#include <stdbool.h>

#include "MQTTClient.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MQTT_TEST_NORMAL              0
#define MQTT_TEST_SSL                 1
#define MQTT_TEST_RETAIN_OFF          0
#define MQTT_TEST_RETAIN_ON           1
#define MQTT_TEST_ERROR_NONE          0
#define MQTT_TEST_ERROR_COUNT         -1
#define MQTT_TEST_ERROR_QOS           -2
#define MQTT_TEST_ERROR_RETAIN        -3
#define MQTT_TEST_ERROR_DUP           -4
#define MQTT_TEST_ERROR_MESSAGE       -5
#define MQTT_TEST_SOCKET              4
#define MQTT_TEST_SEND_BUF_SIZE       128
#define MQTT_TEST_RECV_BUF_SIZE       128
#define MQTT_TEST_COMMAND_TIMEOUT     10000   /* msec */
#define MQTT_TEST_LIST_MULTI          MAX_MESSAGE_HANDLERS

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void mqtt_test_initialize( int ssl, const char* cafile);
void mqtt_test_finalize( void );
int mqtt_test_connect( int sock, const char* host, int port,
                       const char* clientID, int interval, int qos,
                       const char* willtopic, const char* willmessage );
int mqtt_test_disconnect( int sock );

int mqtt_test_subscribe( int sock, const char* topic, int qos );
int mqtt_test_unsubscribe( int sock, const char* topic );
int mqtt_test_publish( int sock, const char* topic, int qos, int retain,
                       const char* message);
int mqtt_test_yield( int sock, int msec);

void mqtt_test_reset_last( void );
int mqtt_test_compare_last( int qos, int retain, int dup,
                                   const char* message );

#endif /* __TEST_MQTT_MQTT_SUB_H */
