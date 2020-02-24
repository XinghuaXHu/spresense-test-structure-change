/*
 * msgq_id.h -- Message queue pool ID and macro definition.
 *
 * This file was created by msgq_layout.conf
 * !!! CAUTION! don't edit this file manually !!!
 *
 *   Notes: (C) Copyright 2012 Sony Corporation
 */

#ifndef MSGQ_ID_H_INCLUDED
#define MSGQ_ID_H_INCLUDED

/* Message area size: 544 bytes */
#define MSGQ_TOP_DRM	0xfc000
#define MSGQ_END_DRM	0xfc220

/* Message area fill value after message poped */
#define MSG_FILL_VALUE_AFTER_POP	0x0

/* Message parameter type match check */
#define MSG_PARAM_TYPE_MATCH_CHECK	false

/* Message queue pool IDs */
#define MSGQ_NULL	0
#define MSGQ_AUD_DSP	1
#define MSGQ_AUD_SRC	2
#define NUM_MSGQ_POOLS	3

/* User defined constants */

/************************************************************************/
#define MSGQ_AUD_DSP_QUE_BLOCK_DRM	0xfc044
#define MSGQ_AUD_DSP_N_QUE_DRM	0xfc0cc
#define MSGQ_AUD_DSP_N_SIZE	20
#define MSGQ_AUD_DSP_N_NUM	5
#define MSGQ_AUD_DSP_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_DSP_H_SIZE	0
#define MSGQ_AUD_DSP_H_NUM	0
/************************************************************************/
#define MSGQ_AUD_SRC_QUE_BLOCK_DRM	0xfc088
#define MSGQ_AUD_SRC_N_QUE_DRM	0xfc130
#define MSGQ_AUD_SRC_N_SIZE	48
#define MSGQ_AUD_SRC_N_NUM	5
#define MSGQ_AUD_SRC_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_SRC_H_SIZE	0
#define MSGQ_AUD_SRC_H_NUM	0
#endif /* MSGQ_ID_H_INCLUDED */
