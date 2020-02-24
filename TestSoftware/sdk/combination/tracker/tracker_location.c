/****************************************************************************
 * demo/collet_box/tracker/tracker_location.c
 *
 *   Copyright (C) 2017 Sony Corporation. All rights reserved.
 *   Author: Yutaka Miyajima <Yutaka.Miyajima@sony.com>
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
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <nuttx/config.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <poll.h>
#include <arch/chip/gnss.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MY_GNSS_SIG               18

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef struct
{
  int8_t   sing;
  uint8_t  degree;
  uint8_t  minute;
  uint32_t frac;
} ST_DMS;

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct cxd56_gnss_positiondata_s posdat;
static uint32_t                         posfixflag;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void double_to_dmf(double x, ST_DMS *dmf)
{
  int    b, d, m;
  double f, t;

  if (x < 0)
    {
      b = 1;
      x = -x;
    }
  else
    {
      b = 0;
    }
  d = (int)x; /* = floor(x), x is always positive */
  t = (x - d) * 60;
  m = (int)t; /* = floor(t), t is always positive */
  f = (t - m) * 10000;

  dmf->sing   = b;
  dmf->degree = d;
  dmf->minute = m;
  dmf->frac   = f;
}

static int read_and_print(int fd)
{
  ST_DMS dmf;
  int    ret;

  ret = read(fd, &posdat, sizeof(posdat));
  if (ret < 0)
    {
      printf("read error\n");
      goto _err;
    }

  printf(">Hour:%d, minute:%d, sec:%d, usec:%d\n",
         posdat.receiver.time.hour, posdat.receiver.time.minute,
         posdat.receiver.time.sec, posdat.receiver.time.usec);
  if (posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
    {
      posfixflag = 1;

      double_to_dmf(posdat.receiver.latitude, &dmf);
      printf(">LAT %d.%d.%04d\n", dmf.degree, dmf.minute, dmf.frac);

      double_to_dmf(posdat.receiver.longitude, &dmf);
      printf(">LNG %d.%d.%04d\n", dmf.degree, dmf.minute, dmf.frac);
    }
  else
    {
      printf(">No Positioning Data\n");
    }

_err:
  return ret;
}

static void positioned(int signo, FAR siginfo_t *info, FAR void *ucontext)
{
  FAR struct cxd56_gnss_signal_info_s *gnssinfo;

#ifdef CONFIG_CAN_PASS_STRUCTS
  gnssinfo = (FAR struct cxd56_gnss_signal_info_s *)info->si_value.sival_int;
#else
  gnssinfo = (FAR struct cxd56_gnss_signal_info_s *)info->si_value.sival_ptr;
#endif

  read_and_print(gnssinfo->fd);
}

static int gnss_start(void)
{
  int fd;
  int ret;
  sigset_t                         mask;
  struct sigaction                 sa;
  struct cxd56_gnss_signal_setting_s setting;

  fd = open("/dev/gps", O_RDONLY);
  if (fd <= 0)
    {
      printf("open error:%d,%d\n", fd, errno);
      return -ENODEV;
    }

  sigemptyset(&mask);
  sigaddset(&mask, MY_GNSS_SIG);
  ret = sigprocmask(SIG_UNBLOCK, &mask, NULL);
  if (ret != OK)
    {
      printf("sigprocmask failed. %d\n", ret);
      goto _err;
    }

  sa.sa_sigaction = positioned;
  sa.sa_flags = SA_SIGINFO;
  sigfillset(&sa.sa_mask);
  sigdelset(&sa.sa_mask, MY_GNSS_SIG);
  ret = sigaction(MY_GNSS_SIG, &sa, NULL);
  if (ret != OK)
    {
      printf("sigaction failed. %d\n", ret);
      goto _err;
    }

  setting.fd      = fd;
  setting.enable  = 1;
  setting.gnsssig = CXD56_GNSS_SIG_GNSS;
  setting.signo   = MY_GNSS_SIG;
  setting.data    = NULL;

  ret = ioctl(fd, CXD56_GNSS_IOCTL_SIGNAL_SET, (unsigned long)&setting);


  /* Initial positioning measurement becomes cold start if specified hot
   * start, so working period should be long term to receive ephemeris. */

  posfixflag = 0;

  ret = ioctl(fd, CXD56_GNSS_IOCTL_START, CXD56_GNSS_STMOD_HOT);
  if (ret < 0)
    {
      printf("start error:%d\n", errno);
    }

  return ret;

_err:
  ret = close(fd);
  if (ret < 0)
    {
      printf("close error\n");
    }
  return -1;
}

static void location_task(void)
{
  gnss_start();

  for(;;)
    {
      sleep(1);
    }
}

void tracker_get_location_data(struct cxd56_gnss_positiondata_s *loc_data)
{
  memcpy(loc_data, &posdat, sizeof(struct cxd56_gnss_positiondata_s));
}

int tracker_location_start(void)
{
  task_create("location_task", 100, 2048, (main_t)location_task, NULL );

  return 0;
}
