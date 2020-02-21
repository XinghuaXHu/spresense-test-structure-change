/****************************************************************************
 * audio_recorder/include/mem_layout.h
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

#ifndef MEM_LAYOUT_H_INCLUDED
#define MEM_LAYOUT_H_INCLUDED

/*
 * Memory devices
 */
/* AUD_SRAM: type=RAM, use=0x0003f440, remainder=0x00000bc0 */
#define AUD_SRAM_ADDR  0x000c0000
#define AUD_SRAM_SIZE  0x00040000

/* RESERVED: type=RAM, use=0x00000080, remainder=0x0003ff80 */
#define RESERVED_ADDR  0x0e000000
#define RESERVED_SIZE  0x00040000

/*
 * Fixed areas
 */
#define AUDIO_WORK_AREA_ALIGN   0x00000008
#define AUDIO_WORK_AREA_ADDR    0x000c0000
#define AUDIO_WORK_AREA_DRM     0x000c0000 /* _DRM is obsolete macro. to use _ADDR */
#define AUDIO_WORK_AREA_SIZE    0x0003c000

#define MSG_QUE_AREA_ALIGN   0x00000008
#define MSG_QUE_AREA_ADDR    0x000fc000
#define MSG_QUE_AREA_DRM     0x000fc000 /* _DRM is obsolete macro. to use _ADDR */
#define MSG_QUE_AREA_SIZE    0x00003140

#define MEMMGR_WORK_AREA_ALIGN   0x00000008
#define MEMMGR_WORK_AREA_ADDR    0x000ff140
#define MEMMGR_WORK_AREA_DRM     0x000ff140 /* _DRM is obsolete macro. to use _ADDR */
#define MEMMGR_WORK_AREA_SIZE    0x00000200

#define MEMMGR_DATA_AREA_ALIGN   0x00000008
#define MEMMGR_DATA_AREA_ADDR    0x000ff340
#define MEMMGR_DATA_AREA_DRM     0x000ff340 /* _DRM is obsolete macro. to use _ADDR */
#define MEMMGR_DATA_AREA_SIZE    0x00000100

#define SPL_MGR_AREA_ALIGN   0x00000008
#define SPL_MGR_AREA_ADDR    0x0e000000
#define SPL_MGR_AREA_DRM     0x0e000000 /* _DRM is obsolete macro. to use _ADDR */
#define SPL_MGR_AREA_SIZE    0x00000040

#define APU_LOG_AREA_ALIGN   0x00000008
#define APU_LOG_AREA_ADDR    0x0e000040
#define APU_LOG_AREA_DRM     0x0e000040 /* _DRM is obsolete macro. to use _ADDR */
#define APU_LOG_AREA_SIZE    0x00000040

/*
 * Memory Manager max work area size
 */
#define MEMMGR_MAX_WORK_SIZE  0x00000060

/*
 * Pool IDs
 */
#define NULL_POOL  0
#define OUTPUT_BUF_POOL  1
#define MIC_IN_BUF_POOL  2
#define ENC_APU_CMD_POOL  3
#define SRC_APU_CMD_POOL  4

#define NUM_MEM_LAYOUTS  1
#define NUM_MEM_POOLS  5


/*
 * Pool areas
 */
/* Layout0: */
#define MEMMGR_L0_WORK_SIZE   0x00000060

/* Skip 0x0004 bytes for alignment. */
#define L0_OUTPUT_BUF_POOL_ALIGN    0x00000008
#define L0_OUTPUT_BUF_POOL_L_FENCE  0x000c0004
#define L0_OUTPUT_BUF_POOL_ADDR     0x000c0008
#define L0_OUTPUT_BUF_POOL_SIZE     0x00006000
#define L0_OUTPUT_BUF_POOL_U_FENCE  0x000c6008
#define L0_OUTPUT_BUF_POOL_NUM_SEG  0x00000002
#define L0_OUTPUT_BUF_POOL_SEG_SIZE 0x00003000

#define L0_MIC_IN_BUF_POOL_ALIGN    0x00000008
#define L0_MIC_IN_BUF_POOL_L_FENCE  0x000c600c
#define L0_MIC_IN_BUF_POOL_ADDR     0x000c6010
#define L0_MIC_IN_BUF_POOL_SIZE     0x0000f000
#define L0_MIC_IN_BUF_POOL_U_FENCE  0x000d5010
#define L0_MIC_IN_BUF_POOL_NUM_SEG  0x00000005
#define L0_MIC_IN_BUF_POOL_SEG_SIZE 0x00003000

#define L0_ENC_APU_CMD_POOL_ALIGN    0x00000008
#define L0_ENC_APU_CMD_POOL_L_FENCE  0x000d5014
#define L0_ENC_APU_CMD_POOL_ADDR     0x000d5018
#define L0_ENC_APU_CMD_POOL_SIZE     0x00000114
#define L0_ENC_APU_CMD_POOL_U_FENCE  0x000d512c
#define L0_ENC_APU_CMD_POOL_NUM_SEG  0x00000003
#define L0_ENC_APU_CMD_POOL_SEG_SIZE 0x0000005c

/* Skip 0x0004 bytes for alignment. */
#define L0_SRC_APU_CMD_POOL_ALIGN    0x00000008
#define L0_SRC_APU_CMD_POOL_L_FENCE  0x000d5134
#define L0_SRC_APU_CMD_POOL_ADDR     0x000d5138
#define L0_SRC_APU_CMD_POOL_SIZE     0x00000114
#define L0_SRC_APU_CMD_POOL_U_FENCE  0x000d524c
#define L0_SRC_APU_CMD_POOL_NUM_SEG  0x00000003
#define L0_SRC_APU_CMD_POOL_SEG_SIZE 0x0000005c

/* Remainder AUDIO_WORK_AREA=0x00026db0 */

#endif /* MEM_LAYOUT_H_INCLUDED */