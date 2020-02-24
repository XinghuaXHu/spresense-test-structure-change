/****************************************************************************
 * sqa/singlefunction/power_test/power_test.c
 *
 *   Copyright (C) 2017 Sony Corporation
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
 * 3. Neither the name NuttX nor Sony nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
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

#include <sdk/config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <asmp/mpshm.h>
#include <arch/chip/pm.h>
#include <fcntl.h>
#include <errno.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static mpshm_t s_shm[CONFIG_TEST_MAX_TILE_NUM];

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static int tile_test(void)
{
  int i;
  int ret;

  printf("Tile power on start\n");
  sleep(CONFIG_POWER_TEST_SLEEP_TIME);

  for (i = 0; i < CONFIG_TEST_MAX_TILE_NUM ; i++)
    {
      printf("mpshm_init num=%d\n", i);
      ret = mpshm_init(&s_shm[i], 1, 1024 * 128);
      if (ret < 0)
        {
          printf("mpshm_init() failure. %d\n", ret);
          return 1;
        }
      printf("pon:sleep start %d\n", i);
      sleep(CONFIG_POWER_TEST_SLEEP_TIME);
      printf("pon:sleep end %d\n", i);
    }

  printf("Tile power off start\n");

  for (i = CONFIG_TEST_MAX_TILE_NUM - 1; i >= 0 ; i--)
    {
      printf("mpshm_destroy num=%d\n", i);
      ret = mpshm_destroy(&s_shm[i]);
      if (ret < 0)
        {
          printf("mpshm_destroy() failure. %d\n", ret);
        }
      printf("poff:sleep start %d\n", i);
      sleep(CONFIG_POWER_TEST_SLEEP_TIME);
      printf("poff:sleep end %d\n", i);
    }

  printf("Exit tile test\n");

  return 0;
}

static int gps_test(void)
{
  int      fd;
  int      ret;

  printf("GPS power test\n");
  sleep(CONFIG_POWER_TEST_SLEEP_TIME);
  printf("sleep end\n");

  fd = open("/dev/gps", O_RDONLY);
  if (fd < 0)
    {
      printf("open error:%d,%d\n", fd, errno);
      return -ENODEV;
    }

  printf("GPS power on\n");
  sleep(CONFIG_POWER_TEST_SLEEP_TIME);
  printf("sleep end\n");

  printf("GPS power off start\n");
  ret = close(fd);
  if (ret < 0)
    {
      printf("close error\n");
    }

  printf("GPS power off\n");
  sleep(CONFIG_POWER_TEST_SLEEP_TIME);
  printf("sleep end\n");

  printf("Exit gps test\n");

  return 0;
}

static void show_usage(FAR const char *progname)
{
  printf("\nUsage: %s [ tile | gps | help]\n\n",
         progname);
  printf("Description:\n");
  printf(" Power test operation\n");
  printf("Parameter:\n");
  printf(" tile : test tile power on/off.\n");
  printf(" gps : test gps power on/off.\n");
  printf(" help : Show this message\n");
}

/****************************************************************************
 * power_test_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int power_test_main(int argc, char *argv[])
#endif
{
  int ret;

  if (argc <= 1)
  {
    show_usage(argv[0]);
    return EXIT_FAILURE;
  }

  if (0 == strncmp("tile", argv[1], 4))
    {
      ret = tile_test();
      if (ret)
        {
          printf("Tile test failure.\n");
        }
    }
  else if (0 == strncmp("gps", argv[1], 3))
    {
      ret = gps_test();
      if (ret)
        {
          printf("GPS test failure.\n");
        }
    }
  else
    {
      show_usage(argv[0]);
      return EXIT_FAILURE;
    }

  printf("Exit power test\n");

  return 0;
}
