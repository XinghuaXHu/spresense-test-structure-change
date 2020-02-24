
#include "include/test_api.h"

#include <stdio.h>

#include "memutils/message/Message.h"
#include "include/fixed_fence.h"
#include "include/mem_layout.h"
#include "include/memory_layout.h"
#include "include/msgq_pool.h"

using namespace MemMgrLite;


#ifndef DSPBIN_FILE_PATH
#define DSPBIN_FILE_PATH "/mnt/sd0/BIN"
#endif

/* Definition for selection of output device.
 * Select speaker output or I2S output.
 */
#  define PLAYER_OUTPUT_DEV AS_SETPLAYER_OUTPUTDEVICE_SPHP
#  define PLAYER_MIXER_OUT  AS_OUT_SP


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
    else if (AUDRLT_ERRORATTENTION == result.header.result_code)
    {
        printf("Command code(0x%x): AUDRLT_ERRORATTENTION\n", command_code);
        return false;
    }
    return true;
}

bool Audio_power_on(void)
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

bool Audio_power_off(void)
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

bool Audio_set_ready(void)
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

uint8_t Audio_get_status(void)
{
    AudioCommand command;
    command.header.packet_length = LENGTH_GETSTATUS;
    command.header.command_code  = AUDCMD_GETSTATUS;
    command.header.sub_code      = 0x00;
    AS_SendAudioCommand(&command);

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    return result.notify_status.status_info;
}

bool Audio_init_output_select(void)
{
    AudioCommand command;
    command.header.packet_length = LENGTH_INITOUTPUTSELECT;
    command.header.command_code  = AUDCMD_INITOUTPUTSELECT;
    command.header.sub_code      = 0;
    command.init_output_select_param.output_device_sel = PLAYER_MIXER_OUT;
    AS_SendAudioCommand(&command);

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    return printAudCmdResult(command.header.command_code, result);
}

bool Audio_set_volume(int16_t input1_db, int16_t input2_db, int16_t master_db)
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

bool Audio_set_player_status(AsPlayerInputDeviceHdlrForRAM* input_device0, AsPlayerInputDeviceHdlrForRAM* input_device1)
{
    AudioCommand command;
    command.header.packet_length = LENGTH_SET_PLAYER_STATUS;
    command.header.command_code = AUDCMD_SETPLAYERSTATUS;
    command.header.sub_code = 0x00;
    command.set_player_sts_param.active_player         = AS_ACTPLAYER_BOTH;
    command.set_player_sts_param.player0.input_device  = AS_SETPLAYER_INPUTDEVICE_RAM;
    command.set_player_sts_param.player0.ram_handler   = input_device0;
    command.set_player_sts_param.player0.output_device = PLAYER_OUTPUT_DEV;
    command.set_player_sts_param.player1.input_device  = AS_SETPLAYER_INPUTDEVICE_RAM;
    command.set_player_sts_param.player1.ram_handler   = input_device1;
    command.set_player_sts_param.player1.output_device = PLAYER_OUTPUT_DEV;
    AS_SendAudioCommand(&command);
    printf("AS_SendAudioCommand() executed.\n");

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    return printAudCmdResult(command.header.command_code, result);
}

bool Audio_set_clkmode(int clk_mode)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SETRENDERINGCLK;
  command.header.command_code  = AUDCMD_SETRENDERINGCLK;
  command.header.sub_code      = 0x00;
  command.set_renderingclk_param.clk_mode = clk_mode;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

bool Audio_init_player(AsPlayerId id,
                    uint8_t codec_type,
                    uint32_t sampling_rate,
                    uint8_t channel_number,
                    uint8_t bit_length)
{
    AudioCommand command;
    command.header.packet_length = LENGTH_INIT_PLAYER;
    command.header.command_code  = AUDCMD_INITPLAYER;
    command.header.sub_code      = 0x00;
    command.player.player_id                 = id;
    command.player.init_param.codec_type     = codec_type;
    command.player.init_param.bit_length     = bit_length;
    command.player.init_param.channel_number = channel_number;
    command.player.init_param.sampling_rate  = sampling_rate;
    snprintf(command.player.init_param.dsp_path,
             AS_AUDIO_DSP_PATH_LEN,
             "%s",
             DSPBIN_FILE_PATH);
    AS_SendAudioCommand(&command);

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    return printAudCmdResult(command.header.command_code, result);
}

