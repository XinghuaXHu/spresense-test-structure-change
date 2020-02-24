/****************************************************************************
 * player_test/player_test_main.cxx
 *
 *   Copyright 2018 Sony Semiconductor Solutions Corporation
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
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
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
#include <stdio.h>
#include <arch/board/board.h>
#include "include/test_api.h"
#include "include/cmd_parser.h"
#include "include/player_test_common.h"


/****************************************************************************
 * Public Data
 ****************************************************************************/
volatile bool s_is_playing = false;
AsPlayerId s_playing_id = AS_PLAYER_ID_0;


/****************************************************************************
 * Public Functions
 ****************************************************************************/
#ifdef CONFIG_BUILD_KERNEL
extern "C" int main(int argc, FAR char *argv[])
#else
extern "C" int player_test_main(int argc, char *argv[])
#endif
{
    app_cmd_help();
    printf("Start Player Test\n");

    uint8_t st = AS_MNG_STATUS_READY;
    s_clk_mode = -1;

    /* First, initialize the shared memory and memory utility used by AudioSubSystem. */
    if (!app_init_libraries())
    {
        printf("Error: init_libraries() failure.\n");
        return 1;
    }

    /* Next, Create the features used by AudioSubSystem. */
    if (!app_create_audio_sub_system())
    {
        printf("Error: act_audiosubsystem() failure.\n");
        /* Abnormal termination processing */
        goto errout_act_audio_sub_system;
    }

    /* Open directory of play contents. */
    if (!app_open_contents_dir())
    {
        printf("Error: app_open_contents_dir() failure.\n");

        /* Abnormal termination processing */
        goto errout_open_contents_dir;
    }

    /* Initialize frequency lock parameter. */
    app_init_freq_lock();

    /* Lock cpu frequency to high. */
    app_freq_lock();

    /* Change AudioSubsystem to Ready state so that I/O parameters can be changed. */
    if (!app_power_on())
    {
        printf("Error: app_power_on() failure.\n");

        /* Abnormal termination processing */
        goto errout_power_on;
    }

    /* Initialize simple fifo. */
    if (!app_init_simple_fifo())
    {
        printf("Error: app_init_simple_fifo() failure.\n");

        /* Abnormal termination processing */
        goto errout_init_simple_fifo;
    }

    /* Set the device to output the mixed audio. */
    if (!app_init_output_select())
    {
        printf("Error: app_init_output_select() failure.\n");

        /* Abnormal termination processing */
        goto errout_init_output_select;
    }

    app_process_cmd_loop();

    if (s_is_playing)
    {
        app_stop_player(s_playing_id, AS_STOPPLAYER_NORMAL);
        if (!app_close_play_file(s_playing_id))
        {
            printf("Error: app_close_play_file() failure.\n");
            return 1;
        }
        s_is_playing = false;
    }

    if (board_external_amp_mute_control(true) != OK)
    {
        printf("Error: board_external_amp_mute_control(true) failuer.\n");
        /* Abnormal termination processing */
        return 1;
    }

  /* Return the state of AudioSubSystem before voice_call operation. */

errout_init_simple_fifo:
errout_init_output_select:
    st = app_get_status();
    if (AS_MNG_STATUS_READY != st)
    {
        if (!app_set_ready())
        {
            printf("Error: app_set_ready() failure.\n");
            return 1;
        }
    }

    /* Change AudioSubsystem to PowerOff state. */
    if (!app_power_off())
    {
        printf("Error: app_power_off() failure.\n");
        return 1;
    }

    /* Unlock cpu frequency. */
    app_freq_release();

errout_power_on:
    /* Close directory of play contents. */
    if (!app_close_contents_dir())
    {
        printf("Error: app_close_contents_dir() failure.\n");
        return 1;
    }

  /* Deactivate the features used by AudioSubSystem. */

errout_open_contents_dir:
    app_deact_audio_sub_system();

  /* finalize the shared memory and memory utility used by AudioSubSystem. */

errout_act_audio_sub_system:
    if (!app_finalize_libraries())
    {
        printf("Error: finalize_libraries() failure.\n");
        return 1;
    }

    printf("Exit Player Test\n");

    return 0;
}

