/****************************************************************************
 * test/sqa/singlefunction/audio_recog/recog_struct.h
 *
 *   Copyright (C) 2019 Sony Corporation
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

typedef struct recog_tdata /* data for a input command */
{
  char cmddata[CMD_SZ];
  char cmdnum;
  int cnt;
}recog_tdata_t;

/*
    main data structure 
 */
typedef struct recog_info  /* values for a common resource */
{
  pid_t my_pid; /* main task process */
  pid_t th_pid; /* thread process */

  /* for signal */
  sigset_t sig_d;
  sigset_t th_sig_d;

//  mpmutex_t sem;
//  mpmq_t mq;
  mpshm_t shm;

  /* for test process */
  char test_no[CMD_SZ]; /* the string of test number */
  int  selno;       /* the number of test */
  int  index;    /* index value for cmds data */
  int  rslt;     /* for judge */ 
  int  subrslt;  /* for sub judge */

  /* for input process */ 
  recog_tdata_t tdata;
}recog_info_t;


typedef struct recog_cmd
{
   char item[CMD_SZ];
   int  tno; 
}recog_cmd_t;
