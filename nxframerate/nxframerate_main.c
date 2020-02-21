/****************************************************************************
 * examples/nxframerate/nxframerate_main.c
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

#include <nuttx/config.h>
#include <sdk/config.h>

#include <sys/ioctl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <nuttx/lcd/lcd.h>
#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxglib.h>

#include "nximage.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/* Sanity checks */

#ifndef CONFIG_NX_LCDDRIVER
#  error This test needs CONFIG_LCD and CONFIG_NX in the kernel.
#endif

#ifndef CONFIG_TEST_NXFRAMERATE_LCD_DEVNO
#  define CONFIG_TEST_NXFRAMERATE_LCD_DEVNO 0
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

struct nximage_data_s g_nximage =
{
  NULL,          /* hnx */
  NULL,          /* hbkgd */
  0,             /* xres */
  0,             /* yres */
  false,         /* havpos */
  { 0 },         /* sem */
  0              /* exit code */
};

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

static inline int nximage_initialize(void)
{
  FAR NX_DRIVERTYPE *dev;
  nxgl_mxpixel_t color;
  int ret;

  /* Initialize the LCD device */

  printf("nximage_initialize: Initializing LCD\n");
  ret = board_lcd_initialize();
  if (ret < 0)
    {
      printf("nximage_initialize: board_lcd_initialize failed: %d\n", -ret);
      return ERROR;
    }

  /* Get the device instance */

  dev = board_lcd_getdev(CONFIG_TEST_NXFRAMERATE_LCD_DEVNO);
  if (!dev)
    {
      printf("nximage_initialize: board_lcd_getdev failed, devno=%d\n",
             CONFIG_TEST_NXFRAMERATE_LCD_DEVNO);
      return ERROR;
    }

  /* Turn the LCD on at 75% power */

  (void)dev->setpower(dev, ((3*CONFIG_LCD_MAXPOWER + 3)/4));

  /* Then open NX */

  printf("nximage_initialize: Open NX\n");
  g_nximage.hnx = nx_open(dev);
  if (!g_nximage.hnx)
    {
      printf("nximage_initialize: nx_open failed: %d\n", errno);
      return ERROR;
    }

  /* Set background color to black */

  color = 0;
  nx_setbgcolor(g_nximage.hnx, &color);
  ret = nx_requestbkgd(g_nximage.hnx, &g_nximagecb, NULL);
  if (ret < 0)
    {
      printf("nxframerate_main: nx_requestbkgd failed: %d\n", errno);
      nx_close(g_nximage.hnx);
      return ERROR;
    }

  while (!g_nximage.havepos)
    {
      (void) sem_wait(&g_nximage.sem);
    }
  printf("nxframerate_main: Screen resolution (%d,%d)\n", g_nximage.xres, g_nximage.yres);

  return 0;
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

/****************************************************************************
 * nxframerate_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int nxframerate_main(int argc, FAR char *argv[])
#else
int nxframerate_main(int argc, char *argv[])
#endif
{
  void *framebuffer[2];
  int size;
  struct timespec start, end, delta;
  int ret;
  int i;

  ret = nximage_initialize();
  if (ret < 0)
    {
      printf("nxframerate_main: Failed to get NX handle: %d\n", errno);
      return 1;
    }

  size = g_nximage.xres * g_nximage.yres * sizeof(nxgl_mxpixel_t);

  framebuffer[0] = memalign(32, size);
  framebuffer[1] = memalign(32, size);

  /* Clean up frame buffers with white & black */

  memset(framebuffer[0], 0xff, size);
  memset(framebuffer[1], 0, size);

  clock_gettime(CLOCK_REALTIME, &start);

  for (i = 0; i < 100; i++)
    {
      /* Toggle each frame buffers for confirm by tester */

      nximage_image(g_nximage.hbkgd, framebuffer[i & 1]);
    }

  clock_gettime(CLOCK_REALTIME, &end);

  timespecdelta(&end, &start, &delta);
  printf("time: %d.%09d\n", delta.tv_sec, delta.tv_nsec);

  nx_releasebkgd(g_nximage.hbkgd);
  nx_close(g_nximage.hnx);

  free(framebuffer[0]);
  free(framebuffer[1]);

  return 0;
}
