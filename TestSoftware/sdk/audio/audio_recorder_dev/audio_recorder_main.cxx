/****************************************************************************
 * test/audio_recorder/audio_recorder_main.cxx
 *
 *   Copyright (C) 2017 Sony Corporation. All rights reserved.
 *   Author: Tomonobu Hayakawa<Tomonobu.Hayakawa@sony.com>
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

#include <sdk/config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <math.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "system/readline.h"
#include <sys/stat.h>
#include <asmp/mpshm.h>
#include "memutils/simple_fifo/CMN_SimpleFifo.h"
#include "memutils/os_utils/chateau_osal.h"

#include <arch/chip/cxd56_audio.h>

#include "audio/audio_high_level_api.h"
#include "memutils/memory_manager/MemHandle.h"
#include "memutils/message/Message.h"
#include "msgq_id.h"
#include "mem_layout.h"
#include "memory_layout.h"
#include "msgq_pool.h"
#include "pool_layout.h"
#include "fixed_fence.h"

#if !defined(CONFIG_SDK_AUDIO) || !defined(CONFIG_AUDIOUTILS_RECORDER)
#error "Configs [SDK audio] and [Audio Recorder] are required."
#endif

using namespace MemMgrLite;

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define MP3

#define MY_TIMER_SIGNAL 17
#define SIGVALUE_INT  42
#define MAX_PATH_LENGTH   128
#define READ_SIMPLE_FIFO_SIZE	12288
#define SIMPLE_FIFO_FRAME_NUM	10
#define SIMPLE_FIFO_BUF_SIZE	(READ_SIMPLE_FIFO_SIZE * SIMPLE_FIFO_FRAME_NUM)

#define CHUNKID_RIFF		("RIFF")
#define FORMAT_WAVE			("WAVE")
#define SUBCHUNKID_FMT		("fmt ")
#define SUBCHUNKID_DATA		("data")
#define AUDIO_FORMAT_PCM	(0x0001)
#define FMT_SIZE			(0x10)

#ifndef CONFIG_TEST_AUDIO_RECORDER_FILE_MOUNTPT
#  define CONFIG_TEST_AUDIO_RECORDER_FILE_MOUNTPT "/mnt/sd0/REC"
#endif

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Type Declarations
 ****************************************************************************/
typedef struct {
	uint8_t		riff[4];		/* "RIFF" */
	uint32_t	total_size;
	uint8_t		wave[4];		/* "WAVE" */
	uint8_t		fmt[4];			/* "fmt " */
	uint32_t	fmt_size;		/* fmt chunk size */
	uint16_t	format;			/* format type */
	uint16_t	channel;
	uint32_t	rate;			/* sampling rate */
	uint32_t	avgbyte;		/* rate * block */
	uint16_t	block;			/* channels * bit / 8 */
	uint16_t	bit;			/* bit length */
	uint8_t		data[4];		/* "data" */
	uint32_t	data_size;
} WAVEFORMAT;


/****************************************************************************
 * Private Data
 ****************************************************************************/
static uint32_t m_simple_fifo_buf[SIMPLE_FIFO_BUF_SIZE/sizeof(uint32_t)];
static uint8_t m_ram_buf[READ_SIMPLE_FIFO_SIZE];

static CMN_SimpleFifoHandle m_simple_fifo_handle;
static AsRecorderOutputDeviceHdlr m_output_device_handler;

static FILE  *m_fd;
static uint8_t m_codec_type;

static WAVEFORMAT m_wav_format;

static uint32_t m_output_data_size;
static mpshm_t s_shm;

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static void outputDeviceCallback(uint32_t size)
{
    /* do nothing */
}

/****************************************************************************/
static bool write_wav_header(bool type)
{
	ssize_t ret;
	if (!type) {
		fseek(m_fd, 0, SEEK_SET);
		m_wav_format.total_size = m_output_data_size + sizeof(WAVEFORMAT) - 8;
		m_wav_format.data_size = m_output_data_size;
	}
	ret = fwrite(&m_wav_format, 1, sizeof(WAVEFORMAT), m_fd);
	if (ret < 0) {
		printf("Fail to write file(wav header)\n");
		return false;
	}
	return true;
}

