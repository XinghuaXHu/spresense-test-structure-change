
#include "include/cmd_parser.h"

#include <pthread.h>
#include <arch/board/board.h>

#include "include/test_api.h"
#include "include/audio_extensions_test_common.h"

/* Default Volume. -20dB */
#define PLAYER_DEF_VOLUME -200

bool g_quit = false;

static pthread_t s_tid;

typedef bool (*CMD_PROCESS_FUNC)(unsigned int count, const char** tp);
struct CMD_PROCESS_MAP
{
    char cmd[32];
    CMD_PROCESS_FUNC func;
};

uint8_t g_codec_type[PLAYER_NUM];

AsClkMode s_clk_mode = AS_CLKMODE_NORMAL;

CMD_PROCESS_MAP s_cmd_process_map[] =
{
    {"HELP", app_cmd_help},
    {"QUIT", app_cmd_quit},

    {"INIT_MEM", app_cmd_init_libraries},
    {"FIN_MEM", app_cmd_finalize_libraries},

    {"INIT_FIFO", app_cmd_init_simple_fifo},

    {"FREQ_LOCK", app_cmd_freq_lock},
    {"CLK", app_cmd_set_clkmode},
    {"AMP_MUTE", app_cmd_amp_mute},
    {"DIR", app_cmd_open_contents_dir},
    {"OPEN_PLAY_FILE", app_cmd_open_play_file},

    {"CREATE", app_cmd_create_audio_sub_system},
    {"DELETE", app_cmd_deact_audio_sub_system},

    {"MANAGER", app_cmd_audio_manager},
    {"PLYER", app_cmd_player},
    {"MIXER", app_cmd_mixer},
    {"RENDERER", app_cmd_renderer},
    {"RECORER", app_cmd_recorder},
    {"CAPTURE", app_cmd_capture},

    {"POWER", app_cmd_power},
    {"OUTSEL", app_cmd_init_output_select},
    {"STATUS", app_cmd_set_status},
    {"BEEP", app_cmd_beep},
    {"VOL", app_cmd_set_volume},
    {"MUTE", app_cmd_mute},
    {"INITP", app_cmd_init_player},
    {"PLAY", app_cmd_play},
    {"PAUSE", app_cmd_pause},
    {"STOP_PLAYER", app_cmd_stop_player},
    {"INITR", app_cmd_init_recorder},
    {"START_RECORDER", app_cmd_start_recorder},
    {"STOP_RECORDER", app_cmd_stop_recorder},
    {"MICGAIN", app_cmd_init_mic_gain},
};


bool app_cmd_init_libraries(unsigned int count, const char** tp)
{
    bool ret = false;
    if (count == 2)
    {
        if (strcmp(tp[1], "true") == 0 || strcmp(tp[1], "recorder") == 0)
        {
            g_is_recorder = true;
            ret = app_init_libraries(g_is_recorder);
        }
        else if (strcmp(tp[1], "false") == 0 || strcmp(tp[1], "player") == 0)
        {
            g_is_recorder = false;
            ret = app_init_libraries(g_is_recorder);
        }
        else
        {
            printf("Command Parameter Incorrect\n");
            ret = false;
        }
    }

/*
    const char* tpc[2] = {"CREATE", "player"};
    app_cmd_create_audio_sub_system(2, tpc);

    const char* tpd[2] = {"DIR", "open"};
    app_cmd_open_contents_dir(2, tpd);

    const char* tpf[2] = {"FREQ_LOCK", "true"};
    app_cmd_freq_lock(2, tpf);

    const char* tpp[2] = {"POWER", "on"};
    app_cmd_power(2, tpp);

    const char* tpfifo[1] = {"INIT_FIFO"};
    app_cmd_init_simple_fifo(1, tpfifo);

    const char* tpo[1] = {"OUTSEL"};
    app_cmd_init_output_select(1, tpo);

    const char* tpfile[3] = {"OPEN_PLAY_FILE", "player0", "m2.mp3"};
    app_cmd_open_play_file(3, tpfile);

    const char* tpfile1[3] = {"OPEN_PLAY_FILE", "player1", "a2.mp3"};
    app_cmd_open_play_file(3, tpfile1);

    const char* tpclk[2] = {"CLK", "normal"};
    app_cmd_set_clkmode(2, tpclk);

    const char* tp2[2] = {"STATUS", "player"};
    app_cmd_set_status(2, tp2);
*/
    return ret;
}