bool Audio_play_player(bool player0, bool player1)
{
    if (!player0 && !player1)
    {
        printf("Error: At least one player needs to be played.\n");
        return false;
    }

    AudioCommand command;
    command.header.packet_length = LENGTH_PLAY_PLAYER;
    command.header.command_code  = AUDCMD_PLAYPLAYER;
    command.header.sub_code      = 0x00;
    if (player0)
    {
        command.player.player_id     = AS_PLAYER_ID_0;
        AS_SendAudioCommand(&command);

        AudioResult result;
        AS_ReceiveAudioResult(&result);
        if (!printAudCmdResult(command.header.command_code, result))
        {
            return false;
        }
    }
    if (player1)
    {
        command.player.player_id     = AS_PLAYER_ID_1;
        AS_SendAudioCommand(&command);

        AudioResult result;
        AS_ReceiveAudioResult(&result);
        if (!printAudCmdResult(command.header.command_code, result))
        {
            return false;
        }
    }
    return true;
}

bool Audio_stop_player(AsPlayerId id, AsStopPlayerStopMode mode)
{
    AudioCommand command;
    command.header.packet_length = LENGTH_STOP_PLAYER;
    command.header.command_code  = AUDCMD_STOPPLAYER;
    command.header.sub_code      = 0x00;
    command.player.player_id            = id;
    command.player.stop_param.stop_mode = mode;
    AS_SendAudioCommand(&command);

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    return printAudCmdResult(command.header.command_code, result);
}

bool Audio_set_mute(bool isInput1Mute, bool isInput2Mute, bool isMasterMute)
{
    AudioCommand command;
    command.header.packet_length = LENGTH_SETVOLUMEMUTE;
    command.header.command_code  = AUDCMD_SETVOLUMEMUTE;
    command.header.sub_code      = 0;

    command.set_volume_mute_param.input1_mute = isInput1Mute ? AS_VOLUMEMUTE_MUTE : AS_VOLUMEMUTE_UNMUTE;
    command.set_volume_mute_param.input2_mute = isInput2Mute ? AS_VOLUMEMUTE_MUTE : AS_VOLUMEMUTE_UNMUTE;
    command.set_volume_mute_param.master_mute = isMasterMute ? AS_VOLUMEMUTE_MUTE : AS_VOLUMEMUTE_UNMUTE;
    AS_SendAudioCommand(&command);

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    return printAudCmdResult(command.header.command_code, result);
}

static void app_attention_callback(uint8_t module_id,
                                   uint8_t error_code,
                                   uint8_t sub_code,
                                   const char* file_name,
                                   uint32_t line)
{
    printf("Attention!! %s L%d ecode %d subcode %d\n", file_name, line, error_code, sub_code);
}

bool Audio_audio_manager(bool is_create)
{
    if (is_create)
    {
        /* Create manager of AudioSubSystem. */
        AudioSubSystemIDs ids;
        ids.app         = MSGQ_AUD_APP;
        ids.mng         = MSGQ_AUD_MNG;
        ids.player_main = MSGQ_AUD_PLY;
        ids.player_sub  = MSGQ_AUD_SUB_PLY;
        ids.mixer       = MSGQ_AUD_OUTPUT_MIX;
        ids.recorder    = MSGQ_AUD_RECORDER;
        ids.effector    = 0xFF;
        ids.recognizer  = 0xFF;

        AS_CreateAudioManager(ids, app_attention_callback);
    }
    else
    {
        AS_DeleteAudioManager();
    }
    return true;
}

bool Audio_player(bool is_create)
{
    bool result = true;

    if (is_create)
    {
        /* Create player feature. */
        AsCreatePlayerParam_t player_create_param;
        player_create_param.msgq_id.player   = MSGQ_AUD_PLY;
        player_create_param.msgq_id.mng      = MSGQ_AUD_MNG;
        player_create_param.msgq_id.mixer    = MSGQ_AUD_OUTPUT_MIX;
        player_create_param.msgq_id.dsp      = MSGQ_AUD_DSP;
        player_create_param.pool_id.es       = DEC_ES_MAIN_BUF_POOL;
        player_create_param.pool_id.pcm      = REND_PCM_BUF_POOL;
        player_create_param.pool_id.dsp      = DEC_APU_CMD_POOL;
        //player_create_param.pool_id.src_work = SRC_WORK_BUF_POOL;

        /* When calling AS_CreatePlayerMulti(), use the pool area
        * for multi-core playback processing.
        * When calling AS_CreatePlayer(), use the heap area.
        */
        result = AS_CreatePlayer(AS_PLAYER_ID_0, &player_create_param);
        if (!result)
        {
            printf("Error: AS_CratePlayer() failure. system memory insufficient!\n");
            return false;
        }

        player_create_param.msgq_id.player   = MSGQ_AUD_SUB_PLY;
        player_create_param.pool_id.es       = DEC_ES_SUB_BUF_POOL;
        player_create_param.pool_id.pcm      = REND_PCM_SUB_BUF_POOL;
        result = AS_CreatePlayer(AS_PLAYER_ID_1, &player_create_param);
        if (!result)
        {
            printf("Error: AS_CratePlayer() failure. system memory insufficient!\n");
            return false;
        }
    }
    else
    {
        AS_DeletePlayer(AS_PLAYER_ID_1);
        AS_DeletePlayer(AS_PLAYER_ID_0);
    }
    return result;
}

