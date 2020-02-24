/****************************************************************************
 * test/awsiot/awsiot_sub.h
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

#ifndef __TEST_AWSIOT_AWSIOT_SUB_H
#define __TEST_AWSIOT_AWSIOT_SUB_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sdk/config.h>

#include <stdint.h>
#include <stdbool.h>

#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define AWSIOT_TEST_STATUS_NONE            0
#define AWSIOT_TEST_STATUS_MQTT            1
#define AWSIOT_TEST_STATUS_SHADOW          2
#define AWSIOT_TEST_RETAIN_OFF             0
#define AWSIOT_TEST_RETAIN_ON              1
#define AWSIOT_TEST_ERROR_NONE             0
#define AWSIOT_TEST_ERROR_COUNT            -1
#define AWSIOT_TEST_ERROR_QOS              -2
#define AWSIOT_TEST_ERROR_RETAIN           -3
#define AWSIOT_TEST_ERROR_DUP              -4
#define AWSIOT_TEST_ERROR_TOPIC            -5
#define AWSIOT_TEST_ERROR_MESSAGE          -6
#define AWSIOT_TEST_SEND_BUF_SIZE          128
#define AWSIOT_TEST_RECV_BUF_SIZE          128
#define AWSIOT_TEST_SOCKET                 2
#define AWSIOT_TEST_MQTT_USERNAME          "testuser"
#define AWSIOT_TEST_MQTT_PASSWORD          "testpassword"
#define AWSIOT_TEST_SHADOW_TIMEOUT_SEC     2
#define AWSIOT_TEST_SHADOW_STRING_LEN      256
#define AWSIOT_TEST_CALLBACK_GET           0
#define AWSIOT_TEST_CALLBACK_DELETE        1
#define AWSIOT_TEST_CALLBACK_UPDATE        2
#define AWSIOT_TEST_CALLBACK_DELTA         3
#define AWSIOT_TEST_CALLBACK_NUMBER        4
#define AWSIOT_TEST_DESIRED_ITEM           5

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void awsiot_test_initialize( void );
void awsiot_test_finalize( void );
int awsiot_test_yield(int sock, int msec);

void awsiot_test_reset_last( void );
int awsiot_test_compare_last( int qos, int retain, int dup, const char* topic, const char* message );
int awsiot_test_callback_clear(int param);
int awsiot_test_callback_check(int param, const char* key, int type, void* data);

int awsiot_test_mqtt_connect( const char* cafile, const char* clifile, const char* keyfile, const char* host, int port, const char* clientID, int interval, int reconnect, const char* willtopic, const char* willmessage );
int awsiot_test_mqtt_disconnect( int sock );
int awsiot_test_mqtt_subscribe(int sock, const char* topic, int qos);
int awsiot_test_mqtt_unsubscribe(int sock, const char* topic);
int awsiot_test_mqtt_publish(int sock, const char* topic, int qos, const char* message);

int awsiot_test_shadow_connect( const char* cafile, const char* clifile, const char* keyfile, const char* host, int port, const char* clientID, int reconnect, const char* thing );
int awsiot_test_shadow_disconnect( int sock );
int awsiot_test_shadow_delta(int sock, const char* key, int type);
int awsiot_test_shadow_update(int sock, const char* thing, char* buffer);
int awsiot_test_shadow_get(int sock, const char* thing);
int awsiot_test_shadow_delete(int sock, const char* thing);
int awsiot_test_shadow_enable_delta_recv(void);
int awsiot_test_shadow_json_init(char* buffer, int length);
int awsiot_test_shadow_json_fin(char* buffer, int length);
int awsiot_test_shadow_json_add_reported(char* buffer, int length, const char* key, int type, void* data);
int awsiot_test_shadow_json_add_single(char* buffer, int length, ...);
int awsiot_test_shadow_json_add_desired(char* buffer, int length, ...);

#endif /* __TEST_AWSIOT_AWSIOT_SUB_H */
