/****************************************************************************
 * test/sqa/singlefunction/gnss/gnss_test_utility.c
 *
 *   Copyright (C) 2016,2017 Sony. All rights reserved.
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
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
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
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <arch/chip/gnss.h>
#include "gnss_test_main.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

uint32_t                         posfixflag;
struct cxd56_gnss_positiondata_s gnss_posdat;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: double_to_dmf()
 *
 * Description:
 *   Convert from double format to degree-minute-frac format.
 *
 * Input Parameters:
 *   x   - double value.
 *   dmf - Address to store the conversion result.
 *
 * Returned Value:
 *   none.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

void double_to_dmf(double x, struct cxd56_gnss_dms_s * dmf)
{
  int    b;
  int    d;
  int    m;
  double f;
  double t;

  if (x < 0)
    {
      b = 1;
      x = -x;
    }
  else
    {
      b = 0;
    }

  /* = floor(x), x is always positive */

  d = (int)x;
  t = (x - d) * 60;

  /* = floor(t), t is always positive */

  m = (int)t;
  f = (t - m) * 10000;

  dmf->sign   = b;
  dmf->degree = d;
  dmf->minute = m;
  dmf->frac   = f;
}

/****************************************************************************
 * Name: read_and_print()
 *
 * Description:
 *   Read and print POS data.
 *
 * Input Parameters:
 *   fd - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int read_and_print(int fd)
{
  int ret;
  struct cxd56_gnss_dms_s dmf;

  /* Read POS data. */

  ret = read(fd, &gnss_posdat, sizeof(gnss_posdat));
  if (ret < 0)
    {
      printf("read error\n");
      goto _err;
    }
  else if (ret != sizeof(gnss_posdat))
    {
      ret = ERROR;
      printf("read size error\n");
      goto _err;
    }
  else
    {
      ret = OK;
    }

  /* Print POS data. */

  /* Print time. */

  printf(">%02d:%02d:%02d.%06d, ",
         gnss_posdat.receiver.time.hour, gnss_posdat.receiver.time.minute,
         gnss_posdat.receiver.time.sec, gnss_posdat.receiver.time.usec);
  if (gnss_posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
    {

      /* 2D fix or 3D fix.
       * Convert latitude and longitude into dmf format and print it. */

      posfixflag = 1;

      double_to_dmf(gnss_posdat.receiver.latitude, &dmf);
      printf("LAT %d.%d.%04d, ", dmf.degree, dmf.minute, dmf.frac);

      double_to_dmf(gnss_posdat.receiver.longitude, &dmf);
      printf("LNG %d.%d.%04d\n", dmf.degree, dmf.minute, dmf.frac);
    }
  else
    {

      /* No measurement. */

      printf("No Positioning Data\n");
    }

_err:
  return ret;
}

