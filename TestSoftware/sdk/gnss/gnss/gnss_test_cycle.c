/****************************************************************************
 * test/sqa/singlefunction/gnss/gnss_test_cycle.c
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
#include <string.h>
#include <fcntl.h>
#include <arch/chip/gnss.h>
#include "gnss_test_main.h"

#define TEST_TIMEOUT            (60*3)

static int test_cycle;

static int gnss_cycle_test(int argc, char *argv[])
{
  int      fd;
  int      ret = OK;
  int      ret_tmp;
  int      timecount = 0;
  int      select_satellite = CXD56_GNSS_SAT_GPS | CXD56_GNSS_SAT_GLONASS;
  struct cxd56_gnss_time_s *pTime;
  int PosSec[3] = {0,};

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

  ret = gnss_setopemode(fd, test_cycle);
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

  ret = gnss_start(fd, CXD56_GNSS_STMOD_HOT);
  if (ret < 0)
    {
      goto _err;
    }

  int test_cycle_sec = (int)test_cycle / 1000;

  do
    {

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

      pTime = &gnss_posdat.receiver.time;
      printf(">%02d:%02d:%02d.%06d, ", pTime->hour, pTime->minute, pTime->sec, pTime->usec);

      PosSec[2] = PosSec[1];
      PosSec[1] = PosSec[0];
      PosSec[0] = ((pTime->hour * 60) + pTime->minute * 60) + pTime->sec;

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

          timecount += test_cycle_sec;
        }
      else
        {
          printf("No Positioning Data\n");
        }
    }
  while (timecount < TEST_TIMEOUT);

  /* Stop GNSS. */

  gnss_stop(fd);

  /* Set timeout 2 seconds, SCU may send signal every 1 second. */

  printf("Wait for delayed signal.\n");
  struct timespec timeout;
  timeout.tv_sec  = test_cycle_sec;
  timeout.tv_nsec = 0;
  ret_tmp = sigtimedwait(&mask, NULL, &timeout);
  if (ret_tmp == MY_GNSS_SIG)
  {
    ret = ERROR;
    printf("error! signal comes after stop command.\n");
  }

_err:

  /* GNSS firmware needs to disable the signal after positioning. */

  gnss_clearsignal(fd, &mask);

  /* Release GNSS file descriptor. */

  close(fd);

  /* Check result. */

  if (timecount != 0)
    {
      if( test_cycle_sec != (PosSec[0]-PosSec[1]))
      {
        printf("Check cycle error1\n");
        ret = ERROR;
      }
      else if( test_cycle_sec != (PosSec[1]-PosSec[2]))
      {
        printf("Check cycle error2\n");
        ret = ERROR;
      }
      else
      {
        printf("Check cycle OK\n");
      }

      /* Please try again at 23:59 -> 00:00 during the test. */

    }
  else
    {
      printf("Pos not fixed!!\n");
      ret = ERROR;
    }

  return ret;
}

int gnss_cycle(int argc, char *argv[])
{
  int ret = OK;
  int ret_tmp;

  /* Reserv test pattern. */

  int test_pattern[] = {

   /* Test Link 156260. */

    1000,
    10000,
    60000,
  };
  int test_count = sizeof(test_pattern) / sizeof(int);

  int cnt;
  for(cnt = 0; cnt < test_count; cnt++)
  {

    /* Set test pattern. */

    test_cycle = test_pattern[cnt];

    /* Call test. */

    ret_tmp = gnss_cycle_test(argc, argv);

    printf("%s(%d) ret %d\n", __func__, test_cycle, ret_tmp);

    /* Check result. */

    if(ret_tmp != OK)
    {
      ret = ret_tmp;
    }
  }

  printf("%s() done\n", __func__);

  return ret;
}
