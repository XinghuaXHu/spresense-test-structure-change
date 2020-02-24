/*
 * player_test_common.h
 *
 *  Created on: 2019.1.17
 *      Author: neusoft
 */

#ifndef EXAMPLES_PLAYER_TEST_INCLUDE_PLAYER_TEST_COMMON_H_
#define EXAMPLES_PLAYER_TEST_INCLUDE_PLAYER_TEST_COMMON_H_

#include "audio/audio_high_level_api.h"
#include "memutils/simple_fifo/CMN_SimpleFifo.h"
#include <dirent.h>


/* Definition of FIFO info.
 * The FIFO defined here refers to the buffer to pass playback data
 * between application and AudioSubSystem.
 */

/* Recommended frame size differs for each codec.
 *    MP3       : 1440 bytes
 *    AAC       : 1024 bytes
 *    WAV 16bit : 2560 bytes
 *    WAV 24bit : 3840 bytes
 * If the codec to be played is fixed, use this size.
 * When playing multiple codecs, select the largest size.
 * There is no problem increasing the size. If it is made smaller,
 * FIFO under is possibly generated, so it is necessary to be careful.
 */
#define FIFO_FRAME_SIZE  3840

/* The number of elements in the FIFO queue used varies depending on
 * the performance of the system and must be sufficient.
 */
#define FIFO_ELEMENT_NUM  10

/* Definition depending on player mode. */
#define FIFO_FRAME_NUM  1

/* FIFO control value. */
#define FIFO_ELEMENT_SIZE  (FIFO_FRAME_SIZE * FIFO_FRAME_NUM)
#define FIFO_QUEUE_SIZE    (FIFO_ELEMENT_SIZE * FIFO_ELEMENT_NUM)

#define PLAYER_NUM 2


/* For FIFO */
struct player_fifo_info_s
{
  CMN_SimpleFifoHandle          handle;
  AsPlayerInputDeviceHdlrForRAM input_device;
  uint32_t fifo_area[FIFO_QUEUE_SIZE/sizeof(uint32_t)];
  uint8_t  read_buf[FIFO_ELEMENT_SIZE];
};

struct Track
{
  char title[64];
  uint8_t   channel_number;  /* Channel number. */
  uint8_t   bit_length;      /* Bit length.     */
  uint32_t  sampling_rate;   /* Sampling rate.  */
  uint8_t   codec_type;      /* Codec type.     */
};

/* For play file */
struct player_file_info_s
{
  Track   track;
  int32_t size;
  DIR    *dirp;
  int     fd;
};

/* Player info */
struct player_info_s
{
  struct player_fifo_info_s   fifo;
  struct player_file_info_s   file;
};

extern struct player_info_s  s_player_info[PLAYER_NUM];
extern char s_audio_name[PLAYER_NUM][64];
extern volatile bool s_is_playing;


void app_init_freq_lock(void);
void app_freq_lock(void);
void app_freq_release(void);

bool app_open_contents_dir(void);
bool app_close_contents_dir(void);

bool app_init_libraries(void);
bool app_finalize_libraries(void);

bool app_init_simple_fifo(void);
int app_push_simple_fifo(AsPlayerId id, int fd);
bool app_first_push_simple_fifo(AsPlayerId id, int fd);
bool app_refill_simple_fifo(AsPlayerId id, int fd);

bool app_open_play_file(AsPlayerId i);
bool app_close_play_file(AsPlayerId i);

void app_play_process(AsPlayerId id, uint32_t play_time);
FAR void play_thread(FAR void *arg);


#endif /* EXAMPLES_PLAYER_TEST_INCLUDE_PLAYER_TEST_COMMON_H_ */
