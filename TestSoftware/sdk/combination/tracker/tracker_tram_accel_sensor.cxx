/****************************************************************************
 * demo/collet_box/tracker/tracker_tram_accel_sensor.cxx
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

#include <nuttx/sensors/bmi160.h>
#include "tracker_debug.h"
#include "tracker_tram_accel_sensor.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#ifndef DEMO_COLLET_TRACKER_ACCEL_DEVNAME
#  define DEMO_COLLET_TRACKER_ACCEL_DEVNAME "/dev/accel0"
#endif

#ifndef DEMO_COLLET_TRACKER_CHANGE_SCU_SIGNO
#  define DEMO_COLLET_TRACKER_CHANGE_SCU_SIGNO 16
#endif
#ifndef DEMO_COLLET_TRACKER_ACCEL_TM_SIGNO
#  define DEMO_COLLET_TRACKER_ACCEL_TM_SIGNO 15
#endif
#ifndef DEMO_COLLET_TRACKER_ACCEL_WM_SIGNO
#  define DEMO_COLLET_TRACKER_ACCEL_WM_SIGNO 14
#endif
#ifndef DEMO_COLLET_TRACKER_ACCEL_EV_SIGNO
#  define DEMO_COLLET_TRACKER_ACCEL_EV_SIGNO 13
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
      do {                                                              \
        if (!(expr)) {                                                  \
          printf("check failed. %s, %d\n", __FUNCTION__, __LINE__);     \
          goto failed;                                                  \
        }                                                               \
      } while(0)


#define CHECK_NULL_RET(expr)                                            \
  do {                                                                  \
    if (expr == NULL) {                                                 \
      printf("check failed. %s, %d\n", __FUNCTION__, __LINE__);         \
      return -1;                                                        \
    }                                                                   \
  } while(0)

#define CHECK_NULL_GOTO(expr, failed)                                   \
      do {                                                              \
        if (expr == NULL) {                                             \
          printf("check failed. %s, %d\n", __FUNCTION__, __LINE__);     \
          goto failed;                                                  \
        }                                                               \
      } while(0)

/****************************************************************************
 * Private Types
 ****************************************************************************/
typedef struct accel_t three_axis_s;

/****************************************************************************
 * Private Data
 ****************************************************************************/

static ScuSettings* s_scu_settings;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int createOneshotTimer(timer_t* timerid)
{
  struct sigevent notify;

  notify.sigev_notify            = SIGEV_SIGNAL;
  notify.sigev_signo             = DEMO_COLLET_TRACKER_ACCEL_TM_SIGNO;
  notify.sigev_value.sival_int   = 0;

  return timer_create(CLOCK_REALTIME, &notify, timerid);
}

static int startOneshotTimer(timer_t timerid, uint32_t milliseconds)
{
  struct itimerspec timer;

  timer.it_value.tv_sec     = milliseconds / 1000;
  timer.it_value.tv_nsec    = milliseconds % 1000 * 1000 * 1000;
  timer.it_interval.tv_sec  = 0;
  timer.it_interval.tv_nsec = 0;

  return timer_settime(timerid, 0, &timer, NULL);
}

static int deleteOneshotTimer(timer_t timerid)
{
  return timer_delete(timerid);
}

static int setupScu(AccelSensor* sensor, ScuSettings* settings)
{
  /* Set FIFO size */

  CHECK_FUNC_RET(ioctl(sensor->fd, SCUIOC_SETFIFO, sizeof(three_axis_s) * settings->fifosize));

  /* Set sampling rate */

  CHECK_FUNC_RET(ioctl(sensor->fd, SCUIOC_SETSAMPLE, settings->samplingrate));

  /* Set elements */

  if (settings->elements)
    {
      CHECK_FUNC_RET(ioctl(sensor->fd, SCUIOC_SETELEMENTS, settings->elements));
    }

  /* Set MathFunction filter */

  if (settings->mf)
    {
      CHECK_FUNC_RET(ioctl(sensor->fd, SCUIOC_SETFILTER, (unsigned long)(uintptr_t)settings->mf));
    }

  /* Set event */

  if (settings->ev)
    {
      settings->ev->signo = DEMO_COLLET_TRACKER_ACCEL_EV_SIGNO;
      settings->ev->arg   = &sensor->ev_arg;

      CHECK_FUNC_RET(ioctl(sensor->fd, SCUIOC_SETNOTIFY, (unsigned long)(uintptr_t)settings->ev));
    }

  /* Set water mark */

  if (settings->wm)
    {
      settings->wm->signo = DEMO_COLLET_TRACKER_ACCEL_WM_SIGNO;
      settings->wm->ts    = &sensor->wm_ts;
      CHECK_FUNC_RET(ioctl(sensor->fd, SCUIOC_SETWATERMARK, (unsigned long)(uintptr_t)settings->wm));
    }

  return 0;
}

