
#include "include/cmd_parser.h"
#include <pthread.h>
#include <arch/board/board.h>
#include "include/test_api.h"
#include "include/player_test_common.h"

/* Default Volume. -20dB */
#define PLAYER_DEF_VOLUME -200


static pthread_t s_tid;
int s_clk_mode = -1;


static bool app_pre_param(AsPlayerId id)
{
    if (!app_open_play_file(id))
    {
        /* Abnormal termination processing */
        return false;
    }

    /* Get current clock mode.
     * If the sampling rate is less than 48 kHz,
     * it will be in Normal mode. Otherwise, Hi-Res mode is set.
     */
    int cur_clk_mode;
    if (s_player_info[id].file.track.sampling_rate <= AS_SAMPLINGRATE_48000)
    {
        cur_clk_mode = AS_CLKMODE_NORMAL;
    }
    else
    {
        cur_clk_mode = AS_CLKMODE_HIRES;
    }

    /* If current clock mode is different from the previous clock mode,
     * perform initial setting.
     */
    if (s_clk_mode != cur_clk_mode)
    {
        /* Update clock mode. */
        s_clk_mode = cur_clk_mode;

        /* Since the initial setting is required to be in the Ready state,
         * if it is not in the Ready state, it is set to the Ready state.
         */
        if (AS_MNG_STATUS_READY != app_get_status())
        {
            if (board_external_amp_mute_control(true) != OK)
            {
                printf("Error: board_external_amp_mute_control(true) failuer.\n");

                /* Abnormal termination processing */
                return false;
            }

            if (!app_set_ready())
            {
                printf("Error: app_set_ready() failure.\n");

                /* Abnormal termination processing */
                return false;
            }
        }

        /* Set the clock mode of the output function. */
        if (!app_set_clkmode(s_clk_mode))
        {
            printf("Error: app_set_clkmode() failure.\n");

            /* Abnormal termination processing */
            return false;
        }

        /* Set player operation mode. */
        printf("app_set_player_status() begin.\n");
        if (!app_set_player_status(id, &s_player_info[0].fifo.input_device, &s_player_info[1].fifo.input_device))
        {
            printf("Error: app_set_player_status() failure.\n");

            /* Abnormal termination processing */
            return false;
        }
        printf("app_set_player_status() finished.\n");

        /* Cancel output mute. */
        app_set_volume(PLAYER_DEF_VOLUME);

        if (board_external_amp_mute_control(false) != OK)
        {
            printf("Error: board_external_amp_mute_control(false) failuer.\n");

            /* Abnormal termination processing */
            return false;
        }
    }
    return true;
}

bool app_cmd_param(unsigned int count, char** tp)
{
    AsPlayerId id = AS_PLAYER_ID_0;
    if (count > 1)
    {
        if (strcmp(tp[1], "0") == 0 || strcmp(tp[1], "player0") == 0)
        {
            id = AS_PLAYER_ID_0;
        }
        else if (strcmp(tp[1], "1") == 0 || strcmp(tp[1], "player1") == 0)
        {
            id = AS_PLAYER_ID_1;
        }
        else
        {
            printf("CMD incorrect.\n");
            return false;
        }
    }

    uint8_t codec_type = AS_CODECTYPE_MP3;
    if (count > 2)
    {
        snprintf(s_audio_name[id], sizeof(s_audio_name[id]), "%s", tp[2]);

        char* p = strchr(tp[2], '.') + 1;

        if (strcmp(p, "MP3") == 0 || strcmp(p, "mp3") == 0)
        {
            codec_type = AS_CODECTYPE_MP3;
        }
        else if (strcmp(p, "WAV") == 0 || strcmp(p, "wav") == 0)
        {
            codec_type = AS_CODECTYPE_WAV;
        }
        else
        {
            printf("CMD incorrect.\n");
            return false;
        }
    }

    uint32_t sampling_rate = AS_SAMPLINGRATE_48000;
    if (count > 3)
    {
        if (strcmp(tp[3], "0") == 0 || strcmp(tp[3], "AUTO") == 0)
        {
            sampling_rate = AS_SAMPLINGRATE_AUTO;
        }
        else
        {
            sampling_rate = atoi(tp[3]);
        }
    }

    uint8_t channel_number = AS_CHANNEL_STEREO;
    if (count > 4)
    {
        if (strcmp(tp[4], "2") == 0 || strcmp(tp[4], "STEREO") == 0)
        {
            channel_number = AS_CHANNEL_STEREO;
        }
        else if (strcmp(tp[4], "1") == 0 || strcmp(tp[4], "MONO") == 0)
        {
            channel_number = AS_CHANNEL_MONO;
        }
        else
        {
            printf("CMD incorrect.\n");
            return false;
        }
    }

    uint8_t bit_length = AS_BITLENGTH_16;
    if (count > 5)
    {
        if (strcmp(tp[5], "24") == 0)
        {
            bit_length = AS_BITLENGTH_24;
        }
    }

    s_player_info[id].file.track.channel_number = channel_number;
    s_player_info[id].file.track.bit_length     = bit_length;
    s_player_info[id].file.track.sampling_rate  = sampling_rate;
    s_player_info[id].file.track.codec_type     = codec_type;

    if (!app_pre_param(id))
    {
        printf("Error: app_pre_param() failure.\n");
        return false;
    }
    printf("app_pre_param() finished.\n");

    if (!app_init_player(id, codec_type, sampling_rate, channel_number, bit_length))
    {
        printf("Error: app_init_player() failure.\n");
        app_close_play_file(id);
        return false;
    }
    printf("Param setting finished\n");
    return true;
}