/****************************************************************************/
static bool open_output_file(void)
{
	static char fname[MAX_PATH_LENGTH];

	struct tm *cur_time;
	struct timespec cur_sec;

	clock_gettime(CLOCK_REALTIME, &cur_sec);
	cur_time = gmtime(&cur_sec.tv_sec);
	if (m_codec_type == AS_CODECTYPE_MP3) {
		snprintf(fname, MAX_PATH_LENGTH, "%s/%04d%02d%02d_%02d%02d%02d.mp3", CONFIG_TEST_AUDIO_RECORDER_FILE_MOUNTPT, cur_time->tm_year, cur_time->tm_mon, cur_time->tm_mday, cur_time->tm_hour, cur_time->tm_min, cur_time->tm_sec);
	}
	else if (m_codec_type == AS_CODECTYPE_LPCM) {
		snprintf(fname, MAX_PATH_LENGTH, "%s/%04d%02d%02d_%02d%02d%02d.wav", CONFIG_TEST_AUDIO_RECORDER_FILE_MOUNTPT, cur_time->tm_year, cur_time->tm_mon, cur_time->tm_mday, cur_time->tm_hour, cur_time->tm_min, cur_time->tm_sec);
	}
	else {
		snprintf(fname, MAX_PATH_LENGTH, "%s/%04d%02d%02d_%02d%02d%02d.opus", CONFIG_TEST_AUDIO_RECORDER_FILE_MOUNTPT, cur_time->tm_year, cur_time->tm_mon, cur_time->tm_mday, cur_time->tm_hour, cur_time->tm_min, cur_time->tm_sec);
	}
	m_fd = fopen(fname, "w");
	if (m_fd == 0) {
		printf("open ERROR!!!!!(%s)\n", fname);
		return false;
	}
	printf("Record data to %s.\n", &fname[0]);

	if (m_codec_type == AS_CODECTYPE_LPCM) {
		if (!write_wav_header(true)) {
			return false;
		}
	}
	return true;
}

/****************************************************************************/
static void close_output_file(void)
{
	fclose(m_fd);
}

/****************************************************************************/
static void write_output_file(uint32_t size)
{
	ssize_t ret;

	if (size == 0 || CMN_SimpleFifoGetOccupiedSize(&m_simple_fifo_handle) == 0) {
		return;
	}

	if (CMN_SimpleFifoPoll(&m_simple_fifo_handle, (void*)m_ram_buf, size) == 0) {
		printf("ERROR: Fail to get data from simple FIFO.\n");
		return;
	}
	ret = fwrite(m_ram_buf, 1, size, m_fd);
	if (ret < 0) {
		printf("ERROR: Cannot write recorded data to output file.\n");
		close_output_file();
		return;
	}
	m_output_data_size += size;
}

/****************************************************************************/
static bool init_simple_fifo(void)
{
	if (CMN_SimpleFifoInitialize(&m_simple_fifo_handle, m_simple_fifo_buf, SIMPLE_FIFO_BUF_SIZE, NULL) != 0) {
		printf("Fail to initialize simple FIFO.");
		return false;
	}
	CMN_SimpleFifoClear(&m_simple_fifo_handle);

	m_output_device_handler.simple_fifo_handler = (void*)(&m_simple_fifo_handle);
	m_output_device_handler.callback_function = outputDeviceCallback;

	return true;
}

/****************************************************************************/
static bool pop_simple_fifo(void)
{
	size_t occupied_simple_fifo_size = CMN_SimpleFifoGetOccupiedSize(&m_simple_fifo_handle);
	uint32_t output_size = 0;

	while (occupied_simple_fifo_size > 0) {
		output_size = (occupied_simple_fifo_size > READ_SIMPLE_FIFO_SIZE) ? READ_SIMPLE_FIFO_SIZE : occupied_simple_fifo_size;
		write_output_file(output_size);
		occupied_simple_fifo_size -= output_size;
	}

	return true;
}

