/*
 * cmd_parser.h
 *
 *  Created on: 2019.1.17
 *      Author: neusoft
 */

#ifndef EXAMPLES_PLAYER_TEST_INCLUDE_CMD_PARSER_H_
#define EXAMPLES_PLAYER_TEST_INCLUDE_CMD_PARSER_H_

#include "audio/audio_high_level_api.h"


bool app_close_play_file(AsPlayerId i);
void app_cmd_help();
bool app_process_cmd_loop();
extern FAR void play_thread(FAR void *arg);



extern AsPlayerId s_playing_id;
extern int s_clk_mode;


#endif /* EXAMPLES_PLAYER_TEST_INCLUDE_CMD_PARSER_H_ */
