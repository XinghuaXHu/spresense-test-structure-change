/****************************************************************************
 * test/sqa/singlefunction/audio_player/spr_sdk_15451.c
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

#include "spr_sdk_audio_player_common.h"
#include <arch/board/board.h>
#include <pthread.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Play time(sec) */

#define PLAYER_PLAY_TIME 5

/* Number of test */

#define PLAYER_FMT_TEST2_NUM  1

/* Format ID */

enum fmt_id_e
{
  FMT_AAC = 0,
  FMT_MP3,
  FMT_OPUS,
  FMT_LPCM,
  FMT_NUM
};

/* Format strings table. */

#define FMT_STR_LEN  8

static const char test_fmt_table[FMT_NUM][2][FMT_STR_LEN] = 
{
  {".AAC",  ".aac"},
  {".MP3",  ".mp3"},
  {".OPUS", ".opus"},
  {".WAV",  ".wav"},
};

/* Play format table of test case2. */

static const uint8_t test2_fmt_table[PLAYER_FMT_TEST2_NUM][2] =
{
  {FMT_LPCM, FMT_MP3  },
};

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Public Type Declarations
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* For control player */

static struct player_info_s  s_player_info;
static struct player_info_s  s_player_info_sub;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void play_process(player_info_s *player,
                         player_info_s *sub_player,
                         uint32_t play_time)
{
  /* Timer Start */
  time_t start_time;
  time_t cur_time;

  time(&start_time);

  do
    {
      /* Check the FIFO every 5 ms and fill if there is space. */
      usleep(5 * 1000);
      if (!app_refill_simple_fifo(player))
        {
          break;
        }

      if (sub_player != NULL)
      {
        if (!app_refill_simple_fifo(sub_player))
          {
            break;
          }
      }
    } while((time(&cur_time) - start_time) < play_time);
}

static bool get_track_with_fmt(player_info_s *player, uint8_t fmt_id)
{
  bool result = false;

  while(app_get_next_track(player))
    {
      if ((NULL != strstr(player->track.title, test_fmt_table[fmt_id][0])) ||
          (NULL != strstr(player->track.title, test_fmt_table[fmt_id][1])))
        {
          result = true;
          break;
        }
    }
  player->playlist_ins->restart();
  return result;
}