/****************************************************************************/
static bool power_on(void)
{
	AudioCommand command;
	command.header.packet_length = LENGTH_POWERON;
	command.header.command_code  = AUDCMD_POWERON;
	command.header.sub_code      = 0x00;
	command.power_on_param.enable_sound_effect = AS_DISABLE_SOUNDEFFECT;
	AS_SendAudioCommand(&command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	if (result.header.result_code != AUDRLT_STATUSCHANGED) {
		printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
				command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
		return false;
	}
	return true;
}

/****************************************************************************/
static bool power_off(void)
{
	AudioCommand command;
	command.header.packet_length = LENGTH_SET_POWEROFF_STATUS;
	command.header.command_code  = AUDCMD_SETPOWEROFFSTATUS;
	command.header.sub_code      = 0x00;
	AS_SendAudioCommand(&command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	if (result.header.result_code != AUDRLT_STATUSCHANGED) {
		printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
				command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
		return false;
	}
	return true;
}

/****************************************************************************/
static bool set_ready_status(void)
{
	AudioCommand command;
	command.header.packet_length = LENGTH_SET_READY_STATUS;
	command.header.command_code  = AUDCMD_SETREADYSTATUS;
	command.header.sub_code      = 0x00;
	AS_SendAudioCommand(&command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	if (result.header.result_code != AUDRLT_STATUSCHANGED) {
		printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
				command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
		return false;
	}
	return true;
}

/****************************************************************************/
static bool init_mic_gain_analog(void)
{
	AudioCommand command;

	command.header.packet_length = LENGTH_INITMICGAIN;
	command.header.command_code  = AUDCMD_INITMICGAIN;
	command.header.sub_code      = 0;
	command.init_mic_gain_param.mic_gain[0] = 210;
	command.init_mic_gain_param.mic_gain[1] = 210;
	command.init_mic_gain_param.mic_gain[2] = 210;
	command.init_mic_gain_param.mic_gain[3] = 210;
	command.init_mic_gain_param.mic_gain[4] = AS_MICGAIN_HOLD;
	command.init_mic_gain_param.mic_gain[5] = AS_MICGAIN_HOLD;
	command.init_mic_gain_param.mic_gain[6] = AS_MICGAIN_HOLD;
	command.init_mic_gain_param.mic_gain[7] = AS_MICGAIN_HOLD;

	AS_SendAudioCommand( &command );

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	if (result.header.result_code != AUDRLT_INITMICGAINCMPLT) {
		printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
				command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
		return false;
	}
	return true;
}

/****************************************************************************/
static bool init_mic_gain_digital(void)
{
	AudioCommand command;

	command.header.packet_length = LENGTH_INITMICGAIN;
	command.header.command_code  = AUDCMD_INITMICGAIN;
	command.header.sub_code      = 0;
	command.init_mic_gain_param.mic_gain[0] = 0;
	command.init_mic_gain_param.mic_gain[1] = 0;
	command.init_mic_gain_param.mic_gain[2] = 0;
	command.init_mic_gain_param.mic_gain[3] = 0;
	command.init_mic_gain_param.mic_gain[4] = 0;
	command.init_mic_gain_param.mic_gain[5] = 0;
	command.init_mic_gain_param.mic_gain[6] = 0;
	command.init_mic_gain_param.mic_gain[7] = 0;

	AS_SendAudioCommand( &command );

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	if (result.header.result_code != AUDRLT_INITMICGAINCMPLT) {
		printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
				command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
		return false;
	}
	return true;
}

/****************************************************************************/
static bool set_recorder_status(uint8_t input_device)
{
	AudioCommand command;
	command.header.packet_length = LENGTH_SET_RECORDER_STATUS;
	command.header.command_code = AUDCMD_SETRECORDERSTATUS;
	command.header.sub_code = 0x00;
	command.set_recorder_status_param.input_device = input_device;
 	command.set_recorder_status_param.input_device_handler = 0x00;
 	command.set_recorder_status_param.output_device = AS_SETRECDR_STS_OUTPUTDEVICE_RAM;
	command.set_recorder_status_param.output_device_handler = &m_output_device_handler;
	AS_SendAudioCommand(&command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	if (result.header.result_code != AUDRLT_STATUSCHANGED) {
		printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
				command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
		return false;
	}
	return true;
}

/****************************************************************************/
static bool init_recorder_wav(AudioCommand* command, uint32_t sampling_rate, uint8_t channel_number)
{
	command->recorder.init_param.sampling_rate = sampling_rate;
	command->recorder.init_param.channel_number = channel_number;
	command->recorder.init_param.bit_length = AS_BITLENGTH_16;
	command->recorder.init_param.codec_type = m_codec_type;
	AS_SendAudioCommand(command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	if (result.header.result_code != AUDRLT_INITRECCMPLT) {
		printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x) Error subcode(0x%x)\n",
				command->header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code, result.error_response_param.error_sub_code);
		return false;
	}

	memcpy(m_wav_format.riff, CHUNKID_RIFF, strlen(CHUNKID_RIFF));
	m_wav_format.total_size = 0;
	memcpy(m_wav_format.wave, FORMAT_WAVE, strlen(FORMAT_WAVE));
	memcpy(m_wav_format.fmt, SUBCHUNKID_FMT, strlen(SUBCHUNKID_FMT));
	m_wav_format.fmt_size = FMT_SIZE;
	m_wav_format.format = AUDIO_FORMAT_PCM;
	m_wav_format.channel = channel_number;
	m_wav_format.rate = sampling_rate;
	m_wav_format.avgbyte = sampling_rate * channel_number * 2;
	m_wav_format.block = channel_number * 2;
	m_wav_format.bit = 2 * 8;
	memcpy(m_wav_format.data, SUBCHUNKID_DATA, strlen(SUBCHUNKID_DATA));
	m_wav_format.data_size = 0;
	return true;
}

/****************************************************************************/
static bool init_recorder_mp3(AudioCommand* command, uint32_t sampling_rate, uint8_t channel_number)
{
	command->recorder.init_param.sampling_rate = sampling_rate;
	command->recorder.init_param.channel_number = channel_number;
	command->recorder.init_param.bit_length = AS_BITLENGTH_16;
	command->recorder.init_param.codec_type = m_codec_type;
	command->recorder.init_param.bitrate = AS_BITRATE_96000;
	AS_SendAudioCommand(command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	if (result.header.result_code != AUDRLT_INITRECCMPLT) {
		printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x) Error subcode(0x%x)\n",
				command->header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code, result.error_response_param.error_sub_code);
		return false;
	}
	return true;
}

/****************************************************************************/
static bool init_recorder_opus(AudioCommand* command, uint32_t sampling_rate, uint8_t channel_number)
{
	command->recorder.init_param.sampling_rate = sampling_rate;
	command->recorder.init_param.channel_number = channel_number;
	command->recorder.init_param.bit_length = AS_BITLENGTH_16;
	command->recorder.init_param.codec_type = m_codec_type;
	command->recorder.init_param.bitrate = AS_BITRATE_8000;
	command->recorder.init_param.computational_complexity = AS_INITREC_COMPLEXITY_0;
	AS_SendAudioCommand(command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	if (result.header.result_code != AUDRLT_INITRECCMPLT) {
		printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x) Error subcode(0x%x)\n",
				command->header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code, result.error_response_param.error_sub_code);
		return false;
	}
	return true;
}

/****************************************************************************/
static bool init_recorder(uint8_t codec_type, uint32_t sampling_rate, uint8_t channel_number)
{
	m_codec_type = codec_type;
	AudioCommand command;
	command.header.packet_length = LENGTH_INIT_RECORDER;
	command.header.command_code = AUDCMD_INITREC;
	command.header.sub_code = 0x00;
    snprintf(command.recorder.init_param.dsp_path, AS_AUDIO_DSP_PATH_LEN, "%s", "/mnt/sd0/BIN");

	bool ret = false;
	switch (codec_type) {
	case AS_CODECTYPE_LPCM:
		ret = init_recorder_wav(&command, sampling_rate, channel_number);
		break;
	case AS_CODECTYPE_MP3:
		ret = init_recorder_mp3(&command, sampling_rate, channel_number);
		break;
	case AS_CODECTYPE_OPUS:
		ret = init_recorder_opus(&command, sampling_rate, channel_number);
		break;
	default:
		break;
	}
	return ret;
}

/****************************************************************************/
static bool start_recorder(void)
{
	m_output_data_size = 0;
	if (!open_output_file()) {
		return false;
	}
	CMN_SimpleFifoClear(&m_simple_fifo_handle);

	AudioCommand command;
	command.header.packet_length = LENGTH_START_RECORDER;
	command.header.command_code = AUDCMD_STARTREC;
	command.header.sub_code = 0x00;

	AS_SendAudioCommand(&command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	if (result.header.result_code != AUDRLT_RECCMPLT) {
		printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x) Error subcode(0x%x)\n",
				command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code, result.error_response_param.error_sub_code);
		return false;
	}
	return true;
}

