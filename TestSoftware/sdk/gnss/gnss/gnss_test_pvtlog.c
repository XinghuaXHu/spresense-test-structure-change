/****************************************************************************
 * test/sqa/singlefunction/gnss/gnss_test_pvtlog.c
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

#include <sdk/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <arch/chip/gnss.h>
#include "gnss_test_main.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define TEST_LOOP_TIME            60*10    /* 10 minutes */
#define LONG_TEST_LOOP_TIME       60*60*24 /* 24 hours */
#define TEST_RECORDING_CYCLE      1     /* 1 second */
#define TEST_NOTIFY_THRESHOLD     CXD56_GNSS_PVTLOG_THRESHOLD_HALF
#define FILE_NAME_LEN             256

#if(TEST_NOTIFY_THRESHOLD == CXD56_GNSS_PVTLOG_THRESHOLD_HALF)
#  define PVTLOG_UNITNUM          (CXD56_GNSS_PVTLOG_MAXNUM/2)
#else
#  define PVTLOG_UNITNUM          (CXD56_GNSS_PVTLOG_MAXNUM)
#endif
#define TEST_FILE_COUNT(a)        (1 + (int)(a / PVTLOG_UNITNUM))

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct cxd56_pvtlog_s            pvtlogdat;


/****************************************************************************
 * Name: read_and_print_pvtlog()
 *
 * Description:
 *   Read and print POS and PVTLOG data.
 *
 * Input Parameters:
 *   fd_gps - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

static int read_and_print_pvtlog(int fd_gps)
{
  int ret;
  struct cxd56_gnss_dms_s      dmf;
  struct cxd56_pvtlog_status_s pvtlog_status;

  /* Seek data */

  ret = lseek(fd_gps, CXD56_GNSS_READ_OFFSET_LAST_GNSS, SEEK_SET);
  if (ret < 0)
    {
      ret = errno;
      printf("lseek error %d\n", ret);
      goto _err1;
    }

  /* Read POS data */

  ret = read(fd_gps, &gnss_posdat, sizeof(gnss_posdat));
  if (ret < 0)
    {
      ret = errno;
      printf("read error %d\n", ret);
      goto _err1;
    }
  else if (ret < sizeof(gnss_posdat))
    {
      ret = ERROR;
      printf("read size error %d\n", ret);
      goto _err1;
    }
  else
    {
      ret = OK;
    }

  if (gnss_posdat.receiver.pos_dataexist &&
      (gnss_posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID))
    {
      /* Print POS data */

      printf(" Y=%d, M=%2d, d=%2d", gnss_posdat.receiver.date.year,
             gnss_posdat.receiver.date.month, gnss_posdat.receiver.date.day);

      printf(" h=%2d, m=%2d, s=%2d m=%3d", gnss_posdat.receiver.time.hour,
             gnss_posdat.receiver.time.minute, gnss_posdat.receiver.time.sec,
             (int)(gnss_posdat.receiver.time.usec/1000));

      double_to_dmf(gnss_posdat.receiver.latitude, &dmf);
      printf(", Lat %d:%d:%d",
             dmf.degree, dmf.minute, dmf.frac);

      double_to_dmf(gnss_posdat.receiver.longitude, &dmf);
      printf(" , Lon %d:%d:%d",
             dmf.degree, dmf.minute, dmf.frac);

      /* Get Log status */

      ret = ioctl(fd_gps, CXD56_GNSS_IOCTL_PVTLOG_GET_STATUS,
                  (unsigned long)&pvtlog_status);
      printf(", Log No:%d \n", pvtlog_status.status.log_count);
    }
  else
    {
      printf("No Positioning...\n");
    }

_err1:
  return ret;
}

/****************************************************************************
 * Name: get_pvtlog()
 *
 * Description:
 *   Read PVTLOG data.
 *
 * Input Parameters:
 *   fd_gps - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

static int get_pvtlog(int fd_gps)
{
  int ret;

  /* Seek. */

  ret = lseek(fd_gps, CXD56_GNSS_READ_OFFSET_PVTLOG, SEEK_SET);
  if (ret < 0)
    {
      ret = errno;
      printf("lseek error %d\n", ret);
      goto _err1;
    }

  /* Read PVTLOG. */

  ret = read(fd_gps, &pvtlogdat, sizeof(pvtlogdat));
  if (ret < 0)
    {
      ret = errno;
      printf("read error %d\n", ret);
      goto _err1;
    }
  else if (ret == 0)
    {
      printf("read data size is 0 \n");
      goto _err1;
    }
  else
    {
      ret = OK;
    }

