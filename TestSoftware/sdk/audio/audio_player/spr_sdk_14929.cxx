/****************************************************************************
 * test/sqa/singlefunction/audio_player/spr_sdk_14928.c
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

#include "spr_sdk_audio_player_common.h"
#include <arch/board/board.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Play time(sec) */

#define PLAYER_PLAY_TIME 11

/* Play type table of test case. */

static const int16_t test_volume_table[PLAYER_PLAY_TIME] =
{
  -400,
  -300,
  -200,
  -100,
  -50,
  AS_VOLUME_MUTE,
  -100,
  -200,
  -300,
  -400,
  -500
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

static bool test_case1(player_info_s *player)
{
  /* Get track. */

  if (!app_get_track(player, PLAY_NEXT))
    {
      printf("Error: app_get_track() failuer.\n");
      return false;
    }

  /* Open play contents. */

  if (!app_open_file(player))
    {
      printf("Error: app_open_file() failuer.\n");
      return false;
    }

  /* Start player operation. */

  if (!app_start(player))
    {
      printf("Error: app_start_player() failure.\n");
      return false;
    }

  /* Running... */

  for (int i = 0; i < PLAYER_PLAY_TIME; i++)
    {
      printf("vol = %d\n", test_volume_table[i]);
      app_set_volume(test_volume_table[i], 0, 0);
      play_process(player, NULL, 1);
    }

  /* Stop player operation. */

  if (!app_stop(player))
    {
      printf("Error: app_stop() failure.\n");
      return false;
    }

  return true;
}

static bool test_case2(player_info_s *player)
{
  /* Get track. */

  if (!app_get_track(player, PLAY_NEXT))
    {
      printf("Error: app_get_track() failuer.\n");
      return false;
    }

  /* Open play contents. */

  if (!app_open_file(player))
    {
      printf("Error: app_open_file() failuer.\n");
      return false;
    }

  /* Start player operation. */

  if (!app_start(player))
    {
      printf("Error: app_start_player() failure.\n");
      return false;
    }

  /* Running... */

  for (int i = 0; i < PLAYER_PLAY_TIME; i++)
    {
      app_set_volume(0, test_volume_table[i], 0);
      play_process(player, NULL, 1);
    }

  /* Stop player operation. */

  if (!app_stop(player))
    {
      printf("Error: app_stop() failure.\n");
      return false;
    }

  return true;
}

static bool test_case3(player_info_s *player, player_info_s *sub_player)
{
  /* Get track. */

  if (!app_get_track(player, PLAY_NEXT))
    {
      printf("Error: [main] app_get_track() failuer.\n");
      return false;
    }

  if (!app_get_track(sub_player, PLAY_PREV))
    {
      printf("Error: [sub] app_get_track() failuer.\n");
      return false;
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

  for (int i = 0; i < PLAYER_PLAY_TIME; i++)
    {
      app_set_volume(0, 0, test_volume_table[i]);
      play_process(player, sub_player, 1);
    }

  /* Stop player operation. */

  if (!app_stop(player))
    {
      printf("Error: [main] app_stop() failure.\n");
      return false;
    }
  if (!app_stop(sub_player))
    {
      printf("Error: [sub] app_stop() failure.\n");
      return false;
    }

  return true;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int player_test_spr_sdk_14929(void)
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

  if (!app_open_playlist(&s_player_info, true))
    {
      printf("Error: [main] app_open_playlist() failure.\n");
      return 1;
    }
  if (!app_open_playlist(&s_player_info_sub, true))
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

  app_set_volume(0, 0, PLAYER_DEF_VOLUME);

  if (board_external_amp_mute_control(false) != OK)
    {
      printf("Error: board_external_amp_mute_control(false) failuer.\n");
      return 1;
    }

  /*========================================================================*/
  /* test 1: change volume of input1_db                                     */
  /*========================================================================*/
  printf("---------------------------------------------------\n");
  printf(" test 1: change volume of input1_db                \n");
  printf("---------------------------------------------------\n");
  if (!test_case1(&s_player_info))
    {
      printf("Error: test_case1 failuer.\n");
      return 1;
    }

  /*========================================================================*/
  /* test 2: change volume of input2_db                                     */
  /*========================================================================*/
  printf("---------------------------------------------------\n");
  printf(" test 2: change volume of input2_db                \n");
  printf("---------------------------------------------------\n");
  if (!test_case2(&s_player_info_sub))
    {
      printf("Error: test_case2 failuer.\n");
      return 1;
    }

  /*========================================================================*/
  /* test 2: change volume of master_db                                     */
  /*========================================================================*/
  printf("---------------------------------------------------\n");
  printf(" test 2: change volume of master_db                \n");
  printf("---------------------------------------------------\n");
  if (!test_case3(&s_player_info, &s_player_info_sub))
    {
      printf("Error: test_case3 failuer.\n");
      return 1;
    }

  /* Set output mute. */

  if (board_external_amp_mute_control(true) != OK)
    {
      printf("Error: board_external_amp_mute_control(true) failuer.\n");
      return 1;
    }

  /* Close playlist. */

  if (!app_close_playlist(&s_player_info))
    {
      printf("Error: [main] app_close_playlist() failure.\n");
      return 1;
    }
  if (!app_close_playlist(&s_player_info_sub))
    {
      printf("Error: [sub] app_close_playlist() failure.\n");
      return 1;
    }

  if (!app_close_contents_dir(&s_player_info))
    {
      printf("Error: [main] app_close_contents_dir() failure.\n");
      return 1;
    }
  if (!app_close_contents_dir(&s_player_info_sub))
    {
      printf("Error: [sub] app_close_contents_dir() failure.\n");
      return 1;
    }

  /* Return the state of AudioSubSystem before voice_call operation. */

  if (!app_set_ready())
    {
      printf("Error: app_set_ready() failure.\n");
      return 1;
    }

  /* Change AudioSubsystem to PowerOff state. */

  if (!app_power_off())
    {
      printf("Error: app_power_off() failure.\n");
      return 1;
    }

  /* Deactivate the features used by AudioSubSystem. */

  app_deact_audio_sub_system_main_only();

  /* Free work area. */

  app_free_work_area(&s_player_info);
  app_free_work_area(&s_player_info_sub);

  return 0;
}
