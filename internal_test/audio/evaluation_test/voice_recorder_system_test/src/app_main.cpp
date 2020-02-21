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

#include <debug/dbg_log.h>
#include <audio/high_level_api/as_high_level_api.h>
#include <drivers/peripheral/pd_gpio.h>
#include <debug/dbg_shell.h>
#include <debug/dbg_log.h>
#include <FreeRTOS.h>
#include <task.h>

#include <storage/fs_wrapper.h>
#include <system/sys_rtc_mgr.h>
#include <common/WaveParser.h>
#include "wien2_common_defs.h"

#include "app_main.h"

#define DBG_MODULE DBG_MODULE_DBG

#define DELAY_TIEM	10 /* ms */
#define MAXPATHLENGTH   25

static uint32_t s_rec_size= 0;
void ram_recsequence();

int init_fatfs(void)
{
	int ret = -1;
	int stat;

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
int exit_fatfs(void)
{
	int ret = -1;
	int stat;

	if ( (stat = FS_Unmount('A', FS_UNMOUNT_NORMAL)) != 0 ) {
		DBG_LOGF_ERROR("FS_Unmount:NG %d\n", stat);
	}
	else {
		if ( (stat = FS_Exit()) != 0 ) {
			DBG_LOGF_ERROR("FS_Exit:NG %d\n", stat);
		}
		else {
			ret = 0;
		}
	}
	return ret;
}

extern "C" {

extern DBG_ShellCommandTreeNode_t voiceRecorderCmdTree;

/*--------------------------------------------------------------------
	Main Loop
  --------------------------------------------------------------------*/
void app_main(void)
{
	DBG_LogInit();

	DBG_ShellRegisterCommandTree(&voiceRecorderCmdTree);

	if (init_fatfs()) {
		DBG_LOG_FATAL("Failed initialize in file system\n");
	}

	ram_recsequence();

	vTaskSuspend(NULL);
}

} /* extern "C" */

/*--------------------------------------------------------------------*/
void ram_recsequence(void)
{

	while(1) {
		vTaskDelay(DELAY_TIEM);
		if(s_rec_size >= READ_SIMPLE_FIFO_SIZE) {
			AppMain::getInstance()->write_to_outputfile(READ_SIMPLE_FIFO_SIZE);
			s_rec_size -= READ_SIMPLE_FIFO_SIZE;
		}
	}
}

static void outputdevice_callback(uint32_t size)
{
	s_rec_size += size;
}

/*--------------------------------------------------------------------*/
void AppMain::write_to_outputfile(uint32_t size)
{
	size_t fifo_size;

	if(m_file_fp == NULL) {
		return;
	}

	fifo_size = CMN_SimpleFifoGetOccupiedSize(&m_simple_fifo_handle);

	if ( fifo_size < size ){
	        return;
	}

	if ( fifo_size > 0 ){

		if ( fifo_size < size ){
			size = fifo_size ;
		}

		if (CMN_SimpleFifoPoll(&m_simple_fifo_handle, reinterpret_cast<void*>(m_ram_buf), size) == 0) {
			DBG_LOGF_ERROR("CMN_SimpleFifoPoll:NG\n");
			return;
		}

		if (FS_Fwrite(m_ram_buf, size, 1, m_file_fp) <= 0) {
			DBG_LOGF_ERROR("write_to_outputfile FS_Fwrite:NG\n");
			FS_Fclose(m_file_fp);
			m_file_fp = NULL;
		}
	}
}

/*--------------------------------------------------------------------*/
void AppMain::stop_outputfile()
{
	size_t fifo_size;
	s_rec_size = 0 ;

	if(m_file_fp == NULL) {
		return;
	}

	fifo_size = CMN_SimpleFifoGetOccupiedSize(&m_simple_fifo_handle);

	if (fifo_size != 0){
		while(fifo_size > 0){
			if ( fifo_size > READ_SIMPLE_FIFO_SIZE ){
				if (CMN_SimpleFifoPoll(&m_simple_fifo_handle, reinterpret_cast<void*>(m_ram_buf), READ_SIMPLE_FIFO_SIZE) == 0) {
					DBG_LOGF_ERROR("write_Stop CMN_SimpleFifoPoll:NG\n");
					return;
				}

				if (FS_Fwrite(m_ram_buf, READ_SIMPLE_FIFO_SIZE, 1, m_file_fp) <= 0) {
					DBG_LOGF_ERROR("write_stop FS_Fwrite:NG\n");
					FS_Fclose(m_file_fp);
					m_file_fp = NULL;
					return;
				}
				fifo_size -= READ_SIMPLE_FIFO_SIZE ;
			}
			else {
				if (CMN_SimpleFifoPoll(&m_simple_fifo_handle, reinterpret_cast<void*>(m_ram_buf), fifo_size) == 0) {
					DBG_LOGF_ERROR("write_Stop CMN_SimpleFifoPoll:NG\n");
					return;
				}

				if (FS_Fwrite(m_ram_buf, fifo_size, 1, m_file_fp) <= 0) {
					DBG_LOGF_ERROR("write_stop FS_Fwrite:NG\n");
					FS_Fclose(m_file_fp);
					m_file_fp = NULL;
					return;
				}
				fifo_size = 0 ;
			}
		}
 	}

	s_rec_size = 0;
	output_close();
}

/*--------------------------------------------------------------------*/
void AppMain::powerOn()
{
	AudioCommand command;
	
	command.header.packet_length = LENGTH_POWERON;
	command.header.command_code  = AUDCMD_POWERON;
	command.header.sub_code      = 0x00;
	command.power_on_param.enable_sound_effect = AS_DISABLE_SOUNDEFFECT;

	AS_SendAudioCommand( &command );
	
	AudioResult result;
	AS_ReceiveAudioResult( &result );
	if(result.header.result_code != AUDRLT_STATUSCHANGED){
		DBG_LOGF_ERROR("ERROR(%d)\n", result.header.result_code);
	}
	show_curstate();
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
	if(result.header.result_code != AUDRLT_STATUSCHANGED){
		DBG_LOGF_ERROR("ERROR(%d)\n", result.header.result_code);
	}
	show_curstate();
}

/*--------------------------------------------------------------------*/
void AppMain::initMicGain(void)
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
	if(result.header.result_code != AUDRLT_INITMICGAINCMPLT){
		DBG_LOGF_ERROR("ERROR(%d)\n", result.header.result_code);
	}
}