bool Audio_mixer(bool is_create)
{
    bool result = true;

    if (is_create)
    {
        /* Create mixer feature. */
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
    }
    else
    {
        AS_DeleteOutputMix();
    }
    return true;
}

bool Audio_renderer(bool is_create)
{
    bool result = true;

    if (is_create)
    {
        /* Create renderer feature. */
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
    }
    else
    {
        AS_DeleteRenderer();
    }
    return true;
}
bool Audio_recorder(bool is_create)
{
    if (is_create)
    {
        AsCreateRecorderParam_t recorder_create_param;
        recorder_create_param.msgq_id.recorder      = MSGQ_AUD_RECORDER;
        recorder_create_param.msgq_id.mng           = MSGQ_AUD_MNG;
        recorder_create_param.msgq_id.dsp           = MSGQ_AUD_DSP;
        recorder_create_param.pool_id.input         = INPUT_BUF_POOL;
        recorder_create_param.pool_id.output        = ES_BUF_POOL;
        recorder_create_param.pool_id.dsp           = ENC_APU_CMD_POOL;

        bool result = AS_CreateMediaRecorder(&recorder_create_param);
        if (!result)
        {
            printf("Error: AS_CreateMediaRecorder() failure. system memory insufficient!\n");
            return false;
        }
    }
    else
    {
        AS_DeleteMediaRecorder();
    }
    return true;
}
bool Audio_capture(bool is_create)
{
    if (is_create)
    {
        AsCreateCaptureParam_t capture_create_param;
        capture_create_param.msgq_id.dev0_req  = MSGQ_AUD_CAP;
        capture_create_param.msgq_id.dev0_sync = MSGQ_AUD_CAP_SYNC;
        capture_create_param.msgq_id.dev1_req  = 0xFF;
        capture_create_param.msgq_id.dev1_sync = 0xFF;

        bool result = AS_CreateCapture(&capture_create_param);
        if (!result)
        {
            printf("Error: As_CreateCapture() failure. system memory insufficient!\n");
            return false;
        }
    }
    else
    {
        AS_DeleteCapture();
    }
    return true;
}

bool Audio_create_audio_sub_system(bool is_recorder)
{
    Audio_audio_manager(true);
    if (!is_recorder)
    {
        if (!Audio_player(true))
        {
            return false;
        }

        if (!Audio_mixer(true))
        {
            return false;
        }

        if (!Audio_renderer(true))
        {
            return false;
        }
    }
    else
    {
        if (!Audio_recorder(true))
        {
            return false;
        }

        if (!Audio_capture(true))
        {
            return false;
        }
    }
    printf("Create audio sub system succeed.\n");
    return true;
}

bool Audio_deact_audio_sub_system(bool is_recorder)
{
    Audio_audio_manager(false);
    if (!is_recorder)
    {
        Audio_player(false);
        Audio_mixer(false);
        Audio_renderer(false);
    }
    else
    {
        Audio_recorder(false);
        Audio_capture(false);
    }
    return true;
}

static void app_init_recorder_wav(AudioCommand* command)
{
  command->recorder.init_param.codec_type = AS_CODECTYPE_LPCM;
}

static void app_init_recorder_mp3(AudioCommand* command)
{
  command->recorder.init_param.codec_type = AS_CODECTYPE_MP3;
  command->recorder.init_param.bitrate    = AS_BITRATE_96000;
}

static void app_init_recorder_opus(AudioCommand* command)
{
  command->recorder.init_param.codec_type = AS_CODECTYPE_OPUS;
  command->recorder.init_param.bitrate    = AS_BITRATE_8000;
  command->recorder.init_param.computational_complexity = AS_INITREC_COMPLEXITY_0;
}

