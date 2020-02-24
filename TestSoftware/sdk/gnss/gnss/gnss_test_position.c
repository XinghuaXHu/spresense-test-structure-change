/****************************************************************************
 * test/sqa/singlefunction/gnss/gnss_test_position.c
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
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
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

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: gnss_hotstart()
 *
 * Description:
 *   Run positioning.
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

int gnss_starttest(int argc, char *argv[], uint32_t stmod)
{
  int      fd;
  int      ret;
  int      tmpret;
  int      nofixperiod;

  /* Program start. */

  nofixperiod = atoi(argv[2]);
  if (nofixperiod == 0)
    {
      printf("invalid nofixperiod!!\n");
      return -EINVAL;
    }

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

  posfixflag = 0;

  /* Start GNSS. */

  ret = gnss_start(fd, stmod);
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

          break;
        }
      nofixperiod--;
    }
  while (nofixperiod > 0);

  if (nofixperiod == 0)
    {
      printf("GNSS NO FIX\n");
      ret = ERROR;
    }

  /* Stop GNSS. */

  tmpret = gnss_stop(fd);
  if (tmpret < 0)
    {
      if (ret >= 0)
        {
          ret = tmpret;
        }
    }
  else
    {
      printf("stop GNSS OK\n");
    }

_err:

  /* GNSS firmware needs to disable the signal after positioning. */

  tmpret = gnss_clearsignal(fd, &mask);
  if (tmpret < 0)
    {
      printf("clearsignal error\n");
      if (ret >= 0)
        {
          ret = tmpret;
        }
    }

  /* Release GNSS file descriptor. */

  tmpret = close(fd);
  if (tmpret < 0)
    {
      printf("close error\n");
      if (ret >= 0)
        {
          ret = tmpret;
        }
    }

  printf("%s() done\n", __func__);

  return ret;
}

/****************************************************************************
 * Name: gnss_hotrepeat()
 *
 * Description:
 *   Run positioning.
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

int gnss_hotrepeat(int argc, char *argv[])
{
  int      fd;
  int      ret;
  int      tmpret;
  int      nofixperiod;
  int      i;

  /* Program start. */

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
  if (ret < 0)
    {
      printf("signal error\n");
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

  for (i = 0; i < 100 ; i++)
    {
      nofixperiod = 10;
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

              break;
            }
          nofixperiod--;
        }
      while (nofixperiod > 0);

      if (nofixperiod == 0)
        {
          printf("GNSS NO FIX\n");
          ret = ERROR;
        }

      /* Stop GNSS. */

      tmpret = gnss_stop(fd);
      if (tmpret < 0)
        {
          printf("stop GNSS Error Count=%d\n", i+1);
          if (ret >= 0)
            {
              ret = tmpret;
            }
          goto _err;
        }
      else
        {
          printf("stop GNSS OK Count=%d\n", i+1);
          if (ret < 0)
            {
              goto _err;
            }
        }
    }

_err:

  /* GNSS firmware needs to disable the signal after positioning. */

  tmpret = gnss_clearsignal(fd, &mask);
  if (tmpret < 0)
    {
      printf("clearsignal error\n");
      if (ret >= 0)
        {
          ret = tmpret;
        }
    }

  /* Release GNSS file descriptor. */

  tmpret = close(fd);
  if (tmpret < 0)
    {
      printf("close error\n");
      if (ret >= 0)
        {
          ret = tmpret;
        }
    }

  printf("%s() done\n", __func__);

  return ret;
}

/****************************************************************************
 * Name: gnss_signalonoff()
 *
 * Description:
 *   Run positioning.
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

int gnss_signalonoff(int argc, FAR char *argv[])
{
  int      fd;
  int      ret;
  int      tmpret;
  int      posperiod;
  int      nofixperiod;

  /* Program start. */

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

  posperiod  = 180;
  nofixperiod = 300;
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
      else
        {
          nofixperiod--;
        }
    }
  while ((posperiod > 0) && (nofixperiod > 0));

  if (nofixperiod == 0)
    {
      printf("GNSS NO FIX\n");
      ret = ERROR;
    }

  /* Stop GNSS. */

  tmpret = gnss_stop(fd);
  if (tmpret < 0)
    {
      if (ret >= 0)
        {
          ret = tmpret;
        }
    }
  else
    {
      printf("stop GNSS OK\n");
    }

_err:

  /* GNSS firmware needs to disable the signal after positioning. */

  tmpret = gnss_clearsignal(fd, &mask);
  if (tmpret < 0)
    {
      printf("clearsignal error\n");
      if (ret >= 0)
        {
          ret = tmpret;
        }
    }

  /* Release GNSS file descriptor. */

  tmpret = close(fd);
  if (tmpret < 0)
    {
      printf("close error\n");
      if (ret >= 0)
        {
          ret = tmpret;
        }
    }

  printf("%s() done\n", __func__);

  return ret;
}

