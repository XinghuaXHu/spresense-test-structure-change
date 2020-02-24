/*
 * player_test_common.cxx
 *
 *  Created on: 2019.1.17
 *      Author: neusoft
 */

#include "include/audio_extensions_test_common.h"

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arch/chip/pm.h>
#include <asmp/mpshm.h>
#include "include/memory_layout.h"
#include "include/mem_layout.h"
#include "include/msgq_id.h"
#include "include/pool_layout.h"
#include "memutils/message/Message.h"
#include "container_format_lib/wav_containerformat.h"

using namespace MemMgrLite;


#define PLAYBACK_FILE_PATH "/mnt/sd0/AUDIO"

/* Length of recording file name */
#define MAX_PATH_LENGTH 128

/* For line buffer mode. */
#define STDIO_BUFFER_SIZE 4096

/* The number of elements pushed to the FIFO Queue at a time depends
 * on the load of the system. If the number of elements is small,
 * we will underflow if application dispatching is delayed.
 * Define a sufficient maximum value.
 */
#define PLAYER_FIFO_PUSH_NUM_MAX  5

/* Local error code. */
#define FIFO_RESULT_OK  0
#define FIFO_RESULT_ERR 1
#define FIFO_RESULT_EOF 2
#define FIFO_RESULT_FUL 3

#define PLAYER_NUM 2

bool g_is_recorder = false;

/* For control player */
struct player_info_s  s_player_info[PLAYER_NUM];
struct recorder_info_s s_recorder_info;

char s_audio_name[PLAYER_NUM][64] = {"", ""};

/* For frequency lock. */
static struct pm_cpu_freqlock_s s_player_lock;

/* For share memory. */
static mpshm_t s_shm;

static WavContainerFormat* s_container_format = NULL;
static WAVHEADER  s_wav_header;


void app_init_freq_lock(void)
{
    s_player_lock.count = 0;
    s_player_lock.info = PM_CPUFREQLOCK_TAG('A', 'P', 0);
    s_player_lock.flag = PM_CPUFREQLOCK_FLAG_HV;
}

void app_freq_lock(void)
{
    up_pm_acquire_freqlock(&s_player_lock);
}

void app_freq_release(void)
{
    up_pm_release_freqlock(&s_player_lock);
}

bool app_open_contents_dir(void)
{
    const char *name = PLAYBACK_FILE_PATH;

    DIR *dirp = opendir(name);
    if (!dirp)
    {
        printf("Error: %s directory path error. check the path!\n", name);
        return false;
    }

    s_player_info[0].file.dirp = dirp;
    return true;
}

bool app_close_contents_dir(void)
{
    closedir(s_player_info[0].file.dirp);
    return true;
}

bool app_init_libraries(bool is_recorder)
{
    int ret;
    uint32_t addr = AUD_SRAM_ADDR;

    /* Initialize shared memory.*/
    ret = mpshm_init(&s_shm, 1, 1024 * 128 * 2);
    if (ret < 0)
    {
        printf("Error: mpshm_init() failure. %d\n", ret);
        return false;
    }

    ret = mpshm_remap(&s_shm, (void *)addr);
    if (ret < 0)
    {
        printf("Error: mpshm_remap() failure. %d\n", ret);
        return false;
    }

    /* Initalize MessageLib. */
    err_t err = MsgLib::initFirst(NUM_MSGQ_POOLS, MSGQ_TOP_DRM);
    if (err != ERR_OK)
    {
        printf("Error: MsgLib::initFirst() failure. 0x%x\n", err);
        return false;
    }

    err = MsgLib::initPerCpu();
    if (err != ERR_OK)
    {
      printf("Error: MsgLib::initPerCpu() failure. 0x%x\n", err);
      return false;
    }

    void* mml_data_area = translatePoolAddrToVa(MEMMGR_DATA_AREA_ADDR);
    err = Manager::initFirst(mml_data_area, MEMMGR_DATA_AREA_SIZE);
    if (err != ERR_OK)
    {
        printf("Error: Manager::initFirst() failure. 0x%x\n", err);
        return false;
    }

    err = Manager::initPerCpu(mml_data_area, NUM_MEM_POOLS);
    if (err != ERR_OK)
    {
        printf("Error: Manager::initPerCpu() failure. 0x%x\n", err);
        return false;
    }

    /* Create static memory pool of VoiceCall. */
    NumLayout layout_no = MEM_LAYOUT_PLAYER;
    if (is_recorder)
    {
        layout_no = MEM_LAYOUT_RECORDER;
    }
    void* work_va = translatePoolAddrToVa(MEMMGR_WORK_AREA_ADDR);
    err = Manager::createStaticPools(layout_no,
                             work_va,
                             MEMMGR_MAX_WORK_SIZE,
                             MemoryPoolLayouts[layout_no]);
    if (err != ERR_OK)
    {
        printf("Error: Manager::initPerCpu() failure. %d\n", err);
        return false;
    }

    return true;
}