/****************************************************************************/
static bool stop_recorder(void)
{
	AudioCommand command;
	command.header.packet_length = LENGTH_STOP_RECORDER;
	command.header.command_code = AUDCMD_STOPREC;
	command.header.sub_code = 0x00;

	AS_SendAudioCommand(&command);

	AudioResult result;
	AS_ReceiveAudioResult(&result);
	if (result.header.result_code != AUDRLT_STOPRECCMPLT) {
		printf("ERROR: Command (0x%x) fails. Result code(0x%x) Module id(0x%x) Error code(0x%x)\n",
				command.header.command_code, result.header.result_code, result.error_response_param.module_id, result.error_response_param.error_code);
		return false;
	}

	size_t occupied_simple_fifo_size = CMN_SimpleFifoGetOccupiedSize(&m_simple_fifo_handle);
	uint32_t output_size = 0;

	while (occupied_simple_fifo_size > 0) {
		output_size = (occupied_simple_fifo_size > READ_SIMPLE_FIFO_SIZE) ? READ_SIMPLE_FIFO_SIZE : occupied_simple_fifo_size;
		write_output_file(output_size);
		occupied_simple_fifo_size = CMN_SimpleFifoGetOccupiedSize(&m_simple_fifo_handle);
	}

	if (m_codec_type == AS_CODECTYPE_LPCM) {
		write_wav_header(false);
	}

	close_output_file();
	return true;
}

