/****************************************************************************
 * test/gnss_ioctl/gnss_test_ope_mode.c
 *
 *   Copyright (C) 2016,2017 Sony. All rights reserved.
 *   Author: Tomoyuki Takahashi <Tomoyuki.A.Takahashi@sony.com>
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

#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <arch/chip/gnss.h>
#include "gnss_ioctl_main.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define WAIT_LIMIT  (60*10)
#define IDLE_COUNT  (60*3)

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/
static uint32_t set_interval_sec;
static uint32_t ret_interval;
static uint32_t wait_count;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: read_and_print()
 *
 * Description:
 *   Read and print POS data.
 *   Check notification interval.
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

static int read_and_print(int fd)
{
  int      ret;
  bool     flag_check_interval = TRUE;
  uint32_t i_current_sec;
  uint32_t i_prediction_sec;
  static uint32_t last_interval_sec = 0;
  struct cxd56_gnss_time_s *ptime;
  struct cxd56_gnss_dms_s  dmf;

  /* Read POS data */

  ret = read(fd, &g_gnss_posdat, sizeof(g_gnss_posdat));
  if (ret < 0)
    {
      printf("read error\n");
      goto _err;
    }
  else if (ret < sizeof(g_gnss_posdat))
    {
      ret = ERROR;
      printf("read size error\n");
      goto _err;
    }
  else
    {
      ret = OK;
    }

  /* Print POS data */

  ptime = &(g_gnss_posdat.receiver.time);
  printf(">H:%2d, m:%2d, s:%2d, u:%6d, svCount=%2d",
         ptime->hour, ptime->minute, ptime->sec, ptime->usec,
         g_gnss_posdat.svcount);
  if (g_gnss_posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
    {
      if (g_gnss_posfixflag == 0)
        {
          /* Except:posfix */
          flag_check_interval = FALSE;
        }
      g_gnss_posfixflag = 1;

      gnss_double_to_dmf(g_gnss_posdat.receiver.latitude, &dmf);
      printf(", LAT %d.%d.%04d", dmf.degree, dmf.minute, dmf.frac);

      gnss_double_to_dmf(g_gnss_posdat.receiver.longitude, &dmf);
      printf(", LNG %d.%d.%04d", dmf.degree, dmf.minute, dmf.frac);
    }
  else
    {
      wait_count++;
    }
  printf("\n");

  /* Check interval */

  i_current_sec = I_TIME_TO_SEC(g_gnss_posdat.receiver.time);
  i_prediction_sec = last_interval_sec + (set_interval_sec * 2);

  /* There are exceptions to the check. */

  if (last_interval_sec == 0)
    {
      /* Except:first time */

      flag_check_interval = FALSE;
    }
  else if (last_interval_sec > i_current_sec)
    {
      /* Except:Time lags 1 */

      flag_check_interval = FALSE;
    }
  else if (i_current_sec > i_prediction_sec)
    {
      /* Except:Time lags 2 */

      flag_check_interval = FALSE;
    }

  if (flag_check_interval == TRUE)
    {
      /* Check */

      if ((i_current_sec - last_interval_sec) != set_interval_sec)
        {
          /* Invalid case */

          ret_interval = ERROR;
        }
    }

  last_interval_sec = i_current_sec;

_err:
  return ret;
}

