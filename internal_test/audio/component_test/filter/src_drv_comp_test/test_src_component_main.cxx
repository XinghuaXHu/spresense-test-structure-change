/****************************************************************************
 * test/internal_test/audio/component_test/filter/src_drv_comp_test/
 * test_src_component_main.cxx
 *
 *   Copyright (C) 2017 Sony Corporation
 *   Author: Tomonobu Hayakawa<Tomonobu.Hayakawa@sony.com>
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
#include <sdk/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <math.h>
#include "system/readline.h"
#include <mqueue.h>
#include <asmp/mpshm.h>
#include <sys/wait.h>
#include <nuttx/arch.h>
#include <arch/chip/pm.h>
#include <sys/stat.h>

#include "memutils/os_utils/os_wrapper.h"
#include "memutils/common_utils/common_assert.h"
#include "wien2_common_defs.h"
#include "filter_component.h"
#include "dsp_drv.h"

#include "memutils/os_utils/chateau_osal.h"
#include "audio/audio_high_level_api.h"

#include "memutils/message/MsgQue.h"
#include "memutils/message/MsgQueBlock.h"
#include "memutils/memory_manager/MemHandle.h"
#include "memutils/message/Message.h"
#include "memutils/message/MsgPacket.h"
#include "memutils/s_stl/queue.h"
#include "msgq_id.h"
#include "mem_layout.h"
#include "memory_layout.h"

#include "playlist.h"

#include "fixed_fence.h"
#include "msgq_pool.h"
#include "pool_layout.h"

__USING_WIEN2

using namespace MemMgrLite;

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define NUM_OF_AU          (1)

/* 3î{êîÇ™DMAêßå¿ÇÃ2048à»ì‡Ç…Ç»ÇÈÇÊÇ§Ç…ê›íË */

#define WAV_SAMPLE         (640)

/* sampleêî x byte x Channel */

#define	MAX_ES_BUFF_SIZE   (WAV_SAMPLE * 2 * 2)

/* sampleêî x byte x Channel x 6î{ */

#define	MAX_PCM_BUFF_SIZE  (WAV_SAMPLE * 2 * 2 * 6)
#define MAX_PATH_LENGTH     128

#define ES_EXIST (0)
#define ES_END (1)

#ifndef CONFIG_TEST_SRC_FILE_MOUNTPT
#  define CONFIG_TEST_DECODER_FILE_MOUNTPT "/mnt/vfat/AUDIO"
#endif

#ifndef CONFIG_TEST_SRC_OUTPUT_FILE_MOUNTPT
#  define CONFIG_TEST_SRC_OUTPUT_FILE_MOUNTPT "/mnt/vfat/SRC"
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/
uint32_t g_es_buf[MAX_ES_BUFF_SIZE/sizeof(uint32_t)];
uint32_t g_pcm_buf[MAX_PCM_BUFF_SIZE/sizeof(uint32_t)];

Playlist* p_playlist_ins;

static mpshm_t s_shm;
static int g_out_fd = -1;
static DIR *g_dirp = (DIR *)0;
void*  g_p_src_instance;
static uint32_t g_sampling_rate_cnt;
static uint32_t g_sampling_rate[6] = { 8000, 16000, 24000, 32000, 44100, 48000 };

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/
//extern "C" bool AS_decode_exec(const _ExecDecCompParam& param, void* p_instance);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int init_libraries(void)
{
  int ret;
  uint32_t addr = AUD_SRAM_ADDR;

  ret = mpshm_init(&s_shm, 1, 1024 * 128 * 2);
  if (ret < 0)
    {
      printf("mpshm_init() failure. %d\n", ret);
      return ret;
    }
  ret = mpshm_remap(&s_shm, (void *)addr);
  if (ret < 0)
    {
      printf("mpshm_remap() failure. %d\n", ret);
      return ret;
    }
  
  /* Initalize MessageLib */
  MsgLib::initFirst(NUM_MSGQ_POOLS,MSGQ_TOP_DRM);
  MsgLib::initPerCpu();

  void* mml_data_area = MemMgrLite::translatePoolAddrToVa(MEMMGR_DATA_AREA_ADDR);
  MemMgrLite::Manager::initFirst(mml_data_area, MEMMGR_DATA_AREA_SIZE);
  MemMgrLite::Manager::initPerCpu(mml_data_area, NUM_MEM_POOLS);

  /* Create static memory pool of Layout 1 */
  const MemMgrLite::NumLayout layout_no = MEM_LAYOUT_RECORDER;

  void* work_va = MemMgrLite::translatePoolAddrToVa(MEMMGR_WORK_AREA_ADDR);
  D_ASSERT(layout_no < NUM_MEM_LAYOUTS);
  MemMgrLite::Manager::createStaticPools(layout_no, work_va, MEMMGR_MAX_WORK_SIZE, MemMgrLite::MemoryPoolLayouts[layout_no]);

  if (!MemMgrLite::Manager::isPoolAvailable(SRC_APU_CMD_POOL))
    {
      printf("isPoolAvailable APU CMD buf error\n");
      return -1;
    }

  return 0;
}

