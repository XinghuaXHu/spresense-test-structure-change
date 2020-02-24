/****************************************************************************
 * test/gnss_ioctl/gnss_ioctl_main.h
 *
 *   Copyright (C) 2016 Sony Corporation. All rights reserved.
 *   Author: Tomoyuki Takahashi <Tomoyuki.A.Takahashi@sony.com>
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

#ifndef __TEST_GNSS_IOCTL_GNSS_IOCTL_MAIN_H
#define __TEST_GNSS_IOCTL_GNSS_IOCTL_MAIN_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define GNSS_POLL_FD_NUM          1
#define GNSS_POLL_TIMEOUT_FOREVER -1
#define MY_GNSS_SIG               18

#define D_TIME_TO_SEC(a) \
    (double)((((a.hour*60)+a.minute)*60+a.sec)+((double)a.usec/1000000))
#define I_TIME_TO_SEC(a)    (uint32_t)(((a.hour*60)+a.minute)*60+a.sec)

# define SAT_SELECT_INVAL        (0xFFFFFFFF)    /* Invalid value */

#ifndef MAX
#  define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#  define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/
struct cxd56_gnss_dms_s
{
  int8_t   sign;
  uint8_t  degree;
  uint8_t  minute;
  uint32_t frac;
};

/****************************************************************************
 * Public Data
 ****************************************************************************/
extern struct cxd56_gnss_positiondata_s g_gnss_posdat;
extern uint32_t              g_gnss_posfixflag;

/****************************************************************************
 * Public Functions
 ****************************************************************************/
void gnss_double_to_dmf(double x, struct cxd56_gnss_dms_s *dmf);

int fd_open(void);
int fd_close(int fd);
int dioctl(int fd, int req, unsigned long arg, char *dmsg);

/* Sort as test menu */
int gnss_test_all(int argc, char *argv[]);
int gnss_test_normal(int argc, char *argv[]);
int gnss_test_ope_mode(int argc, char *argv[]);
int gnss_test_select_satellite_system(int argc, char *argv[]);
int gnss_test_start(int argc, char *argv[]);
int gnss_test_set_receiver_position(int argc, char *argv[]);
int gnss_test_set_time(int argc, char *argv[]);
int gnss_test_backup_data(int argc, char *argv[]);
int gnss_test_set_cep_data(int argc, char *argv[]);
int gnss_test_set_tcxo_offset(int argc, char *argv[]);
int gnss_test_set_almanac(int argc, char *argv[]);
int gnss_test_set_ephemeris(int argc, char *argv[]);

#endif /* __TEST_GNSS_IOCTL_GNSS_IOCTL_MAIN_H */

