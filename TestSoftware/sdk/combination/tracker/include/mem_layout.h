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
//#define USE_MEMMGR_FENCE /* TODO This define macro should be set by Kconfig. */

/*
 * User defined constants
 */

/*
 * Memory devices
 */
/* SHM_SRAM: type=RAM, use=0x0001f940, remainder=0x000006c0 */
#define SHM_SRAM_ADDR	0x000e0000
#define SHM_SRAM_SIZE	0x00020000

/* RESERVED: type=RAM, use=0x00000080, remainder=0x0003ff80 */
#define RESERVED_ADDR	0x0e000000
#define RESERVED_SIZE	0x00040000

/*
 * Fixed areas
 */
#define SENSOR_WORK_AREA_ALIGN   0x00020000
#define SENSOR_WORK_AREA_ADDR    0x000e0000
#define SENSOR_WORK_AREA_DRM     0x000e0000 /* _DRM is obsolete macro. to use _ADDR */
#define SENSOR_WORK_AREA_SIZE    0x0001c400

#define MSG_QUE_AREA_ALIGN   0x00000040
#define MSG_QUE_AREA_ADDR    0x000fc400
#define MSG_QUE_AREA_DRM     0x000fc400 /* _DRM is obsolete macro. to use _ADDR */
#define MSG_QUE_AREA_SIZE    0x00003240

#define MEMMGR_WORK_AREA_ALIGN   0x00000008
#define MEMMGR_WORK_AREA_ADDR    0x000ff640
#define MEMMGR_WORK_AREA_DRM     0x000ff640 /* _DRM is obsolete macro. to use _ADDR */
#define MEMMGR_WORK_AREA_SIZE    0x00000200

#define MEMMGR_DATA_AREA_ALIGN   0x00000008
#define MEMMGR_DATA_AREA_ADDR    0x000ff840
#define MEMMGR_DATA_AREA_DRM     0x000ff840 /* _DRM is obsolete macro. to use _ADDR */
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
#define MEMMGR_MAX_WORK_SIZE  0x000000a0

/*
 * Pool IDs
 */
#define NULL_POOL	0
#define SENSOR_DSP_CMD_BUF_POOL	1
#define ACCEL_DATA_BUF_POOL	2
#define MAG_DATA_BUF_POOL	3
#define PRESS_DATA_BUF_POOL	4
#define TEMP_DATA_BUF_POOL	5

#define NUM_MEM_LAYOUTS	1
#define NUM_MEM_POOLS	6


/*
 * Pool areas
 */
/* Layout0: */
#define MEMMGR_L0_WORK_SIZE   0x000000a0

#define L0_SENSOR_DSP_CMD_BUF_POOL_ALIGN    0x00000008
#define L0_SENSOR_DSP_CMD_BUF_POOL_ADDR     0x000e0000
#define L0_SENSOR_DSP_CMD_BUF_POOL_SIZE     0x00000380
#define L0_SENSOR_DSP_CMD_BUF_POOL_NUM_SEG  0x00000008
#define L0_SENSOR_DSP_CMD_BUF_POOL_SEG_SIZE 0x00000070

#define L0_ACCEL_DATA_BUF_POOL_ALIGN    0x00000008
#define L0_ACCEL_DATA_BUF_POOL_ADDR     0x000e0380
#define L0_ACCEL_DATA_BUF_POOL_SIZE     0x00007800
#define L0_ACCEL_DATA_BUF_POOL_NUM_SEG  0x00000008
#define L0_ACCEL_DATA_BUF_POOL_SEG_SIZE 0x00000f00

#define L0_MAG_DATA_BUF_POOL_ALIGN    0x00000008
#define L0_MAG_DATA_BUF_POOL_ADDR     0x000e7b80
#define L0_MAG_DATA_BUF_POOL_SIZE     0x00007800
#define L0_MAG_DATA_BUF_POOL_NUM_SEG  0x00000008
#define L0_MAG_DATA_BUF_POOL_SEG_SIZE 0x00000f00

#define L0_PRESS_DATA_BUF_POOL_ALIGN    0x00000008
#define L0_PRESS_DATA_BUF_POOL_ADDR     0x000ef380
#define L0_PRESS_DATA_BUF_POOL_SIZE     0x00000500
#define L0_PRESS_DATA_BUF_POOL_NUM_SEG  0x00000008
#define L0_PRESS_DATA_BUF_POOL_SEG_SIZE 0x000000a0

#define L0_TEMP_DATA_BUF_POOL_ALIGN    0x00000008
#define L0_TEMP_DATA_BUF_POOL_ADDR     0x000ef880
#define L0_TEMP_DATA_BUF_POOL_SIZE     0x00000500
#define L0_TEMP_DATA_BUF_POOL_NUM_SEG  0x00000008
#define L0_TEMP_DATA_BUF_POOL_SEG_SIZE 0x000000a0

/* Remainder SENSOR_WORK_AREA=0x0000c680 */

#endif /* MEM_LAYOUT_H_INCLUDED */
