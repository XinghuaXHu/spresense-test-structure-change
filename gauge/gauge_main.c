/****************************************************************************
 * examples/gauge/gauge_main.c
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

#include <sdk/config.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>

#include <nuttx/power/battery_gauge.h>
#include <nuttx/power/battery_ioctl.h>

#include <arch/chip/battery_ioctl.h>
#include <arch/board/board.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define DEVPATH "/dev/bat0"

#define VALIDATE(_r) \
if ((_r) < 0) \
  { \
    printf("test failed at %d\n", __LINE__); \
    return -1; \
  } \

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int basic_test(int fd)
{
  enum battery_gauge_status_e status;
  b16_t capacity;
  b16_t vol;
  const char *statestr[] =
    {
      "UNKNOWN",
      "IDLE",
      "FULL",
      "CHARGING",
      "DISCHARGING"
    };
  int ret;
  struct timeval tv;

  ret = ioctl(fd, BATIOC_STATE, (unsigned long)(uintptr_t)&status);
  VALIDATE(ret);

  ret = ioctl(fd, BATIOC_CAPACITY, (unsigned long)(uintptr_t)&capacity);
  VALIDATE(ret);

  ret = ioctl(fd, BATIOC_VOLTAGE, (unsigned long)(uintptr_t)&vol);

  gettimeofday(&tv, NULL);
  printf("%d.%06d: %d mV (%d%%) [%s]\n",
         tv.tv_sec, tv.tv_usec,
         vol, capacity, statestr[status]);

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * gauge_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int gauge_main(int argc, char *argv[])
#endif
{
  int fd;

  board_gauge_initialize(DEVPATH);

  fd = open(DEVPATH, O_RDWR);
  if (fd < 0)
    {
      printf("Device open error.\n");
      return 0;
    }

  (void) basic_test(fd);

  close(fd);

  board_gauge_uninitialize(DEVPATH);

  return 0;
}
