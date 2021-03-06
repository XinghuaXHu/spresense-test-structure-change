/*
 * msgq_id.h -- Message queue pool ID and macro definition.
 *
 * This file was created by sdk_es_msgq_layout.conf
 * !!! CAUTION! don't edit this file manually !!!
 *
 *   Notes: (C) Copyright 2012 Sony Corporation
 */

#ifndef MSGQ_ID_H_INCLUDED
#define MSGQ_ID_H_INCLUDED

#include "spl_id.h"

/* Message area size: 10824 bytes */
#define MSGQ_TOP_DRM	0xd17c000
#define MSGQ_END_DRM	0xd17ea48

/* Message area fill value after message poped */
#define MSG_FILL_VALUE_AFTER_POP	0x0

/* Message parameter type match check */
#define MSG_PARAM_TYPE_MATCH_CHECK	false

/* Message queue pool IDs */
#define MSGQ_NULL	0
#define MSGQ_AUD_MGR	1
#define MSGQ_AUD_PLY	2
#define MSGQ_AUD_APU	3
#define MSGQ_AUD_ADN	4
#define MSGQ_AUD_CAP_COMP	5
#define MSGQ_AUD_RECORDER	6
#define MSGQ_AUD_MEDIA_REC_SINK	7
#define MSGQ_AUD_OUTPUT_MIX	8
#define MSGQ_AUD_SOUND_EFFECT	9
#define MSGQ_DSP_AUD_DRV_USER	10
#define MSGQ_DSP_AUD_BB_MGR	11
#define MSGQ_DSP_AUD_DMA_CTRL0	12
#define MSGQ_DSP_AUD_DMA_CTRL1	13
#define MSGQ_DSP_AUD_DMA_CTRL2	14
#define MSGQ_DSP_AUD_DMA_CTRL3	15
#define MSGQ_DSP_AUD_DMA_CTRL4	16
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_OBJECT_TASK	17
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_COMPONENT_TASK	18
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_OBJECT_TASK	19
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_COMPONENT_TASK	20
#define MSGQ_AUD_APP	21
#define MSGQ_AUD_NGL_SPU	22
#define MSGQ_AUD_APUS_DRV	23
#define NUM_MSGQ_POOLS	24

/* User defined constants */