bool app_cmd_finalize_libraries(unsigned int count, const char** tp)
{
    bool ret = app_finalize_libraries();
    printf("Finalize libraries finished.\n");
    return ret;
}

bool app_cmd_create_audio_sub_system(unsigned int count, const char** tp)
{
    return Audio_create_audio_sub_system(g_is_recorder);
}

bool app_cmd_deact_audio_sub_system(unsigned int count, const char** tp)
{
    Audio_deact_audio_sub_system(g_is_recorder);
    printf("Delete audio sub system finished\n");
    return true;
}

bool app_cmd_audio_manager(unsigned int count, const char** tp)
{
    if (count > 1)
    {
        if (strcmp(tp[1], "true") == 0 || strcmp(tp[1], "create") == 0)
        {
            return Audio_audio_manager(true);
        }
        else
        {
            return Audio_audio_manager(false);
        }
    }
    return false;
}

bool app_cmd_player(unsigned int count, const char** tp)
{
    if (count > 1)
    {
        if (strcmp(tp[1], "true") == 0 || strcmp(tp[1], "create") == 0)
        {
            return Audio_player(true);
        }
        else
        {
            return Audio_player(false);
        }
    }
    return false;
}

bool app_cmd_mixer(unsigned int count, const char** tp)
{
    if (count > 1)
    {
        if (strcmp(tp[1], "true") == 0 || strcmp(tp[1], "create") == 0)
        {
            return Audio_mixer(true);
        }
        else
        {
            return Audio_mixer(false);
        }
    }
    return false;
}

bool app_cmd_renderer(unsigned int count, const char** tp)
{
    if (count > 1)
    {
        if (strcmp(tp[1], "true") == 0 || strcmp(tp[1], "create") == 0)
        {
            return Audio_renderer(true);
        }
        else
        {
            return Audio_renderer(false);
        }
    }
    return false;
}

bool app_cmd_recorder(unsigned int count, const char** tp)
{
    if (count > 1)
    {
        if (strcmp(tp[1], "true") == 0 || strcmp(tp[1], "create") == 0)
        {
            return Audio_recorder(true);
        }
        else
        {
            return Audio_recorder(false);
        }
    }
    return false;
}

bool app_cmd_capture(unsigned int count, const char** tp)
{
    if (count > 1)
    {
        if (strcmp(tp[1], "true") == 0 || strcmp(tp[1], "create") == 0)
        {
            return Audio_capture(true);
        }
        else
        {
            return Audio_capture(false);
        }
    }
    return false;
}

bool app_cmd_power(unsigned int count, const char** tp)
{
    bool ret = false;
    if (count > 1)
    {
        if (strcmp(tp[1], "true") == 0 || strcmp(tp[1], "on") == 0)
        {
            ret = Audio_power_on();
            printf("POWER ON finished\n");
        }
        else
        {
            ret = Audio_power_off();
            printf("POWER OFF finished\n");
        }
    }
    return ret;
}

bool app_cmd_init_output_select(unsigned int count, const char** tp)
{
    bool ret = Audio_init_output_select();
    printf("INIT OUTPUT SELECT finished\n");
    return ret;
}

bool app_cmd_init_simple_fifo(unsigned int count, const char** tp)
{
    return app_init_simple_fifo();
}

bool app_cmd_set_status(unsigned int count, const char** tp)
{
    bool ret = false;
    if (count > 1)
    {
        if (strcmp(tp[1], "ready") == 0)
        {
            if (Audio_get_status() != AS_MNG_STATUS_READY)
            {
                ret = Audio_set_ready();
            }
            else
            {
                ret = true;
            }
            printf("Set READY STATUS END\n");
        }
        else if (strcmp(tp[1], "player") == 0)
        {
            ret = Audio_set_player_status(&s_player_info[0].fifo.input_device, &s_player_info[1].fifo.input_device);
            printf("Set PLAYER STATUS END\n");
        }
        else if (strcmp(tp[1], "recorder") == 0)
        {
            ret = Audio_set_recorder_status(&s_recorder_info.fifo.output_device);
            printf("Set RECORDER STATUS END\n");
        }
    }
    return ret;
}

