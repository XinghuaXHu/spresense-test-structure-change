/****************************************************************************
 * examples/modem_diag/modem_diag_main.c
 *
 *   Copyright (C) 2017 Sony Corporation
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

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

#include <nuttx/modem/alt1160.h>
#include <nuttx/arch.h>
#include <nuttx/sched.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ALT_1160_DEVPATH      "/dev/alt1160"
#define BUFF_MAX              1516

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct alt1160_pm_wakelock_s lock;

/****************************************************************************
 * Private Functions
 ****************************************************************************/


static void pm_callback(uint32_t state)
{
  printf("%s called, state:%d\n", __func__, state);
}

static int read_test(int read_size)
{
  int ret;
  int i;
  int fd;
  char *read_buff;

  read_buff = malloc(read_size);
  if (read_buff == NULL)
    {
      printf("malloc faild\n");
      return -1;
    }

  fd = open(ALT_1160_DEVPATH, O_RDONLY);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = read(fd, read_buff, read_size);
  if (ret == -1)
    {
      printf("read() failed\n");
    }
  else
    {
      printf("read success:%d\n", ret);
      for (i = 0; i < ret; i++)
        {
          printf("0x%x ", read_buff[i]);
        }
      printf("\n");
    }

  close(fd);

  return 0;
}

static int write_test(char *write_buff, int write_size)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = write(fd, write_buff, write_size);
  if (ret == -1)
    {
      printf("write() failed\n");
    }
  else
    {
      printf("write success:%d\n", ret);
    }

  close(fd);

  return 0;
}

static int poweron_test(void)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_POWERON, 0);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  close(fd);

  return 0;
}

static int poweroff_test(void)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_POWEROFF, 0);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  close(fd);

  return 0;
}

static int reset_test(void)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_POWEROFF, 0);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  ret = ioctl(fd, MODEM_IOC_POWERON, 0);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  close(fd);

  return 0;
}

static int read_abort_test(void)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_READABORT, 0);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  close(fd);

  return 0;
}

static int register_callback_test(void)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_PM_REGISTERCB, (unsigned long)pm_callback);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  close(fd);

  return 0;
}

static int deregister_callback_test(void)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_PM_DEREGISTERCB, 0);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  close(fd);

  return 0;
}

static int sleep_req_test(void)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_SLEEP, 0);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  close(fd);

  return 0;
}

static int get_pm_state_test(void)
{
  int ret;
  int fd;
  uint32_t state;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_PM_GETSTATE, (unsigned long)&state);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
      printf("get modem state:%d\n", state);
    }

  close(fd);

  return 0;
}

static int init_wakelock_test(void)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_PM_INITWAKELOCK, (unsigned long)&lock);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  close(fd);

  return 0;
}

static int acqure_wakelock_test(void)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_PM_ACQUIREWAKELOCK, (unsigned long)&lock);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  close(fd);

  return 0;
}

static int release_wakelock_test(void)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_PM_RELEASEWAKELOCK, (unsigned long)&lock);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  close(fd);

  return 0;
}

static int get_numof_wakelock_test(void)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_PM_GETNUMOFWAKELOCK, (unsigned long)&lock);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  close(fd);

  return 0;
}

static int get_wakelock_status_test(void)
{
  int ret;
  int fd;

  fd = open(ALT_1160_DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ALT_1160_DEVPATH, fd);
      return -1;
    }

  ret = ioctl(fd, MODEM_IOC_PM_GETWAKELOCKSTATE, 0);
  if (ret < 0)
    {
      printf("ioctl failed:%d\n", ret);
    }
  else
    {
      printf("ioctl success:%d\n", ret);
    }

  close(fd);

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
/****************************************************************************
 * modme_diag_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int mdiag_main(int argc, char *argv[])
#endif
{
  int cmp_res;
  char *write_buff = "1234567890";
  int read_size = BUFF_MAX;
  int write_size = strlen(write_buff);

  if (argc >= 2)
    {
      /* read command */

      cmp_res = strncmp(argv[1], "read", 4);
      if (cmp_res == 0)
        {
          if (argc >= 3)
            {
              read_size = atoi(argv[2]);
            }
          read_test(read_size);
        }

      /* write command */

      cmp_res = strncmp(argv[1], "write", 5);
      if (cmp_res == 0)
        {
          if (argc >= 3)
            {
              write_size = strlen(argv[2]);
              write_buff = argv[2];
            }
          write_test(write_buff, write_size);
        }

      /* ioctl command */

      cmp_res = strncmp(argv[1], "ioctl", 5);
      if (cmp_res == 0)
        {
          if (argc >= 3)
            {
              /* power on command */

              cmp_res = strncmp(argv[2], "poweron", 7);
              if (cmp_res == 0)
                {
                  poweron_test();
                }

              /* power off command */

              cmp_res = strncmp(argv[2], "poweroff", 8);
              if (cmp_res == 0)
                {
                  poweroff_test();
                }

              /* reset command */

              cmp_res = strncmp(argv[2], "reset", 5);
              if (cmp_res == 0)
                {
                  reset_test();
                }

              /* read abort command */

              cmp_res = strncmp(argv[2], "rabort", 6);
              if (cmp_res == 0)
                {
                  read_abort_test();
                }

              /* register cb command */

              cmp_res = strncmp(argv[2], "regcb", 5);
              if (cmp_res == 0)
                {
                  register_callback_test();
                }

              /* deregister cb command */

              cmp_res = strncmp(argv[2], "deregcb", 8);
              if (cmp_res == 0)
                {
                  deregister_callback_test();
                }

              /* sleep command */

              cmp_res = strncmp(argv[2], "sleep", 5);
              if (cmp_res == 0)
                {
                  sleep_req_test();
                }

              /* get power state command */

              cmp_res = strncmp(argv[2], "getst", 5);
              if (cmp_res == 0)
                {
                  get_pm_state_test();
                }

              /* initialize wakelock command */

              cmp_res = strncmp(argv[2], "ilock", 5);
              if (cmp_res == 0)
                {
                  init_wakelock_test();
                }

              /* acquire wakelock command */

              cmp_res = strncmp(argv[2], "alock", 5);
              if (cmp_res == 0)
                {
                  acqure_wakelock_test();
                }

              /* release wakelock command */

              cmp_res = strncmp(argv[2], "rlock", 5);
              if (cmp_res == 0)
                {
                  release_wakelock_test();
                }

              /* get number of wakelock command */

              cmp_res = strncmp(argv[2], "getnlock", 8);
              if (cmp_res == 0)
                {
                  get_numof_wakelock_test();
                }

              /* get wakelock status command */

              cmp_res = strncmp(argv[2], "getlocksts", 10);
              if (cmp_res == 0)
                {
                  get_wakelock_status_test();
                }
            }
        }
    }

  return 0;
}