static int finalize_libraries(void)
{
  /* Finalize MessageLib */
  MsgLib::finalize();
  
  /* Destroy static pools. */
  MemMgrLite::Manager::destroyStaticPools();

  /* Finalize memory manager */
  MemMgrLite::Manager::finalize();

  /* Destroy MP shared memory. */
  int ret;
  ret = mpshm_detach(&s_shm);
  if (ret < 0)
    {
      printf("mpshm_detach() failure. %d\n", ret);
    }

  ret = mpshm_destroy(&s_shm);
  if (ret < 0)
    {
      printf("mpshm_destroy() failure. %d\n", ret);
    }

  return ret;
}

/*--------------------------------------------------------------------------*/
static int app_open_playlist(void)
{
  p_playlist_ins = new Playlist("TRACK_DB.CSV");

  p_playlist_ins->init();
  p_playlist_ins->setPlayMode(Playlist::PlayModeNormal);
  p_playlist_ins->setRepeatMode(Playlist::RepeatModeOff);

  p_playlist_ins->select(Playlist::ListTypeAllTrack, NULL);

  return 0;
}

/*--------------------------------------------------------------------------*/
static int app_close_playlist(void)
{
  delete p_playlist_ins;

  return 0;
}

/*--------------------------------------------------------------------------*/
static int app_get_next_track(Track* track)
{
  bool ret;
  ret = p_playlist_ins->getNextTrack(track);

  return (ret == true) ? 0 : 1;
}

/*--------------------------------------------------------------------------*/
static int open_contents(Track *track)
{
  int fd;

  if (app_get_next_track(track))
    {
      printf("No more tracks to play.\n");
      return -1;
    }

  char full_path[128];
  snprintf(full_path, sizeof(full_path), "%s/%s", CONFIG_TEST_SRC_FILE_MOUNTPT, track->title);

  fd = open(full_path, O_RDONLY);
  if (fd >= 0)
    {
      printf("Input file: %s\n",track->title);
    }
  else
    {
       printf("%s open error. check paths and files!\n", full_path);
    }

  return fd;
}

/*--------------------------------------------------------------------*/
static bool src_done_callback(DspDrvComPrm_t* p_param)
{
  D_ASSERT2(DSP_COM_DATA_TYPE_STRUCT_ADDRESS == p_param->type, AssertParamLog(AssertIdTypeUnmatch, p_param->type));

  Apu::Wien2ApuCmd* packet = reinterpret_cast<Apu::Wien2ApuCmd*>(p_param->data.pParam);
  SrcFilterCompCmpltParam cmplt;
  cmplt.event_type = static_cast<Wien2::Apu::ApuEventType>(packet->header.event_type);

  switch ( packet->header.event_type )
    {
      case Apu::ExecEvent:
        {
          cmplt.exec_src_param.input_buffer = packet->exec_filter_cmd.input_buffer;
          cmplt.exec_src_param.output_buffer = packet->exec_filter_cmd.output_buffer;

          MEDIA_RECORDER_VDBG("Src s %d\n", cmplt.exec_src_param.output_buffer.size);

          err_t er = MsgLib::send<SrcFilterCompCmpltParam>(MSGQ_AUD_SRC, MsgPriNormal, MSG_AUD_VRC_RST_FILTER, MSGQ_AUD_SRC, cmplt);
          F_ASSERT(er == ERR_OK);
        }
        break;
      case Apu::FlushEvent:
        {
          cmplt.stop_src_param.output_buffer = packet->flush_filter_cmd.flush_src_cmd.output_buffer;

          MEDIA_RECORDER_VDBG("FlsSrc s %d\n", cmplt.stop_src_param.output_buffer.size);

          err_t er = MsgLib::send<SrcFilterCompCmpltParam>(MSGQ_AUD_SRC, MsgPriNormal, MSG_AUD_VRC_RST_FILTER, MSGQ_AUD_SRC, cmplt);
          F_ASSERT(er == ERR_OK);
        }
        break;
      default:
        MEDIA_RECORDER_ERR(AS_ATTENTION_SUB_CODE_DSP_ILLEGAL_REPLY);
        return false;
    }
  return true;
}

