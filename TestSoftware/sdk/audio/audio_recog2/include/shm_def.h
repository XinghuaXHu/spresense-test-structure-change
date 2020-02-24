/****************************************************************************
 * audio_recog2/include/shm_def.h
 *
 *   Copyright 2019 Sony Semiconductor Solutions Corporation
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

#define TARGET_ADDR 0x0D0C0000

#define RCG_M1_ADDR    0x10
#define RCG_M2_ADDR    0x14
#define RCG_M3_ADDR    0x18

#define RCG_I_ADDR    0x20
#define RCG_E_ADDR    0x30
#define RCG_E_ADDR2   0x40
#define RCG_F_ADDR    0x50
#define RCG_S_ADDR    0x60

#define FLD_OFFS     4

#define RCG_M2_KEY     0x11
#define RCG_M3_KEY     0x21
#define RCG_M2_KEY2    0x14
#define RCG_M3_KEY2    0x24

#define RCG1_I_KEY    0xA2
#define RCG1_E_KEY    0xA3
#define RCG1_F_KEY    0xA5
#define RCG1_S_KEY    0xA6

#define RCG2_I_KEY    0x52
#define RCG2_E_KEY    0x53
#define RCG2_F_KEY    0x55
#define RCG2_S_KEY    0x56

#define RCG_OFFS0     0 
#define RCG_OFFS1     1 
#define RCG_OFFS2     2 
#define RCG_OFFS3     3 

#define RCG_INFO0     0x10
#define RCG_INFO1     0x11
#define RCG_INFO2     0x12
#define RCG_INFO3     0x13

#define S_NO0	      0
#define S_NO1	      1 
#define S_NO2	      2 
#define S_NO3	      3

#define SP_INST       0xF0
#define SP_INST2      0xFF
#define T_OFFSET      10000 
