/*
* Copyright 2015 Sony Corporation
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions, and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/

#include "app_main.h"
#include <stdlib.h>
#include <assert.h>
#include <drivers/peripheral/pd_gpio.h>
#include <storage/fs_wrapper.h>
#include <system/sys_flash_mgr.h>
#include <debug/dbg_shell.h>
#include <debug/dbg_log.h>
//#define DBG_MODULE DBG_MODULE_AS

DBG_DEFINE_MODULE(SAMPLE);
#define DBG_MODULE SAMPLE

#define AUDIO_CODEC_DSP_VOL_STEP  5
#define EMMC_AUDIO_ROOT			"A:/audio"

#define DELAY_TIEM	10 /* ms */

static uint32_t s_consumed_es_byte_size= 0;
void ramPlaySequence();

namespace FileSystemUtilities {

int initfFatFs(void)
{
	int ret = -1;
	int stat;
	/* eMMC/FatFs初期化 */
	if ( (stat = FS_Init()) != 0 ) {
		DBG_LOGF_ERROR("FS_Init:NG %d\n", stat);
	}
	else if ( (stat = FS_Mount('A')) != 0 ) {
		DBG_LOGF_ERROR("FS_Mount:NG %d\n", stat);
	}
	else if ( (stat = FS_Chdir("A:/")) != 0 ) {
		DBG_LOGF_ERROR("FS_Chdir:NG %d\n", stat);
	}
	else {
		ret = 0;
	}
	return ret;
}

/*--------------------------------------------------------------------*/
int finalizeFatFs(void)
{
	int ret = -1;
	int stat;
	/* eMMC/FatFs終了処理 */
	if ( (stat = FS_Unmount('A', FS_UNMOUNT_NORMAL)) != 0 ) {
		DBG_LOGF_ERROR("FS_Unmount:NG %d\n", stat);
	}
	else if ( (stat = FS_Exit()) != 0 ) {
		DBG_LOGF_ERROR("FS_Exit:NG %d\n", stat);
	}
	else {
		ret = 0;
	}
	return ret;
}

}; /* end of namespace */

/*--------------------------------------------------------------------
	Main Loop
  --------------------------------------------------------------------*/
extern "C" {

extern DBG_ShellCommandTreeNode_t playerCmdTree;

void app_main(void)
{
	DBG_LOG_DEBUG("Music Player WakeUp!\n");
	DBG_LogInit();

	DBG_ShellRegisterCommandTree(&playerCmdTree);

	/* below message can't be shown on host side */
	DBG_LOG_INFO("player command\n");

	if (FileSystemUtilities::initfFatFs()) {
		DBG_LOG_FATAL("Failed initialize in file system\n");
	}

	/* User can write their own control sequences within this function or
	 * use debug shell to create their own control sequences. 
	 * By default, this sample project shows how to create control sequences
	 * by debug shell.
	 *
	 * If user want to write control sequences within app_main(), check
	 * sampleTestSequence().
	 */
	/* RAM sequences test */
	ramPlaySequence();

	vTaskSuspend(NULL);
}

/*--------------------------------------------------------------------*/
static bool getNextTrackCallback(char* filename) {
	return AppMain::getInstance()->getNextTrack(filename);
}

} /* extern "C" */

/*--------------------------------------------------------------------*/
void sampleTestSequence(void)
{
	/* Player設定の初期化 */
	AppMain::PlayerStatusParam param;
	param.input_device = AS_SETPLAYER_INPUTDEVICE_EMMC;		/* 入力デバイスを規定(eMMC FileSystem:0) */
	param.output_device = AS_SETPLAYER_OUTPUTDEVICE_SPHP;	/* 出力デバイスを規定(Cxd5247 SP/HP:0) */
	AppMain::getInstance()->setPlayerMode(param);

	DBG_LOG_INFO("Music Player Initialization Completed!\n");
}

void ramPlaySequence(void)
{
	while(1) {
		vTaskDelay(DELAY_TIEM);
		if(s_consumed_es_byte_size >= WRITE_SIMPLE_FIFO_SIZE) {
			AppMain::getInstance()->writeToSimpleFifo(WRITE_SIMPLE_FIFO_SIZE);
			s_consumed_es_byte_size -= WRITE_SIMPLE_FIFO_SIZE;
		}
	}
}

static void inputDeviceCallback(uint32_t size)
{
	s_consumed_es_byte_size += size;
}