/****************************************************************************/
static void app_attention_callback(const ErrorAttentionParam *attparam)
{
  printf("Attention!! %s L%d ecode %d subcode %d\n",
          attparam->error_filename,
          attparam->line_number,
          attparam->error_code,
          attparam->error_att_sub_code);
}

/****************************************************************************
 * Public Type Declarations
 ****************************************************************************/
typedef enum {
	ReadyState = 0,
	RecStartState,
	RecState,
	RecStoppedState
} RecorderState;

typedef struct
{
	uint32_t	state;
	uint32_t	sampling_rate;
	uint8_t		channel_number;
	uint8_t		codec_type;
	DIR			*dirp;
} AudioRecorder;

typedef int (*recorder_func)(char* pargs);

struct RecorderCmd {
	const char      *cmd;       /* The command text */
	const char      *arghelp;   /* Text describing the args */
	recorder_func   pFunc;     /* Pointer to command handler */
	const char      *help;      /* The help text */
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
static bool audioRecorder_createContext(AudioRecorder** recorder);
static bool audioRecorder_powerOn(void);
static int  audioRecorder_powerOff(char* parg);
static int  audioRecorder_start(char* parg);
static int  audioRecorder_stop(char* parg);
static int  audioRecorder_setRecInfo(char* parg);
static bool audioRecorder_createDir(void);
static int  audioRecorder_cmdHelp(char* parg);
static int  audioRecorder_inDevice(char* parg);

/****************************************************************************
 * Public Data
 ****************************************************************************/
static  AudioRecorder *p_recorder = NULL;

static struct RecorderCmd g_recorder_cmds[] =
{
	{ "h",					"",					audioRecorder_cmdHelp, "Display help for commands.   ex)recorder> h" },
	{ "help",				"",					audioRecorder_cmdHelp, "Display help for commands.   ex)recorder> help" },
	{ "inDevice",			"device",	        audioRecorder_inDevice,"Used input device.           ex)recorder> inDevice AMIC\n                                                      prarameter: device[AMIC or I2S]\n                                                        caution) First run required after starting recorder application" },
	{ "setRecInfo",			"codeType,samplingRate,channelNumber",	audioRecorder_setRecInfo, "Set recording information.   ex)recorder> setRecInfo mp3,16,2\n                                                      prarameter: codecType[mp3],samplingRate[16 or 48],channelNumber[1 or 2]\n                                                      prarameter: codecType[lpcm],samplingRate[16 or 48],channelNumber[1 or 2]\n                                                      prarameter: codecType[opus],samplingRate[8],channelNumber[1 or 2]\n                                                        caution) Execution is necessary before rec command. However, it can be omitted and in that case it will be [mp3,16,2]" },
	{ "rec",				"",					audioRecorder_start,"Start to record.            ex)recorder> rec" },
	{ "stop",				"",					audioRecorder_stop, "Stop recording.             ex)recorder> stop\n                                                        caution) It will not be saved unless you execute the stop command" },
	{ "q",					"",         		audioRecorder_powerOff,"Exit recorder application.  ex)recorder> q" },
	{ "quit",				"",         		audioRecorder_powerOff,"Exit recorder application.  ex)recorder> quit" }
};
static const int g_recorder_cmd_count = sizeof(g_recorder_cmds) / sizeof(struct RecorderCmd);

/****************************************************************************
 * Public Functions
 ****************************************************************************/
static int audioRecorder_cmdHelp(char* parg)
{
	int   x, len, maxlen = 0;
	int   c;

	for (x = 0; x < g_recorder_cmd_count; x++) {
		len = strlen(g_recorder_cmds[x].cmd) + strlen(g_recorder_cmds[x].arghelp);
		if (len > maxlen) {
			maxlen = len;
		}
	}

	printf("AudioRecorder commands\n===Commands=+=Prarameters=========================+=Description=====================\n");
	for (x = 0; x < g_recorder_cmd_count; x++) {
		printf("  %s %s", g_recorder_cmds[x].cmd, g_recorder_cmds[x].arghelp);
		len = maxlen - (strlen(g_recorder_cmds[x].cmd) + strlen(g_recorder_cmds[x].arghelp));
		for (c = 0; c < len; c++) {
			printf(" ");
		}
		printf("  : %s\n", g_recorder_cmds[x].help);
	}
	return 0;
}

/****************************************************************************/
static bool audioRecorder_createContext(AudioRecorder** recorder)
{
	*recorder = (AudioRecorder *)malloc(sizeof(AudioRecorder));
	if (*recorder == NULL) {
		printf("recorder context creation failed.\n");
		return false;
	}
	memset(*recorder, 0 , sizeof(AudioRecorder));

	return true;
}

/****************************************************************************/
static bool audioRecorder_powerOn(void)
{
	/* Power On */
	if (!power_on()) {
		return false;
	}
	return true;
}

/****************************************************************************/
static int audioRecorder_powerOff(char* parg)
{
	if (p_recorder->state == RecState) {
		if (!stop_recorder()) {
			return 1;
		}
		p_recorder->state = ReadyState;
	}

	/* Set Ready Status */
	if (!set_ready_status()) {
		return 1;
	}

	/* Power Off */
	if (!power_off()) {
		return 1;
	}

	closedir(p_recorder->dirp);

	return 0;
}

/****************************************************************************/
static int audioRecorder_start(char* parg)
{
	p_recorder->state = RecStartState;
	return 0;
}

/****************************************************************************/
static int audioRecorder_stop(char* parg)
{
	p_recorder->state = RecStoppedState;
	return 0;
}

/****************************************************************************/
static int audioRecorder_inDevice(char* parg)
{
	uint8_t input_device;
	char *adr;
	adr = strtok(parg, ",");
	if ((strncmp("amic", adr, 4) == 0) || (strncmp("AMIC", adr, 4) == 0)) {
		input_device = AS_SETRECDR_STS_INPUTDEVICE_MIC_A;
	}
	else if ((strncmp("dmic", adr, 4) == 0) || (strncmp("DMIC", adr, 4) == 0)) {
		input_device = AS_SETRECDR_STS_INPUTDEVICE_MIC_D;
	}
	else if ((strncmp("i2s", adr, 3) == 0) || (strncmp("I2S", adr, 3) == 0)) {
		input_device = AS_SETRECDR_STS_INPUTDEVICE_I2S_IN;
	}
	else {
		printf("Invalid input device(%s)\n", adr);
		return 1;
	}

	/* Init Simple Fifo */
	if (!init_simple_fifo()) {
		return 1;
	}

	if (input_device == AS_SETRECDR_STS_INPUTDEVICE_MIC_A) {
		/* Init Mic Gain(Analog) */
		if (!init_mic_gain_analog()) {
			return 1;
		}
	}
	if (input_device == AS_SETRECDR_STS_INPUTDEVICE_MIC_D) {
		/* Init Mic Gain(Digital) */
		if (!init_mic_gain_digital()) {
			return 1;
		}
	}

	/* Set Recorder Status */
	if (!set_recorder_status(input_device)) {
		return 1;
	}

	return 0;
}

/****************************************************************************/
static int audioRecorder_setRecInfo(char* parg)
{
	char *adr;
	adr = strtok(parg, ",");
	if ((strncmp("mp3", adr, 3) == 0) || (strncmp("MP3", adr, 3) == 0)) {
		p_recorder->codec_type = AS_CODECTYPE_MP3;
	}
	else if ((strncmp("lpcm", adr, 4) == 0) || (strncmp("LPCM", adr, 4) == 0)) {
		p_recorder->codec_type = AS_CODECTYPE_LPCM;
	}
	else if ((strncmp("opus", adr, 4) == 0) || (strncmp("OPUS", adr, 4) == 0)) {
		p_recorder->codec_type = AS_CODECTYPE_OPUS;
	}
	else {
		printf("Invalid codec type(%s)\n", adr);
		return 1;
	}

	adr = strtok(NULL, ",");
	int rate;
	rate = atoi(adr);
	switch(rate) {
	case 8:
		p_recorder->sampling_rate = AS_SAMPLINGRATE_8000;
		break;
	case 16:
		p_recorder->sampling_rate = AS_SAMPLINGRATE_16000;
		break;
	case 48:
		p_recorder->sampling_rate = AS_SAMPLINGRATE_48000;
		break;
	default:
		printf("Invalid sampling rate(%s)\n",adr);
		return 1;
	}

	adr = strtok(NULL, ",");
	rate = atoi(adr);
	switch(rate) {
	case 1:
		p_recorder->channel_number = AS_CHANNEL_MONO;
		break;
	case 2:
		p_recorder->channel_number = AS_CHANNEL_STEREO;
		break;
	case 4:
		p_recorder->channel_number = AS_CHANNEL_4CH;
		break;
	case 6:
		p_recorder->channel_number = AS_CHANNEL_6CH;
		break;
	case 8:
		p_recorder->channel_number = AS_CHANNEL_8CH;
		break;
	default:
		printf("Invalid sampling rate(%s)\n",adr);
		return 1;
	}

	return 0;
}

/****************************************************************************/
static bool audioRecorder_createDir(void)
{
	DIR *dirp;
	int ret;
	const char *name = CONFIG_TEST_AUDIO_RECORDER_FILE_MOUNTPT;

	dirp = opendir("/mnt");
	if(!dirp){
		printf("opendir err(errno:%d)\n",errno);
		return false;
	}
	ret = mkdir(name,0777);
	if(ret != 0){
		if(errno != EEXIST){
			printf("mkdir err(errno:%d)\n",errno);
			return false;
		}
	}
	p_recorder->dirp = dirp;
	return true;
}

/****************************************************************************
 * Name: recorder_daemon
 ****************************************************************************/
static int recorder_loop(int argc,  const char* argv[])
{
	while(1) {
		usleep(1000);

		if (p_recorder->state == RecStartState) {
			/* Init Recorder */
			if (init_recorder(p_recorder->codec_type, p_recorder->sampling_rate, p_recorder->channel_number)) {
				/* Start Recorder */
				if (start_recorder()) {
					p_recorder->state = RecState;
				}
				else {
					p_recorder->state = ReadyState;
				}
			}
			else {
				p_recorder->state = ReadyState;
			}
		}

		if (p_recorder->state == RecStoppedState) {
			if (stop_recorder()) {
				p_recorder->state = ReadyState;
			}
		}
		/* Pop data to simple fifo */
		pop_simple_fifo();
	}
	return 0;
}

/****************************************************************************/
static bool init_libraries(void)
{
  int ret;
  uint32_t addr = AUD_SRAM_ADDR;

  ret = mpshm_init(&s_shm, 1, 1024 * 128 * 2);
  if (ret < 0)
    {
      printf("mpshm_init() failure. %d\n", ret);
      return false;
    }

  ret = mpshm_remap(&s_shm, (void *)addr);
  if (ret < 0)
    {
      printf("mpshm_remap() failure. %d\n", ret);
      return false;
    }

   /* Initalize MessageLib */

  MsgLib::initFirst(NUM_MSGQ_POOLS, MSGQ_TOP_DRM);
  MsgLib::initPerCpu();

  void* mml_data_area = translatePoolAddrToVa(MEMMGR_DATA_AREA_ADDR);
  Manager::initFirst(mml_data_area, MEMMGR_DATA_AREA_SIZE);
  Manager::initPerCpu(mml_data_area, NUM_MEM_POOLS);

  /* Create static memory pool of VoiceCall */

  const NumLayout layout_no = MEM_LAYOUT_RECORDER;
  void* work_va = translatePoolAddrToVa(MEMMGR_WORK_AREA_ADDR);
  Manager::createStaticPools(layout_no, work_va, MEMMGR_MAX_WORK_SIZE, MemoryPoolLayouts[layout_no]);
  return true;
}

/****************************************************************************/
static void fin_libraries(void)
{
  /* Finalize MessageLib. */

  MsgLib::finalize();

  /* destroy static memory pool */

  Manager::destroyStaticPools();

  /* Finalize memory manager. */

  MemMgrLite::Manager::finalize();

  /* Destroy MP shared memory. */
  int ret;
  ret = mpshm_detach(&s_shm);
  if (ret < 0)
    {
      printf("mpshm_detach() failure. %d\n", ret);
    }

  ret = mpshm_destroy(&s_shm);
  if (ret < 0)
    {
      printf("mpshm_destroy() failure. %d\n", ret);
    }
}

/****************************************************************************
 * audio_main
 ****************************************************************************/
#ifdef CONFIG_BUILD_KERNEL
extern "C" int main(int argc, FAR char *argv[])
#else
extern "C" int recorder_main(int argc, char *argv[])
#endif
{
	int		len, x, running;
	char	buffer[64];
	char	*cmd, *arg;
    pid_t rec_pid;

	if (!audioRecorder_createContext(&p_recorder)) {
		return 1;
	}
	p_recorder->codec_type = AS_CODECTYPE_MP3;
	p_recorder->sampling_rate = AS_SAMPLINGRATE_16000;
	p_recorder->channel_number = AS_CHANNEL_STEREO;

	if (!audioRecorder_createDir()) {
		return 1;
	}

	if (!init_libraries()) {
		return 1;
	}

  /* create task of AudioSubSystem */

  AudioSubSystemIDs ids;
  ids.app = MSGQ_AUD_APP;
  ids.mng = MSGQ_AUD_MGR;
  ids.player_main = 0xFF;
  ids.player_sub = 0xFF;
  ids.mixer = 0xFF;
  ids.recorder = MSGQ_AUD_RECORDER;
  ids.effector = 0xFF;
  ids.recognizer = 0xFF;

  AS_CreateAudioManager(ids, app_attention_callback);

  /* create task of MediaRecorder */

  AsCreateRecorderParam_t recorder_create_param;
  recorder_create_param.msgq_id.recorder      = MSGQ_AUD_RECORDER;
  recorder_create_param.msgq_id.mng           = MSGQ_AUD_MGR;
  recorder_create_param.msgq_id.dsp           = MSGQ_AUD_DSP;
  recorder_create_param.pool_id.input         = MIC_IN_BUF_POOL;
  recorder_create_param.pool_id.output        = OUTPUT_BUF_POOL;
  recorder_create_param.pool_id.dsp           = ENC_APU_CMD_POOL;

  if (!AS_CreateMediaRecorder(&recorder_create_param))
    {
      printf("AS_CreateMediaRecorder failed. system memory insufficient!\n");
      return 1;
    }

  AsCreateCaptureParam_t capture_create_param;
  capture_create_param.msgq_id.dev0_req  = MSGQ_AUD_CAP;
  capture_create_param.msgq_id.dev0_sync = MSGQ_AUD_CAP_SYNC;
  capture_create_param.msgq_id.dev1_req  = 0xFF;
  capture_create_param.msgq_id.dev1_sync = 0xFF;

  if (!AS_CreateCapture(&capture_create_param))
    {
      printf("AS_CreateCapture failed. system memory insufficient!\n");
      return 1;
    }

  /* create task of Recorder Controller */

 	rec_pid = task_create("REC_APP",  100,  1024 * 2, (main_t)recorder_loop, NULL);
	if (rec_pid < 0) {
		printf("task_create(REC_APP) failed. system memory insufficient!\n");
		return 1;
	}

	if (!audioRecorder_powerOn()) {
		return 1;
	}


	printf("\nh for commands, q to exit\n");

	running = TRUE;
	while(running) {
		printf("recorder> ");
		fflush(stdout);

		len = readline(buffer, sizeof(buffer), stdin, stdout);
		buffer[len] = '\0';
		if (len > 0) {
			if (buffer[len-1] == '\n') {
				buffer[len-1] = '\0';
			}
			cmd = strtok_r(buffer, " \n", &arg);
			if (cmd == NULL) {
				continue;
			}
			while (*arg == ' ') {
				arg++;
			}
			for (x = 0; x < g_recorder_cmd_count; x++) {
				if (strcmp(cmd, g_recorder_cmds[x].cmd) == 0) {
					if (g_recorder_cmds[x].pFunc != NULL) {
						g_recorder_cmds[x].pFunc(arg);
					}
					if (g_recorder_cmds[x].pFunc == audioRecorder_powerOff) {
						running = FALSE;
					}
					break;
				}
			}

			if (x == g_recorder_cmd_count) {
				printf("%s:  unknown recorder command\n", buffer);
			}
		}
	}

  /* deactivate */

  AS_DeleteMediaRecorder();
  AS_DeleteCapture();
  AS_DeleteAudioManager();

  /* finalize memory */

  fin_libraries();

  /* delete task */

  printf("end\n");
  task_delete(rec_pid);

  /* free resource */

  free(p_recorder);
  p_recorder = NULL;

	return 0;
}
