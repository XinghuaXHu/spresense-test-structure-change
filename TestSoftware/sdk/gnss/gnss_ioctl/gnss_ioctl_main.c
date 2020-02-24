/****************************************************************************
 * test/gnss_ioctl/gnss_ioctl_main.c
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
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <arch/chip/gnss.h>
#include <stdlib.h>
#include "gnss_ioctl_main.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/
typedef int (*TestFunc)(int argc, char *argv[]);

struct cxd56_gnss_test_table_s
{
  TestFunc cbFunc;
  char     *message;
};

/****************************************************************************
 * Public Data
 ****************************************************************************/
struct cxd56_gnss_positiondata_s g_gnss_posdat;
uint32_t                         g_gnss_posfixflag;

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct cxd56_gnss_test_table_s gnss_test_table[] = {
  {gnss_test_all,                     "run all test                 "},
  {gnss_test_ope_mode,                "IOCTL_SET_OPE_MODE           "},
  {gnss_test_select_satellite_system, "IOCTL_SELECT_SATELLITE_SYSTEM"},
  {gnss_test_start,                   "IOCTL_START                  "},
  {gnss_test_set_receiver_position,   "IOCTL_SET_RECEIVER_POSITION  "},
  {gnss_test_set_time,                "IOCTL_SET_TIME               "},
  {gnss_test_backup_data,             "IOCTL_SAVE_BACKUP_DATA       "},
  {gnss_test_set_cep_data,            "IOCTL_SET_CEP_DATA           "},
  {gnss_test_set_tcxo_offset,         "IOCTL_SET_TCXO_OFFSET        "},
  {gnss_test_set_almanac,             "IOCTL_SET_ALMANAC            "},
  {gnss_test_set_ephemeris,           "IOCTL_SET_EPHEMERIS          "},
};
static const uint32_t gnss_test_count =
  (uint32_t)(sizeof(gnss_test_table) / sizeof(gnss_test_table[0]));

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: reset_ioctl()
 *
 * Description:
 *   Set default IOCTL parameters.
 *
 * Input Parameters:
 *   void.
 *
 * Returned Value:
 *   Description of the value returned by this function (if any),
 *   including an enumeration of all possible error values.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

static int reset_ioctl(void)
{
  const struct cxd56_gnss_ope_mode_param_s ope_mode_param = { 1, 1000 };
  const uint32_t default_ioctl_table[][2] = {
    {
      CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM,     /* Command */
      (CXD56_GNSS_SAT_GPS | CXD56_GNSS_SAT_GLONASS) /* Param   */
    },
    {
       CXD56_GNSS_IOCTL_SET_OPE_MODE,               /* Command */
       (uint32_t)&ope_mode_param                    /* Param   */
    },
  };
  const uint32_t default_ioctl_table_count =
    (uint32_t)(sizeof(default_ioctl_table) / (sizeof(uint32_t) * 2));

  int ret = ERROR;
  int fd  = fd_open();  /* Open */
  uint32_t count;
  uint32_t command;
  uint32_t param;

  if (fd > 0)
    {
      /* Set parameters according to table */

      for (count = 0; count < default_ioctl_table_count; count++)
        {
          command = default_ioctl_table[count][0];
          param   = default_ioctl_table[count][1];

          /* Set IOCTL */
          ret = ioctl(fd, command, param);
          if (ret < OK)
            {
              printf("commant%d error\n", command);
              break;
            }
        }

      /* Close */

      fd_close(fd);
    }

  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: double_to_dmf()
 *
 * Description:
 *   Convert from double format to degree-minute-frac format.
 *
 * Input Parameters:
 *   x   - double value.
 *   dmf - Address to store the conversion result.
 *
 * Returned Value:
 *   void.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

void gnss_double_to_dmf(double x, struct cxd56_gnss_dms_s *dmf)
{
  int    b;
  int    d;
  int    m;
  double f;
  double t;

  if (x < 0)
    {
      b = 1;
      x = -x;
    }
  else
    {
      b = 0;
    }
  d = (int)x; /* = floor(x), x is always positive */
  t = (x - d) * 60;
  m = (int)t; /* = floor(t), t is always positive */
  f = (t - m) * 10000;

  dmf->sign   = b;
  dmf->degree = d;
  dmf->minute = m;
  dmf->frac   = f;
}

/****************************************************************************
 * Name: fd_open()
 *
 * Description:
 *   Open File descriptor.
 *
 * Input Parameters:
 *   void.
 *
 * Returned Value:
 *   File descriptor; NULL on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int fd_open(void)
{
  /* Get file descriptor to control GNSS. */

  int fd = open("/dev/gps", O_RDONLY);
  if (fd < 0)
    {
      printf("open error:%d,%d\n", fd, errno);
    }
  return fd;
}