/****************************************************************************
 * Name: gnss_test_backup_data()
 *
 * Description:
 *   Change the positioning cycle and check it operates according to the 
 *   set cycle.
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

int gnss_test_ope_mode(int argc, char *argv[])
{
  const uint32_t cycle_table_msec[] = { 1000, 2000, 3000, 10000 };
  const uint32_t cycle_table_count =
    sizeof(cycle_table_msec) / sizeof(uint32_t);
  int fd;
  int ret;
  int ret_tmp;
  int count;
  int result[cycle_table_count];
  int i = 0;
  struct pollfd fds[GNSS_POLL_FD_NUM] = { {0} };
  struct cxd56_gnss_ope_mode_param_s setdata;
  struct cxd56_gnss_ope_mode_param_s getdata;

  /* Program start */

  printf("%s() in\n", __func__);

  /* Get file descriptor to control GNSS. */

  fd = fd_open();
  if (fd < 0)
    {
      return -ENODEV;
    }

  /* Repeat the test for the number of elements in cycle_table. */

  for (count = 0; count < cycle_table_count; count++)
    {
      wait_count       = 0;
      result[count]    = OK;
      ret_interval     = OK;
      setdata.mode     = 1;
      setdata.cycle    = cycle_table_msec[count];
      set_interval_sec = (uint32_t)(cycle_table_msec[count] / 1000);

      printf("test count %d:CXD56_GNSS_IOCTL_SET_OPE_MODE(%d sec)\n", count,
             set_interval_sec);

      /* Call SET_OPE_MODE and GET_OPE_MODE, compare each value. */

      ret = ioctl(fd, CXD56_GNSS_IOCTL_SET_OPE_MODE, (uint32_t)&setdata);
      if (ret < 0)
        {
          printf("ioctl:CXD56_GNSS_IOCTL_SET_OPE_MODE NG!!\n");
          goto _err;
        }

      ret = ioctl(fd, CXD56_GNSS_IOCTL_GET_OPE_MODE, (uint32_t)&getdata);
      if (ret < 0)
        {
          printf("ioctl:CXD56_GNSS_IOCTL_GET_OPE_MODE NG!!\n");
          goto _err;
        }

      /* Compare */

      if (setdata.cycle != getdata.cycle)
        {
          printf("CXD56_GNSS_IOCTL_SET/GET_OPE_MODE NG!!(set:%d, get:%d)\n",
                 setdata.cycle, getdata.cycle);
          goto _err;
        }

      /* Initial positioning measurement becomes cold start if specified hot
       * start, so working period should be long term to receive ephemeris. */

      g_gnss_posfixflag = 0;

      /* Start GNSS. */

      ret = ioctl(fd, CXD56_GNSS_IOCTL_START, CXD56_GNSS_STMOD_COLD);
      if (ret < 0)
        {
          printf("start GNSS ERROR %d\n", errno);
          goto _err;
        }
      else
        {
          printf("start GNSS OK\n");
        }

      fds[0].fd     = fd;
      fds[0].events = POLLIN;

      i = IDLE_COUNT / set_interval_sec;
      do
        {
          /* Wait POS notification. */

          ret_tmp = poll(fds, GNSS_POLL_FD_NUM, GNSS_POLL_TIMEOUT_FOREVER);
          if (ret_tmp <= 0)
            {
              ret = ret_tmp;
              printf("poll error %d,%x,%x\n", ret, fds[0].events,
                     fds[0].revents);
              break;
            }

          /* Read and print POS data. */

          ret_tmp = read_and_print(fd);
          if (ret_tmp < 0)
            {
              ret = ret_tmp;
              break;
            }

          /* Count down started from position fixed. */

          if (g_gnss_posfixflag)
            {
              i--;
            }

          if (wait_count > WAIT_LIMIT)
            {
              break;
            }
        }
      while (i > 0);

      /* Stop GNSS. */

      if (ioctl(fd, CXD56_GNSS_IOCTL_STOP, 0) < 0)
        {
          printf("stop GNSS ERROR\n");
        }
      else
        {
          printf("stop GNSS OK\n");
        }

      result[count] = ret_interval;
    }

  /* Print result */

  for (count = 0; count < cycle_table_count; count++)
    {
      printf("set interval %d, result %d\n", cycle_table_msec[count],
             result[count]);
      if (result[count] != 0)
        {
          if (ret == 0)
            {
              ret = result[count];
            }
        }
    }

_err:
  /* Release GNSS file descriptor. */

  fd_close(fd);

  printf("%s() out %d\n", __func__, ret);

  return ret;
}
