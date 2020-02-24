/*
 * cmd_parser.h
 *
 *  Created on: 2019.1.17
 *      Author: neusoft
 */

#ifndef EXAMPLES_PLAYER_TEST_INCLUDE_CMD_PARSER_H_
#define EXAMPLES_PLAYER_TEST_INCLUDE_CMD_PARSER_H_

#include "audio/audio_high_level_api.h"


bool app_cmd_help(unsigned int count, const char** tp);
bool app_cmd_quit(unsigned int count, const char** tp);

bool app_cmd_init_libraries(unsigned int count, const char** tp);
bool app_cmd_finalize_libraries(unsigned int count, const char** tp);

bool app_cmd_init_simple_fifo(unsigned int count, const char** tp);

bool app_cmd_open_contents_dir(unsigned int count, const char** tp);
bool app_cmd_freq_lock(unsigned int count, const char** tp);
bool app_cmd_open_play_file(unsigned int count, const char** tp);
bool app_cmd_set_clkmode(unsigned int count, const char** tp);
bool app_cmd_amp_mute(unsigned int count, const char** tp);

bool app_cmd_create_audio_sub_system(unsigned int count, const char** tp);
bool app_cmd_deact_audio_sub_system(unsigned int count, const char** tp);

bool app_cmd_audio_manager(unsigned int count, const char** tp);
bool app_cmd_player(unsigned int count, const char** tp);
bool app_cmd_mixer(unsigned int count, const char** tp);
bool app_cmd_renderer(unsigned int count, const char** tp);
bool app_cmd_recorder(unsigned int count, const char** tp);
bool app_cmd_capture(unsigned int count, const char** tp);

bool app_cmd_power(unsigned int count, const char** tp);
bool app_cmd_init_output_select(unsigned int count, const char** tp);
bool app_cmd_set_status(unsigned int count, const char** tp);

///////////////////////////////////////////////////////
// BEEP enable vol freq
//   enable: enable, disable
//      vol: -90 ~ 0  (default: -12)
//     freq: 94 ~ 4085
///////////////////////////////////////////////////////
bool app_cmd_beep(unsigned int count, const char** tp);

///////////////////////////////////////////////////////
// VOL input1_db input2_db master_db
//    input1_db: -1020 ~ 120, 255(HOLD), -1025(MUTE(default))
//    input2_db: -1020 ~ 120, 255(HOLD), -1025(MUTE(default))
//    master_db: -1020 ~ 120, 255(HOLD), -1025(MUTE(default))
///////////////////////////////////////////////////////
bool app_cmd_set_volume(unsigned int count, const char** tp);

///////////////////////////////////////////////////////
// MUTE input1_mute input2_mute master_mute
//    input1_mute: true, false
//    input2_mute: true, false
//    master_mute: true, false
///////////////////////////////////////////////////////
bool app_cmd_mute(unsigned int count, const char** tp);

bool app_cmd_init_player(unsigned int count, const char** tp);
bool app_cmd_play(unsigned int count, const char** tp);
bool app_cmd_pause(unsigned int count, const char** tp);
bool app_cmd_stop_player(unsigned int count, const char** tp);
bool app_cmd_init_recorder(unsigned int count, const char** tp);
bool app_cmd_start_recorder(unsigned int count, const char** tp);
bool app_cmd_stop_recorder(unsigned int count, const char** tp);
bool app_cmd_init_mic_gain(unsigned int count, const char** tp);


bool app_close_play_file(AsPlayerId i);
bool app_process_cmd_loop();
extern FAR void play_thread(FAR void *arg);


extern AsClkMode s_clk_mode;
extern bool g_quit;

#endif /* EXAMPLES_PLAYER_TEST_INCLUDE_CMD_PARSER_H_ */
