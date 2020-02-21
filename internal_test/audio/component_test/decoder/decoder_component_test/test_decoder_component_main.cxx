/****************************************************************************
 * test/internal_test/audio/component_test/decoder/decoder_component_test/
 * test_decoder_component_main.cxx
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
#include "components/decoder/decoder_component.h"
#include "dsp_drv.h"
#include "common/Mp3Parser.h"
#include "common/RamAdtsParser.h"
#include "objects/stream_parser/input_data_mng_obj.h"

#include "memutils/os_utils/chateau_osal.h"
#include "audio/audio_high_level_api.h"

#include "memutils/simple_fifo/CMN_SimpleFifo.h"
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
#define WRITE_SIMPLE_FIFO_SIZE  2560
#define SIMPLE_FIFO_FRAME_NUM   9
#define SIMPLE_FIFO_BUF_SIZE    (WRITE_SIMPLE_FIFO_SIZE * SIMPLE_FIFO_FRAME_NUM)
#define NUM_OF_AU               1
#define MAX_ES_BUFF_SIZE        6144
#define MAX_PCM_BUFF_SIZE       8192
#define MAX_PATH_LENGTH         128

//#define CONFIG_AUDIOUTILS_UNUSE_ORIGINAL_OPUS_FORMAT
#ifdef CONFIG_AUDIOUTILS_UNUSE_ORIGINAL_OPUS_FORMAT
#  define OPUS_DATA_LEN         4
#  define OPUS_FINAL_RANGE_LEN  4
#else
#  define OPUS_DATA_LEN         1
#  define OPUS_FINAL_RANGE_LEN  0
#endif /* CONFIG_AUDIOUTILS_UNUSE_ORIGINAL_OPUS_FORMAT */

#define ES_EXIST 0
#define ES_END   1

#ifndef CONFIG_TEST_DECODER_FILE_MOUNTPT
#  define CONFIG_TEST_DECODER_FILE_MOUNTPT "/mnt/vfat/AUDIO"
#endif

#ifndef CONFIG_TEST_DECODER_OUTPUT_FILE_MOUNTPT
#  define CONFIG_TEST_DECODER_OUTPUT_FILE_MOUNTPT "/mnt/vfat/DEC"
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/
uint32_t m_simple_fifo_buf[SIMPLE_FIFO_BUF_SIZE/sizeof(uint32_t)];
uint8_t m_ram_buf[WRITE_SIMPLE_FIFO_SIZE];
CMN_SimpleFifoHandle m_simple_fifo_handle;
AsPlayerInputDeviceHdlrForRAM m_cur_input_device_handler;
int32_t g_main_file_size = 0;
uint32_t g_es_buf[MAX_ES_BUFF_SIZE/sizeof(uint32_t)];
uint32_t g_pcm_buf[MAX_PCM_BUFF_SIZE/sizeof(uint32_t)];

Playlist* p_playlist_ins;

static mpshm_t s_shm;
static uint32_t g_max_es_buff_size;
static uint32_t g_max_pcm_buff_size;
static int g_pcm_fd;
static DIR *g_dirp = (DIR *)0;
void*  g_p_dec_instance;

MP3PARSER_Handle mp3_handle;
MP3PARSER_Config mp3_config;
AdtsHandle	aac_handle;
CMN_SimpleFifoHandle* p_simple_fifo_handler;

static struct pm_cpu_freqlock_s g_player_lock;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

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
  const MemMgrLite::NumLayout layout_no = MEM_LAYOUT_PLAYER_MAIN_ONLY;

  void* work_va = MemMgrLite::translatePoolAddrToVa(MEMMGR_WORK_AREA_ADDR);
  D_ASSERT(layout_no < NUM_MEM_LAYOUTS);
  MemMgrLite::Manager::createStaticPools(layout_no,
                                         work_va,
                                         MEMMGR_MAX_WORK_SIZE,
                                         MemMgrLite::MemoryPoolLayouts[layout_no]);

  if (!MemMgrLite::Manager::isPoolAvailable(DEC_APU_CMD_POOL))
    {
      printf("isPoolAvailable APU CMD buf error\n");
      return -1;
    }

  g_max_es_buff_size = MAX_ES_BUFF_SIZE;
  g_max_pcm_buff_size = MAX_PCM_BUFF_SIZE;

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
static void app_input_device_callback(uint32_t size)
{
    /* do nothing */
}