/****************************************************************************
 * Name: gnss_start()
 *
 * Description:
 *   Set gnss parameters use ioctl.
 *
 * Input Parameters:
 *   fd - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_start(int fd, uint32_t start_mode)
{
  int      ret = 0;

  /* Set the type of satellite system used by GNSS. */

  ret = ioctl(fd, CXD56_GNSS_IOCTL_START, start_mode);
  if (ret < 0)
    {
      printf("start GNSS ERROR %d\n", errno);
    }
  else
    {
      printf("start GNSS\n");
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_stop()
 *
 * Description:
 *   Set gnss parameters use ioctl.
 *
 * Input Parameters:
 *   fd - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_stop(int fd)
{
  int      ret = 0;

  /* Set the type of satellite system used by GNSS. */

  ret = ioctl(fd, CXD56_GNSS_IOCTL_STOP, 0);
  if (ret < 0)
    {
      printf("stop GNSS ERROR %d\n", errno);
    }
  else
    {
      printf("stop GNSS\n");
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_setsatellite()
 *
 * Description:
 *   Set gnss parameters use ioctl.
 *
 * Input Parameters:
 *   fd - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_setsatellite(int fd, uint32_t set_satellite)
{
  int      ret = 0;

  /* Set the type of satellite system used by GNSS. */

  ret = ioctl(fd, CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM, set_satellite);
  if (ret < 0)
    {
      printf("ioctl(CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM) NG!!\n");
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_setopemode()
 *
 * Description:
 *   Set gnss parameters use ioctl.
 *
 * Input Parameters:
 *   fd - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_setopemode(int fd, int cycle_msec)
{
  int      ret = 0;
  struct cxd56_gnss_ope_mode_param_s set_opemode;

  /* Set the GNSS operation interval. */

  /* Operation mode:Normal(default). */

  set_opemode.mode     = 1;

  /* Position notify cycle(msec step). */

  set_opemode.cycle    = cycle_msec;

  ret = ioctl(fd, CXD56_GNSS_IOCTL_SET_OPE_MODE, (uint32_t)&set_opemode);
  if (ret < 0)
    {
      printf("ioctl(CXD56_GNSS_IOCTL_SET_OPE_MODE) NG!!\n");
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_setparams()
 *
 * Description:
 *   Set gnss parameters use ioctl.
 *
 * Input Parameters:
 *   fd - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_setparams(int fd)
{
  int      ret = 0;

  /* Set the GNSS operation interval. */

  ret = gnss_setopemode(fd, 1000);
  if (ret < 0)
    {
      goto _err;
    }

  /* Set the type of satellite system used by GNSS. */

  ret = gnss_setsatellite(fd, CXD56_GNSS_SAT_GPS | CXD56_GNSS_SAT_GLONASS);
  if (ret < 0)
    {
      goto _err;
    }

_err:
  return ret;
}

struct cxd56_gnss_signal_setting_s setting;
int signal_fd = 0;

/****************************************************************************
 * Name: gnss_setsignal()
 *
 * Description:
 *   Set signal ioctl.
 *
 * Input Parameters:
 *   fd - File descriptor.
 *   pmask - mask pointer.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_setsignal(int fd, sigset_t *pmask)
{
  int ret = 0;

  /* Configure mask to notify GNSS signal. */

  sigemptyset(pmask);
  sigaddset(pmask, MY_GNSS_SIG);
  ret = sigprocmask(SIG_BLOCK, pmask, NULL);
  if (ret != OK)
    {
      printf("sigprocmask failed. %d\n", ret);
      goto _err;
    }

  setting.fd      = fd;
  setting.enable  = 1;
  setting.gnsssig = CXD56_GNSS_SIG_GNSS;
  setting.signo   = MY_GNSS_SIG;
  setting.data    = NULL;

  ret = ioctl(fd, CXD56_GNSS_IOCTL_SIGNAL_SET, (unsigned long)&setting);
  if (ret < 0)
    {
      printf("signal error\n");
    }
  else
    {
      signal_fd = fd;
    }

_err:
  return ret;
}

/****************************************************************************
 * Name: gnss_clearsignal()
 *
 * Description:
 *   clear signal ioctl.
 *
 * Input Parameters:
 *   fd - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_clearsignal(int fd, sigset_t *pmask)
{
  int ret = 0;

  if(signal_fd == fd)
    {
      setting.fd     = fd;
      setting.enable = 0;
      ret = ioctl(fd, CXD56_GNSS_IOCTL_SIGNAL_SET, (unsigned long)&setting);
      if (ret < 0)
        {
          printf("signal error\n");
        }
    }
  else
    {
      printf("signal fd error\n");
      ret = ERROR;
    }

  sigprocmask(SIG_UNBLOCK, pmask, NULL);

  return ret;
}

/****************************************************************************
 * Name: gnss_set_time()
 *
 * Description:
 *   set time.
 *
 * Input Parameters:
 *   argc - time info Number.
 *   argv - time info.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_set_time(int argc, char *argv[])
{
  int fd;
  int ret;
  int i = 3;
  struct cxd56_gnss_datetime_s settime;

  fd = open("/dev/gps", O_RDONLY);
  if (fd < 0)
    {
      printf("open error:%d,%d\n", fd, errno);
      return -ENODEV;
    }

  settime.date.year   = atoi(argv[i++]);
  settime.date.month  = atoi(argv[i++]);
  settime.date.day    = atoi(argv[i++]);
  settime.time.hour   = atoi(argv[i++]);
  settime.time.minute = atoi(argv[i++]);
  settime.time.sec    = atoi(argv[i++]);
  settime.time.usec   = atoi(argv[i++]);
  printf("year=%d, month=%d, day=%d\n",
          settime.date.year, settime.date.month, settime.date.day);
  printf("hour=%d, minute=%d, sec=%d, usec=%d\n",
          settime.time.hour, settime.time.minute, settime.time.sec,
          settime.time.usec);
  ret = ioctl(fd, CXD56_GNSS_IOCTL_SET_TIME, (uint32_t)&settime);

  if (ret < 0)
    {
      printf("CXD56_GNSS_IOCTL_SET_TIME error:%d\n", errno);
    }
  else
    {
      printf("CXD56_GNSS_IOCTL_SET_TIME OK\n");
    }

  ret = close(fd);
  if (ret < 0)
    {
      printf("close error\n");
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_set_position()
 *
 * Description:
 *    set positioning.
 *
 * Input Parameters:
 *   argc - position info Number.
 *   argv - position info.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_set_position(int argc, char *argv[])
{
  int fd;
  int ret;
  int i = 3;
  struct cxd56_gnss_ellipsoidal_position_s  ellipsoidal;

  fd = open("/dev/gps", O_RDONLY);
  if (fd < 0)
    {
      printf("open error:%d,%d\n", fd, errno);
      return -ENODEV;
    }

  ellipsoidal.latitude  = atof(argv[i++]);
  ellipsoidal.longitude = atof(argv[i++]);
  ellipsoidal.altitude  = atof(argv[i++]);
  printf("latitude=%d, longitude=%d, altitude=%d\n",
          (int)ellipsoidal.latitude, (int)ellipsoidal.longitude, 
          (int)ellipsoidal.altitude);
  ret = ioctl(fd, CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ELLIPSOIDAL,
              (uint32_t)&ellipsoidal);
  if (ret < 0)
    {
      printf("CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION error:%d\n", errno);
    }
  else
    {
      printf("CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ELLIPSOIDAL OK\n");
    }

  ret = close(fd);
  if (ret < 0)
    {
      printf("close error\n");
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_get_ephameris()
 *
 * Description:
 *    Run positioning.
 *
 * Input Parameters:
 *   argc - Does not use.
 *   argv - Does not use.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_get_ephameris(int argc, char *argv[])
{
  int      fd;
  int      ret;
  int      posperiod;
  sigset_t mask;

  /* Get file descriptor to control GNSS. */

  fd = open("/dev/gps", O_RDONLY);
  if (fd < 0)
    {
      printf("open error:%d,%d\n", fd, errno);
      return -ENODEV;
    }

  /* Set the signal to notify GNSS events. */

  ret = gnss_setsignal(fd, &mask);
  if (ret < 0)
    {
      goto _err;
    }

  /* Set GNSS parameters. */

  ret = gnss_setparams(fd);
  if (ret != OK)
    {
      printf("gnss_setparams failed. %d\n", ret);
      goto _err;
    }

  /* Initial positioning measurement becomes cold start if specified hot
   * start, so working period should be long term to receive ephemeris. */

  posperiod = atof(argv[3]);
  posfixflag = 0;

  /* Start GNSS. */

  ret = gnss_start(fd, CXD56_GNSS_STMOD_HOT);
  if (ret < 0)
    {
      goto _err;
    }

  do
    {

      /* Wait for positioning to be fixed. After fixed,
       * idle for the specified seconds. */

      ret = sigwaitinfo(&mask, NULL);
      if (ret != MY_GNSS_SIG)
        {
          printf("sigwaitinfo error %d\n", ret);
          break;
        }

      /* Read and print POS data. */

      ret = read_and_print(fd);
      if (ret < 0)
        {
          break;
        }

      if (posfixflag)
        {

          /* Count down started after POS fixed. */

          posperiod--;
        }
    }
  while (posperiod > 0);

  /* Stop GNSS. */

  ret = gnss_stop(fd);
  if (ret < 0)
    {
      printf("stop GNSS ERROR\n");
    }
  else
    {
      printf("stop GNSS OK\n");
    }

_err:

  /* GNSS firmware needs to disable the signal after positioning. */

  ret = gnss_clearsignal(fd, &mask);
  if (ret < 0)
    {
      printf("signal error\n");
    }

  /* Release GNSS file descriptor. */

  ret = close(fd);
  if (ret < 0)
    {
      printf("close error\n");
    }

  printf("End of GNSS Sample:%d\n", ret);

  return ret;
}

/****************************************************************************
 * Name: gnss_save_backupdata()
 *
 * Description:
 *    Call SAVE BACKUP_DATA.
 *
 * Input Parameters:
 *   argc - Does not use.
 *   argv - Does not use.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_save_backupdata(int argc, char *argv[])
{
  int      fd;
  int      ret;

  /* Get file descriptor to control GNSS. */

  fd = open("/dev/gps", O_RDONLY);
  if (fd < 0)
    {
      printf("open error:%d,%d\n", fd, errno);
      return -ENODEV;
    }

  ret = ioctl(fd, CXD56_GNSS_IOCTL_SAVE_BACKUP_DATA, 0);
  if (ret < 0)
    {
      printf("CXD56_GNSS_IOCTL_SAVE_BACKUP_DATA error:%d\n", errno);
    }
  else
    {
      printf("CXD56_GNSS_IOCTL_SAVE_BACKUP_DATA OK\n");
    }

  /* Release GNSS file descriptor. */

  ret = close(fd);
  if (ret < 0)
    {
      printf("close error\n");
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_erase_backupdata()
 *
 * Description:
 *    Call ERASE BACKUP_DATA.
 *
 * Input Parameters:
 *   argc - Does not use.
 *   argv - Does not use.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_erase_backupdata(int argc, char *argv[])
{
  int fd;
  int ret;

  fd = open("/dev/gps", O_RDONLY);
  if (fd < 0)
    {
      printf("open error:%d,%d\n", fd, errno);
      return -ENODEV;
    }

  ret = ioctl(fd, CXD56_GNSS_IOCTL_ERASE_BACKUP_DATA, 0);
  if (ret < 0)
    {
      printf("CXD56_GNSS_IOCTL_ERASE_BACKUP_DATA error:%d\n", errno);
    }
  else
    {
      printf("CXD56_GNSS_IOCTL_ERASE_BACKUP_DATA OK\n");
    }

  ret = close(fd);
  if (ret < 0)
    {
      printf("close error\n");
    }

  return ret;
}

int argparse(const char *parameters[], char *argv)
{
  int ret = ERROR;
  int i;
  int cnt;

  if ((parameters != NULL) && (argv != NULL))
    {
      for (i = 0; parameters[i] != NULL; i++)
        {
          if(strcmp(parameters[i], argv) == 0)
            {
              ret = i;
              break;
            }
        }
    }

  if(ret == ERROR)
    {
      printf("argument \"%s\" not found\n", argv);
      printf("Please use the following arguments.\n");
      for(cnt=0; parameters[cnt] != NULL; cnt++)
        {
          printf("  %s\n", parameters[cnt]);
        }
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_testutility()
 *
 * Description:
 *   Set parameters and run positioning.
 *
 * Input Parameters:
 *   argc - Does not use.
 *   argv - Does not use.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_testutility(int argc, char *argv[])
{
  int ret;

  const char *parameters[] = {
    "settime",      // 0
    "setpos",       // 1
    "getephameris", // 2
    "saveback",     // 3
    "eraseback",    // 4
  };

  switch(argparse(parameters, argv[2]))
  {
    case 0:
      ret = gnss_set_time(argc, argv);
      break;

    case 1:
      ret = gnss_set_position(argc, argv);
      break;

    case 2:
      ret = gnss_get_ephameris(argc, argv);
      break;

    case 3:
      ret = gnss_save_backupdata(argc, argv);
      break;

    case 4:
      ret = gnss_erase_backupdata(argc, argv);
      break;

    case ERROR:
    default:
      ret = ERROR;
      break;

  }

  return ret;
}