bool app_cmd_play(unsigned int count, char** tp)
{
    bool ret = true;
    static AsPlayerId id = AS_PLAYER_ID_0;
    if (count > 1 && (strcmp(tp[1], "1") == 0 || strcmp(tp[1], "player1") == 0))
    {
        id = AS_PLAYER_ID_1;
    }

    if (!app_play_player(id))
    {
        printf("Error: app_play_player() failure.\n");
        app_close_play_file(id);
        ret = false;
    }

    if (ret)
    {
        s_playing_id = id;

        pthread_attr_t           tattr;
        struct sched_param       param;
        pthread_attr_init(&tattr);
        tattr.stacksize      = 2048;
        param.sched_priority = SCHED_PRIORITY_DEFAULT;
        pthread_attr_setschedparam(&tattr, &param);

        int retThread = pthread_create(&s_tid, &tattr, (pthread_startroutine_t)play_thread,
                             (pthread_addr_t)&id);
        if (retThread != 0)
        {
            ret = false;
        }
        if (ret)
        {
            printf("Playing\n");
        }
    }
    return ret;
}

bool app_cmd_stop(unsigned int count, char** tp)
{
    bool result = true;
    AsPlayerId id = AS_PLAYER_ID_0;
    if (count > 1)
    {
        if (strcmp(tp[1], "1") == 0 || strcmp(tp[1], "player1") == 0)
        {
            id = AS_PLAYER_ID_1;
        }
    }
    uint8_t mode = AS_STOPPLAYER_NORMAL;
    if (count > 2)
    {
        if (strcmp(tp[2], "ESEND") == 0)
        {
            mode = AS_STOPPLAYER_ESEND;
        }
    }

    s_is_playing = false;
    if (!app_stop_player(id, mode))
    {
        printf("Error: app_stop_player() failure.\n");
        result = false;
    }

    if (!app_close_play_file(id))
    {
          printf("Error: app_close_play_file() failure.\n");
          result = false;
    }

    if (!result)
        exit(1);
    printf("Stopped\n");
    return result;
}

bool app_cmd_mute(unsigned int count, char** tp)
{
    bool s_isInput1Mute = true;
    bool s_isInput2Mute = true;
    bool s_isMasterMute = true;

    if (count > 1)
    {
        if (strcmp(tp[1], "1") != 0 && strcmp(tp[1], "true") != 0)
        {
            s_isInput1Mute = false;
        }
    }
    if (count > 2)
    {
        if (strcmp(tp[2], "1") != 0 && strcmp(tp[2], "true") != 0)
        {
            s_isInput2Mute = false;
        }
    }
    if (count > 3) {
        if (strcmp(tp[3], "1") != 0 && strcmp(tp[3], "true") != 0)
        {
            s_isMasterMute = false;
        }
    }
    bool ret = app_set_mute(s_isInput1Mute, s_isInput2Mute, s_isMasterMute);
    if (ret)
        printf("Set Mute %d,%d,%d\n", s_isInput1Mute, s_isInput2Mute, s_isMasterMute);
    return ret;
}

void app_cmd_help()
{
    printf("------------------Command List---------------\n");
    printf("HELP\n");
    printf("PARAM player_id audio_file_name sample_rate channel_number bit_length\n");
    printf("    e.g., PARAM 0 m2.mp3 24000 2 16\n");
    printf("PLAY player_id\n");
    printf("    e.g., PLAY 0\n");
    printf("STOP player_id stop_mode\n");
    printf("    e.g., STOP 0 NORMAL\n");
    printf("QUIT\n");
    printf("---------------------------------------------\n");
}

bool app_process_cmd_loop()
{
    char command[64];

    bool isQuit = false;

    while (!isQuit)
    {
        gets(command);
        printf("%s\n", command);

        char* tp[6];
        tp[0] = strtok(command, " ");

        unsigned int count = 0;
        while (tp[count] != NULL && count < sizeof(tp)/sizeof(tp[0]) - 1)
        {
            count++;
            tp[count] = strtok(NULL, " ");
        }

        if (tp[0] == NULL)
            continue;
        printf("[%s] COMMAND\n", tp[0]);

        if (strcmp(tp[0], "PARAM") == 0 || strcmp(tp[0], "param") == 0)
        {
            if (!app_cmd_param(count, tp))
            {
                isQuit = true;
            }
        }
        else if (strcmp(tp[0], "PLAY") == 0 || strcmp(tp[0], "play") == 0)
        {
            if (!app_cmd_play(count, tp))
            {
                isQuit = true;
            }
        }
        else if (strcmp(tp[0], "STOP") == 0 || strcmp(tp[0], "stop") == 0)
        {
            if (!app_cmd_stop(count, tp))
            {
                isQuit = true;
            }
        }
        else if (strcmp(tp[0], "MUTE") == 0 || strcmp(tp[0], "mute") == 0)
        {
            if (!app_cmd_mute(count, tp))
            {
                isQuit = true;
            }
        }
        else if (strcmp(tp[0], "QUIT") == 0 || strcmp(tp[0], "quit") == 0)
        {
            isQuit = true;
        }
        else if (strcmp(tp[0], "HELP") == 0 || strcmp(tp[0], "help") == 0)
        {
            app_cmd_help();
        }
        else {
            printf("wrong command.\n");
        }
    }
    return true;
}

