/****************************************************************************
 * test/sqa/singlefunction/audio_player/spr_sdk_audio_player_common.c
 *
 *   Copyright (C) 2017 Sony Corporation
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
 * 3. Neither the name Sony nor the names of its contributors
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
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include "audio/audio_high_level_api.h"
#include "memutils/simple_fifo/CMN_SimpleFifo.h"
#include "memutils/message/Message.h"
#include "include/msgq_id.h"
#include "include/mem_layout.h"
#include "playlist/playlist.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define AUDIOFILE_ROOTPATH "/mnt/sd0/AUDIO"
#define PLAYLISTFILE_PATH "/mnt/sd0/PLAYLIST"
#define DSPBIN_PATH "/mnt/sd0/BIN"

/* PlayList file name */

#define PLAY_LIST_NAME "TRACK_DB.CSV"

/* For FIFO */

/* WRITE_SIMPLE_FIFO_SIZE.
 *  This SIMPLE_FIFO_SIZE will be decided from your application system. 
 *  Correctly, on the SDK side, read the following memory.
 *    MP3: maximum 1440 bytes
 *    AAC: maximum 1024 bytes
 *    WAV: 16bit-2560 bytes, 24bit-3840 bytes
 *  It can be selected with the codec to be played.
 *  When playing multiple codecs, please select the largest memory size.
 *  There is no problem increasing the memory size. If it is made smaller,
 *  FIFO under is possibly generated, so it is necessary to be careful.
 *  Please adjust yourself.
 *
 *  This application sets the size based on playing WAV.
 *  Moreover, it is making it the minimum size to reduce the memory amount.
 */

#define WRITE_SIMPLE_FIFO_SIZE  (3840 * 4)
#define SIMPLE_FIFO_FRAME_NUM   10
#define SIMPLE_FIFO_BUF_SIZE    WRITE_SIMPLE_FIFO_SIZE * SIMPLE_FIFO_FRAME_NUM

#define FIFO_RESULT_OK  0
#define FIFO_RESULT_ERR 1
#define FIFO_RESULT_EOF 2
#define FIFO_RESULT_FUL 3

/* Default Volume. -20dB */

#define PLAYER_DEF_VOLUME  -200

/* Mute Volume. */

#define PLAYER_MUTE_VOLUME AS_VOLUME_MUTE

#define PLAY_NEXT 0
#define PLAY_PREV 1

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Public Type Declarations
 ****************************************************************************/

/* For FIFO */

struct player_fifo_info_s
{
  CMN_SimpleFifoHandle          handle;
  AsPlayerInputDeviceHdlrForRAM input_device;
  uint32_t *fifo_area;
  uint8_t  *read_buf;
};

/* For play file */

struct player_file_info_s
{
  int32_t size;
  DIR    *dirp;
  int     fd;
};

enum player_type_e
{
  PLAYER_TYPE_MAIN,
  PLAYER_TYPE_SUB
};

/* Player info */