/*--------------------------------------------------------------------*/
void AppMain::writeToSimpleFifo(uint32_t size)
{
	if(m_file_fp == NULL) {
		return;
	}
	uint32_t save_size = size;
	while (1) {
		if (m_file_fp == NULL) {
			if (m_audio_contents_config_ctrl.play_list_num > m_audio_contents_config_ctrl.play_list_cnt) {
				m_file_fp = FM_FileOpen(reinterpret_cast<char *>(m_audio_contents_config_play_list[m_audio_contents_config_ctrl.play_list_cnt]), FM_MODE_READ);
				if (m_file_fp == NULL) {
					DBG_LOGF_ERROR("FM_FileOpen:NG\n");
					return;
				}
				DBG_LOGF_INFO("PlayMusic:[%d] %s\n", m_audio_contents_config_ctrl.play_list_cnt, m_audio_contents_config_play_list[m_audio_contents_config_ctrl.play_list_cnt]);
				m_audio_contents_config_ctrl.play_list_cnt++;
				size = save_size;
			}
		}
		if (FM_FileRead(m_ram_buf, &size, m_file_fp) != 0) {
			DBG_LOGF_ERROR("FM_FileRead:NG\n");
			if (FM_FileClose(m_file_fp) != 0) {
				DBG_LOGF_ERROR("FM_FileClose:NG\n");
				return;
			}
			m_file_fp = NULL;
			return;
		}
		if (size == 0) {
			DBG_LOGF_INFO("FM_FileRead:END\n");
			if (FM_FileClose(m_file_fp) != 0) {
				DBG_LOGF_ERROR("FM_FileClose:NG\n");
				return;
			}
			m_file_fp = NULL;
			if (m_audio_contents_config_ctrl.play_list_num > m_audio_contents_config_ctrl.play_list_cnt) {
				continue;
			}
			else {
				return;
			}
		}
		break;
	}

	/* (640 sample * 16bit * 2ch) frames */
	if (CMN_SimpleFifoOffer(&m_simple_fifo_handle, reinterpret_cast<const void*>(m_ram_buf), size) == 0) {
		DBG_LOGF_ERROR("CMN_SimpleFifoOffer:NG\n");
	}
}

/*--------------------------------------------------------------------
	Functions for Debug Shell
  --------------------------------------------------------------------*/
void AppMain::playMusic(void)
{
	{
		if (m_cur_input_device == AS_SETPLAYER_INPUTDEVICE_RAM) {
			s_consumed_es_byte_size = 0;
			m_file_fp = NULL;
			m_audio_contents_config_ctrl.play_list_cnt = 0;

			CMN_SimpleFifoClear(&m_simple_fifo_handle);

			/* write 40 frames first */
			uint32_t size;
			for (int i = 0; i < 40; i++) {
				if (m_file_fp == NULL) {
					if (m_audio_contents_config_ctrl.play_list_num > m_audio_contents_config_ctrl.play_list_cnt) {
						m_file_fp = FM_FileOpen(reinterpret_cast<char *>(m_audio_contents_config_play_list[m_audio_contents_config_ctrl.play_list_cnt]), FM_MODE_READ);
						if (m_file_fp == NULL) {
								DBG_LOGF_ERROR("FM_FileOpen:NG\n");
								return;
						}
						DBG_LOGF_INFO("PlayMusic:[%d] %s\n", m_audio_contents_config_ctrl.play_list_cnt, m_audio_contents_config_play_list[m_audio_contents_config_ctrl.play_list_cnt]);
						m_audio_contents_config_ctrl.play_list_cnt++;
					}
				}
				size = WRITE_SIMPLE_FIFO_SIZE;
				if (FM_FileRead(m_ram_buf, &size, m_file_fp) != 0) {
					DBG_LOGF_ERROR("FM_FileRead:NG\n");
					return;
				}
				if (size == 0) {
					DBG_LOGF_INFO("FM_FileRead:END\n");
					if (FM_FileClose(m_file_fp) != 0) {
						DBG_LOGF_ERROR("FM_FileClose:NG\n");
						return;
					}
					m_file_fp = NULL;
					if (m_audio_contents_config_ctrl.play_list_num > m_audio_contents_config_ctrl.play_list_cnt) {
						continue;
					}
					else {
						break;
					}
				}
				if (CMN_SimpleFifoOffer(&m_simple_fifo_handle, reinterpret_cast<const void*>(m_ram_buf), size) == 0) {
					DBG_LOGF_ERROR("CMN_SimpleFifoOffer:NG\n");
					return;
				}
			}
		}
	}

	{
		AudioCommand command;
		command.header.packet_length = LENGTH_PLAY_PLAYER;
		command.header.command_code = AUDCMD_PLAYPLAYER;
		command.header.sub_code =0x00;
		AS_SendAudioCommand(&command);
	}

	{
		AudioResult result;
		AS_ReceiveAudioResult(&result);
		DBG_LOGF_INFO("playMusic result_code = %x\n",result.header.result_code);
		printCommandResult(result.header.result_code);
	}
}

