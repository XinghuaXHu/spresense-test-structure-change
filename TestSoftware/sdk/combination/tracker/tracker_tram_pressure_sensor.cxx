/****************************************************************************
 * demo/collet_box/tracker/tracker_tram_pressure_sensor.cxx
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
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAPRESSES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAPRESSE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <nuttx/config.h>
#include <sdk/config.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include "tracker_tram_pressure_sensor.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#ifndef DEMO_COLLET_TRACKER_PRESSURE_DEVNAME
#  define DEMO_COLLET_TRACKER_PRESSURE_DEVNAME "/dev/press0"
#endif

#ifndef DEMO_COLLET_TRACKER_PRESS_SIGNO
#  define DEMO_COLLET_TRACKER_PRESS_SIGNO 11
#endif

/* task priority */

#define SENSING_TASK_PRIORITY  150

/* value check macros */

#define CHECK_FUNC_RET(func)                                            \
  do {                                                                  \
    if ((func) < 0) {                                                   \
      printf("return error, %s, %d\n", __FUNCTION__, __LINE__);         \
      return -1;                                                        \
    }                                                                   \
  } while(0)

#define CHECK_FUNC_GOTO(func, error_tag)                                \
  do {                                                                  \
    if ((func) < 0) {                                                   \
      printf("return error, %s, %d\n", __FUNCTION__, __LINE__);         \
      goto error_tag;                                                   \
    }                                                                   \
  } while(0)

#define CHECK_TRUE_RET(expr)                                            \
  do {                                                                  \
    if (!(expr)) {                                                      \
      printf("check failed. %s, %d\n", __FUNCTION__, __LINE__);         \
      return -1;                                                        \
    }                                                                   \
  } while(0)

#define CHECK_TRUE_GOTO(expr, failed)                                   \
  do {                                                                  \
    if (!(expr)) {                                                      \
      printf("check failed. %s, %d\n", __FUNCTION__, __LINE__);         \
      goto failed;                                                      \
    }                                                                   \
  } while(0)


#define CHECK_NULL_RET(expr)                                            \
  do {                                                                  \
    if (expr == NULL) {                                                 \
      printf("check failed. %s, %d\n", __FUNCTION__, __LINE__);         \
      return -1;                                                        \
    }                                                                   \
  } while(0)

#define CHECK_NULL_GOTO(expr, failed)                                   \
  do {                                                                  \
    if (expr == NULL) {                                                 \
      printf("check failed. %s, %d\n", __FUNCTION__, __LINE__);         \
      goto failed;                                                      \
    }                                                                   \
  } while(0)

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static int PressureSensorSetupScu(PressureSensor* sensor)
{
  struct scufifo_wm_s   wm;
  
  /* Get BMP280 sensitivity adjustment value */

  CHECK_FUNC_RET(ioctl(sensor->fd, SNIOC_GETADJ, (unsigned long)(uintptr_t)&sensor->sens_adj));

  /* Set adjustment value to Barometer class */

  sensor->owner->setAdjustParam(&sensor->sens_adj);

  /* Set FIFO size to 3 bytes * 8 Hz = 24 */

  CHECK_FUNC_RET(ioctl(sensor->fd, SCUIOC_SETFIFO, sizeof(struct bmp280_meas_s) * PRESSURE_WATERMARK_NUM));

  /* Set sequencer sampling rate 8 Hz
   * (if config CXD56_SCU_PREDIV = 64)
   * 32768 / 64 / (2 ^ 6) = 8
   */

  CHECK_FUNC_RET(ioctl(sensor->fd, SCUIOC_SETSAMPLE, 6));

  /* set watermark */

  wm.signo     = DEMO_COLLET_TRACKER_PRESS_SIGNO;
  wm.ts        = &sensor->wm_ts;
  wm.watermark = PRESSURE_WATERMARK_NUM;

  CHECK_FUNC_RET(ioctl(sensor->fd, SCUIOC_SETWATERMARK, (unsigned long)(uintptr_t)&wm));

  return 0;
}

/*--------------------------------------------------------------------*/
static int PressureNotifyData(PressureSensor* sensor, MemMgrLite::MemHandle &mh_src, MemMgrLite::MemHandle &mh_dst)
{
  struct bmp280_meas_s *p_src = reinterpret_cast<struct bmp280_meas_s*>(mh_src.getVa());
  int32_t              *p_dst = reinterpret_cast<int32_t*>(mh_dst.getVa());
  
  for (int i = 0; i < PRESSURE_WATERMARK_NUM; i++, p_src++, p_dst++)
    {
      *p_dst = (int32_t)((((uint32_t)(p_src->msb)) << 12)
        | ((uint32_t)p_src->lsb << 4)
        | ((uint32_t)p_src->xlsb >> 4));
    }

  if ((sensor->handler != NULL) && (sensor->stopped != 1))
    {
      sensor->handler(sensor->context, mh_dst);
    }

  return 0;
}

