/****************************************************************************
 * test/gnss_ioctl/gnss_test_select_satellite_system.c
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
static uint32_t wait_count;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

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
  if (g_gnss_posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
    {
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

_err:
  return ret;
}

/****************************************************************************
 * Name: gnss_test_select_satellite_system()
 *
 * Description:
 *   Select satellite system and confirm positioning with each setting.
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

int gnss_test_select_satellite_system(int argc, char *argv[])
{
  const uint32_t satellite_system_table[] = {
    CXD56_GNSS_SAT_GPS | CXD56_GNSS_SAT_GLONASS,    /* idle */
    CXD56_GNSS_SAT_GPS | CXD56_GNSS_SAT_GLONASS,    /* test1 */
    CXD56_GNSS_SAT_GPS,                             /* test2 */
    CXD56_GNSS_SAT_GLONASS                          /* test3 */
  };
  const uint32_t table_count =
    sizeof(satellite_system_table) / sizeof(uint32_t);
  int      fd;
  int      ret;
  int      ret_tmp;
  int      result[table_count];
  int      i = 0;
  uint32_t count;
  uint32_t svcount;
  uint32_t set_satellite_system;
  uint32_t get_satellite_system;
  struct pollfd fds[GNSS_POLL_FD_NUM] = { {0} };

  /* Program start */

  printf("%s() in\n", __func__);

  /* Get file descriptor to control GNSS. */

  fd = fd_open();
  if (fd < 0)
    {
      return -ENODEV;
    }

  /* Set parameters */

  uint32_t   srart_mode     = CXD56_GNSS_STMOD_COLD;
  uint32_t   posperiod      = 10;
  const char *satellite_dbg = "pre_test:idle";

  for (count = 0; count < table_count; count++)
    {

      /* Initial positioning measurement becomes cold start if specified hot
       * start, so working period should be long term to receive ephemeris. */

      g_gnss_posfixflag = 0;
      wait_count = 0;

      /* Set parameter */

      set_satellite_system = satellite_system_table[count];

      /* Set debug message */

      switch (set_satellite_system)
        {
        case CXD56_GNSS_SAT_GLONASS:
          satellite_dbg = "GLN";
          break;
        case (CXD56_GNSS_SAT_GPS | CXD56_GNSS_SAT_GLONASS):
          satellite_dbg = "GPS&GLN";
          break;
        case CXD56_GNSS_SAT_GPS:
          satellite_dbg = "GPS";
          break;
        default:
          satellite_dbg = "unknown";
          break;
        }

      /* Set INVALID VALUE */

      get_satellite_system = SAT_SELECT_INVAL;

      printf("test count %d:CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM(%s)\n",
             count, satellite_dbg);

      /* Set IOCTL */

      ret = ioctl(fd, CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM,
                  set_satellite_system);
      if (ret < 0)
        {
          printf("ioctl:CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM NG!!\n");
          goto _err;
        }

      /* Get IOCTL */

      ret = ioctl(fd, CXD56_GNSS_IOCTL_GET_SATELLITE_SYSTEM,
                  (uint32_t)&get_satellite_system);
      if (ret < 0)
        {
          printf("ioctl:CXD56_GNSS_IOCTL_GET_SATELLITE_SYSTEM NG!!\n");
          goto _err;
        }

      /* Compare */

      if (set_satellite_system != get_satellite_system)
        {
          printf("CXD56_GNSS_IOCTL_SELECT/GET_SATELLITE_SYSTEM NG!!\n");
          goto _err;
        }

      /* Start GNSS. */

      ret = ioctl(fd, CXD56_GNSS_IOCTL_START, srart_mode);
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

      if (count == 0)
        {
          /* Set next parameter */

          srart_mode    = CXD56_GNSS_STMOD_HOT;
          posperiod     = IDLE_COUNT;
          result[count] = OK;
        }
      else
        {
          /* check satellite type */

          uint32_t result_satellite_type = 0;
          struct cxd56_gnss_sv_s *psvdata = NULL;

          /* Check all satellite */

          for (svcount = 0; svcount < CXD56_GNSS_MAX_SV_NUM; svcount++)
            {
              if (svcount >= g_gnss_posdat.svcount)
                {
                  break;
                }
              psvdata = &g_gnss_posdat.sv[svcount];
              result_satellite_type |= psvdata->type;
              printf("sv%d result:type:%d, svid:%d, stat:%d, ",
                     svcount, psvdata->type, psvdata->svid, psvdata->stat);
              printf("elevation:%d, azimuth:%d\n",
                     psvdata->elevation, psvdata->azimuth);
            }

          /* Compare */

          if (result_satellite_type == set_satellite_system)
            {
              result[count] = OK;
            }
          else
            {
              result[count] = ERROR;
            }
        }

    }

  /* Check result */

  for (count = 0; count < table_count; count++)
    {
      printf("result%d %d\n", count, result[count]);
      if (result[count] != OK)
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
