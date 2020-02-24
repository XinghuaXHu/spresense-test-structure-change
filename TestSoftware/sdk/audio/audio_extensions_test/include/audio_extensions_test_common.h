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

/* FIFO recorder definition */
#define USE_MIC_CHANNEL_NUM 2
#define READ_SIMPLE_FIFO_SIZE (1536 * USE_MIC_CHANNEL_NUM)
#define SIMPLE_FIFO_BUF_SIZE (READ_SIMPLE_FIFO_SIZE * FIFO_ELEMENT_NUM)

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


struct recorder_fifo_info_s
{
  CMN_SimpleFifoHandle        handle;
  AsRecorderOutputDeviceHdlr  output_device;
  uint32_t fifo_area[SIMPLE_FIFO_BUF_SIZE/sizeof(uint32_t)];
  uint8_t  write_buf[READ_SIMPLE_FIFO_SIZE];
};

struct recorder_file_info_s
{
  uint32_t  sampling_rate;
  uint8_t   channel_number;
  uint8_t   codec_type;
  uint16_t  format_type;
  uint32_t  size;
  DIR      *dirp;
  FILE     *fd;
};

struct recorder_info_s
{
  struct recorder_fifo_info_s  fifo;
  struct recorder_file_info_s  file;
};

enum format_type_e
{
  FORMAT_TYPE_RAW = 0,
  FORMAT_TYPE_WAV,
  FORMAT_TYPE_OGG,
  FORMAT_TYPE_NUM
};

enum PLAY_STATE
{
    PLAY_STATE_STOP,
    PLAY_STATE_PLAYING,
    PLAY_STATE_PAUSE,
};

extern bool g_is_recorder;
extern struct player_info_s  s_player_info[PLAYER_NUM];
extern struct recorder_info_s s_recorder_info;
extern char s_audio_name[PLAYER_NUM][64];
extern volatile PLAY_STATE s_is_playing[PLAYER_NUM];
extern volatile bool g_is_recording;


void app_init_freq_lock(void);
void app_freq_lock(void);
void app_freq_release(void);

bool app_open_contents_dir(void);
bool app_close_contents_dir(void);

bool app_init_libraries(bool is_recorder);
bool app_finalize_libraries(void);

bool app_init_simple_fifo(void);
int app_push_simple_fifo(AsPlayerId id, int fd);
bool app_first_push_simple_fifo(AsPlayerId id, int fd);
int app_refill_simple_fifo(AsPlayerId id, int fd);

bool app_open_play_file(AsPlayerId i);
bool app_close_play_file(AsPlayerId i);

void app_play_process(AsPlayerId id);
FAR void play_thread(FAR void *arg);

void app_record_process();
FAR void record_thread(FAR void *arg);

bool app_open_output_file(const char* filename);
void app_close_output_file(void);
void app_write_output_file(uint32_t size);
bool app_update_wav_file_size(void);
void app_pop_simple_fifo(void);


#endif /* EXAMPLES_PLAYER_TEST_INCLUDE_PLAYER_TEST_COMMON_H_ */
