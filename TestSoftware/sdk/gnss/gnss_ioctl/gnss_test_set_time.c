/****************************************************************************
 * test/gnss_ioctl/gnss_test_set_time.c
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
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <arch/chip/gnss.h>
#include "gnss_ioctl_main.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define TIME_COMPARE_MERGIN     20
#define IDLE_COUNT              10

#define RESULT_UNEXECUTED       1
#define RESULT_OK               0
#define RESULT_DIFF_DATE        (-1)
#define RESULT_DIFF_TIME        (-2)

/****************************************************************************
 * Private Types
 ****************************************************************************/
struct cxd56_gnss_ioctl_time_s
{
  uint32_t command;
  struct cxd56_gnss_datetime_s *pdatetime;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * test parameter
 ****************************************************************************/

/* Case1:Normal */
static struct cxd56_gnss_datetime_s settime1 = {
  {2018, 3, 3}, {1, 1, 1, 999999}
};

/* Case2:minutes increment */
static struct cxd56_gnss_datetime_s settime2 = {
  {2018, 3, 3}, {1, 1, 59, 0}
};

/* Case3:hour increment */
static struct cxd56_gnss_datetime_s settime3 = {
  {2018, 3, 3}, {1, 59, 59, 0}
};

/* Case4:date increment */
/* Date increment not support 
 * static struct cxd56_gnss_datetime_s settime4 = {
 *   {2018,3,31}, {23,59,59,0}
 * };
 */
static struct cxd56_gnss_datetime_s gettime;

static const struct cxd56_gnss_ioctl_time_s ioctl_table[] = {
  {CXD56_GNSS_IOCTL_SET_TIME, &settime1},   /* Case1 */
  {CXD56_GNSS_IOCTL_SET_TIME, &settime2},   /* Case2 */
  {CXD56_GNSS_IOCTL_SET_TIME, &settime3},   /* Case3 */

/*{CXD56_GNSS_IOCTL_SET_TIME, &settime4},*/ /* Case4 */
};

static const uint32_t ioctl_table_count =
  (uint32_t)(sizeof(ioctl_table) / (sizeof(ioctl_table[0])));

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: read_and_print()
 *
 * Description:
 *   Read and print POS data.
 *   Store the 1st notified date and time.
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
  int ret;
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

  /* Store 1st notify date and time */

  if (gettime.date.year == 0)
    {
      gettime.date = g_gnss_posdat.receiver.date;
      gettime.time = g_gnss_posdat.receiver.time;
    }

  if (g_gnss_posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
    {
      g_gnss_posfixflag = 1;

      gnss_double_to_dmf(g_gnss_posdat.receiver.latitude, &dmf);
      printf(", LAT %d.%d.%04d", dmf.degree, dmf.minute, dmf.frac);

      gnss_double_to_dmf(g_gnss_posdat.receiver.longitude, &dmf);
      printf(", LNG %d.%d.%04d", dmf.degree, dmf.minute, dmf.frac);
    }
  printf("\n");

_err:
  return ret;
}

/****************************************************************************
 * Name: gnss_test_set_time()
 *
 * Description:
 *   Compare set time and positioning start time.
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

int gnss_test_set_time(int argc, char *argv[])
{
  int      fd;
  int      ret;
  int      ret_tmp;
  int      count;
  int      result[ioctl_table_count];
  int      i;
  int      size = sizeof(struct cxd56_gnss_date_s);
  uint32_t posperiod = IDLE_COUNT;
  struct cxd56_gnss_datetime_s *psettime;
  struct pollfd fds[GNSS_POLL_FD_NUM] = { {0} };

  /* Program start */

  printf("%s() in\n", __func__);

  /* Get file descriptor to control GNSS. */

  fd = fd_open();
  if (fd < 0)
    {
      return -ENODEV;
    }

  /* Test start */

  for (count = 0; count < ioctl_table_count; count++)
    {

      /* Initial positioning measurement becomes cold start if specified hot
       * start, so working period should be long term to receive ephemeris. */

      g_gnss_posfixflag = 0;
      memset(&gettime, 0x00, sizeof(struct cxd56_gnss_datetime_s));
      result[count] = RESULT_UNEXECUTED;

      /* Set time */

      /* Set before start */

      ret = ioctl(fd, ioctl_table[count].command,
                  (uint32_t)ioctl_table[count].pdatetime);
      if (ret < 0)
        {
          printf("SET_RECEIVER_POSITION error\n");
          break;
        }
      psettime = ioctl_table[count].pdatetime;

      /* Start GNSS. */

      ret = ioctl(fd, CXD56_GNSS_IOCTL_START, CXD56_GNSS_STMOD_HOT);
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

      i = posperiod;
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

      /* Compare settime-gettime */

      result[count] = RESULT_OK;
      if (memcmp(&psettime->date, &gettime.date, size) != 0)
        {
          result[count] = RESULT_DIFF_DATE;
        }
      else
        {
          int time_diff =
            abs(I_TIME_TO_SEC(psettime->time) - I_TIME_TO_SEC(gettime.time));

          if (time_diff > TIME_COMPARE_MERGIN)
            {
              result[count] = RESULT_DIFF_TIME;
            }
          else
            {
              result[count] = RESULT_OK;
            }
        }
    }

  /* Check result */

  printf("\n");
  for (count = 0; count < ioctl_table_count; count++)
    {
      printf("result%d %d\n", count, result[count]);
      if (result[count] != 0)
        {
          ret = result[count];
        }
    }

_err:
  /* Release GNSS file descriptor. */

  fd_close(fd);

  printf("%s() out %d\n", __func__, ret);

  return ret;
}
