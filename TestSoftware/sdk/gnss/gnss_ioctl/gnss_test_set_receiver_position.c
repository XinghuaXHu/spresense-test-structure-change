/****************************************************************************
 * test/gnss_ioctl/gnss_test_set_receiver_position.c
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
#include <fcntl.h>
#include <poll.h>
#include <arch/chip/gnss.h>
#include <string.h>
#include "gnss_ioctl_main.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define POSITION_MERGIN           0.000100
#define IDLE_COUNT                60

#define RESULT_UNEXECUTED 1
#define RESULT_OK         0
#define RESULT_DIFF_LAT   (-1)
#define RESULT_DIFF_LON   (-2)

/****************************************************************************
 * Private Types
 ****************************************************************************/
struct cxd56_gnss_ioctl_pos_s
{
  const uint32_t command;
  FAR const void *setdata;
  FAR const struct cxd56_gnss_ellipsoidal_position_s *comparedata;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* honatsugi-station */
static const struct cxd56_gnss_ellipsoidal_position_s ellipsoidal_position1 =
  { 35.439339, 139.364338, 10 };

/* increment or decriment case */
static const struct cxd56_gnss_ellipsoidal_position_s ellipsoidal_position2 =
  { 35.000000, 139.999999, 10 };
static const struct cxd56_gnss_orthogonal_position_s orthogonal_position1 =
  { -3947902.449, 3388026.331, 3677724.854 };
static const struct cxd56_gnss_orthogonal_position_s orthogonal_position2 =
  { -4006780.672, 3362088.304, 3637904.674 };
static struct cxd56_gnss_ellipsoidal_position_s start_position;
static const struct cxd56_gnss_ioctl_pos_s ioctl_table[] = {
  {
    CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ELLIPSOIDAL, /* Command */
    &ellipsoidal_position1,                             /* Setdata */
    &ellipsoidal_position1                              /* Comparedata */
  },
  {
    CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ELLIPSOIDAL, /* Command */
    &ellipsoidal_position2,                             /* Setdata */
    &ellipsoidal_position2                              /* Comparedata */
  },
  {
    CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ORTHOGONAL,  /* Command */
    &orthogonal_position1,                              /* Setdata */
    &ellipsoidal_position1                              /* Comparedata */
  },
  {
    CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ORTHOGONAL,  /* Command */
    &orthogonal_position2,                              /* Setdata */
    &ellipsoidal_position2                              /* Comparedata */
  }
};

static const uint32_t ioctl_table_count =
  (uint32_t) (sizeof(ioctl_table) / (sizeof(struct cxd56_gnss_ioctl_pos_s)));

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: compare_potision()
 *
 * Description:
 *   Compare the two positions and check if they are within the error margin.
 *
 * Input Parameters:
 *   pcmp1 - The position notified first.
 *   pcmp2 - Comparative position.
 *
 * Returned Value:
 *   Zero (OK) on success(compare OK); Negative value on error(compare NG).
 *
 * Assumptions/Limitations:
 *   Anything else that one might need to know to use this function.
 *
 ****************************************************************************/

static int compare_potision(const struct cxd56_gnss_ellipsoidal_position_s
                            *pcmp1,
                            const struct cxd56_gnss_ellipsoidal_position_s
                            *pcmp2)
{
  int ret = RESULT_OK;

  /*  Check Latitude */

  if (abs(pcmp1->latitude - pcmp2->latitude) > POSITION_MERGIN)
    {
      /* Latitude is not equal */

      ret = RESULT_DIFF_LAT;
    }

  /*  Check Longitude */

  else if (abs(pcmp1->longitude - pcmp2->longitude) > POSITION_MERGIN)
    {
      /* Longitude is not equal */

      ret = RESULT_DIFF_LON;
    }

  return ret;
}

/****************************************************************************
 * Name: read_and_print()
 *
 * Description:
 *   Read and print POS data.
 *   Store 1st notify position.
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

  /* Store 1st notify position */

  if (start_position.latitude == 0)
    {
      start_position.latitude  = g_gnss_posdat.receiver.latitude;
      start_position.longitude = g_gnss_posdat.receiver.longitude;
    }

  printf(", type %d, pos_fixmode %d", g_gnss_posdat.receiver.type,
         g_gnss_posdat.receiver.pos_fixmode);
  if (g_gnss_posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
    {
      g_gnss_posfixflag = 1;
    }

  /* Print position without reference to posfix. */

  gnss_double_to_dmf(g_gnss_posdat.receiver.latitude, &dmf);
  printf(", LAT %d.%d.%04d", dmf.degree, dmf.minute, dmf.frac);

  gnss_double_to_dmf(g_gnss_posdat.receiver.longitude, &dmf);
  printf(", LNG %d.%d.%04d", dmf.degree, dmf.minute, dmf.frac);

  printf("\n");

_err:
  return ret;
}

/****************************************************************************
 * Name: gnss_test_set_receiver_position()
 *
 * Description:
 *   Compare set position and positioning result.
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

int gnss_test_set_receiver_position(int argc, char *argv[])
{
  int fd;
  int ret = OK;
  int ret_tmp;
  int count;
  int result[ioctl_table_count];
  int i;
  uint32_t posperiod = IDLE_COUNT;
  const FAR struct cxd56_gnss_ioctl_pos_s *ptable;
  struct pollfd fds[GNSS_POLL_FD_NUM] = { {0} };

  /* Program start */

  printf("%s() in\n", __func__);

  /* Get file descriptor to control GNSS. */

  fd = fd_open();
  if (fd < 0)
    {
      return -ENODEV;
    }

  /* Set parameter */

  for (count = 0; count < ioctl_table_count; count++)
    {
      /* Reset gnss */

      dioctl(fd, CXD56_GNSS_IOCTL_START, CXD56_GNSS_STMOD_WARM, NULL);
      dioctl(fd, CXD56_GNSS_IOCTL_STOP, 0, NULL);

      /* Initial positioning measurement becomes cold start if specified hot
       * start, so working period should be long term to receive ephemeris. */

      g_gnss_posfixflag = 0;
      result[count] = RESULT_UNEXECUTED;

      /* Set position */

      /* Set before start */

      ptable = &ioctl_table[count];
      ret = ioctl(fd, ptable->command, (uint32_t)ptable->setdata);
      memset(&start_position, 0x00, sizeof(start_position));
      if (ret < 0)
        {
          printf("SET_RECEIVER_POSITION error\n");
          break;
        }

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

      result[count] = RESULT_OK;
      if (g_gnss_posfixflag != 0)
        {
          /* Compare result */

          result[count] =
            compare_potision(&start_position, ptable->comparedata);
        }
    }

  /* Check result */

  printf("\n");
  for (count = 0; count < ioctl_table_count; count++)
    {
      printf("result%d %d\n", count, result[count]);
      if (result[count] != RESULT_OK)
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