bool Audio_init_recorder(uint8_t codec_type, uint32_t sampling_rate, uint8_t ch_type)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_INIT_RECORDER;
  command.header.command_code  = AUDCMD_INITREC;
  command.header.sub_code      = 0x00;
  command.recorder.init_param.sampling_rate  = sampling_rate;
  command.recorder.init_param.channel_number = ch_type;
  command.recorder.init_param.bit_length     = AS_BITLENGTH_16;
  snprintf(command.recorder.init_param.dsp_path, AS_AUDIO_DSP_PATH_LEN, "%s", DSPBIN_FILE_PATH);

  switch (codec_type)
    {
      case AS_CODECTYPE_LPCM:
        app_init_recorder_wav(&command);
        break;
      case AS_CODECTYPE_MP3:
        app_init_recorder_mp3(&command);
        break;
      case AS_CODECTYPE_OPUS:
        app_init_recorder_opus(&command);
        break;
      default:
        break;
  }

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

bool Audio_set_recorder_status(AsRecorderOutputDeviceHdlr* output_device)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_RECORDER_STATUS;
  command.header.command_code  = AUDCMD_SETRECORDERSTATUS;
  command.header.sub_code      = 0x00;
  command.set_recorder_status_param.input_device          = AS_SETRECDR_STS_INPUTDEVICE_MIC_A;
  command.set_recorder_status_param.input_device_handler  = 0x00;
  command.set_recorder_status_param.output_device         = AS_SETRECDR_STS_OUTPUTDEVICE_RAM;
  command.set_recorder_status_param.output_device_handler = output_device;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static void app_set_mic_gain_analog(AudioCommand* command, int16_t mic_gain)
{
  command->header.packet_length = LENGTH_INITMICGAIN;
  command->header.command_code  = AUDCMD_INITMICGAIN;
  command->header.sub_code      = 0;
  command->init_mic_gain_param.mic_gain[0] = mic_gain;
  command->init_mic_gain_param.mic_gain[1] = mic_gain;
  command->init_mic_gain_param.mic_gain[2] = mic_gain;
  command->init_mic_gain_param.mic_gain[3] = mic_gain;
  command->init_mic_gain_param.mic_gain[4] = AS_MICGAIN_HOLD;
  command->init_mic_gain_param.mic_gain[5] = AS_MICGAIN_HOLD;
  command->init_mic_gain_param.mic_gain[6] = AS_MICGAIN_HOLD;
  command->init_mic_gain_param.mic_gain[7] = AS_MICGAIN_HOLD;
}

static void app_set_mic_gain_digital(AudioCommand* command)
{
  command->header.packet_length = LENGTH_INITMICGAIN;
  command->header.command_code  = AUDCMD_INITMICGAIN;
  command->header.sub_code      = 0;
  command->init_mic_gain_param.mic_gain[0] = 0;
  command->init_mic_gain_param.mic_gain[1] = 0;
  command->init_mic_gain_param.mic_gain[2] = 0;
  command->init_mic_gain_param.mic_gain[3] = 0;
  command->init_mic_gain_param.mic_gain[4] = 0;
  command->init_mic_gain_param.mic_gain[5] = 0;
  command->init_mic_gain_param.mic_gain[6] = 0;
  command->init_mic_gain_param.mic_gain[7] = 0;
}

bool Audio_init_mic_gain(bool is_analog, int16_t mic_gain)
{

    AudioCommand command;
    if (is_analog)
    {
        app_set_mic_gain_analog(&command, mic_gain);
    }
    else
    {
        app_set_mic_gain_digital(&command);
    }
    AS_SendAudioCommand(&command);

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    return printAudCmdResult(command.header.command_code, result);
}

bool Audio_start_recorder(void)
{
    AudioCommand command;
    command.header.packet_length = LENGTH_START_RECORDER;
    command.header.command_code  = AUDCMD_STARTREC;
    command.header.sub_code      = 0x00;
    AS_SendAudioCommand(&command);

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    return printAudCmdResult(command.header.command_code, result);
}

bool Audio_stop_recorder(void)
{
    AudioCommand command;
    command.header.packet_length = LENGTH_STOP_RECORDER;
    command.header.command_code  = AUDCMD_STOPREC;
    command.header.sub_code      = 0x00;
    AS_SendAudioCommand(&command);

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    return printAudCmdResult(command.header.command_code, result);
}

bool Audio_beep(bool enable, int16_t vol, uint16_t freq)
{
    AudioCommand command;
    command.header.packet_length = LENGTH_SETBEEPPARAM;
    command.header.command_code  = AUDCMD_SETBEEPPARAM;
    command.header.sub_code      = 0x00;
    command.set_beep_param.beep_en = enable ? 1 : 0;
    command.set_beep_param.beep_vol = vol;
    command.set_beep_param.beep_freq = freq;
    AS_SendAudioCommand(&command);

    AudioResult result;
    AS_ReceiveAudioResult(&result);
    return printAudCmdResult(command.header.command_code, result);
}