bool app_finalize_libraries(void)
{
  /* Finalize MessageLib. */
  MsgLib::finalize();

  /* Destroy static pools. */
  MemMgrLite::Manager::destroyStaticPools();

  /* Finalize memory manager. */
  MemMgrLite::Manager::finalize();

  /* Destroy shared memory. */
  int ret;
  ret = mpshm_detach(&s_shm);
  if (ret < 0)
    {
      printf("Error: mpshm_detach() failure. %d\n", ret);
      return false;
    }

  ret = mpshm_destroy(&s_shm);
  if (ret < 0)
    {
      printf("Error: mpshm_destroy() failure. %d\n", ret);
      return false;
    }

  return true;
}

static void app_input_device_callback(uint32_t size)
{
    /* do nothing */
}

bool app_init_simple_fifo(void)
{
    if (!g_is_recorder)
    {
        for (int i=0; i<PLAYER_NUM; i++)
        {
            if (CMN_SimpleFifoInitialize(&s_player_info[i].fifo.handle,
                                       s_player_info[i].fifo.fifo_area,
                                       FIFO_QUEUE_SIZE,
                                       NULL) != 0)
            {
                printf("Error: Fail to initialize simple FIFO.");
                return false;
            }
            CMN_SimpleFifoClear(&s_player_info[i].fifo.handle);

            s_player_info[i].fifo.input_device.simple_fifo_handler = (void*)(&s_player_info[i].fifo.handle);
            s_player_info[i].fifo.input_device.callback_function = app_input_device_callback;
        }
    }
    else
    {
        if (CMN_SimpleFifoInitialize(&s_recorder_info.fifo.handle,
                                     s_recorder_info.fifo.fifo_area,
                                     SIMPLE_FIFO_BUF_SIZE, NULL) != 0)
        {
            printf("Error: Fail to initialize simple FIFO.");
            return false;
        }
        CMN_SimpleFifoClear(&s_recorder_info.fifo.handle);

        s_recorder_info.fifo.output_device.simple_fifo_handler = (void*)(&s_recorder_info.fifo.handle);
        s_recorder_info.fifo.output_device.callback_function = app_input_device_callback;
    }

    return true;
}

int app_push_simple_fifo(AsPlayerId id, int fd)
{
    player_info_s* info = &s_player_info[id];
    int ret = read(fd, &info->fifo.read_buf, FIFO_ELEMENT_SIZE);
    if (ret < 0)
    {
        printf("Error: Fail to read file. errno:%d\n", get_errno());
        return FIFO_RESULT_ERR;
    }

    if (CMN_SimpleFifoOffer(&info->fifo.handle, (const void*)(info->fifo.read_buf), ret)
            == 0)
    {
        return FIFO_RESULT_FUL;
    }
    info->file.size = (info->file.size - ret);
    if (info->file.size == 0)
    {
        return FIFO_RESULT_EOF;
    }
    return FIFO_RESULT_OK;
}

bool app_first_push_simple_fifo(AsPlayerId id, int fd)
{
    int i;
    int ret = 0;

    for(i = 0; i < FIFO_ELEMENT_NUM - 1; i++)
    {
        if ((ret = app_push_simple_fifo(id, fd)) != FIFO_RESULT_OK)
        {
            break;
        }
    }

    return (ret != FIFO_RESULT_ERR) ? true : false;
}

