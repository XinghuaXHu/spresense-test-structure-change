/****************************************************************************
 * test/sqa/singlefunction/multicpu_sec/shared_memory.c
 *
 *   Copyright 2018,2019 Sony Semiconductor Solutions Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/shm.h>
#include <unistd.h>

#include <nuttx/drivers/ramdisk.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <debug.h>
#include <errno.h>

#include <asmp/asmp.h>
#include <asmp/mptask.h>
#include <asmp/mpshm.h>
#include <asmp/mpmq.h>
#include <asmp/mpmutex.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_FS_ROMFS
#  include "worker/romfs.h"

#  define SECTORSIZE   512
#  define NSECTORS(b)  (((b)+SECTORSIZE-1)/SECTORSIZE)
#  define MOUNTPT "/romfs"
#endif

#ifndef MOUNTPT
#  define MOUNTPT "/mnt/vfat/BIN"
#endif

/* MP object keys. Must be synchronized with worker. */

#define KEY_SHM   1
#define KEY_MQ    2
#define KEY_MUTEX 3

#define MSG_ID_SAYHELLO 1

/* Check configuration.  This is not all of the configuration settings that
 * are required -- only the more obvious.
 */

#if CONFIG_NFILE_DESCRIPTORS < 1
#  error "You must provide file descriptors via CONFIG_NFILE_DESCRIPTORS in your configuration file"
#endif

#define message(format, ...)    printf(format, ##__VA_ARGS__)
#define err(format, ...)        fprintf(stderr, format, ##__VA_ARGS__)

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef struct {
  mptask_t task;
  mpmutex_t  sem;
  mpmq_t   mq;
} mptask_info_t;

/****************************************************************************
 * Private Data
 ****************************************************************************/
static char fullpath[128];
static mptask_info_t mptasks[4];

/****************************************************************************
 * Symbols from Auto-Generated Code
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int setup_tasks(const char *filename, mpshm_t *shm, mptask_info_t *task)
{
  int ret;

  /* Initialize MP task */

  ret = mptask_init(&task->task, filename);
  if (ret != 0)
    {
      err("mptask_init() failure. %d\n", ret);
      return ret;
    }
  ret = mptask_assign(&task->task);
  if (ret != 0)
    {
      err("mptask_asign() failure. %d\n", ret);
      return ret;
    }
  message("assigned at CPU%d\n", mptask_getcpuid(&task->task));

  /* Initialize MP mutex and bind it to MP task */

  ret = mpmutex_init(&task->sem, KEY_MUTEX);
  if (ret < 0)
    {
      err("mpmutex_init() failure. %d\n", ret);
      return ret;
    }
  ret = mptask_bindobj(&task->task, &task->sem);
  if (ret < 0)
    {
      err("mptask_bindobj(mutex) failure. %d\n", ret);
      return ret;
    }

  /* Initialize MP message queue with asigned CPU ID, and bind it to MP task */

  ret = mpmq_init(&task->mq, KEY_MQ, mptask_getcpuid(&task->task));
  if (ret < 0)
    {
      err("mpmq_init() failure. %d\n", ret);
      return ret;
    }
  ret = mptask_bindobj(&task->task, &task->mq);
  if (ret < 0)
    {
      err("mptask_bindobj(mq) failure. %d\n", ret);
      return ret;
    }

  /* Bind it to MP task */

  ret = mptask_bindobj(&task->task, shm);
  if (ret < 0)
    {
      err("mptask_binobj(shm) failure. %d\n", ret);
      return ret;
    }

  /* Run worker */

  ret = mptask_exec(&task->task);
  if (ret < 0)
    {
      err("mptask_exec() failure. %d\n", ret);
      return ret;
    }

  return 0;
}

static int exec_tasks(mptask_info_t *task)
{
  uint32_t msgdata;
  int ret, wret;

  /* Send command to worker */

  ret = mpmq_send(&task->mq, MSG_ID_SAYHELLO, 0xdeadbeef);
  if (ret < 0)
    {
      err("mpmq_send() failure. %d\n", ret);
      return ret;
    }

  /* Wait for worker message */

  ret = mpmq_receive(&task->mq, &msgdata);
  if (ret < 0)
    {
      err("mpmq_recieve() failure. %d\n", ret);
      return ret;
    }
  message("Worker response: ID = %d, data = %08x\n",
          ret, msgdata);

  /* Show worker copied data */

  /* Lock mutex for synchronize with worker after it's started */

  mpmutex_lock(&task->sem);

  message("Worker said: %s\n", (char*)msgdata);

  mpmutex_unlock(&task->sem);

  /* Destroy worker */

  wret = -1;
  ret = mptask_destroy(&task->task, false, &wret);
  if (ret < 0)
    {
      err("mptask_destroy() failure. %d\n", ret);
      return ret;
    }

  message("Worker exit status = %d\n", wret);

  /* Finalize all of MP objects */

  mpmutex_destroy(&task->sem);
  mpmq_destroy(&task->mq);

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int multicpu_shared_memory(void)
{
  mpshm_t shm;
  char *shared;
  int ret;
  int i;

#ifdef CONFIG_FS_ROMFS
  struct stat buf;

  ret = stat(MOUNTPT, &buf);
  if (ret < 0)
    {
      message("Registering romdisk at /dev/ram0\n");
      ret = romdisk_register(0, (FAR uint8_t *)romfs_img,
                             NSECTORS(romfs_img_len), SECTORSIZE);
      if (ret < 0)
        {
          err("ERROR: romdisk_register failed: %d\n", ret);
          exit(1);
        }

      message("Mounting ROMFS filesystem at target=%s with source=%s\n",
              MOUNTPT, "/dev/ram0");

      ret = mount("/dev/ram0", MOUNTPT, "romfs", MS_RDONLY, NULL);
      if (ret < 0)
        {
          err("ERROR: mount(%s,%s,romfs) failed: %s\n",
              "/dev/ram0", MOUNTPT, errno);
        }
    }
#endif

#ifdef CONFIG_FS_ROMFS
    snprintf(fullpath, 128, "%s/%s", MOUNTPT, "hello");
#else
    snprintf(fullpath, 128, "%s/%s", MOUNTPT, "HELLO");
#endif

  /* Initialize MP shared memory */

  ret = mpshm_init(&shm, KEY_SHM, 1024);
  if (ret < 0)
    {
      err("mpshm_init() failure. %d\n", ret);
      return ret;
    }

  /* Map shared memory to virtual space */

  shared = mpshm_attach(&shm, 0);
  if (!shared)
    {
      err("mpshm_attach() failure.\n");
      return ret;
    }
  message("attached at %08x\n", (uintptr_t)shared);
  memset(shared, 0, 1024);

  for (i = 0; i < 4; i++)
    {
      setup_tasks(fullpath, &shm, &mptasks[i]);
    }

  for (i = 0; i < 4; i++)
    {
      exec_tasks(&mptasks[i]);
    }

  mpshm_detach(&shm);
  mpshm_destroy(&shm);

  return OK;
}