/*--------------------------------------------------------------------*/
void AppMain::stopMusic(void)
{
	{
		if (m_cur_input_device == AS_SETPLAYER_INPUTDEVICE_RAM) {
			if (m_file_fp != NULL) {
				if (FM_FileClose(m_file_fp) != 0) {
					DBG_LOGF_ERROR("FM_FileClose:NG\n");
					return;
				}
				m_file_fp = NULL;
			}
		}
	}

	{
		AudioCommand command;
		command.header.packet_length = LENGTH_STOP_PLAYER;
		command.header.command_code = AUDCMD_STOPPLAYER;
		command.header.sub_code =0x00;
		AS_SendAudioCommand(&command);
	}

	{
		AudioResult result;
		AS_ReceiveAudioResult(&result);
		DBG_LOGF_INFO("stopMusic result_code = %x\n",result.header.result_code);
		printCommandResult(result.header.result_code);
	}
}

/*--------------------------------------------------------------------*/
void AppMain::pauseMusic(void)
{
	{
		AudioCommand command;
		command.header.packet_length = LENGTH_PAUSE_PLAYER;
		command.header.command_code = AUDCMD_PAUSEPLAYER;
		command.header.sub_code =0x00;
		AS_SendAudioCommand(&command);
	}

	{
		AudioResult result;
		AS_ReceiveAudioResult(&result);
		DBG_LOGF_INFO("pauseMusic result_code = %x\n",result.header.result_code);
		printCommandResult(result.header.result_code);
	}
}

/*--------------------------------------------------------------------*/
void AppMain::nextMusic(void)
{
	{
		AudioCommand command;
		command.header.packet_length = LENGTH_NEXT_PLAY;
		command.header.command_code = AUDCMD_NEXTPLAY;
		command.header.sub_code =0x00;
		AS_SendAudioCommand(&command);
	}

	{
		AudioResult result;
		AS_ReceiveAudioResult(&result);
		DBG_LOGF_INFO("nextMusic result_code = %x\n",result.header.result_code);
		printCommandResult(result.header.result_code);
	}
}
	
/*--------------------------------------------------------------------*/
void AppMain::prevMusic(void)
{
	{
		AudioCommand command;
		command.header.packet_length = LENGTH_PREVIOUS_PLAY;
		command.header.command_code = AUDCMD_PREVPLAY;
		command.header.sub_code =0x00;
		AS_SendAudioCommand(&command);
	}

	{
		AudioResult result;
		AS_ReceiveAudioResult(&result);
		DBG_LOGF_INFO("prevMusic result_code = %x\n",result.header.result_code);
		printCommandResult(result.header.result_code);
	}
}

/*--------------------------------------------------------------------*/
void AppMain::setPlayerParam(const PlayerModeParam& param)
{
	{
		AudioCommand command;
		command.header.packet_length = LENGTH_SET_PLAYER_PARAM;
		command.header.command_code = AUDCMD_SETPLAYERPARAM;
		command.header.sub_code = 0x00;
		command.set_player_param_param.play_filter = param.play_filter;
		command.set_player_param_param.play_mode   = param.play_mode;
		command.set_player_param_param.repeat_mode = param.repeat_mode;
		AS_SendAudioCommand(&command);
	}

	{
		AudioResult result;
		AS_ReceiveAudioResult(&result);
		DBG_LOGF_INFO("setPlayerParam result_code = %x\n",result.header.result_code);
		printCommandResult(result.header.result_code);
	}
}

/*--------------------------------------------------------------------*/
void AppMain::initPlayer(AsInitPlayerCodecType codec_type, \
		AsInitPlayerBitLength bit_length, \
		AsInitPlayerChannelNumberIndex ch_num, \
		AsInitPlayerSamplingRateIndex sampling_rate)
{
	{
		AudioCommand command;
		command.header.packet_length = LENGTH_INIT_PLAYER;
		command.header.command_code = AUDCMD_INITPLAYER;
		command.header.sub_code = 0x00;
		command.init_player_param.codec_type	= codec_type;
		command.init_player_param.bit_length	= bit_length;
		command.init_player_param.channel_number= ch_num;
		command.init_player_param.sampling_rate	= sampling_rate;
		AS_SendAudioCommand(&command);
	}

	{
		AudioResult result;
		AS_ReceiveAudioResult(&result);
		DBG_LOGF_INFO("initPlayer result_code = %x\n",result.header.result_code);
		printCommandResult(result.header.result_code);
	}

	if (m_cur_input_device == AS_SETPLAYER_INPUTDEVICE_RAM) {	
		audioContentsGenPlayList(codec_type, bit_length, ch_num, sampling_rate);
	}
}

/*--------------------------------------------------------------------*/
void AppMain::incVol(void)
{
	{
		AudioCommand command;
		int master_db = AS_AC_CODEC_VOL_DAC;

		command.header.packet_length = LENGTH_SETVOLUME;
		command.header.command_code  = AUDCMD_SETVOLUME;
		command.header.sub_code      = 0;
		if( AS_VOLUME_MAX >= m_audio_codec_dsp_vol + AUDIO_CODEC_DSP_VOL_STEP ){
			m_audio_codec_dsp_vol += AUDIO_CODEC_DSP_VOL_STEP;
			master_db = m_audio_codec_dsp_vol;
		} else {
			master_db = AS_VOLUME_HOLD;
		}
		command.set_volume_param.input1_db = 0;
		command.set_volume_param.input2_db = AS_VOLUME_HOLD;
		command.set_volume_param.master_db = master_db;
		AS_SendAudioCommand(&command);
	}

	{
		AudioResult result;
		AS_ReceiveAudioResult(&result);
		DBG_LOGF_INFO("incVol result_code = %x\n",result.header.result_code);
		printCommandResult(result.header.result_code);
	}
}

