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
#include <audio/high_level_api/as_high_level_api.h>

#include "app_main.h"

#include <debug/dbg_log.h>
#define DBG_MODULE DBG_MODULE_AS

#define CMD_PREFIX_ARGS_NUM	(2)	/* prefix + command type */
#define CMD_VOICE_RECORDER_SET_RECORDER_PARAM_ARGS_MIN		(3)
#define CMD_VOICE_RECORDER_INIT_RECORDER_PARAM_ARGS_MIN		(6)

uint8_t g_output_device  = AS_SETRECDR_STS_OUTPUTDEVICE_EMMC; /* default = eMMc */

extern "C" {

/*--------------------------------------------------------------------*/
void voice_recorder_StartRec(int argc, const char *args[], void *userData)
{
	AppMain::getInstance()->rec_start();
}

/*--------------------------------------------------------------------*/
void voice_recorder_StopRec(int argc, const char *args[], void *userData)
{
	AppMain::getInstance()->rec_stop();
}

/*--------------------------------------------------------------------*/
void voice_recorder_SetRecParam(int argc, const char *args[])
{
	if (argc != CMD_VOICE_RECORDER_SET_RECORDER_PARAM_ARGS_MIN) {
		DBG_LOG_INFO("Parameter error!\n");
		return;
	}

	AppMain::getInstance()->set_channelno(atoi(args[argc-1]));

}
/*--------------------------------------------------------------------*/
void voice_recorder_SetReadyStatus(int argc, const char *args[])
{
 	AppMain::getInstance()->set_samplingrate(DEFAULT_SAMPLING_RATE);
	AppMain::getInstance()->set_codectype(AS_CODECTYPE_MP3); /* default MP3 */
	AppMain::getInstance()->set_channelno(DEFAULT_CHANNEL_NO);

	AppMain::getInstance()->set_ready_status();
}

/*--------------------------------------------------------------------*/
void voice_recorder_SetRecorderMode(int argc, const char *args[])
{
 	/* Voice Recorder Ý’è‚Ì‰Šú‰» */
	AsSetRecorderStatusParam param;

	param.input_device = AS_SETRECDR_STS_INPUTDEVICE_MIC_A;
	param.input_device_handler = 0x00;

	if (argc == CMD_VOICE_RECORDER_SET_RECORDER_PARAM_ARGS_MIN - 1) { /* ƒpƒ‰ƒ[ƒ^È—ªŽž‚ÍeMMc */
			g_output_device = 0;
	} else {
		g_output_device = atoi(args[argc-1]);
		if (g_output_device == AS_SETRECDR_STS_OUTPUTDEVICE_RAM) {
			AppMain::getInstance()->set_codectype(AS_CODECTYPE_WAV); /* default WAV */
		}
	}

	param.output_device = g_output_device;
	param.output_device_handler = 0x00;

	AppMain::getInstance()->set_recorder_status(param);

	DBG_LOG_DEBUG("Voice Recorder WakeUp!\n");
}

/*--------------------------------------------------------------------*/
void voice_recorder_InitRecorder(int argc, const char *args[])
{
	uint8_t cur_arg_index = CMD_PREFIX_ARGS_NUM;
	AsInitRecorderParam param;	

	/* set sampling rate */
	if (strcmp(args[cur_arg_index], "16000") == 0) {
		param.sampling_rate = AS_SAMPLINGRATE_16000;
	}
	else if (strcmp(args[cur_arg_index], "48000") == 0 ) {
		param.sampling_rate = AS_SAMPLINGRATE_48000;
	}
	else if (strcmp(args[cur_arg_index], "96000") == 0 ) {
		param.sampling_rate = AS_SAMPLINGRATE_96000;
	}
	else if (strcmp(args[cur_arg_index], "192000") == 0 ) {
		param.sampling_rate = AS_SAMPLINGRATE_192000;
	}
	else {
		param.sampling_rate = atoi(args[cur_arg_index]);
	}
	
 	AppMain::getInstance()->set_samplingrate(atoi(args[cur_arg_index]));

	cur_arg_index++;

	/* set channel number */
	if (strcmp(args[cur_arg_index], "1") == 0) {
		param.channel_number = AS_CHANNEL_MONO;
	}
	else if (strcmp(args[cur_arg_index], "2") == 0) {
		param.channel_number = AS_CHANNEL_STEREO;
	}
	else if (strcmp(args[cur_arg_index], "4") == 0) {
		param.channel_number = AS_CHANNEL_4CH;
	}
	else if (strcmp(args[cur_arg_index], "6") == 0) {
		param.channel_number = AS_CHANNEL_6CH;
	}
	else if (strcmp(args[cur_arg_index], "8") == 0) {
		param.channel_number = AS_CHANNEL_8CH;
	}
	else {
		param.channel_number = atoi(args[cur_arg_index]);
	}

	AppMain::getInstance()->set_channelno(atoi(args[cur_arg_index]));
	
	cur_arg_index++;	

	/* set bit length */
	if (strcmp(args[cur_arg_index], "16") == 0) {
		param.bit_length = AS_BITLENGTH_16;
		AppMain::getInstance()->set_bitlength(AS_BITLENGTH_16);
	}
	else if (strcmp(args[cur_arg_index], "24") == 0) {
		param.bit_length = AS_BITLENGTH_24;
		AppMain::getInstance()->set_bitlength(AS_BITLENGTH_24);
	}
	else {
		param.bit_length = atoi(args[cur_arg_index]);
		AppMain::getInstance()->set_bitlength(atoi(args[cur_arg_index]));
	}

	cur_arg_index++;

	/* set codec type */
	if (strcmp(args[cur_arg_index], "MP3") == 0) {
			param.codec_type = AS_CODECTYPE_MP3;
			AppMain::getInstance()->set_codectype(AS_CODECTYPE_MP3);
		}
	else if (strcmp(args[cur_arg_index], "WAV") == 0) {
		param.codec_type = AS_CODECTYPE_WAV;
		AppMain::getInstance()->set_codectype(AS_CODECTYPE_WAV); 
	}
	else {
		param.codec_type = atoi(args[cur_arg_index]);
		AppMain::getInstance()->set_codectype(atoi(args[cur_arg_index])); 
	}

	AppMain::getInstance()->init_recorder(param);	
}

/*--------------------------------------------------------------------*/
void voice_recorder_PowerOn(int argc, const char *args[])
{
	AppMain::getInstance()->powerOn();
}

/*--------------------------------------------------------------------*/
void voice_recorder_SetPowerOffStatus(int argc, const char *args[])
{
	AppMain::getInstance()->setPowerOffStatus();
}

/*--------------------------------------------------------------------*/
const DBG_ShellCommandTreeNode_t voiceRecorderSubCmdTree[] = {
	{
		"SetRecParam",
		0,
		(DBG_ShellCommandTreeNode_t *)voice_recorder_SetRecParam,
		"output_channel_number\n   - output_channel_number:1,2,4\n",
		"[usage]\n  $ voice_recorder SetRecParam output_channel_number ex) $ voice_recorder SetRecorderParam 2",
		NULL
	},
	{
		"SetRecorderMode",
		0,
		(DBG_ShellCommandTreeNode_t *)voice_recorder_SetRecorderMode,
		"Set VoiceRecorderMode State select output device (0: eMMc, 1: RAM) If you omit output device is eMMc",
		"[usage]\n  $ voice_recorder SetRecorderMode outputDevice (0:eMMc(If you omit),1:RAM)",
		NULL
	},
	{
		"InitRecorder",
		0,
		(DBG_ShellCommandTreeNode_t *)voice_recorder_InitRecorder,
		"Initialize voice recorder output_sampling_rate(16000 or 48000) input_channel_number(1 or 2 or 4) input_pcm_bit_length(16 or 24) codec_type(MP3 or WAV)",
		"[usage]\n  $ voice_recorder InitRecorder output_sampling_rate input_channel_number input_pcm_bit_length codec_type\n ex.) $voice_recorder InitRecorder 16000 2 16 WAV",
		NULL
	},
	{
		"StartRec",
		0,
		(DBG_ShellCommandTreeNode_t *)voice_recorder_StartRec,
		"voice_recorder StartRec",
		"[usage]\n  $ voice_recorder StartRec",
		NULL
	},
	{
		"StopRec",
		0,
		(DBG_ShellCommandTreeNode_t *)voice_recorder_StopRec,
		"voice_recorder StopRec",
		"[usage]\n  $ voice_recorder StopRec",
		NULL
	},
	{
		"SetReadyStatus",
		0,
		(DBG_ShellCommandTreeNode_t *)voice_recorder_SetReadyStatus,
		"Transit state from voice recorder state to ready state",
		"[usage]\n  $ voice_recorder SetReadyStatus",
		NULL
	},
	{
		"PowerOn",
		0,
		(DBG_ShellCommandTreeNode_t *)voice_recorder_PowerOn,
		"voice_recorder PowerOn",
		"[usage]\n  $ voice_recorder PowerOn",
		NULL
	},
	{
		"SetPowerOff",
		0,
		(DBG_ShellCommandTreeNode_t *)voice_recorder_SetPowerOffStatus,
		"voice_recorder SetPowerOff",
		"[usage]\n  $ voice_recorder SetPowerOff",
		NULL
	},
};

DBG_ShellCommandTreeNode_t voiceRecorderCmdTree = {
    "voice_recorder",
    sizeof(voiceRecorderSubCmdTree) / sizeof(DBG_ShellCommandTreeNode_t),
	(DBG_ShellCommandTreeNode_t *)voiceRecorderSubCmdTree,
    NULL,
    "This is specific help for category voice_recorder\n",
    NULL
};

} /* extern "C" */