/****************************************************************************
 * Name: fd_close()
 *
 * Description:
 *   Close File descriptor.
 *
 * Input Parameters:
 *   fd   - File descriptor.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int fd_close(int fd)
{
  /* Release GNSS file descriptor. */

  int ret = close(fd);
  if (ret != OK)
    {
      printf("close error\n");
    }
  return ret;
}

/****************************************************************************
 * Name: dioctl()
 *
 * Description:
 *   Call IOCTL and output debug message.
 *
 * Input Parameters:
 *   fd   - File descriptor.
 *   req  - IOCTL request.
 *   arg  - IOCTL argument.
 *   dmsg - debug message.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int dioctl(int fd, int req, unsigned long arg, char *dmsg)
{
  /* Call IOCTL */

  int ret = ioctl(fd, req, arg);

  /* Debug message specified? */

  if (dmsg != NULL)
    {
      /* Check result */

      if (ret != OK)
        {
          /* Error case */

          printf("%s:error!!\n", dmsg);
        }
      else
        {
          /* Normal case */

          printf("%s:OK\n", dmsg);
        }
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_test_all()
 *
 * Description:
 *   Run all tests.
 *
 * Input Parameters:
 *   argc - Follow each test.
 *   argv - Follow each test.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int gnss_test_all(int argc, char *argv[])
{
  int      result[gnss_test_count];
  int      ret = OK;
  uint32_t count;

  result[0] = OK;   /* Initialize unused variable  */

  /* Program start */

  /* Run all tests */

  for (count = 1; count < gnss_test_count; count++)
    {
      /* Reset ioctl */

      reset_ioctl();

      /* Call test function */

      printf("\n");
      printf("call %s\n", gnss_test_table[count].message);
      result[count] = gnss_test_table[count].cbFunc(argc, argv);
    }

  /* Print and check result */

  printf("#      , test ioctl                   , result\n");
  for (count = 1; count < gnss_test_count; count++)
    {
      printf("test %d, %s, ret %d\n", count, gnss_test_table[count].message,
             result[count]);

      /* Check result */

      if (result[count] != OK)
        {
          /* Set error code */

          ret = result[count];
        }
    }

  return ret;
}

/****************************************************************************
 * Name: gnss_ioctl_main()
 *
 * Description:
 *   Execute the test pattern numbered by argument.
 *
 * Input Parameters:
 *   argv[1] - test number
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int gnss_ioctl_main(int argc, char *argv[])
#endif
{
  int      ret;
  uint32_t count;

  /* Program start */

  printf("Hello, IOCTL SAMPLE!!\n");

  /* Check argument */

  if (argc < 2)
    {
      /* Print test  */

      printf("set test No\n");
      for (count = 0; count < gnss_test_count; count++)
        {
          printf("gnss_ioctl %d:%s\n", count, gnss_test_table[count].message);
        }
      ret = OK;
    }
  else
    {
      /* Check argument */

      count = atoi(argv[1]);
      if (count >= gnss_test_count)
        {
          printf("argument error!! %d\n", count);
          ret = ERROR;
        }
      else if (gnss_test_table[count].cbFunc == NULL)
        {
          printf("table error!! cbFunc==NULL\n");
          ret = ERROR;
        }
      else
        {
          /* Run test */

          /* Reset ioctl */

          reset_ioctl();

          /* Call test function */

          printf("\n");
          printf("call %s\n", gnss_test_table[count].message);
          sleep(1);
          ret = gnss_test_table[count].cbFunc(argc, argv);
        }
    }

  printf("End of IOCTL Sample:%d\n", ret);

  return ret;
}
