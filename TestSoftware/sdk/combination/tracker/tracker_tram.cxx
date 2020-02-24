/****************************************************************************
 * demo/collet_box/tracker_tram.cxx
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
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>
#include <asmp/mpshm.h>
#include <arch/board/board.h>

#include "memutils/s_stl/queue.h"
#include "sensing/sensor_command.h"
#include "sensing/sensor_id.h"
#include "sensing/barometer.h"
#include "sensing/transport_mode.h"
#include "memutils/message/Message.h"
#include "mem_layout.h"
#include "msgq_pool.h"
#include "memutils/memory_manager/MemHandle.h"
#include "pool_layout.h"
#include "fixed_fence.h"
#include "tracker_debug.h"
#include "tracker_tram_sensor_control.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define DETECTED_MF_EV  0
#define CHANGE_SCU_EV  1

/****************************************************************************
 * Private Data
 ****************************************************************************/

static mpshm_t s_shm;
static mqd_t s_mqfd = (mqd_t)-1;
static pthread_t s_thread_id;
static uint8_t g_result_data = 0;

static const char* s_class_strings[] =
{
  "Undetermined",
  "Staying",
  "Walking",
  "Running",
  "Ascending stairs",
  "Descending stairs",
  "Going up on escalator",
  "Going down on escalator",
  "Going up in elevator",
  "Going down in elevator",
  "Getting on train",
  "Getting on bus",
  "Getting in car",
  "Riding bicycle",
};

/****************************************************************************
 * Callback Function
 ****************************************************************************/

static bool app_receive_event(sensor_command_data_t& data)
{
  tracker_debug_info("data.self: %d\n", data.self);

  switch (data.self)
    {
    case accelID:
      {
        /* send message */
        
        char ev = DETECTED_MF_EV;
        mq_send(s_mqfd, (char *)&ev, sizeof(uint8_t), 10);
      }
      break;

    case tramID:
      {
        uint8_t result_type = get_async_msgtype(data.data);
        uint8_t result_data = get_async_msgparam(data.data);

        if (result_type == TramCmdTypeResult)
          {
            g_result_data = result_data;
            printf("Mode: %s\n", s_class_strings[result_data]);
          }
        else if (result_type == TramCmdTypeTrans)
          {
            tracker_debug_info("message: 0x%x\n", result_data);

            char state;
            switch (result_data)
              {
              case ChangeScuSettings:
                state = CHANGE_SCU_EV;
                break;

              default:
                printf("invalide message! %d\n", result_data);
                break;
              }

            /* send message */

            mq_send(s_mqfd, (char *)&state, sizeof(uint8_t), 10);
            break;
        }
      }
    }

  return true;
}

static bool app_receive_result(sensor_command_data_mh_t& data)
{
  tracker_debug_info("data.self: %d\n", data.self);

  SensorDspCmd* result_data = reinterpret_cast<SensorDspCmd*>(data.mh.getVa());

  if ( SensorOK != result_data->result.exec_result )
    {
      printf("received error: result[%d], code[%d]\n",
             result_data->result.exec_result,
             result_data->result.assert_info.code);

      return false;
    }

  switch (result_data->exec_tram_cmd.type)
    {
    case TramSensorAcc:
      break;
    case TramSensorMag:
      break;
    case TramSensorBar:
      break;
    default:
      break;
    }

  return true;
}

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void regist_app_callback(void)
{
  sensor_command_register_t reg;

  reg.header.size       = 0;
  reg.header.code       = ResisterClient;
  reg.self              = app0ID;
  reg.subscriptions     = (0x01 << accelID) | (0x01 << tramID);
  reg.callback          = app_receive_event;
  reg.callback_mh       = app_receive_result;
  reg.callback_pw       = NULL;
  SF_SendSensorResister(&reg);
}

static int init_shm(void)
{
  int ret;
  uint32_t addr = SHM_SRAM_ADDR;
  ret = mpshm_init(&s_shm, 1, SHM_SRAM_SIZE);
  if (ret < 0)
    {
      printf("mpshm_init() failure. %d\n", ret);
      return -1;
    }
  ret = mpshm_remap(&s_shm, (void *)addr);
  if (ret < 0)
    {
      printf("mpshm_remap() failure. %d\n", ret);
      return -1;
    }
  return 0;
}