bool app_cmd_beep(unsigned int count, const char** tp)
{
    if (count > 1)
    {
        bool enable = false;
        if (strcmp(tp[1], "enable") == 0)
        {
            enable = true;
        }
        else if (strcmp(tp[1], "disable") == 0)
        {
            enable = false;
        }
        else
        {
            printf("Command parameter incorrect\n");
            return false;
        }

        int16_t vol = AS_BEEP_VOL_HOLD;
        if (count > 2)
        {
            vol = atoi(tp[2]);
        }
        uint16_t freq = AS_BEEP_FREQ_HOLD;
        if (count > 3)
        {
            freq = atoi(tp[3]);
        }

        bool ret = Audio_beep(enable, vol, freq);
        if (!ret)
        {
            printf("Error: app_beep() failure.\n");
            return false;
        }
        if (enable)
        {
            printf("Audio beep enabled\n");
        }
        else
        {
            printf("Audio beep disabled\n");
        }
        return true;
    }
    return false;
}

bool app_cmd_set_clkmode(unsigned int count, const char** tp)
{
    if (count > 1)
    {
        /* Set the clock mode of the output function. */
        AsClkMode cur_clk_mode = AS_CLKMODE_NORMAL;
        if (strcmp(tp[1], "normal") == 0)
        {
            cur_clk_mode = AS_CLKMODE_NORMAL;
        }
        else if (strcmp(tp[1], "hires") == 0)
        {
            cur_clk_mode = AS_CLKMODE_HIRES;
        }
        else
        {
            printf("Command parameter incorrect\n");
            return false;
        }

        if (!Audio_set_clkmode(cur_clk_mode))
        {
            printf("Error: app_set_clkmode() failure.\n");
            return false;
        }
        s_clk_mode = cur_clk_mode;
        return true;
    }
    printf("Command parameter number incorrect\n");
    return false;
}

bool app_cmd_amp_mute(unsigned int count, const char** tp)
{
    if (count > 1)
    {
        bool mute = false;
        if (strcmp(tp[1], "false") == 0)
        {
            mute = false;
        }
        else if (strcmp(tp[1], "true") == 0)
        {
            mute = true;
        }
        else
        {
            return false;
        }

        if (board_external_amp_mute_control(mute) != OK)
        {
            printf("Error: board_external_amp_mute_control failure.\n");
            return false;
        }
        return true;
    }
    return false;
}

bool app_cmd_open_contents_dir(unsigned int count, const char** tp)
{
    if (count > 1)
    {
        if (strcmp(tp[1], "true") == 0 || strcmp(tp[1], "open") == 0)
        {
            return app_open_contents_dir();
        }
        else if (strcmp(tp[1], "false") == 0 || strcmp(tp[1], "close") == 0)
        {
            return app_close_contents_dir();
        }
        else
        {
            return false;
        }
    }
    return false;
}

bool app_cmd_open_play_file(unsigned int count, const char** tp)
{
    if (count == 3)
    {
        AsPlayerId id = AS_PLAYER_ID_0;
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

        snprintf(s_audio_name[id], sizeof(s_audio_name[id]), "%s", tp[2]);

        char* p = strchr(tp[2], '.') + 1;

        if (strcmp(p, "MP3") == 0 || strcmp(p, "mp3") == 0)
        {
            g_codec_type[id] = AS_CODECTYPE_MP3;
        }
        else if (strcmp(p, "WAV") == 0 || strcmp(p, "wav") == 0)
        {
            g_codec_type[id] = AS_CODECTYPE_WAV;
        }
        else
        {
            printf("CMD incorrect.\n");
            return false;
        }

        bool ret = app_open_play_file(id);
        return ret;
    }
    printf("CMD incorrect.\n");
    return false;
}

