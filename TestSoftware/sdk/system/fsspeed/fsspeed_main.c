/****************************************************************************
 * examples/fsspeed/fsspeed_main.c
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
#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define DEV_SMARTFS "/dev/smart0d1"
#define MOUNTPT     "/smart"
#define TESTFILE    MOUNTPT "/test.data"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static char *g_buffer;

static void prologue(void)
{
  int ret;

  g_buffer = (char *)malloc(100 * 1024);
  if (!g_buffer)
    {
      printf("memory allocation failure.\n");
      return;
    }

  ret = mount(DEV_SMARTFS, MOUNTPT, "smartfs", 0, NULL);
  if (ret != 0)
    {
      printf("SmartFS mount failed. %d\n", errno);
    }
}

static void epilogue(void)
{
  free(g_buffer);
  umount(MOUNTPT);
}

/*
 * Calculate delta of timespec a and b to c.
 * c = a - b
 */
static void timespecdelta(struct timespec *a, struct timespec *b,
                          struct timespec *c)
{
  c->tv_sec = a->tv_sec - b->tv_sec;
  if (a->tv_nsec >= b->tv_nsec)
    {
      c->tv_nsec = a->tv_nsec - b->tv_nsec;
    }
  else
    {
      c->tv_sec--;
      c->tv_nsec = (a->tv_nsec + 1000000000) - b->tv_nsec;
    }
}

static void write_speed_test(size_t size, size_t memb, int repeat)
{
  struct timespec start, end, delta;
  FILE *fp;
  int i;

  for (i = 0; i < repeat; i++)
    {
      fp = fopen(TESTFILE, "w");
      if (!fp)
        {
          printf("File open error.\n");
          break;
        }

      clock_gettime(CLOCK_REALTIME, &start);
      fwrite(g_buffer, size, memb, fp);
      clock_gettime(CLOCK_REALTIME, &end);

      timespecdelta(&end, &start, &delta);
      printf("lapse[%d]: %d.%09d\n", i, delta.tv_sec, delta.tv_nsec);

      fclose(fp);
      unlink(TESTFILE);
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * fsspeed_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int fsspeed_main(int argc, char *argv[])
#endif
{
  prologue();

  printf("bs=4096 count=10 (40KiB)\n");
  write_speed_test(4096, 10, 10);

  printf("bs=1024 count=100 (100KiB)\n");
  write_speed_test(1024, 100, 10);

  epilogue();

  return 0;
}
