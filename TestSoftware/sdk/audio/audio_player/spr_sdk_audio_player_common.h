/****************************************************************************
 * test/sqa/singlefunction/audio_player/spr_sdk_audio_player_common.h
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

#ifndef CONFIG_AUDIOUTILS_PLAYLIST_PLAYFILE_MOUNTPT
#  define CONFIG_AUDIOUTILS_PLAYLIST_PLAYFILE_MOUNTPT "/mnt/sd0/AUDIO"
#endif

/* PlayList file name */

#define PLAY_LIST_NAME "TRACK_DB.CSV"

/* For FIFO */

#define WRITE_SIMPLE_FIFO_SIZE  2560
#define SIMPLE_FIFO_FRAME_NUM   9
#define SIMPLE_FIFO_BUF_SIZE    WRITE_SIMPLE_FIFO_SIZE * SIMPLE_FIFO_FRAME_NUM

#define FIFO_RESULT_OK  0
#define FIFO_RESULT_ERR 1
#define FIFO_RESULT_EOF 2
#define FIFO_RESULT_FUL 3

/* Default Volume. -20dB */

#define PLAYER_DEF_VOLUME  -200

/* Mute Volume. */

#define PLAYER_MUTE_VOLUME AS_VOLUME_MUTE

/* Play list control. */

#define PLAY_NEXT 0
#define PLAY_PREV 1

/****************************************************************************
 * Public Type
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
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

bool app_open_contents_dir(player_info_s *player);
bool app_close_contents_dir(player_info_s *player);
bool app_open_playlist(player_info_s *player, bool repeat);
bool app_close_playlist(player_info_s *player);
bool app_get_next_track(player_info_s *player);
bool app_get_prev_track(player_info_s *player);
void app_input_device_callback(uint32_t size);
bool app_init_simple_fifo(player_info_s *player);
int app_push_simple_fifo(player_info_s *player, int fd);
bool app_first_push_simple_fifo(player_info_s *player);
bool app_refill_simple_fifo(player_info_s *player);
bool printAudCmdResult(uint8_t command_code, AudioResult& result);
void app_attention_callback(uint8_t module_id,
                                   uint8_t error_code,
                                   uint8_t sub_code,
                                   const char* file_name,
                                   uint32_t line);
bool app_init_attention(void);
bool app_act_audio_sub_system_main_sub(void);
bool app_act_audio_sub_system_main_only(void);
void app_deact_audio_sub_system_main_sub(void);
void app_deact_audio_sub_system_main_only(void);
bool app_power_on(void);
bool app_power_off(void);
bool app_set_ready(void);
bool app_init_output_select(void);
bool app_init_output_select_i2s(void);
int app_init_i2s_param(void);
bool app_set_volume(int input1_db, int input2_db, int master_db);
bool app_set_player_status(player_info_s *player, player_info_s *sub_player);
bool app_set_player_i2s_status(player_info_s *player, player_info_s *sub_player);
int app_init_player(player_type_e type,
                           uint8_t codec_type,
                           uint32_t sampling_rate,
                           uint8_t channel_number);
int app_play_player(player_type_e type);
bool app_stop_player(player_type_e type);
int app_play_file_open(FAR const char *file_path, FAR int32_t *file_size);
bool app_get_track(player_info_s *player, uint8_t type);
bool app_open_file(player_info_s *player);
bool app_start(player_info_s *player);
bool app_stop(player_info_s *player);
void app_init_fifo_info(player_type_e type, player_info_s *player);
bool app_allocate_work_area(player_info_s *player);
void app_free_work_area(player_info_s *player);

