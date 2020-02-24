/* This file is generated automatically. */
/****************************************************************************
 * mem_layout.h
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

#ifndef MEM_LAYOUT_H_INCLUDED
#define MEM_LAYOUT_H_INCLUDED

/*
 * Memory devices
 */

/* AUD_SRAM: type=RAM, use=0x0003f300, remainder=0x00000d00 */

#define AUD_SRAM_ADDR  0x000a0000
#define AUD_SRAM_SIZE  0x00040000

/* SHM_SRAM: type=RAM, use=0x0001e000, remainder=0x00002000 */

#define SHM_SRAM_ADDR  0x000e0000
#define SHM_SRAM_SIZE  0x00020000

/*
 * Fixed areas
 */

#define AUDIO_WORK_AREA_ALIGN   0x00000008
#define AUDIO_WORK_AREA_ADDR    0x000a0000
#define AUDIO_WORK_AREA_DRM     0x000a0000 /* _DRM is obsolete macro. to use _ADDR */
#define AUDIO_WORK_AREA_SIZE    0x0003d000

#define MSG_QUE_AREA_ALIGN   0x00000008
#define MSG_QUE_AREA_ADDR    0x000dd000
#define MSG_QUE_AREA_DRM     0x000dd000 /* _DRM is obsolete macro. to use _ADDR */
#define MSG_QUE_AREA_SIZE    0x00002000

#define MEMMGR_WORK_AREA_ALIGN   0x00000008
#define MEMMGR_WORK_AREA_ADDR    0x000df000
#define MEMMGR_WORK_AREA_DRM     0x000df000 /* _DRM is obsolete macro. to use _ADDR */
#define MEMMGR_WORK_AREA_SIZE    0x00000200

#define MEMMGR_DATA_AREA_ALIGN   0x00000008
#define MEMMGR_DATA_AREA_ADDR    0x000df200
#define MEMMGR_DATA_AREA_DRM     0x000df200 /* _DRM is obsolete macro. to use _ADDR */
#define MEMMGR_DATA_AREA_SIZE    0x00000100

#define SENSOR_WORK_AREA_ALIGN   0x00000008
#define SENSOR_WORK_AREA_ADDR    0x000e0000
#define SENSOR_WORK_AREA_DRM     0x000e0000 /* _DRM is obsolete macro. to use _ADDR */
#define SENSOR_WORK_AREA_SIZE    0x0001e000

/*
 * Memory Manager max work area size
 */

#define S0_MEMMGR_WORK_AREA_ADDR  MEMMGR_WORK_AREA_ADDR
#define S0_MEMMGR_WORK_AREA_SIZE  0x000000f4

/*
 * Section IDs
 */

#define SECTION_NO0       0

/*
 * Number of sections
 */

#define NUM_MEM_SECTIONS  1

/*
 * Pool IDs
 */

const MemMgrLite::PoolId S0_NULL_POOL                = { 0, SECTION_NO0};  /*  0 */
const MemMgrLite::PoolId S0_ES_BUF_POOL              = { 1, SECTION_NO0};  /*  1 */
const MemMgrLite::PoolId S0_PREPROC_BUF_POOL         = { 2, SECTION_NO0};  /*  2 */
const MemMgrLite::PoolId S0_INPUT_BUF_POOL           = { 3, SECTION_NO0};  /*  3 */
const MemMgrLite::PoolId S0_ENC_APU_CMD_POOL         = { 4, SECTION_NO0};  /*  4 */
const MemMgrLite::PoolId S0_SRC_APU_CMD_POOL         = { 5, SECTION_NO0};  /*  5 */
const MemMgrLite::PoolId S0_PRE_APU_CMD_POOL         = { 6, SECTION_NO0};  /*  6 */
const MemMgrLite::PoolId S0_SENSOR_DSP_CMD_BUF_POOL  = { 7, SECTION_NO0};  /*  7 */
const MemMgrLite::PoolId S0_ACCEL_DATA_BUF_POOL      = { 8, SECTION_NO0};  /*  8 */
const MemMgrLite::PoolId S0_GNSS_DATA_BUF_POOL       = { 9, SECTION_NO0};  /*  9 */

#define NUM_MEM_S0_LAYOUTS   1
#define NUM_MEM_S0_POOLS    10

#define NUM_MEM_LAYOUTS      1
#define NUM_MEM_POOLS       10

/*
 * Pool areas
 */

/* Section0 Layout0: */

#define MEMMGR_S0_L0_WORK_SIZE   0x000000f4

/* Skip 0x0004 bytes for alignment. */

#define S0_L0_ES_BUF_POOL_ALIGN    0x00000008
#define S0_L0_ES_BUF_POOL_L_FENCE  0x000a0004
#define S0_L0_ES_BUF_POOL_ADDR     0x000a0008
#define S0_L0_ES_BUF_POOL_SIZE     0x00006000
#define S0_L0_ES_BUF_POOL_U_FENCE  0x000a6008
#define S0_L0_ES_BUF_POOL_NUM_SEG  0x00000002
#define S0_L0_ES_BUF_POOL_SEG_SIZE 0x00003000

