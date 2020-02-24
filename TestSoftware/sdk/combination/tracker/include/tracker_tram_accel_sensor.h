/****************************************************************************
 * demo/collet_box/tracker/include/tracker_tram_accel_sensor.h
 *
 *   Copyright (C) 2017 Sony Corporation. All rights reserved.
 *   Author: Satoru Hata <Satoru.Hata@sony.com>
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
 * 3. Neither the name NuttX nor Sony nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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

#ifndef __DEMO_COLLET_BOX_TRACKER_TRACKER_TRAM_ACCEL_SENSOR_H
#define __DEMO_COLLET_BOX_TRACKER_TRACKER_TRAM_ACCEL_SENSOR_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <pthread.h>
#include <arch/chip/cxd56_scu.h>
#include "memutils/memory_manager/MemHandle.h"
#include "mem_layout.h"

#include "sensing/transport_mode.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Macros
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/
typedef enum
{
  ACCEL_EV_MF = 0,     /* event of math-function */
  ACCEL_EV_WM,         /* event of watermark     */
  ACCEL_EV_NUM
} AccelEvent;

typedef struct
{
  float accel_x;        /* X axis standard gravity acceleration.[G] */
  float accel_y;        /* Y axis standard gravity acceleration.[G] */
  float accel_z;        /* Z axis standard gravity acceleration.[G] */
} AccelDOF;

typedef int (*AccelEventHandler) (uint32_t context, AccelEvent ev_type, MemMgrLite::MemHandle &mh);

typedef struct
{
  AccelEventHandler     handler;      /* Indicates the event handler of the acceleration sensor.               */
  uint32_t              context;      /* Indicates the context of the acceleration sensor.                     */
  uint32_t              stopped;      /* Please stand when stopping the acceleration sensor (physical sensor). */
  pthread_t             thread_id;    /* Indicates the ID of receiving thread.                                 */
  int                   fd;           /* Indicates the file discriptor of driver.                              */
  sigset_t              sig_set;      /* Indicates the information of signal from driver.                      */
  struct scutimestamp_s wm_ts;        /* Indicates the time stamp information of water mark.                   */
  struct scuev_arg_s    ev_arg;       /* Indicates the information of scu event.                               */
} AccelSensor;


/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
int AccelSensorCreate(AccelSensor** sensor);
int AccelSensorRegisterHandler(AccelSensor* sensor, AccelEventHandler handler, uint32_t context);
int AccelSensorStartSensing(AccelSensor *sensor, ScuSettings* settings);
int AccelSensorDestroy(AccelSensor* sensor);
int AccelSensorStopSensing(AccelSensor* sensor);
int AccelSensorChangeScuSetting(AccelSensor* sensor, ScuSettings* settings);

#endif /* __DEMO_COLLET_BOX_TRACKER_TRACKER_TRAM_ACCEL_SENSOR_H */
