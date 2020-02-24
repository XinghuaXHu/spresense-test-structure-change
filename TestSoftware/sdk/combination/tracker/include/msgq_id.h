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

/* Message area size: 456 bytes */
#define MSGQ_TOP_DRM	0xfc400
#define MSGQ_END_DRM	0xfc5c8

/* Message area fill value after message poped */
#define MSG_FILL_VALUE_AFTER_POP	0x0

/* Message parameter type match check */
#define MSG_PARAM_TYPE_MATCH_CHECK	false

/* Message queue pool IDs */
#define MSGQ_NULL	0
#define MSGQ_SEN_MGR	1
#define NUM_MSGQ_POOLS	2

/* User defined constants */

/************************************************************************/
#define MSGQ_SEN_MGR_QUE_BLOCK_DRM	0xfc444
#define MSGQ_SEN_MGR_N_QUE_DRM	0xfc488
#define MSGQ_SEN_MGR_N_SIZE	40
#define MSGQ_SEN_MGR_N_NUM	8
#define MSGQ_SEN_MGR_H_QUE_DRM	0xffffffff
#define MSGQ_SEN_MGR_H_SIZE	0
#define MSGQ_SEN_MGR_H_NUM	0
#endif /* MSGQ_ID_H_INCLUDED */
