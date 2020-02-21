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

/*******************************************************
    Include
*******************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <debug/dbg_shell.h>
#include <debug/dbg_log.h>
DBG_DECLARE_MODULE(SAMPLE);
#define DBG_MODULE SAMPLE

#include <audio/high_level_api/as_high_level_api.h>
#include "app_main.h"

#define CMD_PLAYER_SETSPHP_ARGS_MIN	3
#define CMD_PLAYER_INIT_PLAYER_ARGS_MIN	6
#define CMD_PLAYER_SET_INPUT_DEVICE_ARGS_MIN	3
#define CMD_PLAYER_SET_INPUT_DEVICE_ARGS_MAX	3
#define CMD_PLAYER_SET_OUTPUT_DEVICE_ARGS_MIN	3

#define CMD_PREFIX_LEN	(1)
#define CMD_TYPE_LEN	(1)
#define CMD_ARG_START_INDEX	(CMD_PREFIX_LEN + CMD_TYPE_LEN)

#define CMD_PLAYER_PARAM_ARGS_MIN	5

AsSetPlayerInputDevice	g_input_device	= AS_SETPLAYER_INPUTDEVICE_EMMC;		/* Default = eMMC FileSystem */
AsSetPlayerOutputDevice	g_output_device	= AS_SETPLAYER_OUTPUTDEVICE_SPHP;	/* Default = Cxd5247 SP/HP */

AsInitPlayerSamplingRateIndex	g_cur_sampling_rate	= AS_SAMPLINGRATE_48000;
AsInitPlayerChannelNumberIndex	g_cur_ch_num		= AS_CHANNEL_STEREO;
AsInitPlayerBitLength		g_cur_bit_length	= AS_BITLENGTH_16;
AsInitPlayerCodecType		g_cur_codec_type	= AS_CODECTYPE_WAV;

