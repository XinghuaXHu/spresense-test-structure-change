/****************************************************************************
 * test/sqa/singlefunction/multicpu/security/multicpu/sec_prot.h
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

/* Note: signal number seems to be decided to be able to use */
#define THREAD_SIG_NO      30
#define THREAD_SIG_NO_ERR  31
#define THREAD_SIG_ACT     16 

#define NG (-1)

#define AUD_CL_01_ERR    0x51
#define AUD_CL_02_ERR    0x52
#define AUD_CL_TEST3_4_1 0x10
#define AUD_CL_OK        0 
#define AUD_CL_NG        1 
#define CMD_SZ           20

#define TEST_MAX_NO      50 

#define CRE_SMODE_0   0  /* basic */
#define CRE_SMODE_1   1  /* 1-1-1 */
#define CRE_SMODE_2   2  /* 1-1-2 */ 
#define CRE_SMODE_1_1_1     0x10  /* 1-1-1 */ 
#define CRE_SMODE_1_1_2_1   0x11  /* 1-1-2 (1)*/ 
#define CRE_SMODE_1_1_2_2   0x12  /* 1-1-2 (2)*/ 
#define CRE_SMODE_1_1_2_3   0x13  /* 1-1-2 (3)*/ 
#define CRE_SMODE_1_1_3     0x14  /* 1-1-3 */ 
#define CRE_SMODE_1_2_1     0x15  /* 1-2-1 */ 
#define CRE_SMODE_1_2_2     0x16  /* 1-2-2 */ 
#define CRE_SMODE_2_1_2     0x17  /* 2-1-2 */ 
#define CRE_SMODE_2_1_3     0x18  /* 2-1-3 */ 
#define CRE_SMODE_2_2_1     0x19  /* 2-2-1 */ 
#define CRE_SMODE_2_2_1_2   0x1A  /* 2-2-1-2 */ 
#define CRE_SMODE_2_2_2     0x1B  /* 2-2-2 */ 
#define CRE_SMODE_3_1_2     0x1D  /* 3-1-2 */ 
#define CRE_SMODE_3_2_2     0x1F  /* 3-2-2 */ 
#define CRE_SMODE_3_3_2     0x21  /* 3-3-2 */ 
#define CRE_SMODE_3_4_1     0x22  /* 3-4-1 */ 
#define CRE_SMODE_3_4_2     0x23  /* 3-4-2 */ 
#define CRE_SMODE_3_4_3     0x24  /* 3-4-3 */ 

#define LOOP_TIMES_1  100 /**/

#define DEFAULT_CNT 1
#define CASE_NUM 4

#define TEST_FAIL  0 
#define TEST_OK    1
#define TEST_NONE  2 
 