/*--------------------------------------------------------------------*/
static int notifyData(AccelSensor* sensor, MemMgrLite::MemHandle &mh_src, MemMgrLite::MemHandle &mh_dst)
{
  three_axis_s *p_src = reinterpret_cast<three_axis_s*>(mh_src.getVa());
  AccelDOF     *p_dst = reinterpret_cast<AccelDOF*>(mh_dst.getVa());

  for (int i = 0; i < ACCEL_WATERMARK_NUM; ++i)
    {
      p_dst->accel_x = (float)p_src->x * 2 / 32768;
      p_dst->accel_y = (float)p_src->y * 2 / 32768;
      p_dst->accel_z = (float)p_src->z * 2 / 32768;

      p_src++;
      p_dst++;
    }

  if ((sensor->handler != NULL) && (sensor->stopped != 1))
    {
      sensor->handler(sensor->context, ACCEL_EV_WM, mh_dst);
    }
  
  return 0;
}

static int restartScu(AccelSensor* sensor)
{
  /* Stop SCU */

  CHECK_FUNC_RET(ioctl(sensor->fd, SCUIOC_STOP, 0));

  /* Free SCU FIFO */

  CHECK_FUNC_RET(ioctl(sensor->fd, SCUIOC_FREEFIFO, 0));

  /* Set up SCU */

  CHECK_FUNC_RET(setupScu(sensor, s_scu_settings));

  /* Restart SCU */

  CHECK_FUNC_RET(ioctl(sensor->fd, SCUIOC_START, 0));

  return 0;
}

