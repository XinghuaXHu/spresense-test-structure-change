/****************************************************************************
 * test/gnss_ioctl/gnss_test_set_ephemeris.c
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
#define POS_PERIOD_TEST     (60*3)  /* Idling time after posfix */

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

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
  printf("\n");

_err:
  return ret;
}

/****************************************************************************
 * Name: gnss_test_set_ephemeris()
 *
 * Description:
 *   Check ephemeris IOCTL command on each satellite system.
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

int gnss_test_set_ephemeris(int argc, char *argv[])
{
  const uint32_t tbl_gdtype[] = {
    CXD56_GNSS_DATA_GPS,
    CXD56_GNSS_DATA_GLONASS
    /* , CXD56_GNSS_DATA_QZSSL1CA */
  };
  const uint32_t allocsize =
    MAX(CXD56_GNSS_GPS_EPHEMERIS_SIZE, CXD56_GNSS_GLONASS_EPHEMERIS_SIZE);
  const uint32_t gdtype_count =
    (uint32_t)(sizeof(tbl_gdtype) / sizeof(uint32_t));

  int      fd;
  int      ret;
  int      ret_tmp;
  int      result[gdtype_count];
  int      count;
  int      i;
  uint32_t set_stmod;
  struct cxd56_gnss_orbital_param_s orbitalparam;
  struct pollfd fds[GNSS_POLL_FD_NUM] = { {0} };

  /* Program start */

  printf("%s() in\n", __func__);

  /* Malloc */

  orbitalparam.data = malloc(allocsize);
  if (orbitalparam.data == NULL)
    {
      printf("alloc error\n");
      return -ENOMEM;
    }

  /* Get file descriptor to control GNSS. */

  fd = fd_open();
  if (fd < 0)
    {
      ret = -ENODEV;
      goto _err;
    }

  set_stmod = CXD56_GNSS_STMOD_COLD;

  for (count = 0; count < gdtype_count; count++)
    {

      /* Initial positioning measurement becomes cold start if specified hot
       * start, so working period should be long term to receive ephemeris. */

      g_gnss_posfixflag = 0;

      i = POS_PERIOD_TEST;

      /* Start GNSS. */

      ret = ioctl(fd, CXD56_GNSS_IOCTL_START, set_stmod);
      if (ret < 0)
        {
          printf("test count %d:start GNSS ERROR %d\n", count, errno);
          goto _err;
        }
      else
        {
          printf("test count %d:start GNSS OK\n", count);
        }

      fds[0].fd     = fd;
      fds[0].events = POLLIN;

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

      /* Set param */

      orbitalparam.type = tbl_gdtype[count];
      memset(orbitalparam.data, 0x00, allocsize);

      /* Get */

      ret_tmp = ioctl(fd, CXD56_GNSS_IOCTL_GET_EPHEMERIS,
                      (unsigned long)&orbitalparam);

      /* Check ret_tmp */

      if (ret_tmp < 0)
        {
          printf("CXD56_GNSS_IOCTL_GET_EPHEMERIS() NG!!\n");
          /* Continue test */

        }
      else
        {
          printf("CXD56_GNSS_IOCTL_GET_EPHEMERIS() OK\n");

          /* Set */

          ret_tmp = ioctl(fd, CXD56_GNSS_IOCTL_SET_EPHEMERIS,
                          (unsigned long)&orbitalparam);

          /* Check ret_tmp */

          if (ret_tmp < 0)
            {
              printf("CXD56_GNSS_IOCTL_SET_EPHEMERIS() NG!!\n");
              /* Continue test */

            }
          else
            {
              printf("CXD56_GNSS_IOCTL_SET_EPHEMERIS() OK\n");
            }
        }
      result[count] = ret_tmp;

      set_stmod = CXD56_GNSS_STMOD_HOT;
    }

  /* Check result */
  for (count = 0; count < gdtype_count; count++)
    {
      if (result[count] != 0)
        {
          ret = result[count];
        }
    }
_err:

  /* Free */

  if (orbitalparam.data != NULL)
    {
      free(orbitalparam.data);
      orbitalparam.data = NULL;
    }

  /* Release GNSS file descriptor. */

  if (fd >= 0)
    {
      fd_close(fd);
    }

  printf("%s() out %d\n", __func__, ret);

  return ret;
}
