/*
 * mem_layout.h -- MemMgrLite layout definition.
 *
 * This file was created by mem_layout.conf
 * !!! CAUTION! don't edit this file manually !!!
 *
 *   Notes: (C) Copyright 2014 Sony Corporation
 */
#ifndef MEM_LAYOUT_H_INCLUDED
#define MEM_LAYOUT_H_INCLUDED

/*
 * Memory Manager Configurations
 */
//#define USE_MEMMGR_FENCE

/*
 * User defined constants
 */

/*
 * Memory devices
 */
/* AUD_SRAM: type=RAM, use=0x0003f440, remainder=0x00000bc0 */
#define AUD_SRAM_ADDR	0x000c0000
#define AUD_SRAM_SIZE	0x00040000

/* RESERVED: type=RAM, use=0x00000080, remainder=0x0003ff80 */
#define RESERVED_ADDR	0x0e000000
#define RESERVED_SIZE	0x00040000

/*
 * Fixed areas
 */
#define AUDIO_WORK_AREA_ALIGN   0x00020000
#define AUDIO_WORK_AREA_ADDR    0x000c0000
#define AUDIO_WORK_AREA_DRM     0x000c0000 /* _DRM is obsolete macro. to use _ADDR */
#define AUDIO_WORK_AREA_SIZE    0x0003c000

#define MSG_QUE_AREA_ALIGN   0x00000040
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
#define MEMMGR_MAX_WORK_SIZE  0x00000024

/*
 * Pool IDs
 */
#define NULL_POOL	0
#define DEC_APU_CMD_POOL	1

#define NUM_MEM_LAYOUTS	1
#define NUM_MEM_POOLS	2


/*
 * Pool areas
 */
/* Layout0: */
#define MEMMGR_L0_WORK_SIZE   0x00000024

/* Skip 0x0004 bytes for alignment. */
#define L0_DEC_APU_CMD_POOL_ALIGN    0x00000008
#define L0_DEC_APU_CMD_POOL_L_FENCE  0x000c0004
#define L0_DEC_APU_CMD_POOL_ADDR     0x000c0008
#define L0_DEC_APU_CMD_POOL_SIZE     0x00000398
#define L0_DEC_APU_CMD_POOL_U_FENCE  0x000c03a0
#define L0_DEC_APU_CMD_POOL_NUM_SEG  0x0000000a
#define L0_DEC_APU_CMD_POOL_SEG_SIZE 0x0000005c

/* Remainder AUDIO_WORK_AREA=0x0003bc5c */

#endif /* MEM_LAYOUT_H_INCLUDED */
