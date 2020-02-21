/****************************************************************************
 * test/gnss_ioctl/gnss_test_start.c
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
#define INTERVAL            1       /* Pos interval */
#define POS_PERIOD_COLD     (60*3)  /* Idling time after posfix */
#define POS_PERIOD_TEST     10      /* Idling time after posfix */
#define STMOD_LOOP_COUNT    (1+3)   /* 1:cold start, 2-max:test mode */

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/
static double start_sec;
static double posfix_sec;
static double coldstartoffset_sec;
static double last_sec;

static const uint32_t stmod_table[] =
  { CXD56_GNSS_STMOD_WARM, CXD56_GNSS_STMOD_HOT };

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: read_and_print()
 *
 * Description:
 *   Read and print POS data.
 *   In order to measure the time called TTFF(Time To First Fix), check 
 *   FIX for positioning.
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
  int    ret;
  double current_sec;
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

  /* Measure the time until POSFIX. */

  current_sec = D_TIME_TO_SEC(g_gnss_posdat.receiver.time);

  if (start_sec == 0.0)
    {
      /* Record start time */
      start_sec = current_sec;
    }
  else if (g_gnss_posfixflag == 0)
    {
      /* Check time lag */
      if ((last_sec > current_sec) ||
          (current_sec > (last_sec + (INTERVAL * 2))))
        {
          /* Record offset time */
          coldstartoffset_sec = (last_sec - start_sec) + INTERVAL;
          /* Overwrite start time */
          start_sec = current_sec;
        }
    }

  /* Update last time */

  last_sec = current_sec;

  /* Print POS data */

  ptime = &(g_gnss_posdat.receiver.time);
  printf(">H:%2d, m:%2d, s:%2d, u:%6d, svCount=%2d",
         ptime->hour, ptime->minute, ptime->sec, ptime->usec,
         g_gnss_posdat.svcount);
  if (g_gnss_posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
    {
      if (g_gnss_posfixflag == 0)
        {
          posfix_sec = current_sec;
        }
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
 * Name: gnss_test_start()
 *
 * Description:
 *   Switch the arguments at the start to check the positioning time.
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

int gnss_test_start(int argc, char *argv[])
{
  int      fd;
  int      ret;
  int      ret_tmp;
  int      count;
  int      i;
  uint32_t param_stmod;
  uint32_t st_max = (uint32_t)(sizeof(stmod_table) / sizeof(uint32_t));
  uint32_t st_count;
  uint32_t set_stmod;
  double   result_sec[STMOD_LOOP_COUNT];
  const char *dbg_msg;
  struct pollfd fds[GNSS_POLL_FD_NUM] = { {0} };

  /* Program start */

  printf("%s() in\n", __func__);

  /* Get file descriptor to control GNSS. */

  fd = fd_open();
  if (fd < 0)
    {
      return -ENODEV;
    }

  for (st_count = 0; st_count < st_max; st_count++)
    {

      i = POS_PERIOD_COLD;

      /* Set test parameter */
      param_stmod = stmod_table[st_count];
      switch (param_stmod)
        {
        case CXD56_GNSS_STMOD_COLD:
          dbg_msg = "cold start";
          break;
        case CXD56_GNSS_STMOD_WARM:
          dbg_msg = "warm start";
          break;
        case CXD56_GNSS_STMOD_HOT:
        default:
          param_stmod = CXD56_GNSS_STMOD_HOT;
          dbg_msg = "hot start";
          break;
        }
      printf("test CXD56_GNSS_IOCTL_START(test mode %s) start\n", dbg_msg);

      set_stmod = CXD56_GNSS_STMOD_COLD;
      for (count = 0; count < STMOD_LOOP_COUNT; count++)
        {

          /* Initial positioning measurement becomes cold start if specified
           * hot start, so working period should be long term to receive
           * ephemeris. */

          g_gnss_posfixflag   = 0;
          start_sec           = 0.0;
          posfix_sec          = 0.0;
          coldstartoffset_sec = 0.0;
          last_sec            = 0.0;

          /* Start GNSS. */

          printf("test count %d:start GNSS(mode %d)\n", count, set_stmod);
          ret = ioctl(fd, CXD56_GNSS_IOCTL_START, set_stmod);
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

          do
            {
              ret_tmp = poll(fds, GNSS_POLL_FD_NUM,
                             GNSS_POLL_TIMEOUT_FOREVER);
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
          i = POS_PERIOD_TEST;

          /* Stop GNSS. */

          if (ioctl(fd, CXD56_GNSS_IOCTL_STOP, 0) < 0)
            {
              printf("stop GNSS ERROR\n");
            }
          else
            {
              printf("stop GNSS OK\n");
            }

          ret = ioctl(fd, CXD56_GNSS_IOCTL_SAVE_BACKUP_DATA, 0);
          if (ret < 0)
            {
              printf("backup error\n");
            }

          /* Record result */
          result_sec[count] = coldstartoffset_sec + posfix_sec - start_sec;

          set_stmod = param_stmod;
        }

      /* Dump result */
      for (count = 0; count < STMOD_LOOP_COUNT; count++)
        {
          uint32_t dmp_sec  = (uint32_t)result_sec[count];
          uint32_t dmp_usec =
            (uint32_t)((result_sec[count] - (double)dmp_sec) * 1000000.0);
          printf("result%d, %d.%d sec\n", count, dmp_sec, dmp_usec);
        }

    }

_err:
  /* Release GNSS file descriptor. */

  fd_close(fd);

  printf("%s() out %d\n", __func__, ret);

  return ret;
}