#define S0_L0_PREPROC_BUF_POOL_ALIGN    0x00000008
#define S0_L0_PREPROC_BUF_POOL_L_FENCE  0x000a600c
#define S0_L0_PREPROC_BUF_POOL_ADDR     0x000a6010
#define S0_L0_PREPROC_BUF_POOL_SIZE     0x0000f000
#define S0_L0_PREPROC_BUF_POOL_U_FENCE  0x000b5010
#define S0_L0_PREPROC_BUF_POOL_NUM_SEG  0x00000005
#define S0_L0_PREPROC_BUF_POOL_SEG_SIZE 0x00003000

#define S0_L0_INPUT_BUF_POOL_ALIGN    0x00000008
#define S0_L0_INPUT_BUF_POOL_L_FENCE  0x000b5014
#define S0_L0_INPUT_BUF_POOL_ADDR     0x000b5018
#define S0_L0_INPUT_BUF_POOL_SIZE     0x0000f000
#define S0_L0_INPUT_BUF_POOL_U_FENCE  0x000c4018
#define S0_L0_INPUT_BUF_POOL_NUM_SEG  0x00000005
#define S0_L0_INPUT_BUF_POOL_SEG_SIZE 0x00003000

#define S0_L0_ENC_APU_CMD_POOL_ALIGN    0x00000008
#define S0_L0_ENC_APU_CMD_POOL_L_FENCE  0x000c401c
#define S0_L0_ENC_APU_CMD_POOL_ADDR     0x000c4020
#define S0_L0_ENC_APU_CMD_POOL_SIZE     0x00000114
#define S0_L0_ENC_APU_CMD_POOL_U_FENCE  0x000c4134
#define S0_L0_ENC_APU_CMD_POOL_NUM_SEG  0x00000003
#define S0_L0_ENC_APU_CMD_POOL_SEG_SIZE 0x0000005c

/* Skip 0x0004 bytes for alignment. */

#define S0_L0_SRC_APU_CMD_POOL_ALIGN    0x00000008
#define S0_L0_SRC_APU_CMD_POOL_L_FENCE  0x000c413c
#define S0_L0_SRC_APU_CMD_POOL_ADDR     0x000c4140
#define S0_L0_SRC_APU_CMD_POOL_SIZE     0x00000114
#define S0_L0_SRC_APU_CMD_POOL_U_FENCE  0x000c4254
#define S0_L0_SRC_APU_CMD_POOL_NUM_SEG  0x00000003
#define S0_L0_SRC_APU_CMD_POOL_SEG_SIZE 0x0000005c

/* Skip 0x0004 bytes for alignment. */

#define S0_L0_PRE_APU_CMD_POOL_ALIGN    0x00000008
#define S0_L0_PRE_APU_CMD_POOL_L_FENCE  0x000c425c
#define S0_L0_PRE_APU_CMD_POOL_ADDR     0x000c4260
#define S0_L0_PRE_APU_CMD_POOL_SIZE     0x00000114
#define S0_L0_PRE_APU_CMD_POOL_U_FENCE  0x000c4374
#define S0_L0_PRE_APU_CMD_POOL_NUM_SEG  0x00000003
#define S0_L0_PRE_APU_CMD_POOL_SEG_SIZE 0x0000005c

#define S0_L0_SENSOR_DSP_CMD_BUF_POOL_ALIGN    0x00000008
#define S0_L0_SENSOR_DSP_CMD_BUF_POOL_ADDR     0x000e0000
#define S0_L0_SENSOR_DSP_CMD_BUF_POOL_SIZE     0x00000380
#define S0_L0_SENSOR_DSP_CMD_BUF_POOL_NUM_SEG  0x00000008
#define S0_L0_SENSOR_DSP_CMD_BUF_POOL_SEG_SIZE 0x00000070

#define S0_L0_ACCEL_DATA_BUF_POOL_ALIGN    0x00000008
#define S0_L0_ACCEL_DATA_BUF_POOL_ADDR     0x000e0380
#define S0_L0_ACCEL_DATA_BUF_POOL_SIZE     0x00000c00
#define S0_L0_ACCEL_DATA_BUF_POOL_NUM_SEG  0x00000008
#define S0_L0_ACCEL_DATA_BUF_POOL_SEG_SIZE 0x00000180

#define S0_L0_GNSS_DATA_BUF_POOL_ALIGN    0x00000008
#define S0_L0_GNSS_DATA_BUF_POOL_ADDR     0x000e0f80
#define S0_L0_GNSS_DATA_BUF_POOL_SIZE     0x00000180
#define S0_L0_GNSS_DATA_BUF_POOL_NUM_SEG  0x00000008
#define S0_L0_GNSS_DATA_BUF_POOL_SEG_SIZE 0x00000030

/* Remainder AUDIO_WORK_AREA=0x00018c88 */
/* Remainder SENSOR_WORK_AREA=0x0001cf00 */

#endif /* MEM_LAYOUT_H_INCLUDED */