/*--------------------------------------------------------------------*/
void AppMain::decVol(void)
{
	{
		AudioCommand command;
		int master_db = AS_AC_CODEC_VOL_DAC;

		command.header.packet_length = LENGTH_SETVOLUME;
		command.header.command_code  = AUDCMD_SETVOLUME;
		command.header.sub_code      = 0;
		if( AS_VOLUME_MIN <= m_audio_codec_dsp_vol - AUDIO_CODEC_DSP_VOL_STEP ){
			m_audio_codec_dsp_vol -= AUDIO_CODEC_DSP_VOL_STEP;
			master_db = m_audio_codec_dsp_vol;
		} else {
			master_db = AS_VOLUME_HOLD;
		}
		command.set_volume_param.input1_db = 0;
		command.set_volume_param.input2_db = AS_VOLUME_HOLD;
		command.set_volume_param.master_db = master_db;
		AS_SendAudioCommand(&command);
	}

	{
		AudioResult result;
		AS_ReceiveAudioResult(&result);
		DBG_LOGF_INFO("decVol result_code = %x\n",result.header.result_code);
		printCommandResult(result.header.result_code);
	}
}

/*--------------------------------------------------------------------*/
bool AppMain::transferToPlayerStatus(const PlayerStatusParam& param)
{
	{
		m_cur_input_device = param.input_device;
		s_consumed_es_byte_size = 0;

		AudioCommand command;
		command.header.packet_length = LENGTH_SET_PLAYER_STATUS;
		command.header.command_code = AUDCMD_SETPLAYERSTATUS;
		command.header.sub_code = 0x00;

		if (m_cur_input_device == AS_SETPLAYER_INPUTDEVICE_RAM) {
			if (!initializeSimpleFIFO()) {
				return false;
			}

			m_ram_input_device_handler.simple_fifo_handler = reinterpret_cast<void*>(&m_simple_fifo_handle);
			m_ram_input_device_handler.callback_function = inputDeviceCallback;
			command.set_player_sts_param.ram_handler	= &m_ram_input_device_handler;
		}
		else if (m_cur_input_device == AS_SETPLAYER_INPUTDEVICE_EMMC) {
			m_emmc_input_device_handler.playlist_source = m_playlist_source;
			if (m_playlist_source == AS_SETPLAYER_INPUTDEVICE_INTER_PLAYLIST) {
				m_emmc_input_device_handler.callback_function = NULL;
			}
			else if (m_playlist_source == AS_SETPLAYER_INPUTDEVICE_EXT_PLAYLIST) {
				createPlaylist();
				m_emmc_input_device_handler.callback_function	= &getNextTrackCallback;
			}
			command.set_player_sts_param.emmc_handler	= &m_emmc_input_device_handler;
		}

		command.set_player_sts_param.input_device		= param.input_device;

		command.set_player_sts_param.output_device		= param.output_device;
		command.set_player_sts_param.output_device_handler	= 0x00;			/* 出力デバイスをHandlerを規定(T.B.D) */
		
		AS_SendAudioCommand(&command);
	}

	{
		AudioResult result;
		AS_ReceiveAudioResult(&result);
		DBG_LOGF_INFO("transferToPlayerStatus result_code = %x\n",result.header.result_code);
		printCommandResult(result.header.result_code);

		if (AUDRLT_ERRORRESPONSE == result.header.result_code ||
				AUDRLT_ERRORATTENTION == result.header.result_code) {
			return false;
		}
	}
	return true;
}

/*--------------------------------------------------------------------*/
void AppMain::setReadyStatus(void)
{
	{
		AudioCommand command;
		command.header.packet_length = LENGTH_SET_READY_STATUS;
		command.header.command_code = AUDCMD_SETREADYSTATUS;
		command.header.sub_code = 0x00;

		AS_SendAudioCommand(&command);
	}

	{
		AudioResult result;
		AS_ReceiveAudioResult(&result);
		DBG_LOGF_INFO("setReadyStatus result_code = %x\n", result.header.result_code);
		printCommandResult(result.header.result_code);

		m_playlist_source = AS_SETPLAYER_INPUTDEVICE_INTER_PLAYLIST;

		if (m_cur_input_device == AS_SETPLAYER_INPUTDEVICE_RAM) {
			if (m_file_fp != NULL) {
				if (FM_FileClose(m_file_fp) != 0) {
					DBG_LOGF_ERROR("FM_FileClose:NG\n");
					return;
				}
				m_file_fp = NULL;
				s_consumed_es_byte_size = 0;
			}
		}
	}
}

