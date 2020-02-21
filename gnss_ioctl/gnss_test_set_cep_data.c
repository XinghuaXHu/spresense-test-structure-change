/****************************************************************************
 * test/gnss_ioctl/gnss_test_set_cep_data.c
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

#include <errno.h>
#include <stdio.h>
#include <poll.h>
#include <arch/chip/gnss.h>
#include "gnss_ioctl_main.h"

/****************************************************************************
 * Extracted from sys_flash_mgr.h
 ****************************************************************************/

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define TEST_COUNT      3
#define LOOP_COUNT      (1 + (TEST_COUNT * 2))
#define CEPAGE_MAX      32

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: mjd2ymd()
 *
 * Description:
 *   Convert Modified Julian Date(double) to date(struct cxd56_gnss_date_s)
 *
 * Input Parameters:
 *   MJD   - Julian Date(convert src)
 *   pdate - date(convert dst)
 *
 * Returned Value:
 *   void.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

static void mjd2ymd(double MJD, struct cxd56_gnss_date_s *pdate)
{
  /* Check argument */

  if (pdate == NULL)
    {
      return;
    }

  /* Convert Modified Julian Date to date */

  double JD12 = (uint32_t)(MJD + 2400001);
  double e    = (uint32_t)((JD12 - 1867216.25) / 36524.25);
  double g    = JD12 + (e - (uint32_t)(e / 4) + 1) + 1524;
  double h    = (uint32_t)((g - 122.1) / 365.25);
  double i    = (uint32_t)(365.25 * h);
  double j    = (uint32_t)((g - i) / 30.6001);

  double d    = g - i - (uint32_t)(30.6001 * j);
  double m    = j - 12 * (uint32_t)(j / 14) - 1;

  double w    = h - 4716 + (uint32_t)((14 - m) / 12);
  double y;
  if (w > 0)
    {
      y = w;
    }
  else
    {
      y = w - 1;
    }

  /* Set result */

  pdate->day   = (uint8_t)d;
  pdate->month = (uint8_t)m;
  pdate->year  = (uint16_t)y;

  return;
}

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
  struct cxd56_gnss_dms_s  dmf;
  struct cxd56_gnss_time_s *ptime;

  /* Read POS data */

  ret = read(fd, &g_gnss_posdat, sizeof(g_gnss_posdat));
  if (ret < OK)
    {
      printf("read error\n");
    }
  else if (ret < sizeof(g_gnss_posdat))
    {
      ret = ERROR;
      printf("read size error\n");
    }
  else
    {
      ret = OK;

      /* Print POS data */

      ptime = &(g_gnss_posdat.receiver.time);
      printf(">H:%2d,m:%2d,s:%2d,u:%6d, svCount=%2d, ",
             ptime->hour, ptime->minute,
             ptime->sec, ptime->usec, g_gnss_posdat.svcount);
      if (g_gnss_posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
        {
          g_gnss_posfixflag = 1;

          gnss_double_to_dmf(g_gnss_posdat.receiver.latitude, &dmf);
          printf("LAT %d.%d.%04d, ", dmf.degree, dmf.minute, dmf.frac);

          gnss_double_to_dmf(g_gnss_posdat.receiver.longitude, &dmf);
          printf("LNG %d.%d.%04d", dmf.degree, dmf.minute, dmf.frac);
        }
      printf("\n");
    }

  return ret;
}