bool app_cmd_freq_lock(unsigned int count, const char** tp)
{
    if (count > 1)
    {
        if (strcmp(tp[1], "true") == 0)
        {
            app_init_freq_lock();
            app_freq_lock();
            return true;
        }
        else if (strcmp(tp[1], "false") == 0)
        {
            app_freq_release();
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

bool app_cmd_init_player(unsigned int count, const char** tp)
{
    AsPlayerId id = AS_PLAYER_ID_0;
    if (count != 5)
    {
        return false;
    }

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

    uint32_t sampling_rate = AS_SAMPLINGRATE_48000;
    if (strcmp(tp[2], "0") == 0 || strcmp(tp[2], "AUTO") == 0)
    {
        sampling_rate = AS_SAMPLINGRATE_AUTO;
    }
    else
    {
        sampling_rate = atoi(tp[2]);
    }

    uint8_t channel_number = AS_CHANNEL_STEREO;
    if (strcmp(tp[3], "2") == 0 || strcmp(tp[3], "STEREO") == 0)
    {
        channel_number = AS_CHANNEL_STEREO;
    }
    else if (strcmp(tp[3], "1") == 0 || strcmp(tp[3], "MONO") == 0)
    {
        channel_number = AS_CHANNEL_MONO;
    }
    else
    {
        printf("CMD incorrect.\n");
        return false;
    }

    uint8_t bit_length = AS_BITLENGTH_16;
    if (strcmp(tp[4], "24") == 0)
    {
        bit_length = AS_BITLENGTH_24;
    }

    s_player_info[id].file.track.channel_number = channel_number;
    s_player_info[id].file.track.bit_length     = bit_length;
    s_player_info[id].file.track.sampling_rate  = sampling_rate;
    s_player_info[id].file.track.codec_type     = g_codec_type[id];

    if (!Audio_init_player(id, g_codec_type[id], sampling_rate, channel_number, bit_length))
    {
        printf("Error: app_init_player() failure.\n");
        app_close_play_file(id);
        return false;
    }
    printf("Player%d Param setting finished\n", id);
    return true;
}

bool app_cmd_play(unsigned int count, const char** tp)
{
    bool ret = true;

    if (count != 3)
    {
        printf("Error: Play command needs two parameters.\n");
        return false;
    }

    bool player0 = false;
    bool player1 = false;

    if (strcmp(tp[1], "1") == 0 || strcmp(tp[1], "true") == 0)
    {
        player0 = true;
    }
    if (strcmp(tp[2], "1") == 0 || strcmp(tp[2], "true") == 0)
    {
        player1 = true;
    }

    if (!Audio_play_player(player0 && s_is_playing[AS_PLAYER_ID_0] != PLAY_STATE_PLAYING, player1 && s_is_playing[AS_PLAYER_ID_1] != PLAY_STATE_PLAYING))
    {
        printf("Error: app_play_player() failure.\n");
        ret = false;
    }

    if (ret)
    {
        static pthread_t s_tid0, s_tid1;

        pthread_attr_t tattr;
        pthread_attr_init(&tattr);
        tattr.stacksize = 2048;

        struct sched_param param;
        param.sched_priority = SCHED_PRIORITY_DEFAULT;

        pthread_attr_setschedparam(&tattr, &param);

        if (player0 && s_is_playing[AS_PLAYER_ID_0] != PLAY_STATE_PLAYING)
        {
            static AsPlayerId id = AS_PLAYER_ID_0;
            int retThread = pthread_create(&s_tid0, &tattr, (pthread_startroutine_t)play_thread, (pthread_addr_t)&id);
            if (retThread != 0)
            {
                ret = false;
            }
            if (ret)
            {
                printf("Player0 playing\n");
            }
        }
        if (player1 && ret && s_is_playing[AS_PLAYER_ID_1] != PLAY_STATE_PLAYING)
        {
            static AsPlayerId id = AS_PLAYER_ID_1;
            int retThread = pthread_create(&s_tid1, &tattr, (pthread_startroutine_t)play_thread, (pthread_addr_t)&id);
            if (retThread != 0)
            {
                ret = false;
            }
            if (ret)
            {
                printf("Player1 playing\n");
            }
        }
    }
    return ret;
}

bool app_cmd_pause(unsigned int count, const char** tp)
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

    s_is_playing[id] = PLAY_STATE_PAUSE;

    if (!Audio_stop_player(id, AS_STOPPLAYER_NORMAL))
    {
        printf("Error: app_stop_player() failure.\n");
        result = false;
    }

    printf("Player%d Paused\n", id);
    return result;
}

bool app_cmd_stop_player(unsigned int count, const char** tp)
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

    if (s_is_playing[id] == PLAY_STATE_STOP)
    {
        printf("Error: not playing.\n");
        return false;
    }

    if (s_is_playing[id] == PLAY_STATE_PLAYING)
    {
        s_is_playing[id] = PLAY_STATE_STOP;
        AsStopPlayerStopMode mode = AS_STOPPLAYER_NORMAL;
        if (count > 2)
        {
            if (strcmp(tp[2], "ESEND") == 0)
            {
                mode = AS_STOPPLAYER_ESEND;
            }
        }

        if (!Audio_stop_player(id, mode))
        {
            printf("Error: app_stop_player() failure.\n");
            result = false;
        }
    }
    s_is_playing[id] = PLAY_STATE_STOP;

    if (!app_close_play_file(id))
    {
        printf("Error: app_close_play_file() failure.\n");
        result = false;
    }

    printf("Player%d Stopped\n", id);
    return result;
}

bool app_cmd_set_volume(unsigned int count, const char** tp)
{
    if (count == 4)
    {
        int16_t input1_db = atoi(tp[1]);
        int16_t input2_db = atoi(tp[2]);
        int16_t master_db = atoi(tp[3]);
        return Audio_set_volume(input1_db, input2_db, master_db);
    }
    return false;
}

bool app_cmd_mute(unsigned int count, const char** tp)
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
    bool ret = Audio_set_mute(s_isInput1Mute, s_isInput2Mute, s_isMasterMute);
    if (ret)
        printf("Set Mute %d,%d,%d\n", s_isInput1Mute, s_isInput2Mute, s_isMasterMute);
    return ret;
}