static bool test_case2(player_info_s *player, player_info_s *sub_player)
{
  for (int i = 0; i < PLAYER_FMT_TEST2_NUM; i++)
    {
      printf("[Play %s + %s format file]\n",
        test_fmt_table[test2_fmt_table[i][0]][0],
        test_fmt_table[test2_fmt_table[i][1]][0]);
      if (!get_track_with_fmt(player, test2_fmt_table[i][0]))
        {
          printf("!!Not found file. skip.\n");
          continue;
        }
      if (!get_track_with_fmt(sub_player, test2_fmt_table[i][1]))
        {
          printf("!!Not found file. skip.\n");
          continue;
        }

      /* Open play contents. */

      if (!app_open_file(player))
        {
          printf("Error: [main] app_open_file() failuer.\n");
          return false;
        }
      if (!app_open_file(sub_player))
        {
          printf("Error: [sub] app_open_file() failuer.\n");
          return false;
        }

      /* Start player operation. */

      if (!app_start(player))
        {
          printf("Error: [main] app_start_player() failure.\n");
          return false;
        }
      if (!app_start(sub_player))
        {
          printf("Error: [sub] app_start_player() failure.\n");
          return false;
        }

      /* Running... */

      play_process(player, sub_player, PLAYER_PLAY_TIME);

      /* Stop player operation. */

      if (!app_stop(sub_player))
        {
          printf("Error: [sub] app_stop() failure.\n");
          return false;
        }
      if (!app_stop(player))
        {
          printf("Error: [main] app_stop() failure.\n");
          return false;
        }
    }
  return true;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int player_test_spr_sdk_15451(void)
{
  /* Initialize simple fifo information. */

  app_init_fifo_info(PLAYER_TYPE_MAIN, &s_player_info);
  app_allocate_work_area(&s_player_info);
  app_init_fifo_info(PLAYER_TYPE_SUB, &s_player_info_sub);
  app_allocate_work_area(&s_player_info_sub);

  /* Next, Activate the features used by AudioSubSystem. */

  if (!app_act_audio_sub_system_main_sub())
    {
      printf("Error: act_audiosubsystem() failure.\n");
      return 1;
    }

  /* On and after this point, AudioSubSystem must be active.
   * Register the callback function to be notified when a problem occurs.
   */

  /* Open directory of play contents. */

  if (!app_open_contents_dir(&s_player_info))
    {
      printf("Error: [main] app_open_contents_dir() failure.\n");
      return 1;
    }
  if (!app_open_contents_dir(&s_player_info_sub))
    {
      printf("Error: [sub] app_open_contents_dir() failure.\n");
      return 1;
    }

  /* Change AudioSubsystem to Ready state so that I/O parameters can be changed. */

  if (!app_power_on())
    {
      printf("Error: app_power_on() failure.\n");
      return 1;
    }

  /* Open playlist. */

  if (!app_open_playlist(&s_player_info, false))
    {
      printf("Error: [main] app_open_playlist() failure.\n");
      return 1;
    }
  if (!app_open_playlist(&s_player_info_sub, false))
    {
      printf("Error: [sub] app_open_playlist() failure.\n");
      return 1;
    }

  /* Initialize simple fifo. */

  if (!app_init_simple_fifo(&s_player_info))
    {
      printf("Error: [main] app_init_simple_fifo() failure.\n");
      return false;
    }
  if (!app_init_simple_fifo(&s_player_info_sub))
    {
      printf("Error: [sub] app_init_simple_fifo() failure.\n");
      return false;
    }

  /* Set the device to output the mixed audio. */

  if (!app_init_output_select())
    {
      printf("Error: app_init_output_select() failure.\n");
      return 1;
    }

  /* Set player operation mode. */

  if (!app_set_player_status(&s_player_info, &s_player_info_sub))
    {
      printf("Error: app_set_player_status() failure.\n");
      return 1;
    }

  /* Cancel output mute. */

  app_set_volume(0, 0, 0);

  if (board_external_amp_mute_control(false) != OK)
    {
      printf("Error: board_external_amp_mute_control(false) failuer.\n");
      return 1;
    }

  /*========================================================================*/
  /* play LPCM+MP3                                                  */
  /*========================================================================*/
  printf("---------------------------------------------------\n");
  printf(" play LPCM+MP3                                     \n");
  printf("---------------------------------------------------\n");
  if (!test_case2(&s_player_info, &s_player_info_sub))
    {
      printf("Error: test_case2 failuer.\n");
      goto exit_test;
    }

exit_test:

  /* Set output mute. */

  if (board_external_amp_mute_control(true) != OK)
    {
      printf("Error: board_external_amp_mute_control(true) failuer.\n");
    }

  /* Close playlist. */

  if (!app_close_playlist(&s_player_info))
    {
      printf("Error: [main] app_close_playlist() failure.\n");
    }
  if (!app_close_playlist(&s_player_info_sub))
    {
      printf("Error: [sub] app_close_playlist() failure.\n");
    }

  if (!app_close_contents_dir(&s_player_info))
    {
      printf("Error: [main] app_close_contents_dir() failure.\n");
    }
  if (!app_close_contents_dir(&s_player_info_sub))
    {
      printf("Error: [sub] app_close_contents_dir() failure.\n");
    }

  /* Return the state of AudioSubSystem before voice_call operation. */

  if (!app_set_ready())
    {
      printf("Error: app_set_ready() failure.\n");
    }

  /* Change AudioSubsystem to PowerOff state. */

  if (!app_power_off())
    {
      printf("Error: app_power_off() failure.\n");
    }

  /* Deactivate the features used by AudioSubSystem. */

  app_deact_audio_sub_system_main_sub();

  /* Free work area. */

  app_free_work_area(&s_player_info);
  app_free_work_area(&s_player_info_sub);

  return 0;
}