/****************************************************************************
 * Name: check_cep_date()
 *
 * Description:
 *   Compare cep date and current date.
 *
 * Input Parameters:
 *   start - Start date of validity period.
 *   end   - Termination date of validity period.
 *   test  - Check date.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

static int check_cep_date(struct cxd56_gnss_date_s *start,
                          struct cxd56_gnss_date_s *end,
                          struct cxd56_gnss_date_s *test)
{
  int ret;
  uint32_t int_start = ((start->year * 12 + start->month) * 31 + start->day);
  uint32_t int_end   = ((end->year   * 12 + end->month)   * 31 + end->day);
  uint32_t int_test  = ((test->year  * 12 + test->month)  * 31 + test->day);

  printf("%s()", __func__);
  if (int_start > int_test)
    {
      printf("error 1\n");
      ret = ERROR;
    }
  else if (int_end < int_test)
    {
      printf("error 2\n");
      ret = ERROR;
    }
  else
    {
      printf("ok\n");
      ret = OK;
    }
  printf("  %d/%d/%d\n", start->year, start->month, start->day);
  printf("  %d/%d/%d\n", end->year,   end->month,   end->day);
  printf("  %d/%d/%d\n", test->year,  test->month,  test->day);

  return ret;
}

/****************************************************************************
 * Name: gnss_test_set_cep_data()
 *
 * Description:
 *   Get CEP value and check if it is valid.
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

int gnss_test_set_cep_data(int argc, char *argv[])
{
  int      ret = OK;
  int      ret_tmp;
  int      fd;
  uint32_t count;
  uint32_t time_count;
  uint32_t idle_count;
  uint32_t use_sep_flag;
  unsigned long arg;
  char     *dmsg;
  struct cxd56_gnss_date_s date_start = { 0, 0, 0 };
  struct cxd56_gnss_date_s date_end   = { 0, 0, 0 };
  struct cxd56_gnss_date_s date_test  = { 0, 0, 0 };
  struct pollfd fds[GNSS_POLL_FD_NUM] = { {0} };
  struct cxd56_gnss_cep_age_s cep_age = { 0.0, 0.0 };

  /* Program start */

  printf("%s() in\n", __func__);

  /* Get file descriptor to control GNSS. */

  fd = fd_open();
  if (fd < 0)
    {
      /* Error case */

      return -ENODEV;
    }

  /* Initial positioning measurement becomes cold start if specified hot start, 
   * so working period should be long term to receive ephemeris. */

  for (count = 0; count < LOOP_COUNT; count++)
    {
      time_count        = 0;
      g_gnss_posfixflag = 0;
      use_sep_flag      = count % 2;

      if (count == 0)
        {
          dmsg = "IDLE";
        }
      else if (use_sep_flag)
        {
          /* Cep open */

          dmsg = "CXD56_GNSS_IOCTL_OPEN_CEP_DATA";
          ret_tmp = dioctl(fd, CXD56_GNSS_IOCTL_OPEN_CEP_DATA, 0, dmsg);
          if (ret_tmp != OK)
            {
              /* Error case */

              ret = ret_tmp;

              /* Abandon test */

              break;
            }
          else if (date_start.year == 0)
            {
              /* Cep reset */

              dmsg = "CXD56_GNSS_IOCTL_RESET_CEP_FLAG";
              ret_tmp = dioctl(fd, CXD56_GNSS_IOCTL_RESET_CEP_FLAG, 0, dmsg);
              if (ret_tmp != OK)
                {
                  /* Error case */

                  ret = ret_tmp;

                  /* Abandon test */

                  break;
                }

              /* Get cep age */

              dmsg = "CXD56_GNSS_IOCTL_GET_CEP_AGE";
              arg = (unsigned long)&cep_age;
              ret_tmp = dioctl(fd, CXD56_GNSS_IOCTL_GET_CEP_AGE, arg, dmsg);
              if (ret_tmp != OK)
                {
                  /* Error case */

                  ret = ret_tmp;

                  /* Abandon test */

                  break;
                }
              else if (cep_age.age > CEPAGE_MAX)
                {
                  /* Age error */

                  ret = ERROR;
                  printf("error(cep_age = %d)\n", (int)cep_age.age);

                  /* Abandon test */

                  break;
                }
              else
                {
                  mjd2ymd(cep_age.cepi,               &date_start);
                  mjd2ymd(cep_age.cepi + cep_age.age, &date_end);
                  date_test = g_gnss_posdat.receiver.date;

                  ret_tmp = check_cep_date(&date_start, &date_end, &date_test);
                  if (ret_tmp != OK)
                    {
                      /* Error case */

                      ret = ret_tmp;

                      /* Abandon test */

                      break;
                    }
                }

              /* Check cep data */

              dmsg = "CXD56_GNSS_IOCTL_CHECK_CEP_DATA";
              ret_tmp = dioctl(fd, CXD56_GNSS_IOCTL_CHECK_CEP_DATA, 0, dmsg);
              if (ret_tmp != OK)
                {
                  /* Error case */

                  ret = ret_tmp;

                  /* Abandon test */

                  break;
                }
            }

          dmsg = "USE CEP FILE";
        }
      else
        {
          dmsg = "NOT USE CEP FILE";
        }

      /* Start gnss */

      printf("start GNSS(%d) %s\n", count, dmsg);
      dmsg = "CXD56_GNSS_IOCTL_START";
      arg  = CXD56_GNSS_STMOD_HOT;

      /* Start GNSS. */

      ret_tmp = dioctl(fd, CXD56_GNSS_IOCTL_START, arg, dmsg);
      if (ret_tmp < 0)
        {
          printf("start GNSS ERROR %d\n", errno);
        }
      else
        {
          printf("start GNSS OK\n");

          idle_count = 30;
          fds[0].fd     = fd;
          fds[0].events = POLLIN;

          do
            {
              /* Wait POS notification. */

              if (poll(fds, GNSS_POLL_FD_NUM, GNSS_POLL_TIMEOUT_FOREVER) <= 0)
                {
                  ret = ERROR;
                  printf("poll error %d,%x,%x\n", ret, fds[0].events,
                         fds[0].revents);
                  break;
                }

              /* Read and print POS data. */

              if (read_and_print(fd) < 0)
                {
                  ret = ERROR;
                  break;
                }

              /* Count down started from position fixed. */

              if (g_gnss_posfixflag)
                {
                  idle_count--;
                }
              else
                {
                  time_count++;
                }
            }
          while (idle_count > 0);

          /* Stop gnss */

          dmsg = "CXD56_GNSS_IOCTL_STOP";
          if (dioctl(fd, CXD56_GNSS_IOCTL_STOP, 0, dmsg) < 0)
            {
              printf("stop GNSS ERROR\n");
            }
          else
            {
              printf("stop GNSS OK\n");
            }
          printf("  positioning time %d sec\n", time_count);
        }

      if (use_sep_flag)
        {
          /* Cep close */

          dmsg = "CXD56_GNSS_IOCTL_CLOSE_CEP_DATA";
          ret_tmp = dioctl(fd, CXD56_GNSS_IOCTL_CLOSE_CEP_DATA, 0, dmsg);
          if (ret_tmp < 0)
            {
              printf("stop GNSS ERROR\n");
            }
          else
            {
              printf("stop GNSS OK\n");
            }
        }

      /* Reset gnss */

      dioctl(fd, CXD56_GNSS_IOCTL_START, CXD56_GNSS_STMOD_WARM, NULL);
      dioctl(fd, CXD56_GNSS_IOCTL_STOP, 0, NULL);
      printf("\n");
    }

  /* Release GNSS file descriptor. */

  fd_close(fd);
  fd = 0;

  printf("%s() out %d\n", __func__, ret);

  return ret;
}
