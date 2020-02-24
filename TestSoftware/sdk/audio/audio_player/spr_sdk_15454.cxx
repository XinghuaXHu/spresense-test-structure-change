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

#define PLAYER_PLAY_TIME 10

/* Play time(sec) */

#define PLAY_NEXT 0
#define PLAY_PREV 1

#define PLAY_CNT  6

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
static sem_t g_audio_test;

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
      if (player != NULL)
        {
          if (!app_refill_simple_fifo(player))
            {
              break;
            }
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

static void *test_thread(void *param)
{
  player_info_s *player;
  int main_sub = (int)param;
  char          main_sub_str[5];
  int           test_loop;
  int ret;

  if (main_sub == 0)
    {
      player = &s_player_info;
      test_loop = (12 * 60 * (60 / 3)) /* 12 hour */;
      strncpy(main_sub_str, "main", sizeof(main_sub_str));
    }
  else
    {
      player = &s_player_info_sub;
      test_loop = (12 * 60 * (60 / 5)) /* 12 hour */;
      strncpy(main_sub_str, "sub", sizeof(main_sub_str));
    }
  
  /* playlist:xxxx.mp3, xxxx.wav */

  if (!app_get_next_track(player))
    {
      printf("Error: No more tracks to play(main).\n");
      goto exit;
    }

  if (main_sub)
    {
      if (!app_get_next_track(player))
        {
          printf("Error: No more tracks to play(sub).\n");
          goto exit;
        }
    }

  for (int i = 0; i <= test_loop; i++)
    {

      /* Open play contents. */

      if (!app_open_file(player))
        {
          printf("test_thread() Error: [%s] app_open_file() failuer.\n", main_sub_str);
          break;
        }

      /* Start player operation. */

      while(1)
        {
          ret = sem_wait(&g_audio_test);
          if ((ret != 0) && (errno == EINTR))
            {
              continue;
            }
          else
            {
              break;
            }
        }
      if (!app_start(player))
        {
          ret = sem_post(&g_audio_test);
          printf("test_thread() Error: [%s] app_start_player() failure.\n", main_sub_str);
          break;
        }

      ret = sem_post(&g_audio_test);

      /* Running... */

      play_process(player, NULL, main_sub ? 5 : 3);

      /* Stop player operation. */

      while(1)
        {
          ret = sem_wait(&g_audio_test);
          if ((ret != 0) && (errno == EINTR))
            {
              continue;
            }
          else
            {
              break;
            }
        }
      if (!app_stop(player))
        {
          ret = sem_post(&g_audio_test);
          printf("test_thread() Error: [%s] app_stop() failure.\n", main_sub_str);
          break;
        }
      ret = sem_post(&g_audio_test);
    }

exit:
  pthread_exit(0);

  return 0;
}

static int test_thread_create(pthread_t *id, int main_sub)
{
  int                 ret;
  pthread_attr_t      attr;
  struct sched_param  sch_param;

  (void)pthread_attr_init(&attr);
  sch_param.sched_priority = 110;
  ret = pthread_attr_setschedparam(&attr, &sch_param);
  if (ret != 0)
    {
      printf("pthread_attr_setschedparam() failure. %d\n", ret);
      return ret;
    }

  ret = pthread_create(id,
                       &attr,
                       test_thread,
                       (pthread_addr_t)main_sub);
  if (ret != 0)
    {
      printf("pthread_create() failure. %d\n", ret);
    }

  (void)pthread_attr_destroy(&attr);

  return ret;
}

static bool test_case(void)
{
  bool      ret = true;
  int       err;
  pthread_t thread_id_main;
  pthread_t thread_id_sub;

  err = test_thread_create(&thread_id_main, 0);
  if (err != 0)
    {
      printf("test_thread_create(main) failure. %d\n", err);
      return false;
    }

  err = test_thread_create(&thread_id_sub, 1);
  if (err != 0)
    {
      printf("test_thread_create(sub) failure. %d\n", err);
      (void)pthread_cancel(thread_id_main);
      (void)pthread_join(thread_id_main, NULL);
      return false;
    }

  err = pthread_join(thread_id_main, NULL);
  if (err != 0)
    {
      printf("pthread_join(main) failure. %d\n", err);
      ret = false;
    }

  err = pthread_join(thread_id_sub, NULL);
  if (err != 0)
    {
      printf("pthread_join(sub) failure. %d\n", err);
      ret = false;
    }

  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int player_test_spr_sdk_15454(void)
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

  app_set_volume(0, 0, PLAYER_DEF_VOLUME);

  if (board_external_amp_mute_control(false) != OK)
    {
      printf("Error: board_external_amp_mute_control(false) failuer.\n");
      return 1;
    }

  sem_init(&g_audio_test, 0, 1);
  printf("----------------------------------------------------------\n");
  printf(" test :main play mp3 3sec order, sub play wav 5sec order \n");
  printf("----------------------------------------------------------\n");
  if (!test_case())
    {
      printf("Error: test_case failuer.\n");
    }

  sem_destroy(&g_audio_test);

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