static void* allocPcmBuf(uint32_t size)
{
	return g_pcm_buf;
}

static void _exec_src(void* p_es, uint32_t es_size)
{
  err_t        err_code;
  MsgQueBlock* que;
  MsgPacket*   msg = 0;

  void* p_pcm = allocPcmBuf(MAX_PCM_BUFF_SIZE);

  if (!p_pcm)
    {
      printf("_exec_src:allocPcmBuf error");
      return;
    }

  FilterComponentParam param;
  param.filter_type = Apu::SRC;
  param.callback = src_done_callback;

  param.exec_src_param.input_buffer.p_buffer = reinterpret_cast<unsigned long*>(p_es);
  param.exec_src_param.input_buffer.size = es_size;
  param.exec_src_param.output_buffer.p_buffer = reinterpret_cast<unsigned long*>(p_pcm);
  param.exec_src_param.output_buffer.size = MAX_PCM_BUFF_SIZE;

  if (AS_filter_exec(param) == false)
    {
      printf("_exec_src:AS_filter_exec Error\n");
    }

  err_code = MsgLib::referMsgQueBlock(MSGQ_AUD_SRC, &que);
  if (err_code)
    {
      printf("_decode:referMsgQueBlock error\n");
    }

  err_code = que->recv(TIME_FOREVER, &msg);
  if (err_code)
    {
      printf("_exec_src:que->recv error\n");
    }

  SrcFilterCompCmpltParam dsp_resp = msg->moveParam<SrcFilterCompCmpltParam>();
  if (!AS_filter_recv_done())
    {
      printf("_exec_src:AS_filter_recv_done error\n");
    }

  if (dsp_resp.exec_src_param.output_buffer.size > 0)
    {
      size_t size;
      size = write(g_out_fd, dsp_resp.exec_src_param.output_buffer.p_buffer, dsp_resp.exec_src_param.output_buffer.size);
      if (size != dsp_resp.exec_src_param.output_buffer.size)
        {
          F_ASSERT(0);
        }
    }

  que->pop();
}

/*--------------------------------------------------------------------*/

static uint8_t _src_init(Track *track)
{
  uint32_t dsp_inf;
  uint8_t rst;

  FilterComponentParam filter_param;
  filter_param.filter_type = Apu::SRC;
  filter_param.init_src_param.sample_num = WAV_SAMPLE;

  filter_param.init_src_param.input_sampling_rate = track->sampling_rate;
  filter_param.init_src_param.output_sampling_rate = g_sampling_rate[g_sampling_rate_cnt];
  filter_param.init_src_param.channel_num = track->channel_number;
  filter_param.init_src_param.input_pcm_byte_length = 2;
  filter_param.init_src_param.output_pcm_byte_length = 2;
  filter_param.callback = src_done_callback;

  rst = AS_filter_init(filter_param, &dsp_inf);

  if (!AS_filter_recv_done())
    {
      printf("AS_decode_recv_done error\n");
      return AS_ECODE_QUEUE_OPERATION_ERROR;
    }
  return rst;
}

/*--------------------------------------------------------------------*/
static void _stop(void)
{
  FilterComponentParam param;
  param.filter_type = Apu::SRC;
  param.stop_src_param.output_buffer.p_buffer = reinterpret_cast<unsigned long*>(g_pcm_buf);
  param.stop_src_param.output_buffer.size = MAX_PCM_BUFF_SIZE;
  AS_filter_stop(param);

  err_t        err_code;
  MsgQueBlock* que;
  MsgPacket*   msg = 0;

  err_code = MsgLib::referMsgQueBlock(MSGQ_AUD_SRC, &que);
  if (err_code)
    {
      printf("_stop:referMsgQueBlock error\n");
    }

  err_code = que->recv(TIME_FOREVER, &msg);
  if (err_code)
    {
      printf("_stop:que->recv error\n");
    }

  SrcFilterCompCmpltParam dsp_resp = msg->moveParam<SrcFilterCompCmpltParam>();
  if (!AS_filter_recv_done())
    {
      printf("_stop:AS_filter_recv_done error\n");
    }

  if (dsp_resp.stop_src_param.output_buffer.size > 0)
    {
      size_t size;
      size = write(g_out_fd, dsp_resp.stop_src_param.output_buffer.p_buffer, dsp_resp.stop_src_param.output_buffer.size);
      if (size != dsp_resp.stop_src_param.output_buffer.size)
        {
          F_ASSERT(0);
        }
    }

  que->pop();
}

