/*
 * player_test_common.cxx
 *
 *  Created on: 2019.1.17
 *      Author: neusoft
 */

#include "include/player_test_common.h"
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arch/chip/pm.h>
#include <asmp/mpshm.h>
#include "memutils/message/Message.h"
#include "include/msgq_id.h"
#include "include/mem_layout.h"
#include "include/pool_layout.h"

using namespace MemMgrLite;


#define PLAYBACK_FILE_PATH "/mnt/sd0/AUDIO"

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

/* Play time(sec) */
#define PLAY_TIME_Sound 9
#define PLAY_TIME_m2    20

#define PLAYER_NUM 2

/* For frequency lock. */
static struct pm_cpu_freqlock_s s_player_lock;

/* For control player */
struct player_info_s  s_player_info[PLAYER_NUM];

char s_audio_name[PLAYER_NUM][64] = {"", ""};

/****************************************************************************
 * Private Data
 ****************************************************************************/
/* For share memory. */
static mpshm_t s_shm;


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

bool app_init_libraries(void)
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
    const NumLayout layout_no = 0;
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

    if (CMN_SimpleFifoOffer(&info->fifo.handle,
            (const void*)(info->fifo.read_buf),
            ret) == 0)
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

bool app_refill_simple_fifo(AsPlayerId id, int fd)
{
    int32_t ret = FIFO_RESULT_OK;
    size_t  vacant_size;

    vacant_size = CMN_SimpleFifoGetVacantSize(&s_player_info[id].fifo.handle);

    if ((vacant_size != 0) && (vacant_size > FIFO_ELEMENT_SIZE))
    {
        int push_cnt = vacant_size / FIFO_ELEMENT_SIZE;

        push_cnt = (push_cnt >= PLAYER_FIFO_PUSH_NUM_MAX) ?
                  PLAYER_FIFO_PUSH_NUM_MAX : push_cnt;

        for (int i = 0; i < push_cnt; i++)
        {
            if ((ret = app_push_simple_fifo(id, fd)) != FIFO_RESULT_OK)
            {
                break;
            }
        }
    }

    return (ret == FIFO_RESULT_OK) ? true : false;
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


void app_play_process(AsPlayerId id, uint32_t play_time)
{
    s_is_playing = true;
    /* Timer Start */
    time_t start_time;
    time_t cur_time;

    time(&start_time);

    unsigned int escaped_sec = 0;
    do
    {
        /* Check the FIFO every 2 ms and fill if there is space. */
        usleep(2 * 1000);
        time_t duration = time(&cur_time) - start_time;
        if (duration != escaped_sec)
        {
            escaped_sec = duration;
            printf("Escaped %d sec\n", escaped_sec);
        }
        if (!app_refill_simple_fifo(id, s_player_info[id].file.fd))
        {
            break;
        }
    } while(s_is_playing && (time(&cur_time) - start_time) < play_time);
}

FAR void play_thread(FAR void *arg)
{
    AsPlayerId id = *((AsPlayerId*)arg);
    uint32_t playtime = id == AS_PLAYER_ID_0 ? PLAY_TIME_m2 : PLAY_TIME_Sound;
    /* Running... */
    printf("Running time is %d sec\n", playtime);
    app_play_process(id, playtime);
}