static void init_memutilities(void)
{
  /* initialize MsgLib */
  
  MsgLib::initFirst(NUM_MSGQ_POOLS, MSGQ_TOP_DRM);
  MsgLib::initPerCpu();

  /* initialize MemMgr */
  
  void* mml_data_area = MemMgrLite::translatePoolAddrToVa(MEMMGR_DATA_AREA_ADDR);
  MemMgrLite::Manager::initFirst(mml_data_area, MEMMGR_DATA_AREA_SIZE);
  MemMgrLite::Manager::initPerCpu(mml_data_area, NUM_MEM_POOLS);

  const MemMgrLite::NumLayout layout_no = 0;
  void* work_va = MemMgrLite::translatePoolAddrToVa(MEMMGR_WORK_AREA_ADDR);
  MemMgrLite::Manager::createStaticPools(layout_no, work_va, MEMMGR_MAX_WORK_SIZE, MemMgrLite::MemoryPoolLayouts[layout_no]);

}

extern "C" void *TramReceivingEvThread(FAR void *arg)
{
  int ret = 0;
  char ev;

  while (mq_receive(s_mqfd, &ev, sizeof(uint8_t), 0) != 0)
    {
      switch (ev)
        {
        case DETECTED_MF_EV:
          ret = TramSendMathFuncEvent();
          if (ret != 0)
            {
              printf("failed to handle event: ACCEL_MF_EV\n");
            }
          break;

        case CHANGE_SCU_EV:
          {
            ret = TramChangeScuSettings();
            if (ret != 0)
              {
                printf("failed to change SCU settings.\n");
              }
            break;
          }
        }
    }

  return 0;
}

static int open_message(void)
{
  pthread_attr_t attr;
  struct sched_param sch_param;
  struct mq_attr mqueue_attr;
  
  /* Fill in attributes for message queue */

  mqueue_attr.mq_maxmsg  = 20;
  mqueue_attr.mq_msgsize = 1;
  mqueue_attr.mq_flags   = 0;
  
  s_mqfd = mq_open("tram_mqueue", O_RDWR|O_CREAT, 0666, &mqueue_attr);
  if (s_mqfd < 0)
    {
      printf("failed by mq_open(O_RDWR)\n");
      return EXIT_FAILURE;
    }

  (void)pthread_attr_init(&attr);
  sch_param.sched_priority = 110;
  pthread_attr_setschedparam(&attr, &sch_param);

  pthread_create(&s_thread_id, &attr, TramReceivingEvThread, (void *)NULL);
  
  return 0;
}

static void sensor_manager_api_response(unsigned int code, unsigned int ercd, unsigned int self)
{
  if (ercd != SENSOR_ECODE_OK)
    {
      tracker_debug_info("get api response. code %d, ercd %d, self %d\n",
                         code, ercd, self);
    }

  return;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * initialize_Functions
 ****************************************************************************/

extern "C" int tracker_tram_init(void)
{
  int ret;
  
  /* initialize shared memory */
  
  ret = init_shm();
  if (ret < 0)
    {
      return EXIT_FAILURE;
    }
  
  /* initialize memory utilities */
  
  init_memutilities();
  
  /* open receiving mq */
  
  ret = open_message();
  if(ret < 0)
    {
      return EXIT_FAILURE;
    }

  /* activate sensor manager */

  if (!SF_ActivateSensorSubSystem(MSGQ_SEN_MGR, sensor_manager_api_response))
    {
      printf("SF_ActivateSensorSubSystem() failure.\n");
      return EXIT_FAILURE;
    }
  
  /* open tram state transition */

  ret = TramOpenSensors();
  if (ret < 0)
    {
      return EXIT_FAILURE;
    }
  
  /* regist application callback */
  
  regist_app_callback();
  
  printf("Start sensoring...\n");
  
  TramStartSensors();

  return EXIT_SUCCESS;
}

extern "C" const char* tracker_get_tram_state(void)
{
  return s_class_strings[g_result_data];
}
