/****************************************************************************
 * test/sqa/singlefunction/audio_player/spr_sdk_audio_player.cxx
 *
 *   Copyright (C) 2018 Sony Corporation
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
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <asmp/mpshm.h>
#include <arch/chip/pm.h>
#include <arch/board/board.h>
#include <sys/stat.h>
#include "memutils/os_utils/chateau_osal.h"
#include "audio/audio_high_level_api.h"
#include "memutils/simple_fifo/CMN_SimpleFifo.h"
#include "memutils/memory_manager/MemHandle.h"
#include "memutils/message/Message.h"
#include "include/msgq_id.h"
#include "include/mem_layout.h"
#include "include/memory_layout.h"
#include "include/msgq_pool.h"
#include "include/pool_layout.h"
#include "include/fixed_fence.h"
#include "playlist/playlist.h"

using namespace MemMgrLite;

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef int (*player_func)(void);

/****************************************************************************
 * Public Type Declarations
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

extern int player_test_spr_sdk_14928(void);
extern int player_test_spr_sdk_14929(void);
extern int player_test_spr_sdk_15299(void);
extern int player_test_spr_sdk_15454(void);
extern int player_test_spr_sdk_15455(void);
extern int player_test_spr_sdk_15317(void);
extern int player_test_spr_sdk_15676(void);
extern int player_test_spr_sdk_15451(void);

/****************************************************************************
 * Public Data
 ****************************************************************************/

#define AUDIO_PLAYER_TEST_NUM  8
#define AUDIO_PLAYER_TEST_NAME 64

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* For share memory. */

static mpshm_t g_shm;

/* For frequency lock. */

static struct pm_cpu_freqlock_s g_player_lock;

/* Test name. */

char g_test_name[AUDIO_PLAYER_TEST_NUM][AUDIO_PLAYER_TEST_NAME] =
{
  "spr_sdk_14928",
  "spr_sdk_14929",
  "spr_sdk_15299",
  "spr_sdk_15454",
  "spr_sdk_15455",
  "spr_sdk_15317",
  "spr_sdk_15676",
  "spr_sdk_15451",
};

/* Test function. */

player_func g_test_func[AUDIO_PLAYER_TEST_NUM] =
{
  player_test_spr_sdk_14928,
  player_test_spr_sdk_14929,
  player_test_spr_sdk_15299,
  player_test_spr_sdk_15454,
  player_test_spr_sdk_15455,
  player_test_spr_sdk_15317,
  player_test_spr_sdk_15676,
  player_test_spr_sdk_15451,
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void player_test_init_freq_lock(void)
{
#ifndef CONFIG_PM_DISABLE_FREQLOCK_COUNT
  g_player_lock.count = 0;
#endif
  g_player_lock.info = PM_CPUFREQLOCK_TAG('A', 'P', 0);
  g_player_lock.flag = PM_CPUFREQLOCK_FLAG_HV;
}

static void player_test_freq_lock(void)
{
  up_pm_acquire_freqlock(&g_player_lock);
}

static void player_test_freq_release(void)
{
  up_pm_release_freqlock(&g_player_lock);
}

static bool player_test_init_libraries(void)
{
  int ret;
  uint32_t addr = AUD_SRAM_ADDR;

  /* Initialize shared memory.*/

  ret = mpshm_init(&g_shm, 1, 1024 * 128 * 2);
  if (ret < 0)
    {
      printf("Error: mpshm_init() failure. %d\n", ret);
      return false;
    }

  ret = mpshm_remap(&g_shm, (void *)addr);
  if (ret < 0)
    {
      printf("Error: mpshm_remap() failure. %d\n", ret);
      return false;
    }

  /* Initalize MessageLib. */

  err_t err = MsgLib::initFirst(NUM_MSGQ_POOLS, MSGQ_TOP_DRM);
  if (err != ERR_OK)
    {
      printf("Error: MsgLib::initFirst() failure. 0x%x\n", err);
      return false;
    }

  err = MsgLib::initPerCpu();
  if (err != ERR_OK)
    {
      printf("Error: MsgLib::initPerCpu() failure. 0x%x\n", err);
      return false;
    }

  void* mml_data_area = translatePoolAddrToVa(MEMMGR_DATA_AREA_ADDR);
  err = Manager::initFirst(mml_data_area, MEMMGR_DATA_AREA_SIZE);
  if (err != ERR_OK)
    {
      printf("Error: Manager::initFirst() failure. 0x%x\n", err);
      return false;
    }

  err = Manager::initPerCpu(mml_data_area, NUM_MEM_POOLS);
  if (err != ERR_OK)
    {
      printf("Error: Manager::initPerCpu() failure. 0x%x\n", err);
      return false;
    }

  /* Create static memory pool of VoiceCall. */

  const NumLayout layout_no = MEM_LAYOUT_PLAYER;
  void* work_va = translatePoolAddrToVa(MEMMGR_WORK_AREA_ADDR);
  err = Manager::createStaticPools(layout_no,
                                   work_va,
                                   MEMMGR_MAX_WORK_SIZE,
                                   MemoryPoolLayouts[layout_no]);
  if (err != ERR_OK)
    {
      printf("Error: Manager::initPerCpu() failure. %d\n", err);
      return false;
    }

  return true;
}

static bool player_test_finalize_libraries(void)
{
  /* Finalize MessageLib. */

  MsgLib::finalize();

  /* Destroy static pools. */

  MemMgrLite::Manager::destroyStaticPools();

  /* Finalize memory manager. */

  MemMgrLite::Manager::finalize();

  /* Destroy shared memory. */

  int ret;
  ret = mpshm_detach(&g_shm);
  if (ret < 0)
    {
      printf("Error: mpshm_detach() failure. %d\n", ret);
      return false;
    }

  ret = mpshm_destroy(&g_shm);
  if (ret < 0)
    {
      printf("Error: mpshm_destroy() failure. %d\n", ret);
      return false;
    }

  return true;
}

static int player_test_init(char *test_name)
{
  printf("=========================\n");
  printf("   Start %s \n", test_name);
  printf("=========================\n");

  /* First, initialize the shared memory and memory utility used by AudioSubSystem. */

  if (!player_test_init_libraries())
    {
      printf("Error: init_libraries() failure.\n");
      return -1;
    }

  /* Initialize frequency lock parameter. */

  player_test_init_freq_lock();

  /* Lock cpu frequency to high. */

  player_test_freq_lock();

  return 0;
}

static int player_test_uninit(char *test_name)
{
  /* Unlock cpu frequency. */

  player_test_freq_release();

  /* finalize the shared memory and memory utility used by AudioSubSystem. */

  if (!player_test_finalize_libraries())
    {
      printf("Error: finalize_libraries() failure.\n");
      return -1;
    }

  printf("=========================\n");
  printf("   Exit %s \n", test_name);
  printf("=========================\n");

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int player_test(int test_no)
{
  if (test_no >= AUDIO_PLAYER_TEST_NUM)
    {
      printf("Error test number is invalid!\n");
      return -1;
    }

  player_test_init(g_test_name[test_no]);
  g_test_func[test_no]();
  player_test_uninit(g_test_name[test_no]);

  return 0;
}