/*--------------------------------------------------------------------
	Functions for Debug Shell
  --------------------------------------------------------------------*/
void AppMain::set_recorder_status(const AsSetRecorderStatusParam& param)
{
	initMicGain();

	m_cur_output_device = param.output_device;
	s_rec_size = 0;

	if (m_cur_output_device == AS_SETRECDR_STS_OUTPUTDEVICE_RAM) {
		int stat;
		if ((stat = FS_Chdir("A:/")) != 0) {
			DBG_LOGF_ERROR("FS_Chdir:NG %d\n", stat);
			return;
		}

 		if (!initialize_simplefifo()) {
 			return;
 		}

		m_cur_output_device_handler.simple_fifo_handler = reinterpret_cast<void*>(&m_simple_fifo_handle);
		m_cur_output_device_handler.callback_function = outputdevice_callback;
	}

	AudioCommand command;
	AudioResult result;

	/* set recorder status */
	command.header.packet_length = LENGTH_SET_RECORDER_STATUS;
	command.header.command_code = AUDCMD_SETRECORDERSTATUS;
	command.header.sub_code =0x00;

 	command.set_recorder_status_param.input_device		= param.input_device;
 	command.set_recorder_status_param.input_device_handler	= param.input_device_handler;

 	command.set_recorder_status_param.output_device		= param.output_device;
	
	/* eMMc */
	if (param.output_device == 0) {
		command.set_recorder_status_param.output_device_handler	= 0x00;
	}
	else{
		command.set_recorder_status_param.output_device_handler	= &m_cur_output_device_handler;
	}

	AS_SendAudioCommand(&command);
	AS_ReceiveAudioResult(&result);
	if(result.header.result_code != AUDRLT_STATUSCHANGED){
		DBG_LOGF_ERROR("ERROR(%d)\n", result.header.result_code);
	}
	show_curstate();
}

/*--------------------------------------------------------------------*/
void AppMain::rec_start(void)
{
	if (m_cur_output_device == AS_SETRECDR_STS_OUTPUTDEVICE_RAM) {

		uint8_t byte_length ;
		if (m_bit_length == AS_BITLENGTH_16){
			byte_length = BYTELENGTH_2;
		}
		else{
			byte_length = BYTELENGTH_4;
		}

		CMN_SimpleFifoClear(&m_simple_fifo_handle);

		output_open(m_sampling_rate_default,m_channel_no,byte_length);
	}

	AudioCommand command;
	AudioResult result;

	/* start rec */
	command.header.packet_length = LENGTH_START_RECORDER;
	command.header.command_code = AUDCMD_STARTREC;
	command.header.sub_code =0x00;

	AS_SendAudioCommand(&command);
	AS_ReceiveAudioResult(&result);
	if(result.header.result_code != AUDRLT_RECCMPLT){
		DBG_LOGF_ERROR("ERROR(%d)\n", result.header.result_code);
	}
}