bool app_cmd_init_recorder(unsigned int count, const char** tp)
{
    if (count == 4)
    {
        if (strcmp(tp[1], "mp3") == 0 || strcmp(tp[1], "MP3") == 0)
        {
            s_recorder_info.file.codec_type = AS_CODECTYPE_MP3;
        }
        else if (strcmp(tp[1], "lpcm") == 0 || strcmp(tp[1], "LPCM") == 0)
        {
            s_recorder_info.file.codec_type = AS_CODECTYPE_LPCM;
        }
        else if (strcmp(tp[1], "opus") == 0 || strcmp(tp[1], "OPUS") == 0)
        {
            s_recorder_info.file.codec_type = AS_CODECTYPE_OPUS;
        }
        else
        {
            printf("Error: Invalid codec type(%s)\n", tp[1]);
            return false;
        }

        if (strcmp(tp[2], "8000") == 0 || strcmp(tp[2], "8K") == 0)
        {
            s_recorder_info.file.sampling_rate = AS_SAMPLINGRATE_8000;
        }
        else if (strcmp(tp[2], "16000") == 0 || strcmp(tp[2], "16K") == 0)
        {
            s_recorder_info.file.sampling_rate = AS_SAMPLINGRATE_16000;
        }
        else if (strcmp(tp[2], "48000") == 0 || strcmp(tp[2], "48K") == 0)
        {
            s_recorder_info.file.sampling_rate = AS_SAMPLINGRATE_48000;
        }
        else
        {
            printf("Error: Invalid sampling rate(%s)\n", tp[2]);
            return false;
        }

        if (strcmp(tp[3], "mono") == 0 || strcmp(tp[3], "1") == 0)
        {
            s_recorder_info.file.channel_number = AS_CHANNEL_MONO;
        }
        else if (strcmp(tp[3], "stereo") == 0 || strcmp(tp[3], "2") == 0)
        {
            s_recorder_info.file.channel_number = AS_CHANNEL_STEREO;
        }
        else if (strcmp(tp[3], "4ch") == 0 || strcmp(tp[3], "4") == 0)
        {
            s_recorder_info.file.channel_number = AS_CHANNEL_4CH;
        }
        else if (strcmp(tp[3], "6ch") == 0 || strcmp(tp[3], "6") == 0)
        {
            s_recorder_info.file.channel_number = AS_CHANNEL_6CH;
        }
        else if (strcmp(tp[3], "8ch") == 0 || strcmp(tp[3], "8") == 0)
        {
            s_recorder_info.file.channel_number = AS_CHANNEL_8CH;
        }
        else
        {
            printf("Error: Invalid channel type(%s)\n", tp[3]);
            return false;
        }

        if (s_recorder_info.file.codec_type == AS_CODECTYPE_LPCM)
        {
            s_recorder_info.file.format_type = FORMAT_TYPE_WAV;
        }
        else
        {
            s_recorder_info.file.format_type = FORMAT_TYPE_RAW;
        }
        Audio_init_recorder(s_recorder_info.file.codec_type, s_recorder_info.file.sampling_rate, s_recorder_info.file.channel_number);
        return true;
    }
    return false;
}

