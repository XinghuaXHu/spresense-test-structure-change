/***********************************************************************
 *
 *      File Name: sample_test.cpp
 *
 *      Description: Mp3 decoder component test sample code
 *
 *      Notes: (C) Copyright 2016 Sony Corporation
 *
 *      Author: Hsingying Ho
 *
 ***********************************************************************
 */

#include <storage/fs_wrapper.h>
#include <common/WaveParser.h>
#include <debug/dbg_log.h>
#include "MemHandle.h"
#include "chateau/lib/s_stl/1.01/include/queue.h"
#include "wien2_common_defs.h"
#include "encoder_component.h"
#include "wav_stream_mng.h"
#include "sdk_message_types.h"
#include "playlist_manager.h"

#define DBG_MODULE DBG_MODULE_AS

__USING_WIEN2

/*
 *----------------------------------------------------------------------
 * TODO (Yoshimi) 以下は、iniファイルから読み込む
 -----------------------------------------------------------------------
 */

/* for Input */
#define MAX_INPUT_CH_NUM			(2)
#define MAX_INPUT_PCM_BYTE_WIDTH	(2)
#define	MAX_INPUT_BUFF_SIZE			(SampleNumPerFrame[AudCodecMP3] * MAX_INPUT_PCM_BYTE_WIDTH * MAX_INPUT_CH_NUM)	/* sample数 x byte x Channel */
#define EMMC_AUDIO_ROOT				"A:/rec"

/* for output */
#define TEST_MAX_OUTPATH_LENGTH  	(64+4)	/* Max length of output filename */
static const uint32_t MaxOutBufSize = 384 * 2; /* 144 * 128000bps / 48000Hz * 2au */

#define OUTPUT_SAMPLING_RATE		(48000)
//#define OUTPUT_SAMPLING_RATE		(16000)

/* for queueing */
#define CAPTURE_PCM_BUF_QUE_SIZE	(7)
struct MicInData {
	MemMgrLite::MemHandle mh;
	uint32_t pcm_sample;
};
typedef s_std::Queue<MicInData, CAPTURE_PCM_BUF_QUE_SIZE> MicInMhQueue;	
MicInMhQueue m_mic_in_buf_mh_que;
typedef s_std::Queue<MemMgrLite::MemHandle, APU_COMMAND_QUEUE_SIZE> OutputBufMhQueue;
OutputBufMhQueue m_output_buf_mh_que;

/* */
static uint16_t s_test_input_file_cnt = 0; /* counter of the input stream for test */
PlayListManager	list_mng;
/* */

typedef struct {
	uint8_t		riff[4];		// "RIFF"
	uint32_t	total_size;		// 全体サイズ
	uint8_t		wave[4];		// "WAVE"
	uint8_t		fmt[4];			// "fmt "
	uint32_t	fmt_size;		// fmt チャンクサイズ
	uint16_t	format;			// フォーマットの種類
	uint16_t	channel;		// チャンネル
	uint32_t	rate;			// サンプリングレート
	uint32_t	avgbyte;		// rate * block
	uint16_t	block;			// channels * bit / 8
	uint16_t	bit;			// ビット数
	uint8_t		data[4];		// "data"
	uint32_t	data_size;		// データサイズ（ヘッダ含まず）
} WAVE_FORMAT;