/*--------------------------------------------------------------------*/
void AppMain::rec_stop(void)
{
	AudioCommand command;
	AudioResult result;

	/* stop rec */
	command.header.packet_length = LENGTH_STOP_RECORDER;
	command.header.command_code = AUDCMD_STOPREC;
	command.header.sub_code =0x00;

	AS_SendAudioCommand(&command);
	AS_ReceiveAudioResult(&result);
	if(result.header.result_code != AUDRLT_STOPRECCMPLT){
		DBG_LOGF_ERROR("ERROR(%d)\n", result.header.result_code);
	}
	
	/* Rec File close  */
 	if (m_cur_output_device == AS_SETRECDR_STS_OUTPUTDEVICE_RAM) {
		stop_outputfile();
 	}
}

/*--------------------------------------------------------------------*/
void AppMain::set_ready_status(void)
{
	s_rec_size = 0;

	AudioCommand command;
	AudioResult result;

	command.header.packet_length = LENGTH_SET_READY_STATUS;
	command.header.command_code = AUDCMD_SETREADYSTATUS;
	command.header.sub_code = 0x00;

	AS_SendAudioCommand(&command);
	AS_ReceiveAudioResult(&result);
	if(result.header.result_code != AUDRLT_STATUSCHANGED){
		DBG_LOGF_ERROR("ERROR(%d)\n", result.header.result_code);
	}
	show_curstate();
}

/*--------------------------------------------------------------------*/
void AppMain::init_recorder(const AsInitRecorderParam& param)
{
	AudioCommand command;
	AudioResult result;

	command.header.packet_length = LENGTH_INIT_RECORDER;
	command.header.command_code = AUDCMD_INITREC;
	command.header.sub_code = 0x00;
	command.init_recorder_param = param;

	AS_SendAudioCommand(&command);
	AS_ReceiveAudioResult(&result);
	if(result.header.result_code != AUDRLT_INITRECCMPLT){
		DBG_LOGF_ERROR("ERROR(%d)\n", result.header.result_code);
	}
}

/*--------------------------------------------------------------------*/
void AppMain::show_curstate()
{
	AudioCommand command;
	AudioResult result;

	command.header.packet_length = LENGTH_GETSTATUS;
	command.header.command_code = AUDCMD_GETSTATUS;
	command.header.sub_code = 0x00;

	AS_SendAudioCommand(&command);
	AS_ReceiveAudioResult(&result);
	if(result.header.result_code != AUDRLT_NOTIFYSTATUS){
		DBG_LOGF_ERROR("ERROR(%d)\n", result.header.result_code);
	}
	DBG_LOGF_INFO("Current state(%d)\n", result.notify_status.status_info);
}

/*--------------------------------------------------------------------*/
bool AppMain::initialize_simplefifo()
{
	if (&m_simple_fifo_handle == NULL ||
	    m_simple_fifo_buf == NULL ||
	    CMN_SimpleFifoInitialize(&m_simple_fifo_handle, m_simple_fifo_buf, SIMPLE_FIFO_BUF_SIZE, NULL) != 0) {
		DBG_LOGF_ERROR("FIFO create error\n");
		return false;
	}
	return true;
}
/*--------------------------------------------------------------------*/
void AppMain::output_open(uint32_t fs, uint16_t ch, uint16_t byte_length)
{
	static char fname[MAXPATHLENGTH];
	make_output_file_name(fname, sizeof(fname));
	DBG_LOGF_INFO("Rec fopen %s\n", fname);
	m_file_fp = FS_Fopen(fname, "w");
	if (m_file_fp == 0) {
		DBG_LOGF_ERROR("FS_Fopen:NG\n");
		return ;
	}

	if ( m_codec_type_default == AS_CODECTYPE_WAV ) { /* Codec is WAV */
		static WAVEFORMAT header __attribute__((aligned(4))); /* 4バイトアラインメントのためstaticにする */
		create_wav_header(&header, fs, ch, byte_length);
		FS_Size ret = FS_Fwrite(&header, sizeof(WAVEFORMAT), 1, m_file_fp);
		if (ret != 1) {
			DBG_LOGF_ERROR("FS_Fwrite:NG . failed to write header\n");
			return ;
		}
	}
}

