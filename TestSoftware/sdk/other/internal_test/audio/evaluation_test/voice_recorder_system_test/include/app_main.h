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
#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <audio/high_level_api/as_high_level_api.h>
#include <common/CMN_SimpleFifo.h>
#include <storage/fs_wrapper.h>
#include <common/WaveParser.h>
#include "string.h"

#define DEFAULT_SAMPLING_RATE 16000   /* defaultのSamplingRate */
#define DEFAULT_CHANNEL_NO    2       /* default チャネル数 */
#define BYTELENGTH_2  2               /* 2ByeteLength */
#define BYTELENGTH_4  4               /* 4ByeteLength */
#define SIMPLE_FIFO_BUF_SIZE  153088  /* 6144(READ_SIMPLE_FIFO_SIZE)*24+α */
#define READ_SIMPLE_FIFO_SIZE 6144    /* WAVの最大、48K,4ch,16bitsで768(WAVの1フレーム)サンプル*4(チャンネル数)*2(bytes)=6144 */

class AppMain {

public:
	static AppMain* getInstance()
	{
		static AppMain app_main;
		return &app_main;
	}

	void rec_start(void);
	void rec_stop(void);
	void set_recorder_status(const AsSetRecorderStatusParam& param);
	void set_ready_status(void);
	void powerOn(void);
	void setPowerOffStatus(void);

	void init_recorder(const AsInitRecorderParam& param);
	void write_to_outputfile(uint32_t size);
	void stop_outputfile(void);

	void set_samplingrate(uint32_t sampling_rate);
	void set_codectype(uint8_t codec_type);
	void set_channelno(uint8_t channel_no);
	void set_bitlength(uint8_t bit_length);

private:
	AppMain():
		m_sampling_rate_default(DEFAULT_SAMPLING_RATE),
	        m_codec_type_default(AS_CODECTYPE_MP3),
		m_channel_no(DEFAULT_CHANNEL_NO),
		m_bit_length(AS_BITLENGTH_16),
		m_cur_output_device(AS_SETRECDR_STS_OUTPUTDEVICE_EMMC), /* Default = eMMC FileSystem */
		m_file_fp(NULL)
	{
		memset(&m_cur_output_device_handler, 0x0, sizeof(AsRecorderOutputDeviceHdlr));
		memset(&m_simple_fifo_handle, 0x0, sizeof(CMN_SimpleFifoHandle));
	}
	~AppMain() {}
	AppMain(const AppMain& rhs);
	AppMain& operator=(const AppMain& rhs);

	void initMicGain(void);
	void show_curstate();
	bool initialize_simplefifo();
	void output_open(uint32_t fs, uint16_t ch, uint16_t byte_length);
	void output_close();
	void make_output_file_name(char* fname, uint32_t max_length);
	void create_wav_header(WAVEFORMAT* header, uint32_t fs, uint16_t ch, uint16_t byte_length);

	uint8_t m_cur_output_device;

 	uint32_t m_sampling_rate_default ;
	uint8_t  m_codec_type_default ;
	uint8_t  m_channel_no ;
	uint8_t  m_bit_length ;

	AsRecorderOutputDeviceHdlr m_cur_output_device_handler;
	CMN_SimpleFifoHandle m_simple_fifo_handle;
	uint8_t m_simple_fifo_buf[SIMPLE_FIFO_BUF_SIZE];
	uint8_t m_ram_buf[READ_SIMPLE_FIFO_SIZE];
	FS_File* m_file_fp;

};

#endif /* APP_MAIN_H */
