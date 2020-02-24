#ifndef _TEST_API_H_
#define _TEST_API_H_

#include "audio/audio_high_level_api.h"


/**
 * @brief Print audio command result errorResp/attention info
 *
 * @param[in] command_code: command code
 * @param[in] result: result info
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool printAudCmdResult(uint8_t command_code, AudioResult& result);

/**
 * @brief Create audio sub system
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_create_audio_sub_system(bool is_recorder);

bool Audio_audio_manager(bool is_create);
bool Audio_player(bool is_create);
bool Audio_mixer(bool is_create);
bool Audio_renderer(bool is_create);
bool Audio_recorder(bool is_create);
bool Audio_capture(bool is_create);

/**
 * @brief Deactivate audio sub system
 */
bool Audio_deact_audio_sub_system(bool is_recorder);

/**
 * @brief Power on the audio module
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_power_on(void);
/**
 * @brief Enter POWEROFF status
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_power_off(void);
/**
 * @brief Enter READY status
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_set_ready(void);
/**
 * @brief Enter PLAYER status
 *
 * @param[in] input_device0: input device for player0
 * @param[in] input_device1: input device for player1
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_set_player_status(AsPlayerInputDeviceHdlrForRAM* input_device0, AsPlayerInputDeviceHdlrForRAM* input_device1);
/**
 * @brief Enter RECORDER status
 *
 * @param[in] output_device: output device for recorder
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_set_recorder_status(AsRecorderOutputDeviceHdlr* output_device);
/**
 * @brief Get the current status
 *
 * @retval     true  : success
 * @retval     false : failure
 */
uint8_t Audio_get_status(void);

/**
 * @brief Set the rendering clock mode
 *
 * @param[in] clk_mode: the rendering clock mode
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_set_clkmode(int clk_mode);
/**
 * @brief Initialize the output select
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_init_output_select(void);

/**
 * @brief Initialize the player
 *
 * @param[in] id: player ID
 * @param[in] codec_type: codec type
 * @param[in] sampling_rate: sampling rate
 * @param[in] channel_number: channel number
 * @param[in] bit_length: bit length
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_init_player(AsPlayerId id,
                    uint8_t codec_type,
                    uint32_t sampling_rate,
                    uint8_t channel_number,
                    uint8_t bit_length);
/**
 * @brief Play the music with the specified player
 *
 * @param[in] player0: whether player0 play
 * @param[in] player1: whether player1 play
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_play_player(bool player0, bool player1);
/**
 * @brief stop the specified player
 *
 * @param[in] id: player ID
 * @param[in] mode: stop mode
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_stop_player(AsPlayerId, AsStopPlayerStopMode mode);
/**
 * @brief Set players' volume
 *
 * @param[in] input1_db: player0 volume (DB)
 * @param[in] input2_db: player1 volume (DB)
 * @param[in] master_db: master volume (DB)
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_set_volume(int16_t input1_db, int16_t input2_db, int16_t master_db);
/**
 * @brief Set players' volume mute
 *
 * @param[in] isInput1Mute: whether player0 is mute
 * @param[in] isInput2Mute: whether player1 is mute
 * @param[in] isMasterMute: whether master is mute
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_set_mute(bool isInput1Mute, bool isInput2Mute, bool isMasterMute);

/**
 * @brief Initialize the recorder
 *
 * @param[in] codec_type: codec type (MP3,WAV...)
 * @param[in] sampling_rate: sampling rate
 * @param[in] ch_type: channel number
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_init_recorder(uint8_t codec_type, uint32_t sampling_rate, uint8_t ch_type);
/**
 * @brief Initialize the mic gain
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_init_mic_gain(bool is_analog, int16_t mic_gain);
/**
 * @brief Start the recorder
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_start_recorder(void);
/**
 * @brief Stop the recorder
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_stop_recorder(void);

/**
 * @brief Beep
 *
 * @retval     true  : success
 * @retval     false : failure
 */
bool Audio_beep(bool enable, int16_t vol = AS_BEEP_VOL_HOLD, uint16_t freq = AS_BEEP_FREQ_HOLD);

#endif