/*--------------------------------------------------------------------*/
void AppMain::output_close(void)
{
	if (m_file_fp == 0){
		DBG_LOGF_ERROR("close file pointer is NULL\n");
		return ;
	}
	FS_Position pos;
	int ret = FS_Fgetpos(m_file_fp, &pos);
	if (ret != 0){
		DBG_LOGF_ERROR("FS_Fgetpos:NG\n");
		return ;
	}

	if ( m_codec_type_default == AS_CODECTYPE_WAV ) { /* Codec is WAV */
		/* チャンクデータサイズの書き込み */
		uint32_t size_param = static_cast<uint32_t>(pos - sizeof(WAVEFORMAT));
		ret = FS_Fseek(m_file_fp, offsetof(WAVEFORMAT, data_size), FS_FSEEK_SET);
		if (ret != 0) {
			DBG_LOGF_ERROR("FS_Fseek:NG . failed seek chunk datasize\n");
			return ;
		}

		ret = FS_Fwrite(&size_param, sizeof(size_param), 1, m_file_fp);
		if (ret != 1) {
			DBG_LOGF_ERROR("FS_Fwrite:NG . failed to write chunk datasize\n");
			return ;
		}

		/* totalサイズの書き込み */
		size_param = static_cast<uint32_t>(pos - 8); /* RIFFとsizeを除くサイズ */
		ret = FS_Fseek(m_file_fp, offsetof(WAVEFORMAT, total_size), FS_FSEEK_SET);
		if (ret != 0) {
			DBG_LOGF_ERROR("FS_Fseek:NG . failed seek total size\n");
			return ;
		}
		ret = FS_Fwrite(&size_param, sizeof(size_param), 1, m_file_fp);
		if (ret != 1) {
			DBG_LOGF_ERROR("FS_Fwrite:NG . failed to write total size\n");
			return ;
		}
	}

	s_rec_size = 0;

 	FS_Fclose(m_file_fp);
 	m_file_fp = 0;
}

/*--------------------------------------------------------------------*/
void AppMain::make_output_file_name(char* fname, uint32_t max_length)
{
	SYS_RtcCalendarTime_t cur_time;
	if (SYS_RtcMgrGetCalendarTime(SYS_RTC_LOCAL, &cur_time) != 0) {
		DBG_LOGF_ERROR("SYS_RtcMgrGetCalendarTime:NG . get failed local time\n");
		return ;
	}
	if ( m_codec_type_default == AS_CODECTYPE_MP3 ) { /* Codec is MP3 */
		snprintf(fname, max_length, "%04d%02d%02d_%02d%02d%02d.mp3", cur_time.year, cur_time.month, cur_time.day, cur_time.hour, cur_time.minute, cur_time.second);
	}
	else if ( m_codec_type_default == AS_CODECTYPE_WAV ){ /* Codec is WAV */
		snprintf(fname, max_length, "%04d%02d%02d_%02d%02d%02d.wav", cur_time.year, cur_time.month, cur_time.day, cur_time.hour, cur_time.minute, cur_time.second);
	}
	else {
		snprintf(fname, max_length, "%04d%02d%02d_%02d%02d%02d.non", cur_time.year, cur_time.month, cur_time.day, cur_time.hour, cur_time.minute, cur_time.second);
		DBG_LOGF_ERROR("Non Sport Codec type.\n");
		return ;
        }
}

/*--------------------------------------------------------------------*/
void AppMain::create_wav_header(WAVEFORMAT* header, uint32_t fs, uint16_t ch, uint16_t byte_length)
{
	/* WAVフォーマット形式は固定とする */
	memcpy(header->riff, CHUNKID_RIFF, strlen(CHUNKID_RIFF));
	header->total_size = 0;
	memcpy(header->wave, FORMAT_WAVE, strlen(FORMAT_WAVE));
	memcpy(header->fmt, SUBCHUNKID_FMT, strlen(SUBCHUNKID_FMT));
	header->fmt_size = 0x10; /* according to WAV spec;, we should always set this vlaue to 0x10. */
	header->format = AUDIO_FORMAT_PCM;
	header->channel = ch;
	header->rate = fs;
	header->avgbyte = fs * ch * byte_length; /* Fs * ch * bytelen */
	header->block = ch * byte_length;	/* ch * bytelen */
	header->bit = byte_length * 8; 
	memcpy(header->data, SUBCHUNKID_DATA, strlen(SUBCHUNKID_DATA));
	header->data_size = 0;
}

/*--------------------------------------------------------------------
	Set Member
  --------------------------------------------------------------------*/
void AppMain::set_samplingrate(uint32_t sampling_rate)
{
	m_sampling_rate_default = sampling_rate;
}

void AppMain::set_codectype(uint8_t codec_type)
{
	m_codec_type_default = codec_type;
}

void AppMain::set_channelno(uint8_t channel_no)
{
	m_channel_no = channel_no;
}

void AppMain::set_bitlength(uint8_t bit_length)
{
	m_bit_length = bit_length;
}