bool app_cmd_start_recorder(unsigned int count, const char** tp)
{
    if (count != 2)
    {
        return false;
    }

    s_recorder_info.file.size = 0;
    if (!app_open_output_file(tp[1]))
    {
        return false;
    }
    CMN_SimpleFifoClear(&s_recorder_info.fifo.handle);

    bool ret = Audio_start_recorder();
    if (!ret)
    {
        return false;
    }

    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    tattr.stacksize = 2048;

    struct sched_param param;
    param.sched_priority = SCHED_PRIORITY_DEFAULT;

    pthread_attr_setschedparam(&tattr, &param);

    int retThread = pthread_create(&s_tid, &tattr, (pthread_startroutine_t)record_thread, NULL);
    if (retThread != 0)
    {
        ret = false;
    }
    if (ret)
    {
        printf("Recorder Recording\n");
    }
    return ret;
}

bool app_cmd_stop_recorder(unsigned int count, const char** tp)
{
    if (!Audio_stop_recorder())
    {
        return false;
    }
    g_is_recording = false;
    pthread_join(s_tid, NULL);

    size_t occupied_simple_fifo_size = CMN_SimpleFifoGetOccupiedSize(&s_recorder_info.fifo.handle);
    uint32_t output_size = 0;

    while (occupied_simple_fifo_size > 0)
    {
        output_size = (occupied_simple_fifo_size > READ_SIMPLE_FIFO_SIZE) ?
            READ_SIMPLE_FIFO_SIZE : occupied_simple_fifo_size;
        app_write_output_file(output_size);
        occupied_simple_fifo_size = CMN_SimpleFifoGetOccupiedSize(&s_recorder_info.fifo.handle);
    }

    if (s_recorder_info.file.format_type == FORMAT_TYPE_WAV)
    {
        if (!app_update_wav_file_size())
        {
            printf("Error: app_write_wav_header() failure.\n");
        }
    }

    app_close_output_file();
    printf("Recorder Stopped\n");
    return true;
}

bool app_cmd_init_mic_gain(unsigned int count, const char** tp)
{
    if (count == 2)
    {
        int16_t mic_gain = atoi(tp[1]);
        return Audio_init_mic_gain(true, mic_gain);
    }
    return false;
}

bool app_cmd_help(unsigned int count, const char** tp)
{
    printf("------------------Command List---------------\n");
    printf("HELP\n");
    printf("INIT_MEM\n");
    printf("OPEN_PLAY_FILE player0 m2.mp3\n");
    printf("INITP player_id audio_file_name sample_rate channel_number bit_length\n");
    printf("    e.g., INITP 0 24000 2 16\n");
    printf("PLAY player0 player1\n");
    printf("    e.g., PLAY true false\n");
    printf("STOPP player_id stop_mode\n");
    printf("    e.g., STOP 0 NORMAL\n");
    printf("QUIT\n");
    printf("---------------------------------------------\n");
    return true;
}

bool app_cmd_quit(unsigned int count, const char** tp)
{
    g_quit = true;
    return true;
}


bool app_process_cmd_loop()
{
    char command[64];

    while (!g_quit)
    {
        gets(command);
        //printf("Command[%s] received\n", command);

        const char* tp[6];
        tp[0] = strtok(command, " ");

        unsigned int count = 0;
        while (tp[count] != NULL && count < sizeof(tp)/sizeof(tp[0]) - 1)
        {
            count++;
            tp[count] = strtok(NULL, " ");
        }

        if (tp[0] == NULL || tp[0][0] == '\0')
            continue;

        unsigned int i = 0;
        for (; i<sizeof(s_cmd_process_map)/sizeof(s_cmd_process_map[0]); i++)
        {
            if (strcmp(tp[0], s_cmd_process_map[i].cmd) == 0)
            {
                if (!s_cmd_process_map[i].func(count, tp))
                {
                    //g_quit = true;
                }
                break;
            }
        }

        if (i >= sizeof(s_cmd_process_map)/sizeof(s_cmd_process_map[0]))
        {
            printf("WRONG command.\n");
        }
    }
    return true;
}