/*--------------------------------------------------------------------*/
extern "C" void *PressureSensorReceivingThread(FAR void *arg)
{
  int                  ret;
  char                *p_src;
  struct siginfo       value;
  struct timespec      timeout;
  PressureSensor      *sensor;

  sensor = reinterpret_cast<PressureSensor*>(arg);
  
  /* setup scu */

  ret = PressureSensorSetupScu(sensor);
  if (ret != 0)
    {
      ASSERT(0);
    }

 /* start sequencer */

  ioctl(sensor->fd, SCUIOC_START, 0);
  
  /* set timeout 6 seconds, SCU may send signal every 5 second. */

  timeout.tv_sec  = 6;
  timeout.tv_nsec = 0;

  while(!sensor->stopped)
    {
      ret = sigtimedwait(&sensor->sig_set, &value, &timeout);
      if (ret < 0)
        {
          continue;
        }

      /* get MemHandle */
      
      MemMgrLite::MemHandle mh_src;
      MemMgrLite::MemHandle mh_dst;
      if (mh_src.allocSeg(PRESS_DATA_BUF_POOL, (sizeof(struct bmp280_meas_s)*PRESSURE_WATERMARK_NUM)) != ERR_OK)
        {
          ASSERT(0);
        }

      /* set physical address.(The addess specified for the SCU must be a physical address) */

      p_src = reinterpret_cast<char *>(mh_src.getPa());
      
      if (mh_dst.allocSeg(PRESS_DATA_BUF_POOL, (sizeof(uint32_t)*PRESSURE_WATERMARK_NUM)) != ERR_OK)
        {
          ASSERT(0);
        }
      
      ret = read(sensor->fd, p_src, sizeof(struct bmp280_meas_s) * PRESSURE_WATERMARK_NUM);
      if (ret != (sizeof(struct bmp280_meas_s) * PRESSURE_WATERMARK_NUM))
        {
          ASSERT(0);
        }

      PressureNotifyData(sensor, mh_src, mh_dst);
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int PressureSensorCreate(PressureSensor** sensor)
{
  CHECK_NULL_RET(sensor);

  *sensor = (PressureSensor *)malloc(sizeof(PressureSensor));
  memset(*sensor, 0 , sizeof(PressureSensor));

  return 0;
}

/*--------------------------------------------------------------------*/
int PressureSensorRegisterHandler(PressureSensor* sensor, PressureEventHandler handler, uint32_t context)
{
  sensor->handler = handler;
  sensor->context = context;

  return 0;
}

/*--------------------------------------------------------------------*/
int PressureSensorStartSensing(PressureSensor *sensor)
{
  pthread_attr_t attr;
  struct sched_param sch_param;

  /* open driver */

  CHECK_FUNC_RET(sensor->fd = open(DEMO_COLLET_TRACKER_PRESSURE_DEVNAME, O_RDONLY));

  /* set standby duration*/

  CHECK_FUNC_RET(ioctl(sensor->fd, SNIOC_SETSTB, BMP280_STANDBY_63_MS));
  
  /* set status */

  sensor->stopped = 0;

  /* add signal */

  sigemptyset(&sensor->sig_set);
  sigaddset(&sensor->sig_set, DEMO_COLLET_TRACKER_PRESS_SIGNO);
  
  /* Create receive thread */

  (void)pthread_attr_init(&attr);
  sch_param.sched_priority = SENSING_TASK_PRIORITY;
  CHECK_FUNC_RET(pthread_attr_setschedparam(&attr, &sch_param));

  CHECK_FUNC_RET(pthread_create(&sensor->thread_id, &attr, PressureSensorReceivingThread, (pthread_addr_t)sensor));

  return 0;
}

/*--------------------------------------------------------------------*/
int PressureSensorStopSensing(PressureSensor* sensor)
{
  void *value;

  CHECK_NULL_RET(sensor);
  
  if (sensor->fd == 0)
    {
      /* already stopped */
      
      return 0;
    }

  /* set status */

  sensor->stopped = 1;

  /* cancel thread */

  pthread_cancel(sensor->thread_id);
  pthread_join(sensor->thread_id, &value);

  /* delete signal */

  sigdelset(&sensor->sig_set, DEMO_COLLET_TRACKER_PRESS_SIGNO);

  /* close driver */

  close(sensor->fd);

  return 0;
}

/*--------------------------------------------------------------------*/
int PressureSensorDestroy(PressureSensor* sensor)
{
  free(sensor);
  return 0;
}
