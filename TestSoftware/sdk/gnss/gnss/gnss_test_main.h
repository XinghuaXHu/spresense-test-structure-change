/****************************************************************************
 * test/sqa/singlefunction/gnss/gnss_test_main.h
 *
 *   Copyright (C) 2017 Sony. All rights reserved.
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
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
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

#ifndef __TEST_SQA_SINGLEFUNCTION_GNSS_GNSS_TEST_MAIN_H
#define __TEST_SQA_SINGLEFUNCTION_GNSS_GNSS_TEST_MAIN_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MY_GNSS_SIG               18
#define MY_PVTLOG_SIG             19

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct cxd56_gnss_dms_s
{
  int8_t   sign;
  uint8_t  degree;
  uint8_t  minute;
  uint32_t frac;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

extern uint32_t                         posfixflag;
extern struct cxd56_gnss_positiondata_s gnss_posdat;

/****************************************************************************
 * Test Functions
 ****************************************************************************/

int gnss_starttest(int argc, char *argv[], uint32_t stmod);
int gnss_cycle(int argc, char *argv[]);
int gnss_set_time(int argc, char *argv[]);
int gnss_set_position(int argc, char *argv[]);
int gnss_save_backupdata(int argc, char *argv[]);
int gnss_erase_backupdata(int argc, char *argv[]);
int gnss_testutility(int argc, char *argv[]);
int gnss_hotrepeat(int argc, char *argv[]);
int gnss_signalonoff(int argc, FAR char *argv[]);
int gnss_satellite(int argc, FAR char *argv[]);
int gnss_nmea(int argc, char *argv[]);
int gnss_factory(int argc, char *argv[]);
int gnss_pvtloglong(int argc, char *argv[]);
int gnss_pvtlogstress(int argc, char *argv[]);

/****************************************************************************
 * Test Utility
 ****************************************************************************/

void double_to_dmf(double x, struct cxd56_gnss_dms_s * dmf);
int read_and_print(int fd);
int gnss_start(int fd, uint32_t start_mode);
int gnss_stop(int fd);
int gnss_setsatellite(int fd, uint32_t set_satellite);
int gnss_setopemode(int fd, int cycle_msec);
int gnss_setparams(int fd);
int gnss_setsignal(int fd, sigset_t *pmask);
int gnss_clearsignal(int fd, sigset_t *pmask);
int argparse(const char *params[], char *argv);

#endif /* __TEST_SQA_SINGLEFUNCTION_GNSS_GNSS_TEST_MAIN_H */