/****************************************************************************/
static bool _createDir(void)
{
  int ret;
  const char *name = CONFIG_TEST_SRC_OUTPUT_FILE_MOUNTPT;

  g_dirp = opendir("/mnt");
  if (!g_dirp)
    {
      printf("opendir err(errno:%d)\n",errno);
      return false;
    }
  ret = mkdir(name,0777);
  if (ret != 0)
    {
      if (errno != EEXIST)
        {
          printf("mkdir err(errno:%d)\n",errno);
          return false;
        }
    }
  return true;
}

int open_output(Track *track)
{
  static char fname[MAX_PATH_LENGTH];

  snprintf(fname, MAX_PATH_LENGTH, "%s/%s-%d.src", CONFIG_TEST_SRC_OUTPUT_FILE_MOUNTPT, track->title, g_sampling_rate[g_sampling_rate_cnt]);
  printf("write file:%s\n", fname);
  return open(fname, O_CREAT | O_WRONLY);
}

/****************************************************************************
 * application_main
 ****************************************************************************/
#ifdef CONFIG_BUILD_KERNEL
//int main(int argc, FAR char *argv[])
extern "C" int main(int argc, char *argv[])
#else
extern "C" int test_src_component_main(int argc, char *argv[])
#endif
{
  Track track;
  int fd = -1;;
  int err = 0;
  int ret;
  uint8_t rst;
  uint32_t dsp_inf;

  if (!_createDir())
    {
      goto exit_1;
    }

  err = init_libraries();
  if (err)
    {
      goto exit_1;
    }
  if (app_open_playlist())
    {
      printf("app_open_playlist Error\n");
      goto exit_2;
    }
  fd = open_contents(&track);
  if (fd < 0)
    {
      goto exit;
    }

  rst = AS_filter_activate(SRCOnly, MSGQ_AUD_DSP, SRC_APU_CMD_POOL, &dsp_inf);
  if (rst != AS_ECODE_OK)
    {
      printf("AS_filter_activate Error\n");
      goto exit;
    }

  for (g_sampling_rate_cnt = 0 ; g_sampling_rate_cnt < (sizeof(g_sampling_rate) / sizeof(int)) ; g_sampling_rate_cnt++)
    {
      lseek(fd, 0, SEEK_SET);

      if (track.sampling_rate == g_sampling_rate[g_sampling_rate_cnt])
        {
          continue;
        }

      err = _src_init(&track);
      if (err == AS_ECODE_COMMAND_PARAM_AREA_INSUFFICIENT)
        {
          printf("Skip:Area Insufficient\n");
          continue;
        }
      else if (err != AS_ECODE_OK)
        {
          printf("AS_filter_init error\n");
          break;
        }

      g_out_fd = open_output(&track);
      if (g_out_fd < 0)
        {
          printf("open_output error\n");
          break;
        }
      while (true)
        {
          /* read file */
          ret = read(fd, g_es_buf, MAX_ES_BUFF_SIZE);
          if (ret > 0)
            {
                _exec_src(g_es_buf, ret);
            }
          else if(ret == 0)
            {
              _stop();
              close(g_out_fd);
              g_out_fd = -1;
              break;
            }
          else if(ret < 0)
            {
              printf("Fail to read file\n");
              break;
            }
        }
    }

  if (!AS_filter_deactivate(SRCOnly))
    {
      printf("AS_filter_deactivate Error");
    }

exit:
  if (fd > 0)
    {
      close(fd);
    }

  if (g_out_fd > 0)
    {
      close(g_out_fd);
    }

  app_close_playlist();

exit_2:
  finalize_libraries();

exit_1:
  if (g_dirp > 0)
    {
      closedir(g_dirp);
    }

  printf("test_src exit!\n");
  return 0;
}