int app_refill_simple_fifo(AsPlayerId id, int fd)
{
    int ret = FIFO_RESULT_OK;
    size_t vacant_size = CMN_SimpleFifoGetVacantSize(&s_player_info[id].fifo.handle);

    if ((vacant_size != 0) && (vacant_size > FIFO_ELEMENT_SIZE))
    {
        int push_cnt = vacant_size / FIFO_ELEMENT_SIZE;

        push_cnt = (push_cnt >= PLAYER_FIFO_PUSH_NUM_MAX) ? PLAYER_FIFO_PUSH_NUM_MAX : push_cnt;

        for (int i = 0; i < push_cnt; i++)
        {
            if ((ret = app_push_simple_fifo(id, fd)) != FIFO_RESULT_OK)
            {
                break;
            }
        }
    }

    return ret;
}

static int app_play_file_open(FAR const char *file_path, FAR int32_t *file_size)
{
  int fd = open(file_path, O_RDONLY);

  *file_size = 0;
  if (fd >= 0)
    {
      struct stat stat_buf;
      if (stat(file_path, &stat_buf) == OK)
        {
          *file_size = stat_buf.st_size;
        }
    }

  return fd;
}

bool app_open_play_file(AsPlayerId i)
{
    snprintf(s_player_info[i].file.track.title, sizeof(s_player_info[i].file.track.title), "%s", s_audio_name[i]);

    char full_path[128];
    snprintf(full_path,
           sizeof(full_path),
           "%s/%s",
           PLAYBACK_FILE_PATH,
           s_player_info[i].file.track.title);

    s_player_info[i].file.fd = app_play_file_open(full_path, &s_player_info[i].file.size);
    if (s_player_info[i].file.fd < 0)
    {
        printf("Error: %s open error. check paths and files!\n", full_path);
        return false;
    }
    if (s_player_info[i].file.size == 0)
    {
        close(s_player_info[i].file.fd);
        printf("Error: %s file size is abnormal. check files!\n",full_path);
        return false;
    }
    /* Push data to simple fifo */
    if (!app_first_push_simple_fifo((AsPlayerId)i, s_player_info[i].file.fd))
    {
        printf("Error: app_first_push_simple_fifo() failure.\n");
        CMN_SimpleFifoClear(&s_player_info[i].fifo.handle);
        close(s_player_info[i].file.fd);
        return false;
    }

    return true;
}

bool app_close_play_file(AsPlayerId i)
{
    if (close(s_player_info[i].file.fd) != 0)
    {
      printf("Error: close() failure.\n");
      return false;
    }
    CMN_SimpleFifoClear(&s_player_info[i].fifo.handle);

    return true;
}


void app_play_process(AsPlayerId id)
{
    s_is_playing[id] = PLAY_STATE_PLAYING;
    /* Timer Start */
    time_t start_time;
    time_t cur_time;

    time(&start_time);

    int ret = FIFO_RESULT_OK;

    unsigned int escaped_sec = 0;
    do
    {
        /* Check the FIFO every 2 ms and fill if there is space. */
        usleep(2 * 1000);
        time_t duration = time(&cur_time) - start_time;
        if (duration != escaped_sec)
        {
            escaped_sec = duration;
            printf("Player%d escaped %d sec\n", id, escaped_sec);
        }
        ret = app_refill_simple_fifo(id, s_player_info[id].file.fd);
    } while(s_is_playing[id] == PLAY_STATE_PLAYING && ret == FIFO_RESULT_OK);

    if (ret == FIFO_RESULT_EOF)
    {
        printf("Player%d End of audio file\n", id);
    }
}

FAR void play_thread(FAR void *arg)
{
    AsPlayerId id = *((AsPlayerId*)arg);
    app_play_process(id);
}

void app_record_process()
{
    g_is_recording = true;

    /* Timer Start */
    time_t start_time;
    time_t cur_time;

    time(&start_time);

    unsigned int escaped_sec = 0;
    do
    {
        /* Check the FIFO every 5 ms and fill if there is space. */
        usleep(5 * 1000);
        time_t duration = time(&cur_time) - start_time;
        if (duration != escaped_sec)
        {
            escaped_sec = duration;
            printf("Recorder escaped %d sec\n", escaped_sec);
        }
        app_pop_simple_fifo();

    } while(g_is_recording);
}

FAR void record_thread(FAR void *arg)
{
    app_record_process();
}

void app_pop_simple_fifo(void)
{
  size_t occupied_simple_fifo_size =
    CMN_SimpleFifoGetOccupiedSize(&s_recorder_info.fifo.handle);
  uint32_t output_size = 0;

  while (occupied_simple_fifo_size > 0)
    {
      output_size = (occupied_simple_fifo_size > READ_SIMPLE_FIFO_SIZE) ?
        READ_SIMPLE_FIFO_SIZE : occupied_simple_fifo_size;
      app_write_output_file(output_size);
      occupied_simple_fifo_size -= output_size;
    }
}

