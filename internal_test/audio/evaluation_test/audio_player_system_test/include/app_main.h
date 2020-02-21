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
#include <system/sys_flash_mgr.h>
#include "string.h"
#include <string>
#include <vector>

#define SIMPLE_FIFO_BUF_SIZE	128000
#define WRITE_SIMPLE_FIFO_SIZE	2560	/* (640 sample * 16bit * 2ch) bytes */
#define AUDIO_CONTENTS_CONFIG_BUF_SIZE	256
#define AUDIO_CONTENTS_CONFIG_LIST_MAX_NUM	(50)
#define AUDIO_CONTENTS_CONFIG_FILE_NAME	"audio_file_for_ram_play.cfg"
#define AUDIO_CONTENTS_CONFIG_TOKEN_NUM	(5)

/*--------------------------------------------------------------------*/
struct Track {
        std::string filename;
};

/*--------------------------------------------------------------------*/
class Playlist {
private:
	int32_t m_cur_track_id;
	std::vector<Track> m_playlist;

public:
	Playlist():
		m_cur_track_id(-1)
	{
		m_playlist.clear();
	}

	~Playlist() {}

	void add(const Track& track) {
		m_playlist.push_back(track);
	}

	bool isEmpty() {
		return m_playlist.empty();
	}

	bool getNextTrack(Track* track);
};

/*--------------------------------------------------------------------*/
class AppMain {

public:
	static AppMain* getInstance()
	{
		static AppMain app_main;
		return &app_main;
	}
	struct PlayerStatusParam {
    		uint8_t input_device;
    		uint8_t output_device;
	};

	struct PlayerModeParam {
		uint8_t play_filter;
		uint8_t play_mode;
		uint8_t repeat_mode;
	};

	/* functions that will be called by debug shell */
	void playMusic(void);
	void stopMusic(void);
	void pauseMusic(void);
	void incVol(void);
	void decVol(void);
	void setPlayerMode(const PlayerStatusParam& param);
	void setReadyStatus(void);
	void nextMusic(void);
	void prevMusic(void);
	void setPlayerParam(const PlayerModeParam& param);
	void setPlaylistSource(uint32_t param);
	void powerOn(uint32_t param);
	void setPowerOffStatus(void);
	void initSoundEffect(void);
	void startSoundEffect(void);
	void stopSoundEffect(void);
	
	void initPlayer(AsInitPlayerCodecType codec_type, \
			AsInitPlayerBitLength bit_length, \
			AsInitPlayerChannelNumberIndex ch_num, \
			AsInitPlayerSamplingRateIndex sampling_rate);

	void writeToSimpleFifo(uint32_t size);
	bool getNextTrack(char* filename);

private:
	AppMain():
		m_audio_codec_dsp_vol(AS_AC_CODEC_VOL_DAC),
		m_cur_input_device(AS_SETPLAYER_INPUTDEVICE_EMMC), /* Default = eMMC FileSystem */
		m_file_fp(NULL),
		m_audio_contents_config_file_fp(NULL),
		m_playlist_source(AS_SETPLAYER_INPUTDEVICE_INTER_PLAYLIST),
		m_enable_sound_effect(AS_DISABLE_SOUNDEFFECT)
	{
		memset(&m_emmc_input_device_handler, 0x0, sizeof(AsPlayerInputDeviceHdlrForEMMC));
		memset(&m_ram_input_device_handler, 0x0, sizeof(AsPlayerInputDeviceHdlrForRAM));
		memset(&m_simple_fifo_handle, 0x0, sizeof(CMN_SimpleFifoHandle));
	}

	~AppMain() {}
	AppMain(const AppMain& rhs);
	AppMain& operator=(const AppMain& rhs);

	bool initOutputSelect();
	bool transferToPlayerStatus(const PlayerStatusParam& param);
	void setVolume(uint32_t param);
	bool initializeSimpleFIFO();
	void printCommandResult(uint8_t result);
	bool createPlaylist();
	
	int32_t m_audio_codec_dsp_vol;
	uint8_t m_cur_input_device;
	uint32_t m_playlist_source;
	uint32_t m_enable_sound_effect;
	
	AsPlayerInputDeviceHdlrForEMMC m_emmc_input_device_handler;
	AsPlayerInputDeviceHdlrForRAM m_ram_input_device_handler;

	CMN_SimpleFifoHandle m_simple_fifo_handle;
	uint8_t m_simple_fifo_buf[SIMPLE_FIFO_BUF_SIZE];
	uint8_t m_ram_buf[WRITE_SIMPLE_FIFO_SIZE];
	char* m_play_name;

	Playlist m_playlist;

	FM_FILE* m_file_fp;
	FM_FILE* m_audio_contents_config_file_fp;
	struct AudioContentsConfigCtrl {
		uint32_t remain_data_size;
		uint32_t read_start_position;
		uint32_t read_idx;
		uint32_t play_list_num;
		uint32_t play_list_cnt;
	};
	AudioContentsConfigCtrl m_audio_contents_config_ctrl;
	int8_t m_audio_contents_config_play_list[AUDIO_CONTENTS_CONFIG_LIST_MAX_NUM][64];
	bool audioContentsConfigFileOpen(void);
	bool audioContentsConfigFileClose(void);
	bool audioContentsConfigGets(int8_t *p_buf);
	bool audioContentsGetToken(int32_t argc, int8_t *argv[]);
	bool audioContentsGenPlayList(AsInitPlayerCodecType codec_type, \
			AsInitPlayerBitLength bit_length, \
			AsInitPlayerChannelNumberIndex ch_num, \
			AsInitPlayerSamplingRateIndex sampling_rate);
};

#endif /* APP_MAIN_H */
