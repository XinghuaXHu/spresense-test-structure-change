/*
 * pool_layout.h -- MemMgrLite layout definition.
 *
 * This file was created by mem_layout.conf
 * !!! CAUTION! don't edit this file manually !!!
 *
 *   Notes: (C) Copyright 2014 Sony Corporation
 */
#ifndef POOL_LAYOUT_H_INCLUDED
#define POOL_LAYOUT_H_INCLUDED

#include "memutils/memory_manager/MemMgrTypes.h"

namespace MemMgrLite {

MemPool* static_pools[NUM_MEM_POOLS];

extern const PoolAttr MemoryPoolLayouts[NUM_MEM_LAYOUTS][NUM_MEM_POOLS] = {
 {/* Layout:0 */
  /* pool_ID          type       seg fence  addr        size         */
  { SENSOR_DSP_CMD_BUF_POOL, BasicType,   8, false, 0x000e0000, 0x00000380 },  /* SENSOR_WORK_AREA */
  { ACCEL_DATA_BUF_POOL, BasicType,   8, false, 0x000e0380, 0x00007800 },  /* SENSOR_WORK_AREA */
  { MAG_DATA_BUF_POOL, BasicType,   8, false, 0x000e7b80, 0x00007800 },  /* SENSOR_WORK_AREA */
  { PRESS_DATA_BUF_POOL, BasicType,   8, false, 0x000ef380, 0x00000500 },  /* SENSOR_WORK_AREA */
  { TEMP_DATA_BUF_POOL, BasicType,   8, false, 0x000ef880, 0x00000500 },  /* SENSOR_WORK_AREA */
 },
}; /* end of MemoryPoolLayouts */

}  /* end of namespace MemMgrLite */

#endif /* POOL_LAYOUT_H_INCLUDED */