extern "C" 
{
/*--------------------------------------------------------------------*/
void player_PlayMusic(int argc, const char *args[])
{
	AppMain::getInstance()->playMusic();
}

/*--------------------------------------------------------------------*/
void player_StopMusic(int argc, const char *args[])
{
	AppMain::getInstance()->stopMusic();
}

/*--------------------------------------------------------------------*/
void player_IncVol(int argc, const char *args[])
{
	AppMain::getInstance()->incVol();
}

/*--------------------------------------------------------------------*/
void player_DecVol(int argc, const char *args[])
{
	AppMain::getInstance()->decVol();
}

/*--------------------------------------------------------------------*/
void player_SetPlayerMode(int argc, const char *args[])
{
	AppMain::PlayerStatusParam param;
	param.input_device = g_input_device;
	param.output_device = g_output_device;
	AppMain::getInstance()->setPlayerMode(param);
}

/*--------------------------------------------------------------------*/
void player_InitPlayer(int argc, const char *args[])
{
	if (argc != CMD_PLAYER_INIT_PLAYER_ARGS_MIN) {
		DBG_ShellPrintf("Parameter error!\n");
		return;
	}

	uint32_t cur_index = CMD_ARG_START_INDEX;
	g_cur_codec_type = static_cast<AsInitPlayerCodecType>(atoi(args[cur_index++]));
	g_cur_bit_length = static_cast<AsInitPlayerBitLength>(atoi(args[cur_index++]));
	g_cur_ch_num = static_cast<AsInitPlayerChannelNumberIndex>(atoi(args[cur_index++]));
	g_cur_sampling_rate = static_cast<AsInitPlayerSamplingRateIndex>(atoi(args[cur_index]));

	AppMain::getInstance()->initPlayer(g_cur_codec_type, g_cur_bit_length, g_cur_ch_num, g_cur_sampling_rate);
}

/*--------------------------------------------------------------------*/
void player_SetOutputDevice(int argc, const char *args[])
{
	if (argc != CMD_PLAYER_SET_OUTPUT_DEVICE_ARGS_MIN) {
		DBG_ShellPrintf("Parameter error!\n");
		return;
	}
	g_output_device = static_cast<AsSetPlayerOutputDevice>(atoi(args[CMD_ARG_START_INDEX]));
}

/*--------------------------------------------------------------------*/
void player_SetInputDevice(int argc, const char *args[])
{
	if ((argc != CMD_PLAYER_SET_INPUT_DEVICE_ARGS_MIN)&&(argc != CMD_PLAYER_SET_INPUT_DEVICE_ARGS_MAX)) {
		DBG_ShellPrintf("Parameter error!\n");
		return;
	}
	g_input_device = static_cast<AsSetPlayerInputDevice>(atoi(args[CMD_ARG_START_INDEX]));
}

/*--------------------------------------------------------------------*/
void player_SetReadyStatus(int argc, const char *args[])
{
	AppMain::getInstance()->setReadyStatus();
}

/*--------------------------------------------------------------------*/
void player_NextTrack(int argc, const char *args[])
{
	AppMain::getInstance()->nextMusic();
}

/*--------------------------------------------------------------------*/
void player_PrevTrack(int argc, const char *args[])
{
	AppMain::getInstance()->prevMusic();
}

/*--------------------------------------------------------------------*/
void player_SetPlayerParam(int argc, const char *args[])
{
	AppMain::PlayerModeParam param;

	if (argc != CMD_PLAYER_PARAM_ARGS_MIN) {
		DBG_ShellPrintf("Parameter error!\n");
		return;
	}

	/* filter range check */
	if (strcmp(args[argc-3], "0") == 0 ||
	    strcmp(args[argc-3], "1") == 0 ||
	    strcmp(args[argc-3], "2") == 0) {
		param.play_filter = atoi(args[argc-3]);
	}
	else {
		DBG_ShellPrintf("Filter Parameter range error!\n");
		return;		
	}

	/* play mode range check */
	if (strcmp(args[argc-2], "0") == 0 ||
	    strcmp(args[argc-2], "1") == 0 ||
	    strcmp(args[argc-2], "2") == 0){
		param.play_mode = atoi(args[argc-2]);
	}
	else {
		DBG_ShellPrintf("PlayMode Parameter range error!\n");
		return;		
	}

	/* repeat mode range check */
	if (strcmp(args[argc-1], "0") == 0 ||
	    strcmp(args[argc-1], "1") == 0 ){
		param.repeat_mode = atoi(args[argc-1]);
	}
	else {
		DBG_ShellPrintf("RepeatMode Parameter range error!\n");
		return;		
	}

	AppMain::getInstance()->setPlayerParam(param);
}

/*--------------------------------------------------------------------*/
void player_SetPL(int argc, const char *args[])
{
	if (argc != 3) { 
		DBG_LOG_INFO("Please check command syntax again.\n");
		return;
	}

	AppMain::getInstance()->setPlaylistSource(atoi(args[argc-1]));
}

/*--------------------------------------------------------------------*/
void player_PowerOn(int argc, const char *args[])
{
	AppMain::getInstance()->powerOn(atoi(args[argc-1]));
}

/*--------------------------------------------------------------------*/
void player_SetPowerOffStatus(int argc, const char *args[])
{
	AppMain::getInstance()->setPowerOffStatus();
}

/*--------------------------------------------------------------------*/
void player_InitSoundEffect(int argc, const char *args[])
{
	AppMain::getInstance()->initSoundEffect();
}

/*--------------------------------------------------------------------*/
void player_StartEffect(int argc, const char *args[])
{
	AppMain::getInstance()->startSoundEffect();
}

/*--------------------------------------------------------------------*/
void player_StopSoundEffect(int argc, const char *args[])
{
	AppMain::getInstance()->stopSoundEffect();
}

/*--------------------------------------------------------------------*/
const DBG_ShellCommandTreeNode_t playerSubCmdTree[] = {
	{
		"SetPlayerMode",
		0,
		(DBG_ShellCommandTreeNode_t *)player_SetPlayerMode,
		"player SetPlayerMode",
		"[usage]\n  $ player SetPlayerMode",
		NULL
	},
	{
		"PlayMusic",
		0,
		(DBG_ShellCommandTreeNode_t *)player_PlayMusic,
		"player PlayMusic",
		"[usage]\n  $ player PlayMusic",
		NULL
	},
	{
		"StopMusic",
		0,
		(DBG_ShellCommandTreeNode_t *)player_StopMusic,
		"player StopMusic",
		"[usage]\n  $ player StopMusic",
		NULL
	},
	{
		"IncVol",
		0,
		(DBG_ShellCommandTreeNode_t *)player_IncVol,
		"increase digital volume [+0.5dB]",
		"[usage]\n  $ player IncVol",
		NULL
	},
	{
		"DecVol",
		0,
		(DBG_ShellCommandTreeNode_t *)player_DecVol,
		"decrease digital volume [-0.5dB]",
		"[usage]\n  $ player DecVol",
		NULL
	},
	{
		"SetInputDevice",
		0,
		(DBG_ShellCommandTreeNode_t *)player_SetInputDevice,
		"select input device (0: eMMC FileSystem, 1: A2DP Media Packet FiFo, 2: I2S Input, 3: RAM Input)", 
		"[usage]\n  $ player SetInputDevice {input device[0-3]}",
		NULL
	},
	{
		"SetOutputDevice",
		0,
		(DBG_ShellCommandTreeNode_t *)player_SetOutputDevice,
		"select output device (0: Cxd5247 SP/HP, 1: I2S Output, 2: A2DP Media Packet FiFo)",
		"[usage]\n  $ player SetOutputDevice {type}\n  ex) set type=0 for Cxd5247 SP/HP",
		NULL
	},
	{
		"InitPlayer",
		0,
		(DBG_ShellCommandTreeNode_t *)player_InitPlayer,
		"select: \n\tcodec type (0: MP3, 1: WAV),\n\tbit length (0: 16bit, 1: 24bit),\n\tch(0: mono, 1:stereo),\n\tsampling rate (0: 16kHz, 1: 32kHz, 2: 44.1kHz, 3: 48kHz)",
		"[usage]\n  $ player InitPlayer {codec type} {bit length} {ch} {sampling rate}",
		NULL
	},
	{
		"SetReadyStatus",
		0,
		(DBG_ShellCommandTreeNode_t *)player_SetReadyStatus,
		"Transit state from player state to ready state",
		"[usage]\n  $ player SetReadyStatus",
		NULL
	},
	{
		"NextTrack",
		0,
		(DBG_ShellCommandTreeNode_t *)player_NextTrack,
		"Play next track",
		"[usage]\n  $ player NextTrack",
		NULL
	},
	{
		"PrevTrack",
		0,
		(DBG_ShellCommandTreeNode_t *)player_PrevTrack,
		"Play previous track",
		"[usage]\n  $ player PrevTrack",
		NULL
	},
	{
		"SetPlayerParam",
		0,
		(DBG_ShellCommandTreeNode_t *)player_SetPlayerParam,
		"Set player param(filter type: 0 for files and directories playlist, 1 for album playlist, 2 for artist playlist; \
			play mode: 0 for Normal mode, 1 for Shuffle mode, 2 for play one song; \
			repeat on/off: 0 for disable repeat, 1 for enable repeat)",
		"[usage]\n  $ player SetPlayerparam {filter type} {play mode} {repeat on/off}",
		NULL
	},
	{
		"SetPL",
		0,
		(DBG_ShellCommandTreeNode_t *)player_SetPL,
		"Set playlist source source(0 for using internal playlist provied by SDK; 1 for using external playlist provided by SDK user.)",
		"[usage]\n  $ player SetPL source {playlist source}",
		NULL
	},
	{
		"PowerOn",
		0,
		(DBG_ShellCommandTreeNode_t *)player_PowerOn,
		"player PowerOn",
		"[usage]\n  $ player PowerOn {}",
		NULL
	},
	{
		"SetPowerOff",
		0,
		(DBG_ShellCommandTreeNode_t *)player_SetPowerOffStatus,
		"player SetPowerOff",
		"[usage]\n  $ player SetPowerOff",
		NULL
	},
	{
		"InitSoEffect",
		0,
		(DBG_ShellCommandTreeNode_t *)player_InitSoundEffect,
		"player InitSoEffect",
		"[usage]\n  $ player InitSoEffect",
		NULL
	},
	{
		"StartSoEffect",
		0,
		(DBG_ShellCommandTreeNode_t *)player_StartEffect,
		"player StartSoEffect",
		"[usage]\n  $ player StartSoEffect",
		NULL
	},
	{
		"StopSoEffect",
		0,
		(DBG_ShellCommandTreeNode_t *)player_StopSoundEffect,
		"player StoptSoEffect",
		"[usage]\n  $ player StopSoEffect",
		NULL
	},
};

/*--------------------------------------------------------------------*/
DBG_ShellCommandTreeNode_t playerCmdTree = {
    "player",
    sizeof(playerSubCmdTree) / sizeof(DBG_ShellCommandTreeNode_t),
	(DBG_ShellCommandTreeNode_t *)playerSubCmdTree,
    NULL,
    "This is specific help for category player\n",
    NULL
};

} /* extern "C" */