_err1:
  return ret;
}

/****************************************************************************
 * Name: set_signal()
 *
 * Description:
 *   Call CXD56_GNSS_IOCTL_SIGNAL_SET.
 *
 * Input Parameters:
 *   fd_gps      - File descriptor.
 *   signo   - signal number
 *   gnsssig - GNSS ID
 *   enable  - set enable or disable
 *   mask    - signal mask
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

static int set_signal(int fd_gps, int signo, uint32_t gnsssig, int enable,
                      sigset_t * mask)
{
  int ret;
  struct cxd56_gnss_signal_setting_s setting;

  sigaddset(mask, signo);
  ret = sigprocmask(SIG_BLOCK, mask, NULL);
  if (ret != OK)
    {
      printf("sigprocmask failed. %d\n", ret);
      goto _err1;
    }

  setting.fd      = fd_gps;
  setting.enable  = enable;
  setting.gnsssig = gnsssig;
  setting.signo   = signo;
  setting.data    = NULL;

  ret = ioctl(fd_gps, CXD56_GNSS_IOCTL_SIGNAL_SET, (unsigned long)&setting);

_err1:
  return ret;
}

/****************************************************************************
 * Name: writefile()
 *
 * Description:
 *   Write PVTLOG data.
 *
 * Input Parameters:
 *   file_count - Write file count.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   Write buffer refers to global variable "pvtlogdat".
 *
 ****************************************************************************/

static int writefile(uint32_t file_count)
{
  int fd_write;
  int ret = OK;
  char filename[FILE_NAME_LEN];

  /* Make file name */

  snprintf(filename, FILE_NAME_LEN, "%s%d.dat",
           CONFIG_EXAMPLES_GNSS_TEST_PVTLOG_FILEPATH, file_count);

  /* Open file */

  fd_write = open(filename, O_WRONLY | O_CREAT | O_BINARY);
  if (fd_write < 0)
    {
      printf("%s open error:%d\n", filename, errno);
      ret = ERROR;
    }
  else
    {
      if (write(fd_write, &pvtlogdat, sizeof(struct cxd56_pvtlog_s)) !=
          sizeof(struct cxd56_pvtlog_s))
        {
          printf("%s write error:%d\n", filename, errno);
          ret = ERROR;
        }
      else
        {
          printf("%s write OK\n", filename);
        }

      close(fd_write);
    }
  fd_write = 0;

  return ret;
}

