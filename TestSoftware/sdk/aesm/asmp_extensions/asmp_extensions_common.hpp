/*
 * asmp_extensions_common.hpp
 *
 *  Created on: 2019年3月13日
 *      Author: uas
 */

#ifndef EXAMPLES_ASMP_EXTENSIONS_ASMP_EXTENSIONS_COMMON_HPP_
#define EXAMPLES_ASMP_EXTENSIONS_ASMP_EXTENSIONS_COMMON_HPP_

#include <asmp/mptask.h>
#include <asmp/mpshm.h>
#include <asmp/mpmq.h>
#include <asmp/mpmutex.h>

#define WORKER_CPU_NUM 5

extern char g_workerpath[128];

enum MessageID
{
    MSG_ID_SAYHELLO = 1,
    MSG_ID_WRITE_SHM_START,
    MSG_ID_WRITE_SHM_DONE,
    MSG_ID_READ_SHM_START,
    MSG_ID_READ_SHM_DONE,
    MSG_ID_LOCK_START,
    MSG_ID_LOCK_DONE,
    MSG_ID_UNLOCK_START,
    MSG_ID_UNLOCK_DONE,
    MSG_ID_SEND,
    MSG_ID_SEND_DONE,
    MSG_ID_RECEIVE,
    MSG_ID_RECEIVE_DONE,
    MSG_ID_DO_SEND,
    MSG_ID_WORKER_QUIT,
};


typedef struct {
  mptask_t task;
  mpmutex_t  sem;
  mpmq_t   mq;
  mpshm_t g_shm;
  cpuid_t cpuid;
  key_t mq_key;
  key_t mutex_key;
  key_t shm_key;
  size_t g_shm_size;
  char *g_shared;
  mptask_attr_t attr;
} mptask_info_t;

extern mptask_info_t g_mptasks[WORKER_CPU_NUM];

bool app_init();

#endif /* EXAMPLES_ASMP_EXTENSIONS_ASMP_EXTENSIONS_COMMON_HPP_ */