/*--------------------------------------------------------------------*/
void AppMain::setPlayerMode(const PlayerStatusParam& param)
{
	/* Keep the order */
	/* Cxd5247 Input / Output setting */
	if (m_enable_sound_effect == AS_DISABLE_SOUNDEFFECT) {
		if (!initOutputSelect()) { return; }
	}

	/* Player設定の初期化 */
	if (transferToPlayerStatus(param)) {
		setVolume(0);
	}
}

/*--------------------------------------------------------------------*/
void AppMain::powerOn(uint32_t param)
{
	AudioCommand command;
	
	command.header.packet_length = LENGTH_POWERON;
	command.header.command_code  = AUDCMD_POWERON;
	command.header.sub_code      = 0x00;
	command.power_on_param.enable_sound_effect = param;

	AS_SendAudioCommand( &command );
	
	AudioResult result;
	AS_ReceiveAudioResult( &result );
	DBG_LOGF_INFO("PowerOn result_code = %x\n",result.header.result_code);
	printCommandResult(result.header.result_code);

	m_enable_sound_effect = param;
	if (m_enable_sound_effect == AS_ENABLE_SOUNDEFFECT) {
		initOutputSelect();
	}
}

/*--------------------------------------------------------------------*/
void AppMain::setPowerOffStatus()
{
	AudioCommand command;
	
	command.header.packet_length = LENGTH_SET_POWEROFF_STATUS;
	command.header.command_code  = AUDCMD_SETPOWEROFFSTATUS;
	command.header.sub_code      = 0x00;

	AS_SendAudioCommand( &command );
	
	AudioResult result;
	AS_ReceiveAudioResult( &result );
	DBG_LOGF_INFO("SetPowerOffStatus result_code = %x\n",result.header.result_code);
	printCommandResult(result.header.result_code);

}

/*--------------------------------------------------------------------*/
bool AppMain::initOutputSelect()
{
	/* ATTENTION : Always Keep the following settings */ 
	AudioCommand command;

	command.header.packet_length = LENGTH_INITOUTPUTSELECT;
	command.header.command_code  = AUDCMD_INITOUTPUTSELECT;
	command.header.sub_code      = 0;
	command.init_output_select_param.output_device_sel = AS_OUT_SP;

	AS_SendAudioCommand( &command );

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	DBG_LOGF_INFO("initOutputSelect result_code = %x\n",result.header.result_code);
	printCommandResult(result.header.result_code);

	if ((AUDRLT_ERRORRESPONSE == result.header.result_code) ||
			(AUDRLT_ERRORATTENTION == result.header.result_code)) {
		return false;
	}
	return true;
}

