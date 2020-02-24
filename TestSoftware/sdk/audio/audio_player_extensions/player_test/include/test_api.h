#ifndef _TEST_API_H_
#define _TEST_API_H_

#include "audio/audio_high_level_api.h"


bool printAudCmdResult(uint8_t command_code, AudioResult& result);
bool app_power_on(void);
bool app_power_off(void);
bool app_set_ready(void);
bool app_get_status(void);
bool app_init_output_select(void);
bool app_set_volume(int master_db);
bool app_set_player_status(AsPlayerId id, AsPlayerInputDeviceHdlrForRAM* input_device0, AsPlayerInputDeviceHdlrForRAM* input_device1);
bool app_set_clkmode(int clk_mode);
int app_init_player(AsPlayerId id,
                    uint8_t codec_type,
                    uint32_t sampling_rate,
                    uint8_t channel_number,
                    uint8_t bit_length);
int app_play_player(AsPlayerId id);
bool app_stop_player(AsPlayerId, int mode);
bool app_set_mute(bool isInput1Mute = true,
                    bool isInput2Mute = true,
                    bool isMasterMute = true);

bool app_create_audio_sub_system(void);
void app_deact_audio_sub_system(void);

#endif