/*--------------------------------------------------------------------------*/
static int app_init_simple_fifo(CMN_SimpleFifoHandle* handle, uint32_t* fifo_buf, int32_t fifo_size, AsPlayerInputDeviceHdlrForRAM* dev_handle)
{
  if (CMN_SimpleFifoInitialize(handle, fifo_buf, fifo_size, NULL) != 0)
    {
      printf("Fail to initialize simple FIFO.");
      return 1;
    }
  CMN_SimpleFifoClear(handle);

  dev_handle->simple_fifo_handler = (void*)(handle);
  dev_handle->callback_function = app_input_device_callback;
  return 0;
}

/*--------------------------------------------------------------------------*/
static int push_simple_fifo(int fd)
{
  int ret;

  ret = read(fd, &m_ram_buf, WRITE_SIMPLE_FIFO_SIZE);
  if (ret < 0)
    {
      printf("Fail to read file. errno:%d\n",get_errno());
      return 1;
    }

  if (CMN_SimpleFifoOffer(&m_simple_fifo_handle, (const void*)(m_ram_buf), ret) == 0)
    {
      printf("Simple FIFO is full!\n");
      return 1;
    }
  g_main_file_size = (g_main_file_size - ret);
  if (g_main_file_size == 0)
    {
      return 2;
    }
  return 0;
}

