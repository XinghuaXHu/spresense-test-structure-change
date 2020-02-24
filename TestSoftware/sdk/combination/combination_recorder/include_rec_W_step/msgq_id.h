/* This file is generated automatically. */
/****************************************************************************
 * msgq_id.h
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

#ifndef MSGQ_ID_H_INCLUDED
#define MSGQ_ID_H_INCLUDED

/* Message area size: 2824 bytes */
#define MSGQ_TOP_DRM 0xdd000
#define MSGQ_END_DRM 0xddb08

/* Message area fill value after message poped */
#define MSG_FILL_VALUE_AFTER_POP 0x0

/* Message parameter type match check */
#define MSG_PARAM_TYPE_MATCH_CHECK false

/* Message queue pool IDs */
#define MSGQ_NULL 0
#define MSGQ_AUD_MGR 1
#define MSGQ_AUD_APP 2
#define MSGQ_AUD_DSP 3
#define MSGQ_AUD_RECORDER 4
#define MSGQ_AUD_CAP 5
#define MSGQ_AUD_CAP_SYNC 6
#define MSGQ_AUD_FRONTEND 7
#define MSGQ_AUD_PREDSP 8
#define MSGQ_SEN_MGR 9
#define NUM_MSGQ_POOLS 10

/* User defined constants */

/************************************************************************/
#define MSGQ_AUD_MGR_QUE_BLOCK_DRM 0xdd044
#define MSGQ_AUD_MGR_N_QUE_DRM 0xdd2a8
#define MSGQ_AUD_MGR_N_SIZE 88
#define MSGQ_AUD_MGR_N_NUM 3
#define MSGQ_AUD_MGR_H_QUE_DRM 0xffffffff
#define MSGQ_AUD_MGR_H_SIZE 0
#define MSGQ_AUD_MGR_H_NUM 0
/************************************************************************/
#define MSGQ_AUD_APP_QUE_BLOCK_DRM 0xdd088
#define MSGQ_AUD_APP_N_QUE_DRM 0xdd3b0
#define MSGQ_AUD_APP_N_SIZE 64
#define MSGQ_AUD_APP_N_NUM 2
#define MSGQ_AUD_APP_H_QUE_DRM 0xffffffff
#define MSGQ_AUD_APP_H_SIZE 0
#define MSGQ_AUD_APP_H_NUM 0
/************************************************************************/
#define MSGQ_AUD_DSP_QUE_BLOCK_DRM 0xdd0cc
#define MSGQ_AUD_DSP_N_QUE_DRM 0xdd430
#define MSGQ_AUD_DSP_N_SIZE 20
#define MSGQ_AUD_DSP_N_NUM 5
#define MSGQ_AUD_DSP_H_QUE_DRM 0xffffffff
#define MSGQ_AUD_DSP_H_SIZE 0
#define MSGQ_AUD_DSP_H_NUM 0
/************************************************************************/
#define MSGQ_AUD_RECORDER_QUE_BLOCK_DRM 0xdd110
#define MSGQ_AUD_RECORDER_N_QUE_DRM 0xdd494
#define MSGQ_AUD_RECORDER_N_SIZE 48
#define MSGQ_AUD_RECORDER_N_NUM 5
#define MSGQ_AUD_RECORDER_H_QUE_DRM 0xffffffff
#define MSGQ_AUD_RECORDER_H_SIZE 0
#define MSGQ_AUD_RECORDER_H_NUM 0
/************************************************************************/
#define MSGQ_AUD_CAP_QUE_BLOCK_DRM 0xdd154
#define MSGQ_AUD_CAP_N_QUE_DRM 0xdd584
#define MSGQ_AUD_CAP_N_SIZE 24
#define MSGQ_AUD_CAP_N_NUM 16
#define MSGQ_AUD_CAP_H_QUE_DRM 0xffffffff
#define MSGQ_AUD_CAP_H_SIZE 0
#define MSGQ_AUD_CAP_H_NUM 0
/************************************************************************/
#define MSGQ_AUD_CAP_SYNC_QUE_BLOCK_DRM 0xdd198
#define MSGQ_AUD_CAP_SYNC_N_QUE_DRM 0xdd704
#define MSGQ_AUD_CAP_SYNC_N_SIZE 16
#define MSGQ_AUD_CAP_SYNC_N_NUM 8
#define MSGQ_AUD_CAP_SYNC_H_QUE_DRM 0xffffffff
#define MSGQ_AUD_CAP_SYNC_H_SIZE 0
#define MSGQ_AUD_CAP_SYNC_H_NUM 0
/************************************************************************/
#define MSGQ_AUD_FRONTEND_QUE_BLOCK_DRM 0xdd1dc
#define MSGQ_AUD_FRONTEND_N_QUE_DRM 0xdd784
#define MSGQ_AUD_FRONTEND_N_SIZE 48
#define MSGQ_AUD_FRONTEND_N_NUM 10
#define MSGQ_AUD_FRONTEND_H_QUE_DRM 0xffffffff
#define MSGQ_AUD_FRONTEND_H_SIZE 0
#define MSGQ_AUD_FRONTEND_H_NUM 0
/************************************************************************/
#define MSGQ_AUD_PREDSP_QUE_BLOCK_DRM 0xdd220
#define MSGQ_AUD_PREDSP_N_QUE_DRM 0xdd964
#define MSGQ_AUD_PREDSP_N_SIZE 20
#define MSGQ_AUD_PREDSP_N_NUM 5
#define MSGQ_AUD_PREDSP_H_QUE_DRM 0xffffffff
#define MSGQ_AUD_PREDSP_H_SIZE 0
#define MSGQ_AUD_PREDSP_H_NUM 0
/************************************************************************/
#define MSGQ_SEN_MGR_QUE_BLOCK_DRM 0xdd264
#define MSGQ_SEN_MGR_N_QUE_DRM 0xdd9c8
#define MSGQ_SEN_MGR_N_SIZE 40
#define MSGQ_SEN_MGR_N_NUM 8
#define MSGQ_SEN_MGR_H_QUE_DRM 0xffffffff
#define MSGQ_SEN_MGR_H_SIZE 0
#define MSGQ_SEN_MGR_H_NUM 0
#endif /* MSGQ_ID_H_INCLUDED */