/*--------------------------------------------------------------------*/
void AppMain::setVolume(uint32_t param)
{
	AudioCommand command;
	command.header.packet_length = LENGTH_SETVOLUME;
	command.header.command_code  = AUDCMD_SETVOLUME;
	command.header.sub_code      = 0;
	if (param == 0) {
		command.set_volume_param.input1_db = 0;
		command.set_volume_param.input2_db = AS_VOLUME_HOLD;
	}
	else {
		command.set_volume_param.input1_db = AS_VOLUME_HOLD;
		command.set_volume_param.input2_db = 0;
	}
	command.set_volume_param.master_db = AS_AC_CODEC_VOL_DAC;
	AS_SendAudioCommand(&command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	DBG_LOGF_INFO("setVolume result_code = %x\n",result.header.result_code);
	printCommandResult(result.header.result_code);
}

/*--------------------------------------------------------------------*/
void AppMain::setPlaylistSource(uint32_t param)
{
	m_playlist_source = param;
}

/*--------------------------------------------------------------------*/
void AppMain::initSoundEffect()
{
	if (!initializeSimpleFIFO()) {
		return;
	}

	AudioCommand command;
	command.header.packet_length = LENGTH_INIT_SOUNDEFFECT;
	command.header.command_code  = AUDCMD_INITSOUNDEFFECT;
	command.header.sub_code      = 0;

	command.init_sound_effect_param.sampling_rate = AS_INITSOUNDEFFECT_INPUT_FS_48000;
	command.init_sound_effect_param.channel_number = AS_INITSOUNDEFFECT_CHNL_STEREO;
	command.init_sound_effect_param.bit_length = AS_INITSOUNDEFFECT_BITLENGTH_16;
	command.init_sound_effect_param.codec_type = AS_INITSOUNDEFFECT_WAV;

	AS_SendAudioCommand(&command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	DBG_LOGF_INFO("initSoundEffect result_code = %x\n",result.header.result_code);
	printCommandResult(result.header.result_code);

	setVolume(1);

}
/*--------------------------------------------------------------------*/
void AppMain::startSoundEffect()
{
	AudioCommand command;
	command.header.packet_length = LENGTH_START_SOUNDEFFECT;
	command.header.command_code  = AUDCMD_STARTSOUNDEFFECT;
	command.header.sub_code      = 0;

	AsStartSoundEffectInputDeviceHdlr ram_input_device_handler;
	ram_input_device_handler.simple_fifo_handler = reinterpret_cast<void*>(&m_simple_fifo_handle);
	ram_input_device_handler.mode = AS_STARTSOUNDEFFECT_NORMAL;
	ram_input_device_handler.size = 0x35df2;
	command.start_sound_effect_param.input_device_handler	= &ram_input_device_handler;

	s_consumed_es_byte_size = 0;
	m_file_fp = NULL;
	m_audio_contents_config_ctrl.play_list_cnt = 0;

	CMN_SimpleFifoClear(&m_simple_fifo_handle);

	/* write 40 frames first */
	uint32_t size;
	m_file_fp = FM_FileOpen("se_moa03_48kHz_2ch_16bit.wav", FM_MODE_READ);
	if (m_file_fp == NULL) {
		DBG_LOGF_ERROR("FM_FileOpen:NG\n");
		return;
	}
	DBG_LOGF_INFO("PlayMusic: %s\n", "se_moa03_48kHz_2ch_16bit.wav");

	for (int i = 0; i < 40; i++) {
		size = WRITE_SIMPLE_FIFO_SIZE;
		if (FM_FileRead(m_ram_buf, &size, m_file_fp) != 0) {
			DBG_LOGF_ERROR("FM_FileRead:NG\n");
			return;
		}
		if (size == 0) {
			DBG_LOGF_INFO("FM_FileRead:END\n");
			if (FM_FileClose(m_file_fp) != 0) {
				DBG_LOGF_ERROR("FM_FileClose:NG\n");
				return;
			}
			m_file_fp = NULL;
			break;
		}
		if (CMN_SimpleFifoOffer(&m_simple_fifo_handle, reinterpret_cast<const void*>(m_ram_buf), size) == 0) {
			DBG_LOGF_ERROR("CMN_SimpleFifoOffer:NG\n");
			return;
		}
	}

	AS_SendAudioCommand(&command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	DBG_LOGF_INFO("startSoundEffect result_code = %x\n",result.header.result_code);
	printCommandResult(result.header.result_code);
}
/*--------------------------------------------------------------------*/
void AppMain::stopSoundEffect()
{
	AudioCommand command;
	command.header.packet_length = LENGTH_STOP_SOUNDEFFECT;
	command.header.command_code  = AUDCMD_STOPSOUNDEFFECT;
	command.header.sub_code      = 0;

	AS_SendAudioCommand(&command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	DBG_LOGF_INFO("stopSoundEffect result_code = %x\n",result.header.result_code);
	printCommandResult(result.header.result_code);

	if (m_file_fp != NULL) {
		FM_FileClose(m_file_fp);
		m_file_fp = NULL;
	}
}

/*--------------------------------------------------------------------*/
bool AppMain::initializeSimpleFIFO()
{
	if (&m_simple_fifo_handle == NULL ||
	    m_simple_fifo_buf == NULL ||
	    CMN_SimpleFifoInitialize(&m_simple_fifo_handle, m_simple_fifo_buf, 2560*50, NULL) != 0) {
		DBG_LOGF_ERROR("FIFO create error\n");
		return false;
	}
	return true;
}

/*--------------------------------------------------------------------*/
void AppMain::printCommandResult(uint8_t result)
{
	if (AUDRLT_ERRORRESPONSE == result) {
		DBG_LOGF_ERROR("AUDRLT_ERRORRESPONSE\n");
	}
	else if (AUDRLT_ERRORATTENTION == result) {
		DBG_LOGF_FATAL("AUDRLT_ERRORATTENTION\n");
	}
}

/*--------------------------------------------------------------------*/
#define CR_CODE    (0xd)
#define LF_CODE    (0xa)
#define TAB_CODE   (0x9)
#define SPACE_CODE (0x20)
/*--------------------------------------------------------------------*/
bool AppMain::audioContentsConfigFileOpen()
{
	m_audio_contents_config_file_fp = NULL;
	m_audio_contents_config_file_fp = FM_FileOpen(AUDIO_CONTENTS_CONFIG_FILE_NAME, FM_MODE_READ);
	if (m_audio_contents_config_file_fp == NULL) {
		DBG_LOGF_ERROR("FM_FileOpen:NG - %s\n", AUDIO_CONTENTS_CONFIG_FILE_NAME);
		return false;
	}
	return true;
}

/*--------------------------------------------------------------------*/
bool AppMain::audioContentsConfigFileClose()
{
	if (m_audio_contents_config_file_fp != NULL) {
		if (FM_FileClose(m_audio_contents_config_file_fp) != 0) {
			DBG_LOGF_ERROR("FM_FileClose:NG - %s\n", AUDIO_CONTENTS_CONFIG_FILE_NAME);
			return false;
		}
		m_audio_contents_config_file_fp = NULL;
	}
	return true;
}

/*--------------------------------------------------------------------*/
/* read one line string from audio_file_for_ram_play.cfg */
bool AppMain::audioContentsConfigGets(int8_t *p_buf)
{
	if(m_audio_contents_config_file_fp == NULL) {
		return false;
	}
	/* free total size of the buffer */
	uint32_t read_size = AUDIO_CONTENTS_CONFIG_BUF_SIZE - m_audio_contents_config_ctrl.remain_data_size;
	/* free size from the current reading start position to the end of the buffer */
	uint32_t read_size_buf_tail = AUDIO_CONTENTS_CONFIG_BUF_SIZE - m_audio_contents_config_ctrl.read_start_position;

	static int8_t s_audio_contents_config_buf[AUDIO_CONTENTS_CONFIG_BUF_SIZE+1];

	if (read_size > read_size_buf_tail) {
		/* read the data up to the end of the buffer */
		if (FM_FileRead(&s_audio_contents_config_buf[m_audio_contents_config_ctrl.read_start_position], &read_size_buf_tail, m_audio_contents_config_file_fp) != 0) {
			DBG_LOGF_ERROR("FM_FileRead:NG - %s\n", AUDIO_CONTENTS_CONFIG_FILE_NAME);
			audioContentsConfigFileClose();
			return false;
		}
		read_size -= read_size_buf_tail;
		m_audio_contents_config_ctrl.read_start_position = 0;
	}
	else {
		read_size_buf_tail = 0;
	}

	if (FM_FileRead(&s_audio_contents_config_buf[m_audio_contents_config_ctrl.read_start_position], &read_size, m_audio_contents_config_file_fp) != 0) {
		DBG_LOGF_ERROR("FM_FileRead:NG - %s\n", AUDIO_CONTENTS_CONFIG_FILE_NAME);
		audioContentsConfigFileClose();
		return false;
	}
	m_audio_contents_config_ctrl.remain_data_size += (read_size + read_size_buf_tail);
	m_audio_contents_config_ctrl.read_start_position += read_size;
	if (m_audio_contents_config_ctrl.read_start_position >= AUDIO_CONTENTS_CONFIG_BUF_SIZE) {
		m_audio_contents_config_ctrl.read_start_position = 0;
	}


	uint32_t remain_data_size = m_audio_contents_config_ctrl.remain_data_size;
	uint32_t i;
	for (i=0; s_audio_contents_config_buf[m_audio_contents_config_ctrl.read_idx] != NULL && i<remain_data_size; i++) {
		m_audio_contents_config_ctrl.remain_data_size--;
		if (s_audio_contents_config_buf[m_audio_contents_config_ctrl.read_idx] == LF_CODE) {
			if ((m_audio_contents_config_ctrl.read_idx>0  && s_audio_contents_config_buf[m_audio_contents_config_ctrl.read_idx-1] == CR_CODE) ||
				(m_audio_contents_config_ctrl.read_idx==0 && s_audio_contents_config_buf[AUDIO_CONTENTS_CONFIG_BUF_SIZE-1] == CR_CODE)) {
				p_buf--;
			}
			*p_buf = NULL;
			m_audio_contents_config_ctrl.read_idx++;
			if (m_audio_contents_config_ctrl.read_idx >= AUDIO_CONTENTS_CONFIG_BUF_SIZE) {
				m_audio_contents_config_ctrl.read_idx = 0;
			}
			break;
		}
		else {
			*p_buf = s_audio_contents_config_buf[m_audio_contents_config_ctrl.read_idx];
			p_buf++;
			m_audio_contents_config_ctrl.read_idx++;
			if (m_audio_contents_config_ctrl.read_idx >= AUDIO_CONTENTS_CONFIG_BUF_SIZE) {
				m_audio_contents_config_ctrl.read_idx = 0;
			}
		}
	}
	if (i>= remain_data_size) {
		*p_buf = NULL;
	}
	return true;
}

/*--------------------------------------------------------------------*/
bool AppMain::audioContentsGetToken(int32_t argc, int8_t *argv[])
{
	int8_t buf[AUDIO_CONTENTS_CONFIG_BUF_SIZE+1];
	int8_t *p_src = buf;
	/* */
	do {
		if (audioContentsConfigGets(buf) == false) {
			return false;
		}
		/* Skip the space/TAB charctor at the beginning of the line */
		for (;*p_src != NULL && (*p_src == SPACE_CODE || *p_src == TAB_CODE);p_src++);
		/* Skip the commant line(#...) and no word */
	} while(*p_src == NULL || *p_src == '#');

	int32_t i;
	for (i=0; i<argc; i++) {
		int8_t *p_dst;
		for (;*p_src != NULL && (*p_src == SPACE_CODE || *p_src == TAB_CODE);p_src++);
		for (p_dst = argv[i];*p_src != NULL;p_src++, p_dst++) {
			if (*p_src == SPACE_CODE || *p_src == TAB_CODE) {
				*p_dst = NULL;
				break;
			}
			else {
				*p_dst = *p_src;
			}
		}
		if (*p_src == NULL) {
			*p_dst = NULL;
			break;
		}
	}
	if (argc-1 != i) {
		return false;
	}
	return true;
}

/*--------------------------------------------------------------------*/
bool AppMain::audioContentsGenPlayList(AsInitPlayerCodecType codec_type, \
		AsInitPlayerBitLength bit_length, \
		AsInitPlayerChannelNumberIndex ch_num, \
		AsInitPlayerSamplingRateIndex sampling_rate)
{
	int32_t get_codec_type;
	int32_t get_bit_length;
	int32_t get_ch_num;
	int32_t get_sampling_rate;
	int8_t param1[64], param2[10], param3[10], param4[10], param5[10];
	int8_t *p_tokenbuf[AUDIO_CONTENTS_CONFIG_TOKEN_NUM] = {param1, param2, param3, param4, param5};

	m_audio_contents_config_ctrl.remain_data_size = 0;
	m_audio_contents_config_ctrl.read_start_position = 0;
	m_audio_contents_config_ctrl.read_idx = 0;
	m_audio_contents_config_ctrl.play_list_num = 0;

	if (audioContentsConfigFileOpen() == false) {
		return false;
	}
	do {
		/* get parameters in audio_file_for_ram_play.cfg */
		if (audioContentsGetToken(AUDIO_CONTENTS_CONFIG_TOKEN_NUM, p_tokenbuf) == false) {
			audioContentsConfigFileClose();
			return false;
		}
		get_codec_type = atoi(reinterpret_cast<char *>(p_tokenbuf[1]));
		get_bit_length = atoi(reinterpret_cast<char *>(p_tokenbuf[2]));
		get_ch_num = atoi(reinterpret_cast<char *>(p_tokenbuf[3]));
		get_sampling_rate = atoi(reinterpret_cast<char *>(p_tokenbuf[4]));

		/* pick up the audio file that matches the initPlayer parameter and the parameter of audio_file_for_ram_play.cfg */
		if (get_codec_type == codec_type && get_bit_length == bit_length && get_ch_num == ch_num && get_sampling_rate == sampling_rate) {
			strcpy(reinterpret_cast<char *>(m_audio_contents_config_play_list[m_audio_contents_config_ctrl.play_list_num]), reinterpret_cast<char *>(p_tokenbuf[0]));
			DBG_LOGF_INFO("PlayMusic:List[%d] %s\n", m_audio_contents_config_ctrl.play_list_num, m_audio_contents_config_play_list[m_audio_contents_config_ctrl.play_list_num]);
			m_audio_contents_config_ctrl.play_list_num++;
			if (AUDIO_CONTENTS_CONFIG_LIST_MAX_NUM < m_audio_contents_config_ctrl.play_list_num) {
				DBG_LOGF_ERROR("Number of entries is over %d in %s\n", AUDIO_CONTENTS_CONFIG_LIST_MAX_NUM, AUDIO_CONTENTS_CONFIG_FILE_NAME);
				return false;
			}
		}
	} while (m_audio_contents_config_ctrl.remain_data_size > 0);
	if (audioContentsConfigFileClose() == false) {
		return false;
	}
	return true;
}

/*--------------------------------------------------------------------*/
bool AppMain::createPlaylist()
{
	FS_Dir* dir_descriptor = FS_Opendir(EMMC_AUDIO_ROOT);

	if (dir_descriptor == NULL) {
		DBG_LOGF_ERROR("Cannot open %s folder.\n", EMMC_AUDIO_ROOT);
		return false;
	}

	FS_Dirent dirent;
	Track track;

	for (;;) {
		if (FS_Readdir(dir_descriptor, &dirent) != 0) {
			DBG_LOGF_INFO("Playlist for music player is created.\n");
			break;
		}

		if (FS_ATTR_ARCH == dirent.attr) {
			track.filename = dirent.longName;
			m_playlist.add(track);
			DBG_LOGF_INFO("Add to playlist: %s.\n", track.filename.c_str());
		}
	}

	if (FS_Closedir(dir_descriptor) != 0) {
		DBG_LOGF_ERROR("FS_Closedir error.\n");
		return false;
	}

	return true;
}

/*--------------------------------------------------------------------*/
bool AppMain::getNextTrack(char* filename)
{
	Track next_track;
	if (m_playlist.getNextTrack(&next_track)) {
		strcpy(filename, next_track.filename.c_str());
		return true;
	}

	return false;
}

/*--------------------------------------------------------------------*/
bool Playlist::getNextTrack(Track* track) {
	if (m_playlist.empty()) {
		return false;
	}

	if (m_cur_track_id + 1 < 0) {
		return false;
	}

	if (m_cur_track_id + 1 >= m_playlist.size()) {
		m_cur_track_id = -1;
	}

	*track = m_playlist.at(++m_cur_track_id);
	return true;
}

