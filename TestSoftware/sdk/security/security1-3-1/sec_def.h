/****************************************************************************
 * test/sqa/singlefunction/multicpu/security/multicpu/sec_def.h
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

#define SEC_MAXTASKS  5
//#define SEC_MAXTASKS  4 

#define SEC_MAXCPUS 5 
//#define SEC_MAXCPUS  4 

#define CPUS_NUM SEC_MAXTASKS
 
#define CPUNO_OFFSET 3

#define MY_TASK1_SIG  20  
#define MY_TASK2_SIG  21
#define MY_TASK3_SIG  22 
#define MY_TASK4_SIG  23 
#define MY_TASK5_SIG  24 

#define THREAD_SIG_NO  30 
#define THREAD_SIG_NO_ERR  31 

#define SEC_MAXFULLPATH 128 
#define SEC_SZ_SHM    1024 
//#define SEC_SZ_SHM    512 

#define SIG_INTERVAL_WAIT 6 
#define SIG_INTERVAL_WAIT2 15
#define SEC_MAXTASKS_SIG  5

char sig_type[SEC_MAXTASKS_SIG] = 
{
  MY_TASK1_SIG,
  MY_TASK2_SIG,
  MY_TASK3_SIG,
  MY_TASK4_SIG,
  MY_TASK5_SIG,
};

#define NG (-1)
#define INDX_CPUID   11
#define MASK_CPUID   0x0F
#define OFFSET_CPUID 3 

#define TEST1_LOOP_COUNT 5 
//#define LOOP_TIMEOUT_SEC  60 
#define MSG_STRING "MSG TEST:task0 :count0 "
#define MSG_IND_TASK 13
#define MSG_IND_CNT1  21 
#define MSG_IND_CNT2  22 

#define MSG_DATA_SZ  50 

#define NO_SHM_ALLC   0
#define SET_SHM_ALLC  1 