bool app_update_wav_file_size(void)
{
  fseek(s_recorder_info.file.fd, 0, SEEK_SET);

  s_container_format->getHeader(&s_wav_header, s_recorder_info.file.size);

  size_t ret = fwrite((const void *)&s_wav_header,
                      1,
                      sizeof(WAVHEADER),
                      s_recorder_info.file.fd);
  if (ret != sizeof(WAVHEADER))
    {
      printf("Fail to write file(wav header)\n");
      return false;
    }
  return true;
}

static bool app_write_wav_header(void)
{
  s_container_format->getHeader(&s_wav_header, 0);

  size_t ret = fwrite((const void *)&s_wav_header,
                      1,
                      sizeof(WAVHEADER),
                      s_recorder_info.file.fd);
  if (ret != sizeof(WAVHEADER))
    {
      printf("Fail to write file(wav header)\n");
      return false;
    }
  return true;
}

static bool app_init_wav_header(void)
{
  return s_container_format->init(FORMAT_ID_PCM,
                                 s_recorder_info.file.channel_number,
                                 s_recorder_info.file.sampling_rate);
}

bool app_open_output_file(const char* filename)
{
    char fname[MAX_PATH_LENGTH];
/*
  if ((s_recorder_info.file.codec_type == AS_CODECTYPE_MP3) &&
      (s_recorder_info.file.format_type == FORMAT_TYPE_RAW))
    {
      snprintf(fname, MAX_PATH_LENGTH, "%s/%04d%02d%02d_%02d%02d%02d.mp3",
    }
  else if (s_recorder_info.file.codec_type == AS_CODECTYPE_LPCM)
    {
      if (s_recorder_info.file.format_type == FORMAT_TYPE_WAV)
        {
          snprintf(fname, MAX_PATH_LENGTH, "%s/%04d%02d%02d_%02d%02d%02d.wav",
        }
      else
        {
          snprintf(fname, MAX_PATH_LENGTH, "%s/%04d%02d%02d_%02d%02d%02d.pcm",
        }
    }
  else if ((s_recorder_info.file.codec_type == AS_CODECTYPE_OPUS) &&
           (s_recorder_info.file.format_type == FORMAT_TYPE_RAW))
    {
      snprintf(fname, MAX_PATH_LENGTH, "%s/%04d%02d%02d_%02d%02d%02d.raw",
    }
  else
    {
      printf("Unsupported format\n");
      return false;
    }
*/
    snprintf(fname, MAX_PATH_LENGTH, "%s/%s", PLAYBACK_FILE_PATH, filename);

    s_recorder_info.file.fd = fopen(fname, "w");
    if (s_recorder_info.file.fd == 0)
    {
        printf("open err(%s)\n", fname);
        return false;
    }
    setvbuf(s_recorder_info.file.fd, NULL, _IOLBF, STDIO_BUFFER_SIZE);
    printf("Record data to %s.\n", &fname[0]);

    if (s_recorder_info.file.format_type == FORMAT_TYPE_WAV)
    {
      if (!app_init_wav_header())
      {
        printf("Error: app_init_wav_header() failure.\n");
        return false;
      }

      if (!app_write_wav_header())
      {
        printf("Error: app_write_wav_header() failure.\n");
        return false;
      }
  }
  return true;
}

void app_close_output_file(void)
{
  fclose(s_recorder_info.file.fd);
}

void app_write_output_file(uint32_t size)
{
  ssize_t ret;

  if (size == 0 || CMN_SimpleFifoGetOccupiedSize(&s_recorder_info.fifo.handle) == 0)
    {
      return;
    }

  if (CMN_SimpleFifoPoll(&s_recorder_info.fifo.handle,
                        (void*)s_recorder_info.fifo.write_buf,
                        size) == 0)
    {
      printf("ERROR: Fail to get data from simple FIFO.\n");
      return;
    }

  ret = fwrite(s_recorder_info.fifo.write_buf, 1, size, s_recorder_info.file.fd);
  if (ret < 0) {
    printf("ERROR: Cannot write recorded data to output file.\n");
    app_close_output_file();
    return;
  }
  s_recorder_info.file.size += size;
}



