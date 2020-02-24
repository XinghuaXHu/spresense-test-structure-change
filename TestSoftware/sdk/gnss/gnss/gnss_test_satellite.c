/****************************************************************************
 * test/sqa/singlefunction/gnss/gnss_test_satellite.c
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <arch/chip/gnss.h>
#include "gnss_test_main.h"

#define SV_STAT_POSITIONIG      0x0002
#define TEST_TIMEOUT            600

static int gnss_satellitetest(int argc, char *argv[], int select_satellite)
{
  int      fd;
  int      ret = OK;
  int      ret_tmp;
  int      timecount = 0;
  int      positioning_satellite = 0;

  /* Get file descriptor to control GNSS. */

  fd = open("/dev/gps", O_RDONLY);
  if (fd < 0)
    {
      printf("%s() error:%d,%d\n", __func__, fd, errno);
      return -ENODEV;
    }

  /* Set the signal to notify GNSS events. */

  sigset_t mask;
  ret = gnss_setsignal(fd, &mask);
  if (ret != OK)
    {
      goto _err;
    }

  /* Call SET_OPE_MODE. */

  ret = gnss_setopemode(fd, 1000);
  if (ret != OK)
    {
      goto _err;
    }

  /* Set the type of satellite system used by GNSS. */

  ret = gnss_setsatellite(fd, select_satellite);
  if (ret != OK)
    {
      goto _err;
    }

  /* Start GNSS. */

  ret = gnss_start(fd, CXD56_GNSS_STMOD_COLD);
  if (ret < 0)
    {
      goto _err;
    }

  do
    {
      positioning_satellite = 0;

      /* Wait for positioning to be fixed. After fixed,
       * idle for the specified seconds. */

      ret_tmp = sigwaitinfo(&mask, NULL);
      if (ret_tmp != MY_GNSS_SIG)
        {
          ret = ret_tmp;
          printf("sigwaitinfo error %d\n", ret);
          break;
        }

      /* Read POS data. */

      int read_result;
      read_result = read(fd, &gnss_posdat, sizeof(gnss_posdat));
      if (read_result < 0)
        {
          ret = ERROR;
          printf("read error\n");
          break;
        }
      else if (read_result != sizeof(gnss_posdat))
        {
          ret = ERROR;
          printf("read size error\n");
          break;
        }

      printf(">%02d:%02d:%02d.%06d, ",
             gnss_posdat.receiver.time.hour, gnss_posdat.receiver.time.minute,
             gnss_posdat.receiver.time.sec, gnss_posdat.receiver.time.usec);

      printf("sat %2d, ", gnss_posdat.svcount);

      if (gnss_posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
        {

          /* Print POS data. */

          int32_t upper;
          int32_t lower;

          upper = (int32_t)gnss_posdat.receiver.latitude;
          lower = (int32_t)((gnss_posdat.receiver.latitude - (double)upper) * 1000000);
          printf("LAT %d.%06d, ", upper, lower);

          upper = (int32_t)gnss_posdat.receiver.longitude;
          lower = (int32_t)((gnss_posdat.receiver.longitude - (double)upper) * 1000000);
          printf("LNG %d.%06d\n", upper, lower);

          /* Check satellite type. */

          int cnt;
          for(cnt = 0; cnt<gnss_posdat.svcount; cnt++)
            {
              if(gnss_posdat.sv[cnt].stat & SV_STAT_POSITIONIG)
                {
                  positioning_satellite |= gnss_posdat.sv[cnt].type;
                }
            }
        }
      else
        {
          printf("No Positioning Data\n");
        }

      /* Compare satellite type. */

      if(positioning_satellite == select_satellite)
        {
          break;
        }

      timecount++;
    }
  while (timecount < TEST_TIMEOUT);

  /* Print satellites. */

  int cnt;
  for(cnt = 0; cnt<gnss_posdat.svcount; cnt++)
    {
      printf("sv[%2d]:", cnt);
      if(gnss_posdat.sv[cnt].type & CXD56_GNSS_SAT_GPS)
        {
          printf("GPS,     ");
        }
      else if(gnss_posdat.sv[cnt].type & CXD56_GNSS_SAT_GLONASS)
        {
          printf("GLONASS, ");
        }
      else
        {
          printf("UNKNOWN, ");
        }

      printf("ID=%2d", gnss_posdat.sv[cnt].svid);

      if(gnss_posdat.sv[cnt].stat & 0x0001)
        {
          printf(", Tracking");
        }
      if(gnss_posdat.sv[cnt].stat & 0x0002)
        {
          printf(", Positioning");
        }
      if(gnss_posdat.sv[cnt].stat & 0x0004)
        {
          printf(", Calculating velocity");
        }
      if(gnss_posdat.sv[cnt].stat & 0x0008)
        {
          printf(", Visible satellite");
        }

      printf(".\n");
    }

  /* Stop GNSS. */

  gnss_stop(fd);

_err:

  /* GNSS firmware needs to disable the signal after positioning. */

  gnss_clearsignal(fd, &mask);

  /* Release GNSS file descriptor. */

  close(fd);

  /* Check result. */

  if ((timecount != 0) && (positioning_satellite != select_satellite))
    {
      ret = ERROR;
    }

  return ret;
}

int gnss_satellite(int argc, char *argv[])
{
  int ret = OK;
  int ret_tmp;
  int select_satellite;
  int cnt;

  /* Reserv test pattern. */

  int test_pattern[] = {
    CXD56_GNSS_SAT_GPS,     // Test Link 14948
    CXD56_GNSS_SAT_GLONASS, // Test Link 14949
    CXD56_GNSS_SAT_GPS | CXD56_GNSS_SAT_GLONASS,    // Test Link 14950
  };
  int test_count = sizeof(test_pattern) / sizeof(int);

  /* Check argument. */

  if (argv[2] != NULL)
    {
      /* Run single test. */

      /* Check argument. */

      cnt = atoi(argv[2]);
      if( cnt > test_count)
        {
          cnt = test_count;
        }
      else if(cnt < 0)
        {
          cnt = 0;
        }

      /* Set test pattern. */

      select_satellite = test_pattern[cnt];

      /* Call test. */

      ret = gnss_satellitetest(argc, argv, select_satellite);

      printf("%s(%d) ret %d\n", __func__, select_satellite, ret);
    }
  else
    {
      /* Run all test. */

      for(cnt = 0; cnt < test_count; cnt++)
        {
          /* Set test pattern. */

          select_satellite = test_pattern[cnt];

          /* Call test. */

          ret_tmp = gnss_satellitetest(argc, argv, select_satellite);

          printf("%s(%d) ret %d\n", __func__, select_satellite, ret_tmp);

          /* Check result. */

          if(ret_tmp != OK)
            {
              ret = ret_tmp;
            }
        }
    }

  printf("%s() done\n", __func__);

  return ret;
}
