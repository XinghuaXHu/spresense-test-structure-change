/****************************************************************************
 * examples/asmp_test/asmp_test_main.c
 *
 *   Copyright (C) 2016 Sony Corporation. All rights reserved.
 *   Author: Nobuto Kobayashi <Nobuto.Kobayashi@sony.com>
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
 * 3. Neither the name NuttX nor Sony nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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
#include <unistd.h>
#include <fcntl.h>
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

#include <pthread.h>
#include "uart2.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
pthread_mutex_t mutex;

#ifdef CONFIG_FS_ROMFS
#  include "worker/romfs.h"

#  define SECTORSIZE   512
#  define NSECTORS(b)  (((b)+SECTORSIZE-1)/SECTORSIZE)
#  define MOUNTPT "/romfs"
#endif

#ifndef MOUNTPT
#  define MOUNTPT "/mnt/sd0"
#endif

/* MP object keys. Must be synchronized with worker. */

#define KEY_MQ     5
#define KEY_OFFSET 3

#define MSG_ID_SAYHELLO 1

/* Check configuration.  This is not all of the configuration settings that
 * are required -- only the more obvious.
 */

#if CONFIG_NFILE_DESCRIPTORS < 1
#  error "You must provide file descriptors via CONFIG_NFILE_DESCRIPTORS in your configuration file"
#endif

#define message(format, ...)    printf(format, ##__VA_ARGS__)
#define err(format, ...)        fprintf(stderr, format, ##__VA_ARGS__)

#define CPUMAX 5

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static char fullpath[CPUMAX][128];
static pthread_t cpuload_thid;

/****************************************************************************
 * Symbols from Auto-Generated Code
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int run_worker(const char *filename, int cpu)
{
  mptask_t mptask;
  mpmq_t mq;
  uint32_t msgdata;
  int ret, wret;
  int fd_sd;
  int fd_flash;
  struct timespec curtime;   /* for clock_gettime */
  struct tm       *curgmt;   /* for gmtime        */
  char   logname[26]={0};
  char   logname_f[27]={0};
  char   loginfo[21]={0};
  int cpuid;

  /* Make directory for logfile */

  sprintf(logname, "/mnt/sd0/CPU%d.TXT", cpu);
  sprintf(logname_f, "/mnt/spif/CPU%d.TXT", cpu);

  remove(logname);
  fd_sd = open(logname, O_RDWR | O_CREAT);
  if (fd_sd < 0)
    {
      printf("CPU log create error\n");
      printf(" err sd %d\n", fd_sd);
      printf(" err sd %d\n", errno);
    }

  remove(logname_f);
  fd_flash = open(logname_f, O_RDWR | O_CREAT);
  if (fd_flash < 0)
    {
      printf("CPU log create error\n");
      printf(" err flash %d\n", fd_flash);
      printf(" err flash %d\n", errno);
    }

  for (;;)
    {

      /* Initialize MP task */
      pthread_mutex_lock(&mutex);

      ret = mptask_init(&mptask, filename);
      if (ret != 0)
        {
          printf("mptask_init() failure. %d\n", ret);
          return ret;
        }

      ret = mptask_assign(&mptask);
      if (ret != 0)
        {
          printf("mptask_asign() failure. %d\n", ret);
          return ret;
        }

      cpuid = mptask_getcpuid(&mptask);

      /* Initialize MP message queue with asigned CPU ID, and bind it to MP task */

      ret = mpmq_init(&mq, KEY_OFFSET*cpu + KEY_MQ, cpuid);
      if (ret < 0)
        {
          printf("mpmq_init() failure. %d\n", ret);
          return ret;
        }

      ret = mptask_bindobj(&mptask, &mq);
      if (ret < 0)
        {
          printf("mptask_bindobj(mq) failure. %d\n", ret);
          return ret;
        }

      /* Run worker */

      ret = mptask_exec(&mptask);
      if (ret < 0)
        {
          printf("mptask_exec() failure. %d\n", ret);
          return ret;
        }
      pthread_mutex_unlock(&mutex);

      /* Wait for worker message */

      ret = mpmq_receive(&mq, &msgdata);
      if (ret < 0)
        {
          err("mpmq_recieve() failure. %d\n", ret);
          return ret;
        }
      message("Worker response: ID = %d, data = %08x, cpuid=%d\n",
              ret, msgdata, cpuid);

      /* Save date to SD card and flash memory */

      clock_gettime(CLOCK_REALTIME, &curtime);
      curgmt = gmtime(&curtime.tv_sec);
      sprintf(loginfo,
              "CPU%d %02d/%02d %02d:%02d:%02d\n",
              cpuid,
              curgmt->tm_mon  + 1,
              curgmt->tm_mday,
              curgmt->tm_hour,
              curgmt->tm_min,
              curgmt->tm_sec);

      write(fd_sd, loginfo, strlen(loginfo));
      fsync(fd_sd);
      usleep(1);
      write(fd_flash, loginfo, strlen(loginfo));
      fsync(fd_flash);
      usleep(1);
      uart2_output(loginfo);

      /* Destroy worker */
      pthread_mutex_lock(&mutex);

      wret = -106;
      ret = mptask_destroy(&mptask, true, &wret);
      if (ret < 0)
        {
          err("mptask_destroy() failure. %d\n", ret);
          return ret;
        }

      message("Worker exit status = %d\n", wret);

      /* Finalize all of MP objects */

      mpmq_destroy(&mq);
      pthread_mutex_unlock(&mutex);

    }

  /* Close log files */

  close(fd_sd);
  close(fd_flash);

  return 0;
}

static void eachcpu_thread(int cpu)
{
  (void) run_worker(fullpath[cpu], cpu);

  return;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: asmp_main
 ****************************************************************************/

int asmp(char *argv)
{
  int cpu;
  int cpunum = 3;
  pthread_mutex_init(&mutex, NULL);

#ifdef CONFIG_FS_ROMFS
  int ret;
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

  if ((strcmp(argv, "mp3") == 0) || (strcmp(argv, "wav") == 0))
    {
      cpunum--;
    }

  for (cpu = 0; cpu < cpunum; cpu++)
    {
      snprintf(fullpath[cpu], 128, "%s/%s%d", MOUNTPT, "hello", cpu + 1);

      ret = pthread_create(&cpuload_thid,
                           NULL,
                           (pthread_startroutine_t)eachcpu_thread,
                           (pthread_addr_t) cpu);
      if (ret != 0)
        {
          printf("Failed to create thread for cpuload. ret=%d\n", ret);
          return ret;
        }
      sleep(1);
    }
  return 0;
}