/*--------------------------------------------------------------------------*/
static int app_push_simple_fifo(int fd)
{
  int32_t ret = 0;
  size_t vacant_size;

  vacant_size = CMN_SimpleFifoGetVacantSize(&m_simple_fifo_handle);

  if ((vacant_size != 0) && (vacant_size > WRITE_SIMPLE_FIFO_SIZE))
    {
      int cnt=1;
      if (vacant_size > WRITE_SIMPLE_FIFO_SIZE*3)
        {
          cnt=3;
        }
      else if (vacant_size > WRITE_SIMPLE_FIFO_SIZE*2)
        {
          cnt=2;
        }
      for (int i=0; i < cnt; i++)
        {
          if ((ret = push_simple_fifo(fd)) != 0)
            {
              break;
            }
        }
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
  snprintf(full_path, sizeof(full_path), "%s/%s", CONFIG_TEST_DECODER_FILE_MOUNTPT, track->title);

  fd = open(full_path, O_RDONLY);
  if (fd >= 0)
    {
      struct stat stat_buf;
      stat(full_path, &stat_buf);
      g_main_file_size = stat_buf.st_size;
      printf("dec file: %s\n",track->title);
    }
  else
    {
       printf("%s open error. check paths and files!\n", full_path);
    }

  return fd;
}

static int close_contents(int fd)
{
  CMN_SimpleFifoClear(&m_simple_fifo_handle);
  return close(fd);
}

/*--------------------------------------------------------------------*/
static void* allocPcmBuf(uint32_t size)
{
  return g_pcm_buf;
}

static void _decode(void* p_es, uint32_t es_size)
{
  err_t        err_code;
  MsgQueBlock* que;
  MsgPacket*   msg = 0;

  void* p_pcm = allocPcmBuf(g_max_pcm_buff_size);

  if (!p_pcm)
    {
      return;
    }
  ExecDecCompParam param;
  param.input_buffer.p_buffer = reinterpret_cast<unsigned long*>(p_es);
  param.input_buffer.size = es_size;
  param.output_buffer.p_buffer = reinterpret_cast<unsigned long*>(p_pcm);
  param.output_buffer.size = g_max_pcm_buff_size;
  param.num_of_au = NUM_OF_AU;

  if (AS_decode_exec(param, g_p_dec_instance) == false)
    {
      printf("AS_decode_exec Error\n");
    }

  err_code = MsgLib::referMsgQueBlock(MSGQ_AUD_PLY, &que);
  if (err_code)
    {
      printf("_decode:referMsgQueBlock error\n");
    }

  err_code = que->recv(TIME_FOREVER, &msg);
  if (err_code)
    {
      printf("_decode:que->recv error\n");
    }

  DecCmpltParam dsp_resp = msg->moveParam<DecCmpltParam>();
  AS_decode_recv_done(g_p_dec_instance);

  if (dsp_resp.exec_dec_cmplt.is_valid_frame == false)
    {
      printf("decode error\n");
    }

  if (dsp_resp.exec_dec_cmplt.output_buffer.size > 0)
    {
      size_t size;
      size = write(g_pcm_fd, dsp_resp.exec_dec_cmplt.output_buffer.p_buffer, dsp_resp.exec_dec_cmplt.output_buffer.size);
      if (size != dsp_resp.exec_dec_cmplt.output_buffer.size)
        {
          F_ASSERT(0);
        }
    }

  que->pop();
}

static uint16_t get_cur_wav_au_sample_num()
{
  return 640;
}

static uint16_t getSampleNumPerFrame(Track *track)
{
  switch((AudioCodec)track->codec_type)
    {
    case AudCodecMP3:
      if (track->sampling_rate != AudioFs2ApuValue[AudFs_16000])
        {
          return SampleNumPerFrame[track->codec_type];
        }
      return Mp3SampleNumPerFrameWith16kHz;
    case AudCodecXAVCLPCM:
      return get_cur_wav_au_sample_num();
    case AudCodecSBC:
    case AudCodecAAC:
    case AudCodecWMA:
    case AudCodecLDAC:
    case AudCodecFLAC:
      return SampleNumPerFrame[track->codec_type];
    case AudCodecOPUS:
      if (track->sampling_rate == AudioFs2ApuValue[AudFs_16000])
        {
          return SampleNumPerFrame[track->codec_type]*2;
        }
      return SampleNumPerFrame[track->codec_type];
    default:
      printf("getSampleNumPerFrame error\n");
      break;
    }
  return 0;
}

/*--------------------------------------------------------------------*/
static bool decoder_comp_done_callback(void* p_response, void* p_requester)
{
  DspDrvComPrm_t* p_param = (DspDrvComPrm_t *)p_response;
  if (DSP_COM_DATA_TYPE_STRUCT_ADDRESS != p_param->type)
    {
      MEDIA_PLAYER_ERR(AS_ATTENTION_SUB_CODE_DSP_ILLEGAL_REPLY);
      return false;
    }

  Apu::Wien2ApuCmd* packet = reinterpret_cast<Apu::Wien2ApuCmd*>(p_param->data.pParam);

  DecCmpltParam cmplt;
  cmplt.event_type = static_cast<Wien2::Apu::ApuEventType>(packet->header.event_type);

  switch ( packet->header.event_type )
    {
    case Apu::ExecEvent:
      {
        cmplt.exec_dec_cmplt.input_buffer   = packet->exec_dec_cmd.input_buffer;
        cmplt.exec_dec_cmplt.output_buffer  = packet->exec_dec_cmd.output_buffer;
        cmplt.exec_dec_cmplt.is_valid_frame = (packet->result.exec_result == Apu::ApuExecOK) ? true : false;
        if (!cmplt.exec_dec_cmplt.is_valid_frame)
          {
            MEDIA_PLAYER_WARN(AS_ATTENTION_SUB_CODE_DSP_RESULT_ERROR);
          }

        err_t er = MsgLib::send<DecCmpltParam>(MSGQ_AUD_PLY, MsgPriNormal, MSG_AUD_PLY_CMD_DEC_DONE, MSGQ_AUD_PLY, cmplt);
        F_ASSERT(er == ERR_OK);
/* getterじゃね？:TODO*/
      }
      break;
    case Apu::FlushEvent:
      {
        cmplt.stop_dec_cmplt.output_buffer  = packet->flush_dec_cmd.output_buffer;
        cmplt.stop_dec_cmplt.is_valid_frame = (packet->result.exec_result == Apu::ApuExecOK) ? true : false;
        if (!cmplt.stop_dec_cmplt.is_valid_frame)
          {
            MEDIA_PLAYER_WARN(AS_ATTENTION_SUB_CODE_DSP_RESULT_ERROR);
          }

        err_t er = MsgLib::send<DecCmpltParam>(MSGQ_AUD_PLY, MsgPriNormal, MSG_AUD_PLY_CMD_DEC_DONE, MSGQ_AUD_PLY, cmplt);
        F_ASSERT(er == ERR_OK);
/* getterじゃね？:TODO*/
      }
      break;
    default:
      MEDIA_PLAYER_ERR(AS_ATTENTION_SUB_CODE_DSP_ILLEGAL_REPLY);
      return false;
    }
  return true;
}


static uint8_t _decode_init(Track *track)
{
  uint32_t dsp_inf;
  uint8_t rst;

  InitDecCompParam init_dec_comp_param;
  init_dec_comp_param.codec_type = (AudioCodec)track->codec_type;
  init_dec_comp_param.input_sampling_rate = track->sampling_rate;
  init_dec_comp_param.channel_num = (track->channel_number == AS_CHANNEL_STEREO) ? (TwoChannels) : (MonoChannels);
  init_dec_comp_param.frame_sample_num = getSampleNumPerFrame(track);
  init_dec_comp_param.callback = &decoder_comp_done_callback;
  init_dec_comp_param.p_requester = (void*)0;
  init_dec_comp_param.bit_width = AudPcm16Bit;

  rst = AS_decode_init(init_dec_comp_param, g_p_dec_instance, &dsp_inf);
  if (rst != AS_ECODE_OK)
    {
      printf("AS_decode_init error\n");
    }

  if (!AS_decode_recv_done(g_p_dec_instance))
    {
      printf("AS_decode_recv_done error\n");
      return AS_ECODE_QUEUE_OPERATION_ERROR;
    }
  return rst;
}

/*--------------------------------------------------------------------*/
static bool initEs_aac(InitInputDataManagerParam *param)
{
  int32_t err_detail = 0;

  p_simple_fifo_handler = static_cast<CMN_SimpleFifoHandle *>(param->p_simple_fifo_handler);
  memset(&aac_handle,0,sizeof(AdtsHandle));
  /* ADTSファイルオープンおよびフォーマットチェック（エラー詳細は現状未使用） */
  if ( AdtsParser_Initialize(&aac_handle, (CMN_SimpleFifoHandle *)param->p_simple_fifo_handler, reinterpret_cast<AdtsParserErrorDetail*>(&err_detail)) == ADTS_OK )
    {
      return true;
    }

  printf("initEs_aac: AdtsParser_Initialize error\n");
  return false;
}

static bool initEs(Track *track)
{
  int result;
  InitInputDataManagerParam init_param;

  init_param.p_simple_fifo_handler = (void*)&m_simple_fifo_handle;//(void*)&m_cur_input_device_handler;//m_in_device_handler.simple_fifo_handler;
  init_param.codec_type = (AudioCodec)track->codec_type;
  init_param.in_sampling_rate = track->sampling_rate;
  init_param.in_ch_num = (track->channel_number == AS_CHANNEL_STEREO) ? (TwoChannels) : (MonoChannels);

  switch((AudioCodec)track->codec_type)
    {
    case AudCodecMP3:
      result = Mp3Parser_initialize((MP3PARSER_Handle *)&mp3_handle, (CMN_SimpleFifoHandle *)init_param.p_simple_fifo_handler, (MP3PARSER_Config *)&mp3_config);
      if (result == MP3PARSER_SUCCESS)
        {
          return true;
        }
      printf("Mp3Parser_initialize error\n");
      break;

    case AudCodecXAVCLPCM:
      p_simple_fifo_handler = static_cast<CMN_SimpleFifoHandle *>(init_param.p_simple_fifo_handler);
      return true;

    case AudCodecAAC:
      return initEs_aac(&init_param);

    case AudCodecOPUS:
      p_simple_fifo_handler = static_cast<CMN_SimpleFifoHandle *>(init_param.p_simple_fifo_handler);
      return true;

    default:
      printf("initEs:codec_type error\n");
    }
  return false;
}

/*--------------------------------------------------------------------*/
static int getEs_wav(void* es_buf, uint32_t* es_size)
{
  *es_size = get_cur_wav_au_sample_num() * 2 * 2;
  size_t size = CMN_SimpleFifoGetOccupiedSize(p_simple_fifo_handler);
  uint32_t read_size = 0;

  if (size >= *es_size)
    {
      read_size = *es_size;
    }
  else
    {
      read_size = size;
    }
  size_t ret = CMN_SimpleFifoPoll(p_simple_fifo_handler, es_buf, read_size);
  if (ret)
    {
      *es_size = ret;
      return ES_EXIST;
    }

  return ES_END;
}

static int getEs_mp3(void* es_buf, uint32_t* es_size)
{
  int ret = ES_END;
  int ready_to_extract_frames = 0;

  if (0 < *es_size )
    {
      uint32_t max_buf_size = *es_size;
      if (MP3PARSER_SUCCESS == Mp3Parser_pollSingleFrame((MP3PARSER_Handle *)&mp3_handle, (uint8_t *)es_buf, max_buf_size, es_size, &ready_to_extract_frames))
        {
          ret = ES_EXIST;
        }
    }
  return ret;
}

static int getEs_aac(void* es_buf, uint32_t* es_size)
{
  uint32_t max_es_buf_size = *es_size;
  int ret = ES_EXIST;
  uint32_t read_size = 0;
  uint16_t check_result = 0;
  int32_t err_detail = 0;

  if (0 < max_es_buf_size)
    {
      read_size = max_es_buf_size;
      /* ADTSフレーム切り出し（妥当性チェック結果およびエラー詳細は現状未使用） */
      if (AdtsParser_ReadFrame(&aac_handle,
                               reinterpret_cast<int8_t*>(es_buf),
                               &read_size,
                               &check_result,
                               reinterpret_cast<AdtsParserErrorDetail*>(&err_detail)) != ADTS_OK )
        {
          read_size = 0;
        }
    }

  *es_size = read_size;
  if ( read_size == 0 )
    {
      ret = ES_END;
    }
  return ret;
}

#ifdef CONFIG_AUDIOUTILS_UNUSE_ORIGINAL_OPUS_FORMAT
static uint32_t char_to_int(uint8_t ch[4])
{
  return ((uint32_t)ch[0]<<24) | ((uint32_t)ch[1]<<16) | ((uint32_t)ch[2]<< 8) |  (uint32_t)ch[3];
}
#endif /* CONFIG_AUDIOUTILS_UNUSE_ORIGINAL_OPUS_FORMAT */

static int getEs_opus(void* es_buf, uint32_t* es_size)
{
  size_t size = 0;
  if ((size = CMN_SimpleFifoGetOccupiedSize(p_simple_fifo_handler)) <= 0)
    {
      return ES_END;
    }

  CMN_SimpleFifoPeekHandle pPeekHandle;
  if ((CMN_SimpleFifoPeekWithOffset(p_simple_fifo_handler, &pPeekHandle, OPUS_DATA_LEN, 0)) != OPUS_DATA_LEN)
    {
      return ES_END;
    }
  uint8_t date_size[OPUS_DATA_LEN] = { 0 };
  if ((CMN_SimpleFifoCopyFromPeekHandle(&pPeekHandle, date_size, OPUS_DATA_LEN)) != OPUS_DATA_LEN)
    {
      return ES_END;
    }
#ifdef CONFIG_AUDIOUTILS_UNUSE_ORIGINAL_OPUS_FORMAT
  uint32_t len = char_to_int(date_size);
#else
  uint32_t len = date_size[0];
#endif /* CONFIG_AUDIOUTILS_UNUSE_ORIGINAL_OPUS_FORMAT */
  if (size < len + OPUS_DATA_LEN + OPUS_FINAL_RANGE_LEN)
    {
      return ES_END;
    }
  if ((CMN_SimpleFifoPoll(p_simple_fifo_handler, date_size, OPUS_DATA_LEN)) != OPUS_DATA_LEN)
    {
      return ES_END;
    }
#ifdef CONFIG_AUDIOUTILS_UNUSE_ORIGINAL_OPUS_FORMAT
  if ((CMN_SimpleFifoPoll(p_simple_fifo_handler, date_size, OPUS_FINAL_RANGE_LEN)) != OPUS_FINAL_RANGE_LEN)
    {
      return ES_END;
    }
#endif /* CONFIG_AUDIOUTILS_UNUSE_ORIGINAL_OPUS_FORMAT */

  size_t ret = 0;
  if ((ret = CMN_SimpleFifoPoll(p_simple_fifo_handler, es_buf, len)) <= 0)
    {
      return ES_END;
    }
  *es_size = ret;

  return ES_EXIST;
}

static void* getEs(AudioCodec codec_type, uint32_t *es_size)
{
  int es_ret = ES_END;

  switch(codec_type)
    {
    case AudCodecMP3:
      es_ret = getEs_mp3(g_es_buf, es_size);
      break;

    case AudCodecXAVCLPCM:
      es_ret = getEs_wav(g_es_buf, es_size);
      break;

    case AudCodecAAC:
      es_ret = getEs_aac(g_es_buf, es_size);
      break;

    case AudCodecOPUS:
      es_ret = getEs_opus(g_es_buf, es_size);
      break;

    default:
      printf("getEs: codec_type error\n");
      break;
    }
  if(!es_ret)
    {
      return g_es_buf;
    }

  return NULL;

}

static bool finish_Es(AudioCodec codec_type)
{
  switch(codec_type)
    {
    case AudCodecMP3:
      Mp3Parser_finalize((MP3PARSER_Handle *)&mp3_handle);
      return true;
    case AudCodecXAVCLPCM:
      return true;
    case AudCodecAAC:
      {
        int32_t err_detail = 0;
        AdtsParser_Finalize(&aac_handle, reinterpret_cast<AdtsParserErrorDetail*>(&err_detail));
        /* ハンドル情報をクリア */
        memset(&aac_handle,0,sizeof(AdtsHandle));
        return true;
      }
    case AudCodecOPUS:
      return true;
    default:
      printf("finish_Es: codec_type error\n");
      break;
    }
  return false;
}

/*--------------------------------------------------------------------*/
static void _stop(AudioCodec codec_type)
{
  StopDecCompParam param;
  param.output_buffer.size = g_max_pcm_buff_size;
  param.output_buffer.p_buffer = reinterpret_cast<unsigned long*>(g_pcm_buf);

  if (AS_decode_stop(param, g_p_dec_instance) == false)
    {
      printf("AS_decode_stop error\n");
    }

  err_t        err_code;
  MsgQueBlock* que;
  MsgPacket*   msg = 0;

  err_code = MsgLib::referMsgQueBlock(MSGQ_AUD_PLY, &que);
  if (err_code)
    {
      printf("_decode:referMsgQueBlock error\n");
    }

  err_code = que->recv(TIME_FOREVER, &msg);
  if (err_code)
    {
      printf("_decode:que->recv error\n");
    }

  DecCmpltParam dsp_resp = msg->moveParam<DecCmpltParam>();
  AS_decode_recv_done(g_p_dec_instance);

  if (dsp_resp.stop_dec_cmplt.is_valid_frame == false)
    {
      printf("stop decode error\n");
    }

  if (dsp_resp.stop_dec_cmplt.output_buffer.size > 0)
    {
      size_t size;
      size = write(g_pcm_fd,
                   dsp_resp.stop_dec_cmplt.output_buffer.p_buffer,
                   dsp_resp.stop_dec_cmplt.output_buffer.size);
      if (size != dsp_resp.stop_dec_cmplt.output_buffer.size)
        {
          F_ASSERT(0);
        }
    }

  que->pop();

  finish_Es(codec_type);
}

/*--------------------------------------------------------------------------*/
static void init_freq_lock(void)
{
  g_player_lock.count = 0;
  g_player_lock.info = PM_CPUFREQLOCK_TAG('A', 'P', 0);
  g_player_lock.flag = PM_CPUFREQLOCK_FLAG_HV;
}

static void player_freq_lock(uint32_t mode)
{
  g_player_lock.flag = mode;
  up_pm_acquire_freqlock(&g_player_lock);
}
/*--------------------------------------------------------------------------*/
static void player_freq_release(void)
{
  up_pm_release_freqlock(&g_player_lock);
}

/****************************************************************************/
static bool _createDir(void)
{
  int ret;
  const char *name = CONFIG_TEST_DECODER_OUTPUT_FILE_MOUNTPT;

  g_dirp = opendir("/mnt");
  if (!g_dirp)
    {
      printf("opendir err(errno:%d)\n",errno);
      return false;
    }
  ret = mkdir(name, 0777);
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

  snprintf(fname, MAX_PATH_LENGTH, "%s/%s.pcm",
           CONFIG_TEST_DECODER_OUTPUT_FILE_MOUNTPT, track->title);
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
extern "C" int test_decoder_component_main(int argc, char *argv[])
#endif
{
  Track track;
  int fd  = -1;
  int err = 0;
  bool ret;
  uint8_t rst;
  uint32_t dsp_inf;

  g_p_dec_instance = 0;

  init_freq_lock();
  player_freq_lock(PM_CPUFREQLOCK_FLAG_HV); 

  if (!_createDir())
    {
      goto exit_1;
    }

  err = init_libraries();
  if (err)
    {
      goto exit_1;
    }

  if (app_init_simple_fifo(&m_simple_fifo_handle,
                           m_simple_fifo_buf,
                           SIMPLE_FIFO_BUF_SIZE,
                           &m_cur_input_device_handler))
    {
      printf("app_init_simple_fifo Error\n");
      goto exit_2;
    }

  if (app_open_playlist())
    {
      printf("app_open_playlist Error\n");
      goto exit_2;
    }

  while (true)
    {
      int eof = 0;

      /* contents file open */
      fd = open_contents(&track);
      if (fd < 0)
        {
          break;
        }

      ret = initEs(&track);
      if (ret == false)
        {
          printf("initEs: error\n");
          break;
        }

      g_pcm_fd = open_output(&track);
      if (g_pcm_fd < 0)
        {
          printf("open_output error\n");
          break;
        }

      rst = AS_decode_activate((AudioCodec)track.codec_type,
                               &g_p_dec_instance,
                               DEC_APU_CMD_POOL, MSGQ_AUD_DSP,
                               &dsp_inf);
      if (rst != AS_ECODE_OK)
        {
          printf("AS_decode_activate Error\n");
          break;
        }

      err = _decode_init(&track);
      if (err != AS_ECODE_OK)
        {
          break;
        }

      while (true)
        {
          /* read file */
          if (eof == 0)
          {
            ret = app_push_simple_fifo(fd);
          }

          if (ret == 0)
            {
              uint32_t es_size = g_max_es_buff_size;
              void *es_addr = getEs((AudioCodec)track.codec_type, &es_size);
              if (es_addr)
                {
                  _decode(es_addr, es_size);
                }
              else
                {
                  _stop((AudioCodec)track.codec_type);
                  close(g_pcm_fd);
                  g_pcm_fd = -1;
                  break;
                }
            }
          else if(ret == 2)
            {
              eof = 1;
              ret = 0;
            }
        }

      AS_decode_deactivate(g_p_dec_instance);
      g_p_dec_instance = 0;
      close_contents(fd);
      fd = -1;
    }

  if (g_p_dec_instance)
    {
      AS_decode_deactivate(g_p_dec_instance);
    }

  if (fd > 0)
    {
      close_contents(fd);
    }

  if (g_pcm_fd > 0)
    {
      close(g_pcm_fd);
    }

  app_close_playlist();

exit_2:
  finalize_libraries();

exit_1:
  if (g_dirp > 0)
    {
      closedir(g_dirp);
    }

  player_freq_release();

  printf("test_decord exit!\n");

  return 0;
}