extern "C" {

int initfFatFs(void)
{
	int ret = -1;
	int stat;
	if ((stat = FS_Init()) != 0) {
		DBG_LOGF_ERROR("FS_Init:NG %d\n", stat);
		stat = FS_Errnum();
		DBG_LOGF_ERROR("FS_Errnum=%d\n", stat);
	}
	else if ((stat = FS_Mount('A')) != 0) {
		DBG_LOGF_ERROR("FS_Mount:NG %d\n", stat);
	}
	else if ((stat = FS_Chdir(EMMC_AUDIO_ROOT)) != 0) {
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
	if ((stat = FS_Unmount('A', FS_UNMOUNT_NORMAL)) != 0) {
		DBG_LOGF_ERROR("FS_Unmount:NG %d\n", stat);
	}
	else if ((stat = FS_Exit()) != 0) {
		DBG_LOGF_ERROR("FS_Exit:NG %d\n", stat);
	}
	else {
		ret = 0;
	}
	return ret;
}

/*--------------------------------------------------------------------*/     
void outputCreateFilleName(char* input_fname, char* output_fname, uint32_t max_length)    
{    
	snprintf(output_fname, max_length, "%s.mp3", input_fname);    
} 

/*--------------------------------------------------------------------*/
FS_File* outputFileOpen(char *input_fname)
{
	FS_File* output_file_fp;
	static char output_fname[TEST_MAX_OUTPATH_LENGTH];
	outputCreateFilleName(input_fname, output_fname, sizeof(output_fname));    
	DBG_LOGF_DEBUG("Rec fopen %s\n", output_fname);
	output_file_fp = FS_Fopen(output_fname, "w");
	if (!output_file_fp) {
		DBG_LOG_ERROR("FS_Fopen:NG\n");
		return 0;
	}
	return output_file_fp;
}

/*--------------------------------------------------------------------*/
bool createFileList()
{
	FS_Dir* dir_descriptor = FS_Opendir(EMMC_AUDIO_ROOT);

	if (dir_descriptor == NULL) {
		return false;
	}

	FS_Dirent dirent;

	for (;;) {
		if (FS_Readdir(dir_descriptor, &dirent) != 0) {
			DBG_LOGF_DEBUG("Create SRC convert database done.\n");
			break;
		}

		if (FS_ATTR_ARCH == dirent.attr) {
			list_mng.addTrack(Track(dirent.longName));
		}
	}

	if (FS_Closedir(dir_descriptor) != 0) {
		DBG_LOGF_ERROR("FS_Closedir error.\n");
		return false;
	}

	list_mng.setRepeatMode(PlayRepeatOff);

	return true;
}

/*--------------------------------------------------------------------*/
bool getWaveFormat(char* file_name, WAVE_FORMAT* format_buf)
{
	bool rst = false;
	FS_File* output_file_fp=NULL;
	FS_Chdir(EMMC_AUDIO_ROOT);
	output_file_fp = FS_Fopen(file_name, "r");
	if (output_file_fp) {
		if((FS_Fread((void*)format_buf, sizeof(WAVE_FORMAT), 1, output_file_fp))){
			rst = true;
		}
		FS_Fclose(output_file_fp);
	}
	return rst;
}

/*--------------------------------------------------------------------*/
void* allocMicInBuf(uint16_t sample_num) {
	MemMgrLite::MemHandle mh;
	if (mh.allocSeg(MIC_IN_BUF_POOL, (sample_num * MAX_INPUT_CH_NUM * MAX_INPUT_PCM_BYTE_WIDTH)) != MemMgrLite::NoError) {
		DBG_LOG_ERROR("allocSeg error : MIC_IN_BUF_POOL\n");
		return 0; 
	}

	MicInData data;
	data.mh = mh;
	data.pcm_sample = sample_num;
	if (!m_mic_in_buf_mh_que.push(data)) {
		DBG_LOG_ERROR("m_mic_in_buf_mh_que.push error \n");
		return 0; 
	}
	return mh.getVa();
}

/*--------------------------------------------------------------------*/
void* allocOutputBuf() {
	MemMgrLite::MemHandle mh;
	if (mh.allocSeg(OUTPUT_BUF_POOL, MaxOutBufSize) != MemMgrLite::NoError) {
		DBG_LOG_ERROR("allocSeg error : OUTPUT_BUF_POOL\n");
		return 0; 
	}
	if (!m_output_buf_mh_que.push(mh)) {
		DBG_LOG_ERROR("m_output_buf_mh_que.push error\n");
		return 0; 
	}
	return mh.getVa();
}

/*--------------------------------------------------------------------*/
static bool encoderDoneCallback(ApuComPrmTemp_t* p_param)
{
	D_ASSERT2(APU_COM_DATA_TYPE_STRUCT_ADDRESS == p_param->type, AssertParamLog(AssertIdTypeUnmatch, p_param->type));

	Apu::Wien2ApuCmd* packet = reinterpret_cast<Apu::Wien2ApuCmd*>(p_param->data.pParam);

	EncCmpltParam cmplt;
	cmplt.event_type = static_cast<Wien2::Apu::ApuEventType>(packet->header.event_type);

	switch ( packet->header.event_type ) {
		case Apu::InitEvent:
			MsgLib::send<uint32_t>(MSGQ_AUD_APU, MsgPriNormal, MSG_ISR_APU0, MSGQ_NULL, 0);
			break;
		case Apu::ExecEvent:
			{
			cmplt.exec_enc_cmplt.input_buffer = packet->exec_enc_cmd.input_buffer;
			cmplt.exec_enc_cmplt.output_buffer = packet->exec_enc_cmd.output_buffer;

			err_t er = MsgLib::send<EncCmpltParam>(MSGQ_AUD_MP3_ENC_COMPONENT_TEST, MsgPriNormal, MSG_AUD_VOICE_RECORDER_CMD_ENC_CMPLT, NULL, cmplt);
			F_ASSERT(er == ERR_OK);
			}
			break;
		case Apu::FlushEvent:
			{
			cmplt.stop_enc_cmplt.output_buffer = packet->flush_enc_cmd.output_buffer;

			err_t er = MsgLib::send<EncCmpltParam>(MSGQ_AUD_MP3_ENC_COMPONENT_TEST, MsgPriNormal, MSG_AUD_VOICE_RECORDER_CMD_ENC_CMPLT, NULL, cmplt);
			F_ASSERT(er == ERR_OK);
			}
			break;
		default:
			DBG_LOGF_ERROR("Unsupported Apu event type[%d]\n", packet->header.event_type);
			D_ASSERT(0);
			return false;
	}
	return true;
}

/*--------------------------------------------------------------------*/
bool initEncode(uint32_t sampling_rate, uint16_t ch_num, uint16_t word_width)
{
	InitEncParam enc_param;
	enc_param.codec_type = AudCodecMP3;
	enc_param.input_sampling_rate = sampling_rate;
	enc_param.output_sampling_rate = OUTPUT_SAMPLING_RATE;
	if (word_width == 2) {
		enc_param.bit_width = AudPcm16Bit;
	} else if (word_width == 3) {
		enc_param.bit_width = AudPcm24Bit;
	}
	else {
		DBG_LOGF_ERROR("encode init error: word_width=%d\n", word_width);
		return false;
	}
	enc_param.channel_num = static_cast<uint8_t>(ch_num);
	enc_param.callback = encoderDoneCallback;
	bool result = AS_encode_init(enc_param);
	if (!result) {
		DBG_LOGF_ERROR("encode init error (result=%d) : codec_type=%d input_sampling=%d output_sampling=%d bitwidth=%d ch=%d\n",
		result, enc_param.codec_type, enc_param.input_sampling_rate, enc_param.output_sampling_rate, enc_param.bit_width, enc_param.channel_num);
	}
	return result;
}

/*--------------------------------------------------------------------*/
bool execEncode(void* p_in_buf, void* p_out_buf, uint32_t input_buffer_size, EncCmpltParam *p_enc_result)
{
	ExecEncParam param;
	param.input_buffer.p_buffer = reinterpret_cast<unsigned long*>(p_in_buf);
	param.input_buffer.size = input_buffer_size;
	param.output_buffer.p_buffer = reinterpret_cast<unsigned long*>(p_out_buf);
	param.output_buffer.size = MaxOutBufSize;
	bool result = AS_encode_exec(param);
	if (result) {
		MsgQueBlock& que = MsgLib::referMsgQueBlock(MSGQ_AUD_MP3_ENC_COMPONENT_TEST);
		MsgPacket* msg = que.recv(TIME_FOREVER);
		F_ASSERT( msg->getType() == MSG_AUD_VOICE_RECORDER_CMD_ENC_CMPLT);
		*p_enc_result = msg->moveParam<EncCmpltParam>();
		que.pop();
	}
	else {
		DBG_LOGF_ERROR("encode exec error (result=%d) : param.input_buffer.size=%d param.output_buffer.size=%d\n",
			result, param.input_buffer.size, param.output_buffer.size);
	}
	return result;
}

/*--------------------------------------------------------------------*/
bool stopEncode(void* p_out_buf, EncCmpltParam *p_enc_result)
{
	StopEncParam param;
	param.output_buffer.p_buffer = reinterpret_cast<unsigned long*>(p_out_buf);
	param.output_buffer.size = MaxOutBufSize;
	bool result = AS_encode_stop(param);
	if (result) {
		MsgQueBlock& que = MsgLib::referMsgQueBlock(MSGQ_AUD_MP3_ENC_COMPONENT_TEST);
		MsgPacket* msg = que.recv(TIME_FOREVER);
		F_ASSERT( msg->getType() == MSG_AUD_VOICE_RECORDER_CMD_ENC_CMPLT);
		*p_enc_result = msg->moveParam<EncCmpltParam>();
		que.pop();
	}
	else {
		DBG_LOGF_ERROR("encode stop error (result=%d) : param.input_buffer.size=%d param.output_buffer.size=%d\n",
			result, param.output_buffer.size);
	}
	return result;
}

/*--------------------------------------------------------------------*/
void queuePop() {
	m_mic_in_buf_mh_que.pop();
	m_output_buf_mh_que.pop();
}

/*--------------------------------------------------------------------*/
bool sampleTest()
{
	Track next_track;
	char* next_track_name = NULL;
	if (!list_mng.getNextTrack(&next_track)) {
		return false;
	}
	next_track_name = next_track.getLongName();

	WAVE_FORMAT wave_format;
	if(!getWaveFormat(next_track_name, &wave_format)) {
		return false;
	}
	uint32_t sampling_rate_value = 0;
	sampling_rate_value = wave_format.rate;

	uint16_t input_channel_number = 0;
	input_channel_number = wave_format.channel;

	uint16_t input_bit_per_sample = 0;
	input_bit_per_sample = wave_format.bit;
	uint16_t in_word_len = input_bit_per_sample / BYTE_BIT;

	uint16_t sample_number_per_frame = 0;
	if (sampling_rate_value == 48000) {
		sample_number_per_frame = SampleNumPerFrame[AudCodecMP3];
	}
	else if (sampling_rate_value == 16000) {
		sample_number_per_frame = SampleNumPerFrame[AudCodecMP3]/2; /* 1152 -> 576 samples */
	} else {
		DBG_LOGF_ERROR("invalid sampling rate %d\n", sampling_rate_value);
	}
	uint32_t	input_pcm_size = sample_number_per_frame * input_channel_number * in_word_len;

	WavStreamMng m_stream;
	InitInputDataManagerParam  init_param;
	init_param.input_device = AS_SETPLAYER_INPUTDEVICE_EMMC;
	init_param.p_fname = next_track_name;
	if (!m_stream.init(init_param)) {
		DBG_LOG_ERROR("input file open error\n");
		return false;
	}

	FS_File* output_file_fp = outputFileOpen(next_track_name);
	if (!output_file_fp) {
		DBG_LOG_ERROR("output file open error\n");
		m_stream.finish();
		return false;
	}

	bool result = initEncode(sampling_rate_value, input_channel_number, in_word_len);
	if (!result) {
		m_stream.finish();
		FS_Fclose(output_file_fp);
		return false;
	}
	s_test_input_file_cnt++;

	uint32_t	frame_cnt = 0;
	bool		flag_encode_exec = false;
	do {
		void* p_input_buf = allocMicInBuf(sample_number_per_frame);
		if (!p_input_buf) {
			DBG_LOG_ERROR("allocMicInBuf error\n");
			break;
		}
		void* p_output_buf = allocOutputBuf();
		if (!p_output_buf) {
			DBG_LOG_ERROR("allocOutputBuf error\n");
			break;
		}

		/* read file from eMMC file system */
		InputDataManagerObject::GetEsResult sts = m_stream.getEs(p_input_buf, &input_pcm_size);
		frame_cnt++;
		if (sts == InputDataManagerObject::EsExist) {
			flag_encode_exec = true;
			EncCmpltParam enc_result;
			result = execEncode(p_input_buf, p_output_buf, input_pcm_size, &enc_result);
			if (result) {
				if (enc_result.exec_enc_cmplt.output_buffer.size > 0) {
					if (FS_Fwrite(p_output_buf, enc_result.exec_enc_cmplt.output_buffer.size, 1, output_file_fp) <= 0) {    
						DBG_LOG_ERROR("FS_Fwrite:NG\n");
					}
				}
				queuePop();
				DBG_LOGF_DEBUG("execEncode fame cnt:%d  input_suze:%d  output_size:%d\n", frame_cnt, input_pcm_size, enc_result.exec_enc_cmplt.output_buffer.size);
			}
			else {
				DBG_LOG_ERROR("execEncode error\n");
				if (flag_encode_exec) {
					stopEncode(p_output_buf, &enc_result);
				}
				flag_encode_exec = false;
				queuePop();
				break;
			}
		} 
		else if (sts == InputDataManagerObject::EsEnd) {
			DBG_LOG_DEBUG("EOF \n");
			/* Flush */
			if (flag_encode_exec) {
				EncCmpltParam enc_result;
				result = stopEncode(p_output_buf, &enc_result);
				if (result) {
					if (enc_result.stop_enc_cmplt.output_buffer.size > 0) {
						if (FS_Fwrite(p_output_buf, enc_result.stop_enc_cmplt.output_buffer.size, 1, output_file_fp) <= 0) {
							DBG_LOG_ERROR("FS_Fwrite:NG\n");
						}
					}
					DBG_LOGF_DEBUG("stopEncode fame cnt:%d  output_size:%d\n", frame_cnt, enc_result.stop_enc_cmplt.output_buffer.size);
				}
				else {
					DBG_LOG_ERROR("stopEncode error\n");
				}
			}
			flag_encode_exec = false;
			queuePop();
			break;
		}
		else {
			DBG_LOG_ERROR("EsNone returned from getEs() \n");
			flag_encode_exec = false;
			break;
		}
	} while(1);

	m_stream.finish();
	FS_Fclose(output_file_fp);

	return true;
}
} /* extern "C" */