/****************************************************************************
 * Name: gnss_pvtlog_write()
 *
 * Description:
 *   Enable PVTLOG and run GNSS for 10 minutes.
 *   When notification of PVTLOG comes, save PVTLOG in a file.
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

static int gnss_pvtlog_write(int argc, char *argv[], uint32_t testtime)
{
  int      fd_gps;
  int      ret;
  int      timecount = 0;
  int      sig_id    = -1;
  uint32_t file_count = 1;
  sigset_t mask;
  struct cxd56_pvtlog_setting_s pvtlog_setting;

  /* Program start */

  printf("%s()\n", __func__);

  /* Get file descriptor to control GNSS. */

  fd_gps = open("/dev/gps", O_RDONLY);
  if (fd_gps < 0)
    {
      printf("open error:%d,%d\n", fd_gps, errno);
      return -ENODEV;
    }

  /* Call SET_OPE_MODE. */

  ret = gnss_setopemode(fd_gps, TEST_RECORDING_CYCLE*1000);
  if (ret != OK)
    {
      goto _err3;
    }

  /* Set the type of satellite system used by GNSS. */

  ret = gnss_setsatellite(fd_gps, CXD56_GNSS_SAT_GPS | CXD56_GNSS_SAT_GLONASS);
  if (ret != OK)
    {
      goto _err3;
    }

  sigemptyset(&mask);

  /* Init positioning signal */

  ret = set_signal(fd_gps, MY_GNSS_SIG, CXD56_GNSS_SIG_GNSS, TRUE, &mask);
  if (ret < 0)
    {
      printf("GNSS signal set error\n");
      goto _err3;
    }

  /* Init PVTLOG signal */

  ret = set_signal(fd_gps, MY_PVTLOG_SIG, CXD56_GNSS_SIG_PVTLOG, TRUE, &mask);
  if (ret < 0)
    {
      printf("PVTLOG signal set error\n");
      goto _err2;
    }

  /* Delete Log data */

  ret = ioctl(fd_gps, CXD56_GNSS_IOCTL_PVTLOG_DELETE_LOG, 0);
  if (ret < 0)
    {
      printf("Delete log error\n");
      goto _err1;
    }

  /* Start Log */

  pvtlog_setting.cycle     = TEST_RECORDING_CYCLE;
  pvtlog_setting.threshold = TEST_NOTIFY_THRESHOLD;
  ret = ioctl(fd_gps, CXD56_GNSS_IOCTL_PVTLOG_START,
              (unsigned long)&pvtlog_setting);
  if (ret < 0)
    {
      printf("Start pvtlog error\n");
      goto _err1;
    }

  /* Start GNSS. */

  ret = gnss_start(fd_gps, CXD56_GNSS_STMOD_HOT);
  if (ret < 0)
    {
      goto _err1;
    }

  do
    {

      /* Wait signal */

      sig_id = sigwaitinfo(&mask, NULL);

      switch (sig_id)
        {
        case MY_GNSS_SIG:

          /* Read and print POS data. */

          read_and_print_pvtlog(fd_gps);
          timecount++;
          break;

        case MY_PVTLOG_SIG:

          /* Receive pvtlog signal */

          get_pvtlog(fd_gps);
          writefile(file_count);
          file_count++;
          break;

        default:

          /* Invalid case */

          printf("ret %d\n", ret);
          break;
        }
    }
  while (timecount < testtime);

  /* Stop Log */

  ioctl(fd_gps, CXD56_GNSS_IOCTL_PVTLOG_STOP, 0);


  /* If the last signal is positioning, there is a unsaved PVTLOG. */

  if (sig_id == MY_GNSS_SIG)
    {
      /* Write unsaved logs */

      get_pvtlog(fd_gps);
      writefile(file_count);
    }

  /* Stop GNSS. */

  gnss_stop(fd_gps);

_err1:

  /* PVTLOG signal clear. */

  set_signal(fd_gps, MY_PVTLOG_SIG, CXD56_GNSS_SIG_PVTLOG, FALSE, &mask);

_err2:

  /* GNSS signal clear. */

  set_signal(fd_gps, MY_GNSS_SIG, CXD56_GNSS_SIG_GNSS, FALSE, &mask);