/*--------------------------------------------------------------------*/
extern "C" void *AccelSensorReceivingThread(FAR void *arg)
{
  int                  ret;
  char                *p_src;
  struct siginfo       value;
  struct timespec      timeout;
  AccelSensor         *sensor;
  timer_t timerid;
  int count = 0;
  int dir = 0;

  sensor = reinterpret_cast<AccelSensor*>(arg);
  
  /* setup scu */

  ret = setupScu(sensor, s_scu_settings);
  if (ret != 0)
    {
      ASSERT(0);
    }

  /* create oneshot timer */

  ret = createOneshotTimer(&timerid);
  if (ret != 0)
    {
      printf("create timer failed.");
      ASSERT(0);
    }

  /* start sequencer */

  ret = ioctl(sensor->fd, SCUIOC_START, 0);
  if (ret != 0)
    {
      printf("failed start sensor: %d\n", ret);
    }
  
  /* set timeout 6 seconds, SCU may send signal every 5 second. */

  timeout.tv_sec  = 6;
  timeout.tv_nsec = 0;

  sensor->stopped = 0;
  while(!sensor->stopped)
    {
      ret = sigtimedwait(&sensor->sig_set, &value, &timeout);
      if (ret < 0)
        {
          continue;
        }
      else if (ret == DEMO_COLLET_TRACKER_ACCEL_EV_SIGNO)
        {
          /* mathfunc */

          struct scuev_arg_s *scuev = (struct scuev_arg_s *)value.si_value.sival_ptr;

          if (scuev->type == SCU_EV_RISE)
            {
              dir = SCU_EV_RISE;
              
              if (count == 0)
                {
                  /* Rise event occurs just after sensor starts because of the filter setting.
                   * So check later if it is moving or not.
                   */

                  ret = startOneshotTimer(timerid, 1000);
                  if (ret != 0)
                    {
                      printf("start timer failed.");
                      ASSERT(0);
                    }
                  
                  count++;
                }
              else
                {
                  tracker_debug_info("Received rise event.\n");
              
                  MemMgrLite::MemHandle dummy;
                  sensor->handler(sensor->context, ACCEL_EV_MF, dummy);
                }
            }
          else if (scuev->type == SCU_EV_FALL)
            {
              dir = SCU_EV_FALL;
            }
          
          continue;
        }
      else if (ret == DEMO_COLLET_TRACKER_ACCEL_TM_SIGNO)
        {
          if (dir == SCU_EV_RISE)
            {
              /* still rise, then send rise event */
              
              tracker_debug_info("Received rise event.\n");
              
              MemMgrLite::MemHandle dummy;
              sensor->handler(sensor->context, ACCEL_EV_MF, dummy);
            }
          
          continue;
        }
      else if (ret == DEMO_COLLET_TRACKER_CHANGE_SCU_SIGNO)
        {
          tracker_debug_info("change scu event \n");

          count = 0;

          if (restartScu(sensor) != 0)
            {
              ASSERT(0);
            }

          continue;
        }
      else
        {
          /* watermark */
          tracker_debug_info("Received watermark event.\n");
        }

      /* get MemHandle */

      MemMgrLite::MemHandle mh_src;
      MemMgrLite::MemHandle mh_dst;
      if (mh_src.allocSeg(ACCEL_DATA_BUF_POOL, (sizeof(three_axis_s)*ACCEL_WATERMARK_NUM)) != ERR_OK)
        {
          ASSERT(0);
        }

      /* set physical address.(The addess specified for the SCU must be a physical address) */

      p_src = reinterpret_cast<char *>(mh_src.getPa());
      
      if (mh_dst.allocSeg(ACCEL_DATA_BUF_POOL, (sizeof(AccelDOF)*ACCEL_WATERMARK_NUM)) != ERR_OK)
        {
          ASSERT(0);
        }

      /* read accel data from driver */

      ret = read(sensor->fd, p_src, sizeof(three_axis_s) * ACCEL_WATERMARK_NUM);
      if (ret != (sizeof(three_axis_s) * ACCEL_WATERMARK_NUM))
        {
          ASSERT(0);
        }

      notifyData(sensor, mh_src, mh_dst);

    }

  deleteOneshotTimer(timerid);
    
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int AccelSensorCreate(AccelSensor** sensor)
{
  CHECK_NULL_RET(sensor);

  *sensor = (AccelSensor *)malloc(sizeof(AccelSensor));
  memset(*sensor, 0 , sizeof(AccelSensor));

  (*sensor)->stopped = 1;

  return 0;
}

/*--------------------------------------------------------------------*/
int AccelSensorRegisterHandler(AccelSensor* sensor, AccelEventHandler handler, uint32_t context)
{
  sensor->handler = handler;
  sensor->context = context;

  return 0;
}


/*--------------------------------------------------------------------*/
int AccelSensorStartSensing(AccelSensor *sensor, ScuSettings* settings)
{
  pthread_attr_t attr;
  struct sched_param sch_param;

  /* open driver */

  CHECK_FUNC_RET(sensor->fd = open(DEMO_COLLET_TRACKER_ACCEL_DEVNAME, O_RDONLY));

  /* add signal */

  sigemptyset(&sensor->sig_set);
  sigaddset(&sensor->sig_set, DEMO_COLLET_TRACKER_ACCEL_WM_SIGNO);
  sigaddset(&sensor->sig_set, DEMO_COLLET_TRACKER_ACCEL_EV_SIGNO);
  sigaddset(&sensor->sig_set, DEMO_COLLET_TRACKER_ACCEL_TM_SIGNO);
  sigaddset(&sensor->sig_set, DEMO_COLLET_TRACKER_CHANGE_SCU_SIGNO);

  /* set SCU settings */

  DEBUGASSERT(settings);
  s_scu_settings = settings;

  /* Create receive thread */

  (void)pthread_attr_init(&attr);
  sch_param.sched_priority = SENSING_TASK_PRIORITY;
  CHECK_FUNC_RET(pthread_attr_setschedparam(&attr, &sch_param));

  CHECK_FUNC_RET(pthread_create(&sensor->thread_id, &attr, AccelSensorReceivingThread, (pthread_addr_t)sensor));

  return 0;
}

/*--------------------------------------------------------------------*/
int AccelSensorStopSensing(AccelSensor* sensor)
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

  int ret = ioctl(sensor->fd, SCUIOC_STOP, 0);
  if (ret != 0)
    {
      printf("failed stop sensor: %d\n", ret);
    }
          
  /* delete signal */

  sigdelset(&sensor->sig_set, DEMO_COLLET_TRACKER_ACCEL_WM_SIGNO);
  sigdelset(&sensor->sig_set, DEMO_COLLET_TRACKER_ACCEL_EV_SIGNO);
  sigdelset(&sensor->sig_set, DEMO_COLLET_TRACKER_ACCEL_TM_SIGNO);
  sigdelset(&sensor->sig_set, DEMO_COLLET_TRACKER_CHANGE_SCU_SIGNO);

  /* close driver */

  CHECK_FUNC_RET(close(sensor->fd));

  return 0;
}

/*--------------------------------------------------------------------*/
int AccelSensorDestroy(AccelSensor* sensor)
{
  free(sensor);
  return 0;
}

/*--------------------------------------------------------------------*/
int AccelSensorChangeScuSetting(AccelSensor* sensor, ScuSettings* settings)
{
  DEBUGASSERT(settings);

  if (settings == s_scu_settings)
    {
      /* No need to change */

      return 0;
    }

  s_scu_settings = settings;

#ifdef CONFIG_CAN_PASS_STRUCTS
  union sigval value;
  value.sival_ptr = NULL;
  sigqueue(sensor->thread_id, DEMO_COLLET_TRACKER_CHANGE_SCU_SIGNO, value);
#else
  sigqueue(sensor->thread_id, DEMO_COLLET_TRACKER_CHANGE_SCU_SIGNO, 0);
#endif

  return 0;
}
