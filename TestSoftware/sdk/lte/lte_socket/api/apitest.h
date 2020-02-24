/****************************************************************************
 * test/lte_socket/api/apitest.h
 *
 *   Copyright 2018 Sony Semiconductor Solutions Corporation
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
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
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

#ifndef __TEST_LTE_SOCKET_API_APITEST_H
#define __TEST_LTE_SOCKET_API_APITEST_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef FAR
#  define FAR
#endif

#define TABLE_NUM(table) (sizeof(table)/sizeof(table[0]))
#define DONTCARE_VAL     -1
#define VERIFY_EQ        0  /* equal                : <target value> == <verify_value> */
#define VERIFY_NE        1  /* not equal            : <target value> != <verify_value> */
#define VERIFY_LT        2  /* less than            : <target value> <  <verify_value> */
#define VERIFY_LE        3  /* less than or equal   : <target value> <= <verify_value> */
#define VERIFY_GT        4  /* greater than         : <target value> >  <verify_value> */
#define VERIFY_GE        5  /* greater than or equal: <target value> >= <verify_value> */

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct apitest_verify_param_s
{
  int  verify_flag;
  int  verify_val;
};

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int apitest_verify_eq(int val, FAR struct apitest_verify_param_s *vparam);
int apitest_verify_ne(int val, FAR struct apitest_verify_param_s *vparam);
int apitest_verify_lt(int val, FAR struct apitest_verify_param_s *vparam);
int apitest_verify_le(int val, FAR struct apitest_verify_param_s *vparam);
int apitest_verify_gt(int val, FAR struct apitest_verify_param_s *vparam);
int apitest_verify_ge(int val, FAR struct apitest_verify_param_s *vparam);

int apitest_recvfrom_main(FAR char *hv4, FAR char *hv6, FAR char *port_no);
int apitest_recvfrom_nonblock(FAR char *hv4, FAR char *port_no);
int apitest_recvfrom_timeout(FAR char *hv4, FAR char *port_no);

#ifdef HOST_MAKE
int apitest_accept_main_v4(char *host, char *port_no);
int apitest_accept_main_v6(char *host, char *port_no);
int apitest_accept_noblock(char* host, char *port_no);
int apitest_accept_timeout(char* host, char *port_no);
int apitest_connect_main(int port_no);
int apitest_connect_noblock(int port_no);
int apitest_connect_timeout(int port_no);
int apitest_fcntl_main(char* host, char *port_no);
int apitest_shutdown_main(char *host, char *port_no);
int apitest_close_main(char *host, char *port_no);
int apitest_sendto_main(char* host, char *host_v6, char* port_no);
int apitest_sendto_timeout(char *port_no);
int apitest_sendto_nb(char *port_no);
int apitest_select_main(char *host, char *port_no, int port, int n);
int apitest_send_main(int port_no);
int apitest_send_main_host(FAR char *host, FAR char *port_no);
int apitest_send_nb(int port_no);
int apitest_send_to(int port_no);
int apitest_write_main(int port_no);
int apitest_write_main_host(FAR char *host, FAR char *port_no);
int apitest_write_nb(int port_no);
int apitest_write_to(int port_no);
int apitest_recv_main(char *host, char *port_no);
int apitest_recv_to(char *host, char *port_no);
int apitest_recv_nb(char *host, char *port_no);
int apitest_read_main(char *host, char *port_no);
int apitest_read_to(char *host, char *port_no);
int apitest_read_nb(char *host, char *port_no);
#else
int apitest_accept_main_v4(int port_no);
int apitest_accept_main_v6(int port_no);
int apitest_accept_noblock(int port_no);
int apitest_accept_timeout(int port_no);
int apitest_connect_main(char* host, char *port_no);
int apitest_connect_noblock(char* host, char *port_no);
int apitest_connect_timeout(char* host, char *port_no);
int apitest_bind_main(int port_no);
int apitest_setsockopt_main(void);
int apitest_socket_main(void);
int apitest_fdset_main(void);
int apitest_listen_main(int port_no);
int apitest_getsockopt_main(void);
int apitest_getsockname_main(int port_no);
int apitest_fcntl_main(int port_no);
int apitest_close_main(int port_no);
int apitest_sendto_main(char* host, char *host_v6, char* port_no);
int apitest_sendto_noblock(char* host, char* port_no);
int apitest_sendto_timeout(char* host, char* port_no);
int apitest_select_main(char *host, char *port_no, int port, int n);
int apitest_send_main(FAR char *host, FAR char *port_no);
int apitest_send_main_host(int port_no);
int apitest_send_nb(FAR char *host, FAR char *port_no);
int apitest_send_to(FAR char *host, FAR char *port_no);
int apitest_recv_main(int port_no);
int apitest_recv_nb(int port_no);
int apitest_recv_to(int port_no);
int apitest_shutdown_main(int port_no);
int apitest_write_main(FAR char *host, FAR char *port_no);
int apitest_write_main_host(int port_no);
int apitest_write_nb(FAR char *host, FAR char *port_no);
int apitest_write_to(FAR char *host, FAR char *port_no);
int apitest_read_main(int port_no);
int apitest_read_nb(int port_no);
int apitest_read_to(int port_no);
#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __TEST_LTE_SOCKET_API_APITEST_H */