struct player_info_s
{
  player_type_e type;
  Track   track;
  struct player_fifo_info_s   fifo;
  struct player_file_info_s   file;
#ifdef CONFIG_AUDIOUTILS_PLAYLIST
  Playlist *playlist_ins = NULL;
#else
error "AUDIOUTILS_PLAYLIST is not enable"
#endif
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

bool app_open_contents_dir(player_info_s *player)
{
  DIR *dirp;
  const char *name = AUDIOFILE_ROOTPATH;
  
  dirp = opendir(name);

  if (!dirp)
    {
      printf("Error: %s directory path error. check the path!\n", name);
      return false;
    }

  player->file.dirp = dirp;

  return true;
}

bool app_close_contents_dir(player_info_s *player)
{
  closedir(player->file.dirp);

  return true;
}


bool app_open_playlist(player_info_s *player, bool repeat)
{
  bool result = false;

  if (player->playlist_ins != NULL)
    {
      printf("Error: Open playlist failure. Playlist is already open\n");
      return false;
    }

  player->playlist_ins = new Playlist(PLAY_LIST_NAME);
  
  result = player->playlist_ins->init(PLAYLISTFILE_PATH);
  if (!result)
    {
      printf("Error: Playlist::init() failure.\n");
      return false;
    }

  player->playlist_ins->setPlayMode(Playlist::PlayModeNormal);
  if (!result)
    {
      printf("Error: Playlist::setPlayMode() failure.\n");
      return false;
    }

  if (repeat)
    {
      player->playlist_ins->setRepeatMode(Playlist::RepeatModeOn);
    }
  else
    {
      player->playlist_ins->setRepeatMode(Playlist::RepeatModeOff);
    }
  if (!result)
    {
      printf("Error: Playlist::setRepeatMode() failure.\n");
      return false;
    }

  player->playlist_ins->select(Playlist::ListTypeAllTrack, NULL);
  if (!result)
    {
      printf("Error: Playlist::select() failure.\n");
      return false;
    }

  return true;
}

bool app_close_playlist(player_info_s *player)
{
  if (player->playlist_ins == NULL)
    {
      printf("Error: Close playlist failure. Playlist is not open\n");
      return false;
    }

  delete player->playlist_ins;
  player->playlist_ins = NULL;

  return true;
}

bool app_get_next_track(player_info_s *player)
{
  bool ret;

  if (player->playlist_ins == NULL)
    {
      printf("Error: Get next track failure. Playlist is not open\n");
      return false;
    }

  ret = player->playlist_ins->getNextTrack(&player->track);

  return ret;
}

bool app_get_prev_track(player_info_s *player)
{
  bool ret;

  if (player->playlist_ins == NULL)
    {
      printf("Error: Get next track failure. Playlist is not open\n");
      return false;
    }

  ret = player->playlist_ins->getPrevTrack(&player->track);

  return ret;
}

void app_input_device_callback(uint32_t size)
{
    /* do nothing */
}

bool app_init_simple_fifo(player_info_s *player)
{
  if (CMN_SimpleFifoInitialize(&player->fifo.handle,
                               player->fifo.fifo_area,
                               SIMPLE_FIFO_BUF_SIZE, NULL) != 0)
    {
      printf("Error: Fail to initialize simple FIFO.");
      return false;
    }
  CMN_SimpleFifoClear(&player->fifo.handle);

  player->fifo.input_device.simple_fifo_handler = (void*)(&player->fifo.handle);
  player->fifo.input_device.callback_function = app_input_device_callback;

  return true;
}

int app_push_simple_fifo(player_info_s *player, int fd)
{
  int ret;

  ret = read(fd, player->fifo.read_buf, WRITE_SIMPLE_FIFO_SIZE);
  if (ret < 0)
    {
      printf("Error: Fail to read file. errno:%d\n", get_errno());
      return FIFO_RESULT_ERR;
    }

  if (CMN_SimpleFifoOffer(&player->fifo.handle, (const void*)(player->fifo.read_buf), ret) == 0)
    {
      return FIFO_RESULT_FUL;
    }

  player->file.size = player->file.size - ret;
  if (player->file.size == 0)
    {
      return FIFO_RESULT_EOF;
    }
  return FIFO_RESULT_OK;
}

bool app_first_push_simple_fifo(player_info_s *player)
{
  int i;
  int ret = FIFO_RESULT_OK;

  for(i = 0; i < SIMPLE_FIFO_FRAME_NUM-1; i++)
    {
      if ((ret = app_push_simple_fifo(player, player->file.fd)) != FIFO_RESULT_OK)
        {
          break;
        }
    }
  return (ret != FIFO_RESULT_ERR) ? true : false;
}

bool app_refill_simple_fifo(player_info_s *player)
{
  int32_t ret = FIFO_RESULT_OK;
  size_t  vacant_size;

  vacant_size = CMN_SimpleFifoGetVacantSize(&player->fifo.handle);

  if ((vacant_size != 0) && (vacant_size > WRITE_SIMPLE_FIFO_SIZE))
    {
      int cnt = 1;
      if (vacant_size > WRITE_SIMPLE_FIFO_SIZE*3)
        {
          cnt = 3;
        }
      else if (vacant_size > WRITE_SIMPLE_FIFO_SIZE*2)
        {
          cnt = 2;
        }
      
      for (int i = 0; i < cnt; i++)
        {
          if ((ret = app_push_simple_fifo(player, player->file.fd)) != FIFO_RESULT_OK)
            {
              break;
            }
        }
    }

  return (ret != FIFO_RESULT_ERR) ? true : false;
}

bool printAudCmdResult(uint8_t command_code, AudioResult& result)
{
  if (AUDRLT_ERRORRESPONSE == result.header.result_code) {
    printf("Command code(0x%x): AUDRLT_ERRORRESPONSE:"
           "Module id(0x%x): Error code(0x%x)\n",
            command_code,
            result.error_response_param.module_id,
            result.error_response_param.error_code);
    return false;
  }
  else if (AUDRLT_ERRORATTENTION == result.header.result_code) {
    printf("Command code(0x%x): AUDRLT_ERRORATTENTION\n", command_code);
    return false;
  }
  return true;
}

void app_attention_callback(uint8_t module_id,
                                   uint8_t error_code,
                                   uint8_t sub_code,
                                   const char* file_name,
                                   uint32_t line)
{
  printf("Attention!! %s L%d ecode %d subcode %d\n", file_name, line, error_code, sub_code);
}

bool app_act_audio_sub_system_main_sub(void)
{
  bool result = false;

  /* Activate manager of AudioSubSystem. */

  AudioSubSystemIDs ids;
  ids.app         = MSGQ_AUD_APP;
  ids.mng         = MSGQ_AUD_MNG;
  ids.player_main = MSGQ_AUD_PLY;
  ids.player_sub  = MSGQ_AUD_SUB_PLY;
  ids.mixer       = MSGQ_AUD_OUTPUT_MIX;
  ids.recorder    = 0xFF;
  ids.effector    = 0xFF;
  ids.recognizer  = 0xFF;

  AS_CreateAudioManager(ids, app_attention_callback);

  AsCreatePlayerParam_t player_create_param;
  player_create_param.msgq_id.player = MSGQ_AUD_PLY;
  player_create_param.msgq_id.mng    = MSGQ_AUD_MNG;
  player_create_param.msgq_id.mixer  = MSGQ_AUD_OUTPUT_MIX;
  player_create_param.msgq_id.dsp    = MSGQ_AUD_DSP;
  player_create_param.pool_id.es     = DEC_ES_MAIN_BUF_POOL;
  player_create_param.pool_id.pcm    = REND_PCM_BUF_POOL;
  player_create_param.pool_id.dsp    = DEC_APU_CMD_POOL;

  result = AS_CreatePlayer(AS_PLAYER_ID_0, &player_create_param);

  if (!result)
    {
      printf("Error: AS_CratePlayer() failure. system memory insufficient!\n");
      return false;
    }

  AsCreatePlayerParam_t sub_player_create_param;
  sub_player_create_param.msgq_id.player = MSGQ_AUD_SUB_PLY;
  sub_player_create_param.msgq_id.mng    = MSGQ_AUD_MNG;
  sub_player_create_param.msgq_id.mixer  = MSGQ_AUD_OUTPUT_MIX;
  sub_player_create_param.msgq_id.dsp    = MSGQ_AUD_DSP;
  sub_player_create_param.pool_id.es     = DEC_ES_SUB_BUF_POOL;
  sub_player_create_param.pool_id.pcm    = REND_PCM_SUB_BUF_POOL;
  sub_player_create_param.pool_id.dsp    = DEC_APU_CMD_POOL;

  result = AS_CreatePlayer(AS_PLAYER_ID_1, &sub_player_create_param);

  if (!result)
    {
      printf("Error: AS_CratePlayer() failure. system memory insufficient!\n");
      return false;
    }

  /* Activate mixer feature. */

  AsCreateOutputMixParam_t output_mix_act_param;
  output_mix_act_param.msgq_id.mixer = MSGQ_AUD_OUTPUT_MIX;
  output_mix_act_param.msgq_id.render_path0_filter_dsp = MSGQ_AUD_PFDSP0;
  output_mix_act_param.msgq_id.render_path1_filter_dsp = MSGQ_AUD_PFDSP1;
  output_mix_act_param.pool_id.render_path0_filter_pcm = PF0_PCM_BUF_POOL;
  output_mix_act_param.pool_id.render_path1_filter_pcm = PF1_PCM_BUF_POOL;
  output_mix_act_param.pool_id.render_path0_filter_dsp = PF0_APU_CMD_POOL;
  output_mix_act_param.pool_id.render_path1_filter_dsp = PF1_APU_CMD_POOL;

  result = AS_CreateOutputMixer(&output_mix_act_param);
  if (!result)
    {
      printf("Error: AS_CreateOutputMixer() failed. system memory insufficient!\n");
      return false;
    }

  /* Activate renderer feature. */

  AsCreateRendererParam_t renderer_create_param;
  renderer_create_param.msgq_id.dev0_req  = MSGQ_AUD_RND_PLY;
  renderer_create_param.msgq_id.dev0_sync = MSGQ_AUD_RND_PLY_SYNC;
  renderer_create_param.msgq_id.dev1_req  = MSGQ_AUD_RND_SUB;
  renderer_create_param.msgq_id.dev1_sync = MSGQ_AUD_RND_SUB_SYNC;

  result = AS_CreateRenderer(&renderer_create_param);
  if (!result)
    {
      printf("Error: AS_CreateRenderer() failure. system memory insufficient!\n");
      return false;
    }

  return true;
}

bool app_act_audio_sub_system_main_only(void)
{
  bool result = false;

  /* Activate manager of AudioSubSystem. */

  AudioSubSystemIDs ids;
  ids.app         = MSGQ_AUD_APP;
  ids.mng         = MSGQ_AUD_MNG;
  ids.player_main = MSGQ_AUD_PLY;
  ids.player_sub  = 0xFF;
  ids.mixer       = MSGQ_AUD_OUTPUT_MIX;
  ids.recorder    = 0xFF;
  ids.effector    = 0xFF;
  ids.recognizer  = 0xFF;

  AS_CreateAudioManager(ids, app_attention_callback);

  AsCreatePlayerParam_t player_create_param;
  player_create_param.msgq_id.player = MSGQ_AUD_PLY;
  player_create_param.msgq_id.mng    = MSGQ_AUD_MNG;
  player_create_param.msgq_id.mixer  = MSGQ_AUD_OUTPUT_MIX;
  player_create_param.msgq_id.dsp    = MSGQ_AUD_DSP;
  player_create_param.pool_id.es     = DEC_ES_MAIN_BUF_POOL;
  player_create_param.pool_id.pcm    = REND_PCM_BUF_POOL;
  player_create_param.pool_id.dsp    = DEC_APU_CMD_POOL;

  result = AS_CreatePlayer(AS_PLAYER_ID_0, &player_create_param);

  if (!result)
    {
      printf("Error: AS_CratePlayer() failure. system memory insufficient!\n");
      return false;
    }

  /* Activate mixer feature. */

  AsCreateOutputMixParam_t output_mix_act_param;
  output_mix_act_param.msgq_id.mixer = MSGQ_AUD_OUTPUT_MIX;
  output_mix_act_param.msgq_id.render_path0_filter_dsp = MSGQ_AUD_PFDSP0;
  output_mix_act_param.msgq_id.render_path1_filter_dsp = MSGQ_AUD_PFDSP1;
  output_mix_act_param.pool_id.render_path0_filter_pcm = PF0_PCM_BUF_POOL;
  output_mix_act_param.pool_id.render_path1_filter_pcm = PF1_PCM_BUF_POOL;
  output_mix_act_param.pool_id.render_path0_filter_dsp = PF0_APU_CMD_POOL;
  output_mix_act_param.pool_id.render_path1_filter_dsp = PF1_APU_CMD_POOL;

  result = AS_CreateOutputMixer(&output_mix_act_param);
  if (!result)
    {
      printf("Error: AS_CreateOutputMixer() failed. system memory insufficient!\n");
      return false;
    }

  /* Activate renderer feature. */

  AsCreateRendererParam_t renderer_create_param;
  renderer_create_param.msgq_id.dev0_req  = MSGQ_AUD_RND_PLY;
  renderer_create_param.msgq_id.dev0_sync = MSGQ_AUD_RND_PLY_SYNC;
  renderer_create_param.msgq_id.dev1_req  = 0xFF;
  renderer_create_param.msgq_id.dev1_sync = 0xFF;

  result = AS_CreateRenderer(&renderer_create_param);
  if (!result)
    {
      printf("Error: AS_CreateRenderer() failure. system memory insufficient!\n");
      return false;
    }

  return true;
}

void app_deact_audio_sub_system_main_sub(void)
{
  AS_DeleteAudioManager();
  AS_DeletePlayer(AS_PLAYER_ID_0);
  AS_DeletePlayer(AS_PLAYER_ID_1);
  AS_DeleteOutputMix();
  AS_DeleteRenderer();
}

void app_deact_audio_sub_system_main_only(void)
{
  AS_DeleteAudioManager();
  AS_DeletePlayer(AS_PLAYER_ID_0);
  AS_DeleteOutputMix();
  AS_DeleteRenderer();
}

bool app_power_on(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_POWERON;
  command.header.command_code  = AUDCMD_POWERON;
  command.header.sub_code      = 0x00;
  command.power_on_param.enable_sound_effect = AS_DISABLE_SOUNDEFFECT;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

bool app_power_off(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_POWEROFF_STATUS;
  command.header.command_code  = AUDCMD_SETPOWEROFFSTATUS;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

bool app_set_ready(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_READY_STATUS;
  command.header.command_code  = AUDCMD_SETREADYSTATUS;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

bool app_init_output_select(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_INITOUTPUTSELECT;
  command.header.command_code  = AUDCMD_INITOUTPUTSELECT;
  command.header.sub_code      = 0;
  command.init_output_select_param.output_device_sel = AS_OUT_SP;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

bool app_init_output_select_i2s(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_INITOUTPUTSELECT;
  command.header.command_code  = AUDCMD_INITOUTPUTSELECT;
  command.header.sub_code      = 0;
  command.init_output_select_param.output_device_sel = AS_OUT_I2S;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

int app_init_i2s_param(void)
{
    AudioCommand command;
    command.header.packet_length = LENGTH_INITI2SPARAM;
    command.header.command_code  = AUDCMD_INITI2SPARAM;
    command.header.sub_code      = 0;
    command.init_i2s_param.i2s_id = AS_I2S1;
    command.init_i2s_param.rate = 48000;
    command.init_i2s_param.bypass_mode_en = AS_I2S_BYPASS_MODE_DISABLE;
    AS_SendAudioCommand(&command);

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    if (result.header.result_code != AUDRLT_INITI2SPARAMCMPLT) {
        printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
                command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
        return 1;
    }
    return 0;
}

bool app_set_volume(int input1_db, int input2_db, int master_db)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SETVOLUME;
  command.header.command_code  = AUDCMD_SETVOLUME;
  command.header.sub_code      = 0;
  command.set_volume_param.input1_db = input1_db;
  command.set_volume_param.input2_db = input2_db;
  command.set_volume_param.master_db = master_db;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

bool app_set_player_status(player_info_s *player, player_info_s *sub_player)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_PLAYER_STATUS;
  command.header.command_code = AUDCMD_SETPLAYERSTATUS;
  command.header.sub_code = 0x00;
  command.set_player_sts_param.player0.input_device              = AS_SETPLAYER_INPUTDEVICE_RAM;
  command.set_player_sts_param.player0.ram_handler               = &player->fifo.input_device;
  command.set_player_sts_param.player0.output_device             = AS_SETPLAYER_OUTPUTDEVICE_SPHP;
  if (sub_player != NULL)
    {
      command.set_player_sts_param.active_player                 = AS_ACTPLAYER_BOTH;
      command.set_player_sts_param.player1.input_device          = AS_SETPLAYER_INPUTDEVICE_RAM;
      command.set_player_sts_param.player1.ram_handler           = &sub_player->fifo.input_device;
      command.set_player_sts_param.player1.output_device         = AS_SETPLAYER_OUTPUTDEVICE_SPHP;
    }
  else
    {
      command.set_player_sts_param.active_player                 = AS_ACTPLAYER_MAIN;
      command.set_player_sts_param.player1.input_device          = 0x00;
      command.set_player_sts_param.player1.ram_handler           = NULL;
      command.set_player_sts_param.player1.output_device         = 0x00;
    }
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

bool app_set_player_i2s_status(player_info_s *player, player_info_s *sub_player)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_PLAYER_STATUS;
  command.header.command_code = AUDCMD_SETPLAYERSTATUS;
  command.header.sub_code = 0x00;
  command.set_player_sts_param.player0.input_device              = AS_SETPLAYER_INPUTDEVICE_RAM;
  command.set_player_sts_param.player0.ram_handler               = &player->fifo.input_device;
  command.set_player_sts_param.player0.output_device             = AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT;
  if (sub_player != NULL)
    {
      command.set_player_sts_param.active_player                 = AS_ACTPLAYER_BOTH;
      command.set_player_sts_param.player1.input_device          = AS_SETPLAYER_INPUTDEVICE_RAM;
      command.set_player_sts_param.player1.ram_handler           = &sub_player->fifo.input_device;
      command.set_player_sts_param.player1.output_device         = AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT;
    }
  else
    {
      command.set_player_sts_param.active_player                 = AS_ACTPLAYER_MAIN;
      command.set_player_sts_param.player1.input_device          = 0x00;
      command.set_player_sts_param.player1.ram_handler           = NULL;
      command.set_player_sts_param.player1.output_device         = 0x00;
    }
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

int app_init_player(player_type_e type,
                           uint8_t codec_type,
                           uint32_t sampling_rate,
                           uint8_t channel_number,
                           uint8_t bit_length)
{
  AudioCommand command;

  command.header.packet_length = LENGTH_INIT_PLAYER;
  command.header.command_code  = AUDCMD_INITPLAYER;
  command.header.sub_code      = 0x00;
  command.player.player_id     = (type == PLAYER_TYPE_MAIN) ?
                                  AS_PLAYER_ID_0 : AS_PLAYER_ID_1;
  command.player.init_param.codec_type     = codec_type;
  command.player.init_param.bit_length     = bit_length;
  command.player.init_param.channel_number = channel_number;
  command.player.init_param.sampling_rate  = sampling_rate;
  snprintf(command.player.init_param.dsp_path, AS_AUDIO_DSP_PATH_LEN, "%s", DSPBIN_PATH);
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

int app_play_player(player_type_e type)
{
    AudioCommand command;
    command.header.packet_length = LENGTH_PLAY_PLAYER;
    command.header.command_code  = AUDCMD_PLAYPLAYER;
    command.header.sub_code      = 0x00;
    command.player.player_id     = (type == PLAYER_TYPE_MAIN) ?
                                    AS_PLAYER_ID_0 : AS_PLAYER_ID_1;
    AS_SendAudioCommand(&command);

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    return printAudCmdResult(command.header.command_code, result);
}

bool app_stop_player(player_type_e type)
{
    AudioCommand command;
    command.header.packet_length = LENGTH_STOP_PLAYER;
    command.header.command_code  = AUDCMD_STOPPLAYER;
    command.header.sub_code      = 0x00;
    command.player.player_id     = (type == PLAYER_TYPE_MAIN) ?
                                    AS_PLAYER_ID_0 : AS_PLAYER_ID_1;
    command.player.stop_param.stop_mode = AS_STOPPLAYER_NORMAL; // todo: comment
    AS_SendAudioCommand(&command);

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    return printAudCmdResult(command.header.command_code, result);
}

int app_play_file_open(FAR const char *file_path, FAR int32_t *file_size)
{
  int fd = open(file_path, O_RDONLY);

  *file_size = 0;
  if (fd >= 0)
    {
      struct stat stat_buf;
      if (stat(file_path, &stat_buf) == OK)
        {
          *file_size = stat_buf.st_size;
        }
    }

  return fd;
}

bool app_get_track(player_info_s *player, uint8_t type)
{
  bool track_rst;

  if (type == PLAY_NEXT)
    {
      track_rst = app_get_next_track(player);
    }
  else
    {
      track_rst = app_get_prev_track(player);
    }
  if (!track_rst)
    {
      printf("Error: No more tracks to play.\n");
      return false;
    }
  return true;
}

bool app_open_file(player_info_s *player)
{
  char full_path[128];
  snprintf(full_path, sizeof(full_path), "%s/%s", AUDIOFILE_ROOTPATH, player->track.title);
  printf("open \"%s\"\n", full_path);
  player->file.fd = app_play_file_open(full_path, &player->file.size);
  if (player->file.fd < 0)
    {
      printf("Error: %s open error. check paths and files!\n", full_path);
      return false;
    }
  if (player->file.size == 0)
    {
      close(player->file.fd);
      printf("Error: %s file size is abnormal. check files!\n",full_path);
      return false;
    }

  /* Push data to simple fifo */

  if (!app_first_push_simple_fifo(player))
    {
      printf("Error: app_first_push_simple_fifo() failure.\n");
      CMN_SimpleFifoClear(&player->fifo.handle);
      close(player->file.fd);
      return false;
    }
  return true;
}

bool app_start(player_info_s *player)
{
  /* Init Player */

  if (!app_init_player(player->type,
                       player->track.codec_type,
                       player->track.sampling_rate,
                       player->track.channel_number,
                       player->track.bit_length))
    {
      printf("Error: app_init_player() failure.\n");
      CMN_SimpleFifoClear(&player->fifo.handle);
      close(player->file.fd);
      return false;
    }

  /* Play Player */

  if (!app_play_player(player->type))
    {
      printf("Error: app_play_player() failure.\n");
      CMN_SimpleFifoClear(&player->fifo.handle);
      close(player->file.fd);
      return false;
    }

  return true;
}

bool app_stop(player_info_s *player)
{
  bool result = true;

  if (!app_stop_player(player->type))
    {
      printf("Error: app_stop_player() failure.\n");
      result = false;
    }

  CMN_SimpleFifoClear(&player->fifo.handle);

  if (close(player->file.fd) != 0)
    {
      printf("Error: close() failure.\n");
      result = false;
    }

  return result;
}


void app_init_fifo_info(player_type_e type, player_info_s *player)
{
  player->type = type;
  player->fifo.fifo_area = NULL;
  player->fifo.read_buf  = NULL;
  
}

bool app_allocate_work_area(player_info_s *player)
{
  player->fifo.fifo_area = (uint32_t *)malloc(SIMPLE_FIFO_BUF_SIZE);
  if (player->fifo.fifo_area == NULL)
    {
      printf("Error: malloc(%d) failure.\n", SIMPLE_FIFO_BUF_SIZE);
      return false;
    }
  player->fifo.read_buf = (uint8_t *)malloc(WRITE_SIMPLE_FIFO_SIZE);
  if (player->fifo.read_buf == NULL)
    {
      printf("Error: malloc(%d) failure.\n", WRITE_SIMPLE_FIFO_SIZE);
      free(player->fifo.fifo_area);
      return false;
    }
  return true;
}

void app_free_work_area(player_info_s *player)
{
  if (player->fifo.fifo_area != NULL)
    {
      free(player->fifo.fifo_area);
    }

  if (player->fifo.read_buf != NULL)
    {
      free(player->fifo.read_buf);
    }
}