/************************************************************************/
#define MSGQ_AUD_MGR_QUE_BLOCK_DRM	0xd17c040
#define MSGQ_AUD_MGR_N_QUE_DRM	0xd17c600
#define MSGQ_AUD_MGR_N_SIZE	88
#define MSGQ_AUD_MGR_N_NUM	2
#define MSGQ_AUD_MGR_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_MGR_H_SIZE	0
#define MSGQ_AUD_MGR_H_NUM	0
#define MSGQ_AUD_MGR_OWNER	CPUID_CORE3
#define MSGQ_AUD_MGR_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_PLY_QUE_BLOCK_DRM	0xd17c080
#define MSGQ_AUD_PLY_N_QUE_DRM	0xd17c6c0
#define MSGQ_AUD_PLY_N_SIZE	88
#define MSGQ_AUD_PLY_N_NUM	5
#define MSGQ_AUD_PLY_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_PLY_H_SIZE	0
#define MSGQ_AUD_PLY_H_NUM	0
#define MSGQ_AUD_PLY_OWNER	CPUID_CORE3
#define MSGQ_AUD_PLY_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_APU_QUE_BLOCK_DRM	0xd17c0c0
#define MSGQ_AUD_APU_N_QUE_DRM	0xd17c880
#define MSGQ_AUD_APU_N_SIZE	20
#define MSGQ_AUD_APU_N_NUM	5
#define MSGQ_AUD_APU_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_APU_H_SIZE	0
#define MSGQ_AUD_APU_H_NUM	0
#define MSGQ_AUD_APU_OWNER	CPUID_CORE3
#define MSGQ_AUD_APU_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_ADN_QUE_BLOCK_DRM	0xd17c100
#define MSGQ_AUD_ADN_N_QUE_DRM	0xd17c900
#define MSGQ_AUD_ADN_N_SIZE	32
#define MSGQ_AUD_ADN_N_NUM	2
#define MSGQ_AUD_ADN_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_ADN_H_SIZE	0
#define MSGQ_AUD_ADN_H_NUM	0
#define MSGQ_AUD_ADN_OWNER	CPUID_CORE3
#define MSGQ_AUD_ADN_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_CAP_COMP_QUE_BLOCK_DRM	0xd17c140
#define MSGQ_AUD_CAP_COMP_N_QUE_DRM	0xd17c940
#define MSGQ_AUD_CAP_COMP_N_SIZE	88
#define MSGQ_AUD_CAP_COMP_N_NUM	4
#define MSGQ_AUD_CAP_COMP_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_CAP_COMP_H_SIZE	0
#define MSGQ_AUD_CAP_COMP_H_NUM	0
#define MSGQ_AUD_CAP_COMP_OWNER	CPUID_CORE3
#define MSGQ_AUD_CAP_COMP_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_RECORDER_QUE_BLOCK_DRM	0xd17c180
#define MSGQ_AUD_RECORDER_N_QUE_DRM	0xd17cac0
#define MSGQ_AUD_RECORDER_N_SIZE	88
#define MSGQ_AUD_RECORDER_N_NUM	5
#define MSGQ_AUD_RECORDER_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_RECORDER_H_SIZE	0
#define MSGQ_AUD_RECORDER_H_NUM	0
#define MSGQ_AUD_RECORDER_OWNER	CPUID_CORE3
#define MSGQ_AUD_RECORDER_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_MEDIA_REC_SINK_QUE_BLOCK_DRM	0xd17c1c0
#define MSGQ_AUD_MEDIA_REC_SINK_N_QUE_DRM	0xd17cc80
#define MSGQ_AUD_MEDIA_REC_SINK_N_SIZE	32
#define MSGQ_AUD_MEDIA_REC_SINK_N_NUM	5
#define MSGQ_AUD_MEDIA_REC_SINK_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_MEDIA_REC_SINK_H_SIZE	0
#define MSGQ_AUD_MEDIA_REC_SINK_H_NUM	0
#define MSGQ_AUD_MEDIA_REC_SINK_OWNER	CPUID_CORE3
#define MSGQ_AUD_MEDIA_REC_SINK_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_OUTPUT_MIX_QUE_BLOCK_DRM	0xd17c200
#define MSGQ_AUD_OUTPUT_MIX_N_QUE_DRM	0xd17cd40
#define MSGQ_AUD_OUTPUT_MIX_N_SIZE	88
#define MSGQ_AUD_OUTPUT_MIX_N_NUM	5
#define MSGQ_AUD_OUTPUT_MIX_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_OUTPUT_MIX_H_SIZE	0
#define MSGQ_AUD_OUTPUT_MIX_H_NUM	0
#define MSGQ_AUD_OUTPUT_MIX_OWNER	CPUID_CORE3
#define MSGQ_AUD_OUTPUT_MIX_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_SOUND_EFFECT_QUE_BLOCK_DRM	0xd17c240
#define MSGQ_AUD_SOUND_EFFECT_N_QUE_DRM	0xd17cf00
#define MSGQ_AUD_SOUND_EFFECT_N_SIZE	88
#define MSGQ_AUD_SOUND_EFFECT_N_NUM	5
#define MSGQ_AUD_SOUND_EFFECT_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_SOUND_EFFECT_H_SIZE	0
#define MSGQ_AUD_SOUND_EFFECT_H_NUM	0
#define MSGQ_AUD_SOUND_EFFECT_OWNER	CPUID_CORE3
#define MSGQ_AUD_SOUND_EFFECT_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_DSP_AUD_DRV_USER_QUE_BLOCK_DRM	0xd17c280
#define MSGQ_DSP_AUD_DRV_USER_N_QUE_DRM	0xd17d0c0
#define MSGQ_DSP_AUD_DRV_USER_N_SIZE	40
#define MSGQ_DSP_AUD_DRV_USER_N_NUM	16
#define MSGQ_DSP_AUD_DRV_USER_H_QUE_DRM	0xffffffff
#define MSGQ_DSP_AUD_DRV_USER_H_SIZE	0
#define MSGQ_DSP_AUD_DRV_USER_H_NUM	0
#define MSGQ_DSP_AUD_DRV_USER_OWNER	CPUID_CORE3
#define MSGQ_DSP_AUD_DRV_USER_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_DSP_AUD_BB_MGR_QUE_BLOCK_DRM	0xd17c2c0
#define MSGQ_DSP_AUD_BB_MGR_N_QUE_DRM	0xd17d340
#define MSGQ_DSP_AUD_BB_MGR_N_SIZE	40
#define MSGQ_DSP_AUD_BB_MGR_N_NUM	16
#define MSGQ_DSP_AUD_BB_MGR_H_QUE_DRM	0xffffffff
#define MSGQ_DSP_AUD_BB_MGR_H_SIZE	0
#define MSGQ_DSP_AUD_BB_MGR_H_NUM	0
#define MSGQ_DSP_AUD_BB_MGR_OWNER	CPUID_CORE3
#define MSGQ_DSP_AUD_BB_MGR_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_DSP_AUD_DMA_CTRL0_QUE_BLOCK_DRM	0xd17c300
#define MSGQ_DSP_AUD_DMA_CTRL0_N_QUE_DRM	0xd17d5c0
#define MSGQ_DSP_AUD_DMA_CTRL0_N_SIZE	40
#define MSGQ_DSP_AUD_DMA_CTRL0_N_NUM	16
#define MSGQ_DSP_AUD_DMA_CTRL0_H_QUE_DRM	0xffffffff
#define MSGQ_DSP_AUD_DMA_CTRL0_H_SIZE	0
#define MSGQ_DSP_AUD_DMA_CTRL0_H_NUM	0
#define MSGQ_DSP_AUD_DMA_CTRL0_OWNER	CPUID_CORE3
#define MSGQ_DSP_AUD_DMA_CTRL0_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_DSP_AUD_DMA_CTRL1_QUE_BLOCK_DRM	0xd17c340
#define MSGQ_DSP_AUD_DMA_CTRL1_N_QUE_DRM	0xd17d840
#define MSGQ_DSP_AUD_DMA_CTRL1_N_SIZE	40
#define MSGQ_DSP_AUD_DMA_CTRL1_N_NUM	16
#define MSGQ_DSP_AUD_DMA_CTRL1_H_QUE_DRM	0xffffffff
#define MSGQ_DSP_AUD_DMA_CTRL1_H_SIZE	0
#define MSGQ_DSP_AUD_DMA_CTRL1_H_NUM	0
#define MSGQ_DSP_AUD_DMA_CTRL1_OWNER	CPUID_CORE3
#define MSGQ_DSP_AUD_DMA_CTRL1_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_DSP_AUD_DMA_CTRL2_QUE_BLOCK_DRM	0xd17c380
#define MSGQ_DSP_AUD_DMA_CTRL2_N_QUE_DRM	0xd17dac0
#define MSGQ_DSP_AUD_DMA_CTRL2_N_SIZE	40
#define MSGQ_DSP_AUD_DMA_CTRL2_N_NUM	16
#define MSGQ_DSP_AUD_DMA_CTRL2_H_QUE_DRM	0xffffffff
#define MSGQ_DSP_AUD_DMA_CTRL2_H_SIZE	0
#define MSGQ_DSP_AUD_DMA_CTRL2_H_NUM	0
#define MSGQ_DSP_AUD_DMA_CTRL2_OWNER	CPUID_CORE3
#define MSGQ_DSP_AUD_DMA_CTRL2_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_DSP_AUD_DMA_CTRL3_QUE_BLOCK_DRM	0xd17c3c0
#define MSGQ_DSP_AUD_DMA_CTRL3_N_QUE_DRM	0xd17dd40
#define MSGQ_DSP_AUD_DMA_CTRL3_N_SIZE	40
#define MSGQ_DSP_AUD_DMA_CTRL3_N_NUM	16
#define MSGQ_DSP_AUD_DMA_CTRL3_H_QUE_DRM	0xffffffff
#define MSGQ_DSP_AUD_DMA_CTRL3_H_SIZE	0
#define MSGQ_DSP_AUD_DMA_CTRL3_H_NUM	0
#define MSGQ_DSP_AUD_DMA_CTRL3_OWNER	CPUID_CORE3
#define MSGQ_DSP_AUD_DMA_CTRL3_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_DSP_AUD_DMA_CTRL4_QUE_BLOCK_DRM	0xd17c400
#define MSGQ_DSP_AUD_DMA_CTRL4_N_QUE_DRM	0xd17dfc0
#define MSGQ_DSP_AUD_DMA_CTRL4_N_SIZE	40
#define MSGQ_DSP_AUD_DMA_CTRL4_N_NUM	16
#define MSGQ_DSP_AUD_DMA_CTRL4_H_QUE_DRM	0xffffffff
#define MSGQ_DSP_AUD_DMA_CTRL4_H_SIZE	0
#define MSGQ_DSP_AUD_DMA_CTRL4_H_NUM	0
#define MSGQ_DSP_AUD_DMA_CTRL4_OWNER	CPUID_CORE3
#define MSGQ_DSP_AUD_DMA_CTRL4_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_OBJECT_TASK_QUE_BLOCK_DRM	0xd17c440
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_OBJECT_TASK_N_QUE_DRM	0xd17e240
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_OBJECT_TASK_N_SIZE	40
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_OBJECT_TASK_N_NUM	8
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_OBJECT_TASK_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_OBJECT_TASK_H_SIZE	0
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_OBJECT_TASK_H_NUM	0
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_OBJECT_TASK_OWNER	CPUID_CORE3
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_OBJECT_TASK_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_COMPONENT_TASK_QUE_BLOCK_DRM	0xd17c480
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_COMPONENT_TASK_N_QUE_DRM	0xd17e380
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_COMPONENT_TASK_N_SIZE	40
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_COMPONENT_TASK_N_NUM	8
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_COMPONENT_TASK_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_COMPONENT_TASK_H_SIZE	0
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_COMPONENT_TASK_H_NUM	0
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_COMPONENT_TASK_OWNER	CPUID_CORE3
#define MSGQ_AUD_VOICE_RECOGNITION_TRIGGER_COMPONENT_TASK_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_OBJECT_TASK_QUE_BLOCK_DRM	0xd17c4c0
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_OBJECT_TASK_N_QUE_DRM	0xd17e4c0
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_OBJECT_TASK_N_SIZE	40
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_OBJECT_TASK_N_NUM	8
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_OBJECT_TASK_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_OBJECT_TASK_H_SIZE	0
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_OBJECT_TASK_H_NUM	0
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_OBJECT_TASK_OWNER	CPUID_CORE3
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_OBJECT_TASK_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_COMPONENT_TASK_QUE_BLOCK_DRM	0xd17c500
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_COMPONENT_TASK_N_QUE_DRM	0xd17e600
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_COMPONENT_TASK_N_SIZE	40
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_COMPONENT_TASK_N_NUM	8
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_COMPONENT_TASK_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_COMPONENT_TASK_H_SIZE	0
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_COMPONENT_TASK_H_NUM	0
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_COMPONENT_TASK_OWNER	CPUID_CORE3
#define MSGQ_AUD_VOICE_RECOGNITION_COMMAND_COMPONENT_TASK_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_APP_QUE_BLOCK_DRM	0xd17c540
#define MSGQ_AUD_APP_N_QUE_DRM	0xd17e740
#define MSGQ_AUD_APP_N_SIZE	40
#define MSGQ_AUD_APP_N_NUM	2
#define MSGQ_AUD_APP_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_APP_H_SIZE	0
#define MSGQ_AUD_APP_H_NUM	0
#define MSGQ_AUD_APP_OWNER	CPUID_CORE3
#define MSGQ_AUD_APP_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_NGL_SPU_QUE_BLOCK_DRM	0xd17c580
#define MSGQ_AUD_NGL_SPU_N_QUE_DRM	0xd17e7c0
#define MSGQ_AUD_NGL_SPU_N_SIZE	88
#define MSGQ_AUD_NGL_SPU_N_NUM	5
#define MSGQ_AUD_NGL_SPU_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_NGL_SPU_H_SIZE	0
#define MSGQ_AUD_NGL_SPU_H_NUM	0
#define MSGQ_AUD_NGL_SPU_OWNER	CPUID_CORE3
#define MSGQ_AUD_NGL_SPU_SPINLOCK	SPL_NULL

/************************************************************************/
#define MSGQ_AUD_APUS_DRV_QUE_BLOCK_DRM	0xd17c5c0
#define MSGQ_AUD_APUS_DRV_N_QUE_DRM	0xd17e980
#define MSGQ_AUD_APUS_DRV_N_SIZE	40
#define MSGQ_AUD_APUS_DRV_N_NUM	5
#define MSGQ_AUD_APUS_DRV_H_QUE_DRM	0xffffffff
#define MSGQ_AUD_APUS_DRV_H_SIZE	0
#define MSGQ_AUD_APUS_DRV_H_NUM	0
#define MSGQ_AUD_APUS_DRV_OWNER	CPUID_CORE3
#define MSGQ_AUD_APUS_DRV_SPINLOCK	SPL_NULL

#endif /* MSGQ_ID_H_INCLUDED */