_err3:

  /* Release GNSS file descriptor. */

  ret = close(fd_gps);
  if (ret < 0)
    {
      printf("close error\n");
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_pvtlog_read()
 *
 * Description:
 *   Read and dump pvtlog file.
 *   Make sure that the dump logs of gnss_pvtlog_write() and 
 *   gnss_pvtlog_read() match.
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

static int gnss_pvtlog_read(int argc, char *argv[], uint32_t testtime)
{
  int      ret = OK;
  int      ret_tmp;
  int      fd_read;
  uint32_t file_count;
  uint32_t file_max = TEST_FILE_COUNT(testtime);
  uint32_t log_count;
  uint32_t log_max;
  char     filename[FILE_NAME_LEN];
  struct cxd56_pvtlog_data_s *log;

  /* Program start */

  printf("%s()\n", __func__);

  for (file_count = 1; file_count <= file_max; file_count++)
    {
      /* Make file name */

      snprintf(filename, FILE_NAME_LEN, "%s%d.dat",
               CONFIG_EXAMPLES_GNSS_TEST_PVTLOG_FILEPATH, file_count);

      /* Open file */

      fd_read = open(filename, O_RDONLY | O_BINARY);
      if (fd_read < 0)
        {
          /* Continue */
        }
      else
        {
          /* Read file */

          ret_tmp = read(fd_read, &pvtlogdat, sizeof(pvtlogdat));
          if (ret_tmp < 0)
            {
              ret = errno;
              printf("%s read error:%d\n", filename, errno);
            }
          else if (ret_tmp != sizeof(pvtlogdat))
            {
              ret = ERROR;
              printf("%s read size error:%d\n", filename, errno);
            }
          else
            {
              ret = OK;

              log_max = pvtlogdat.log_count;
              printf("%s read OK(%d line)\n", filename, log_max);

              /* Printf pvtlog file */

              for (log_count = 0; log_count < log_max; log_count++)
                {
                  /* Printf record */

                  log = &pvtlogdat.log_data[log_count];
                  printf(" Y=20%2d, M=%2d, d=%2d",
                         log->date.year, log->date.month, log->date.day);

                  printf(" h=%2d, m=%2d, s=%2d m=%3d", log->time.hour,
                         log->time.minute, log->time.sec, log->time.msec);

                  printf(", Lat %d:%d:%d",log->latitude.degree,
                         log->latitude.minute, log->latitude.frac);

                  printf(" , Lon %d:%d:%d", log->longitude.degree,
                         log->longitude.minute, log->longitude.frac);

                  printf(", Log No:%d \n", (log_count + 1));
                }
            }

          /* Close file */

          close(fd_read);
          fd_read = 0;
        }
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_pvtlog_delete()
 *
 * Description:
 *   Delete pvtlog files.
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

static int gnss_pvtlog_delete(int argc, char *argv[], uint32_t testtime)
{
  int ret = OK;
  uint32_t file_count;
  uint32_t file_max = TEST_FILE_COUNT(testtime);
  char filename[FILE_NAME_LEN];

  /* Program start */

  printf("%s()\n", __func__);

  for (file_count = 1; file_count <= file_max; file_count++)
    {
      /* Make file name */

      snprintf(filename, FILE_NAME_LEN, "%s%d.dat",
               CONFIG_EXAMPLES_GNSS_TEST_PVTLOG_FILEPATH, file_count);

      /* Delete file */

      if (unlink(filename) == OK)
        {
          printf("%s delete ok\n", filename);
        }
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_pvtlog_Stresstest()
 *
 * Description:
 *   Loop "w" and "r" and "d".
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
int gnss_pvtlogstress(int argc, char *argv[])
{
  int test_time = 0;
  int loop_time = TEST_LOOP_TIME;
  int testcount;
  int maxcount = (int)(LONG_TEST_LOOP_TIME / TEST_LOOP_TIME);
  int ret = OK;

  /* Check argument. */

  if(argv[2] != NULL)
    {
      test_time = atoi(argv[2]);
    }

  /* Set test time. */

  if(test_time == 0)
    {
      test_time = LONG_TEST_LOOP_TIME;
    }

  /* Set test time. */

  if(test_time <= 0)
    {
      test_time = LONG_TEST_LOOP_TIME;
    }

  if(test_time < loop_time)
    {
      loop_time = test_time;
    }

  maxcount = (int)(test_time / loop_time);

  printf("maxcount %d, loop_time %d\n", maxcount, loop_time);

  /* Program start */

  for (testcount = 0; testcount < maxcount; testcount++)
    {
      /* Run W and R and D */

      if (gnss_pvtlog_write(argc, argv, loop_time) != OK)
        {
          ret = ERROR;
        }
      if (gnss_pvtlog_read(argc, argv, loop_time) != OK)
        {
          ret = ERROR;
        }
      if (gnss_pvtlog_delete(argc, argv, loop_time) != OK)
        {
          ret = ERROR;
        }
    }

  printf("%s() done\n", __func__);

  return ret;
}

/****************************************************************************
 * Name: gnss_pvtloglong()
 *
 * Description:
 *   Run positioning and write PVTLOG file.
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

int gnss_pvtloglong(int argc, char *argv[])
{
  int ret = OK;
  int loop_time = 0;

  /* Check argument. */

  if(argv[2] != NULL)
    {
      loop_time = atoi(argv[2]);
    }

  /* Set test time. */

  if(loop_time <= 0)
    {
      loop_time = LONG_TEST_LOOP_TIME;
    }

  printf("loop_time %d\n", loop_time);

  /* Program start */

  ret = gnss_pvtlog_write(argc, argv, loop_time);

  printf("%s() done\n", __func__);

  return ret;
}
