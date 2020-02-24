/****************************************************************************
 * demo/collet_box/tracker/tracker_nxfb.c
 *
 *   Copyright (C) 2017 Sony Corporation. All rights reserved.
 *   Author: Kei Yamamoto <Kei.x.Yamamoto@sony.com>
 *           Tomonobu Hayakawa <Tomonobu.Hayakawa@sony.com>
 *           Tomoyuki Takahashi <Tomoyuki.A.Takahashi@sony.com>
 *           Yutaka Miyajima <Yutaka.Miyajima@sony.com>
 *
 * Based on examples/gnss_display/gnss_nxfb.c
 *
 *   Copyright (C) 2011, 2015 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
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

#include <sys/types.h>
#include <sys/boardctl.h>
#include <sys/ioctl.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>

#include <nuttx/video/fb.h>

#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxglib.h>
#include <nuttx/nx/nxfonts.h>

#include "tracker_debug.h"
#include "tracker_nxfb.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define LCD_DEVPATH      "/dev/lcd0"
#define STR_MAXLEN       64

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int open_display(void);
static int close_display(void);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

struct nxfb_data_s tracker_nxfb =
{
  NULL,          /* hnx */
  NULL,          /* hbkgd */
  NULL,          /* hfont */
  0,             /* xres */
  0,             /* yres */
  false,         /* havpos */
  { 0 },         /* sem */
  NXEXIT_SUCCESS, /* exit code */
  -1,            /* drvfd */
};

static sem_t g_nxfb_sem = SEM_INITIALIZER(1);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: open_display
 ****************************************************************************/

static int open_display(void)
{
  tracker_debug_info("%d %s\n", __LINE__, LCD_DEVPATH);

  tracker_nxfb.drvfd = open(LCD_DEVPATH, O_WRONLY);
  if (tracker_nxfb.drvfd < 0)
    {
      tracker_debug_error("failed to open display driver : %d\n",
                          tracker_nxfb.drvfd);
    }

  tracker_nxfb.devinfo.dev->getvideoinfo(tracker_nxfb.devinfo.dev,
                                         &tracker_nxfb.videoinfo);
  tracker_debug_info("videoinfo xres:%d yres:%d\n",
                     tracker_nxfb.videoinfo.xres,
                     tracker_nxfb.videoinfo.yres);

  tracker_nxfb.devinfo.dev->getplaneinfo(tracker_nxfb.devinfo.dev,
                                         CONFIG_EXAMPLES_TRACKER_VPLANE,
                                         &tracker_nxfb.planeinfo);
  tracker_debug_info("planeinfo bpp:%d, fblen:%d\n",
                     tracker_nxfb.planeinfo.bpp,
                     tracker_nxfb.planeinfo.fblen);
  return tracker_nxfb.drvfd;
}

/****************************************************************************
 * Name: close_display
 ****************************************************************************/

static int close_display(void)
{
  int ret;

  ret = close(tracker_nxfb.drvfd);
  if (ret < 0)
    {
      tracker_debug_error("failed to close display driver : %d\n", ret);
    }
  tracker_nxfb.drvfd = -1;
  return ret;
}

/****************************************************************************
 * Name: nxfb_initialize
 ****************************************************************************/

static int nxfb_initialize(void)
{
  int ret;

  /* Use external graphics driver initialization */

  tracker_debug_info("nxfb_initialize: Initializing external graphics device\n");

  /* Initialize params */
  
  tracker_nxfb.hnx = NULL;
  tracker_nxfb.hbkgd = NULL;
  tracker_nxfb.hfont = NULL;
  tracker_nxfb.xres = 0;
  tracker_nxfb.yres = 0;
  tracker_nxfb.havepos = false;
  sem_init(&tracker_nxfb.sem, 0, 0);
  tracker_nxfb.code = NXEXIT_SUCCESS;
  tracker_nxfb.drvfd = -1;

  tracker_nxfb.devinfo.devno = CONFIG_EXAMPLES_TRACKER_DEVNO;
  tracker_nxfb.devinfo.dev = NULL;
  tracker_nxfb.pos.x = 0;
  tracker_nxfb.pos.y = 0;

  ret = boardctl(BOARDIOC_GRAPHICS_SETUP, (uintptr_t)&tracker_nxfb.devinfo);
  if (ret < 0)
    {
      tracker_debug_error("nxfb_initialize: boardctl failed, devno=%d: %d\n",
                          CONFIG_EXAMPLES_TRACKER_DEVNO, errno);
      tracker_nxfb.code = NXEXIT_EXTINITIALIZE;

      return ERROR;
    }

  open_display();

  if (!tracker_nxfb.devinfo.dev)
    {
      tracker_debug_error("nxfb_initialize: up_fbgetvplane failed, vplane=%d\n",
                          CONFIG_EXAMPLES_TRACKER_VPLANE);
      tracker_nxfb.code = NXEXIT_FBGETVPLANE;

      return ERROR;
    }

  /* Then open NX */

  tracker_debug_info("nxfb_initialize: Open NX\n");
  tracker_nxfb.hnx = nx_open(tracker_nxfb.devinfo.dev);
  if (!tracker_nxfb.hnx)
    {
      tracker_debug_error("nxfb_initialize: nx_open failed: %d\n",
                          errno);
      tracker_nxfb.code = NXEXIT_NXOPEN;

      return ERROR;
    }
  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: tracker_nxfb_initialize
 ****************************************************************************/

int tracker_nxfb_initialize(void)
{
  nxgl_mxpixel_t color;
  int ret;

  /* API lock */

  sem_wait(&g_nxfb_sem);

  /* Initialize NX */

  ret = nxfb_initialize();
  tracker_debug_info("%s: NX handle=%p\n", __func__, tracker_nxfb.hnx);
  if (!tracker_nxfb.hnx || ret < 0)
    {
      tracker_debug_error("%s: Failed to get NX handle: %d\n",
                          __func__, errno);
      tracker_nxfb.code = NXEXIT_NXOPEN;
      goto errout;
    }

  /* Get the default font handle */

  tracker_nxfb.hfont = nxf_getfonthandle(CONFIG_EXAMPLES_TRACKER_FONTID);
  if (!tracker_nxfb.hfont)
    {
      tracker_debug_error("%s: Failed to get font handle: %d\n",
                          __func__, errno);
      tracker_nxfb.code = NXEXIT_FONTOPEN;
      goto errout;
    }

  /* Set the background to the configured background color */

  tracker_debug_info("%s: Set background color=%d\n", __func__,
                     CONFIG_EXAMPLES_TRACKER_BGCOLOR);

  color = CONFIG_EXAMPLES_TRACKER_BGCOLOR;
  ret = nx_setbgcolor(tracker_nxfb.hnx, &color);
  if (ret < 0)
    {
      tracker_debug_error("%s: nx_setbgcolor failed: %d\n",
                          __func__, errno);
      tracker_nxfb.code = NXEXIT_NXSETBGCOLOR;
      goto errout_with_nx;
    }

  /* Get the background window */

  ret = nx_requestbkgd(tracker_nxfb.hnx, &tracker_nxfbcb, NULL);
  if (ret < 0)
    {
      tracker_debug_error("%s: nx_setbgcolor failed: %d\n",
                          __func__, errno);
      tracker_nxfb.code = NXEXIT_NXREQUESTBKGD;
      goto errout_with_nx;
    }

  /* Wait until we have the screen resolution.  We'll have this immediately
   * unless we are dealing with the NX server.
   */

  while (!tracker_nxfb.havepos)
    {
      (void)sem_wait(&tracker_nxfb.sem);
    }
  tracker_debug_info("%s: Screen resolution (%d,%d)\n",
                     __func__, tracker_nxfb.xres,
                     tracker_nxfb.yres);

  /* API unlock */

  sem_post(&g_nxfb_sem);

  return 0;

errout_with_nx:
  tracker_debug_error("%s: Close NX\n", __func__);
  nx_close(tracker_nxfb.hnx);
  sem_destroy(&tracker_nxfb.sem);
  close_display();
errout:
  /* API unlock */

  sem_post(&g_nxfb_sem);

  return tracker_nxfb.code;
}

/****************************************************************************
 * Name: tracker_nxfb_terminate
 ****************************************************************************/

int tracker_nxfb_terminate(void)
{
  /* API lock */

  sem_wait(&g_nxfb_sem);

  /* Release background */

  (void)nx_releasebkgd(tracker_nxfb.hbkgd);

  /* Close NX */

  tracker_debug_info("%s: Close NX\n", __func__);
  nx_close(tracker_nxfb.hnx);
  sem_destroy(&tracker_nxfb.sem);
  close_display();

  /* API unlock */

  sem_post(&g_nxfb_sem);

  return tracker_nxfb.code;
}

/****************************************************************************
 * Name: tracker_nxfb_printf
 ****************************************************************************/

int tracker_nxfb_printf(const char *f, ...)
{
  char s[STR_MAXLEN];
  int ret = 0;
  va_list arg;

  /* API lock */

  sem_wait(&g_nxfb_sem);

  va_start(arg, f);
  ret = vsnprintf(s, STR_MAXLEN, f, arg);
  va_end(arg);
  if (ret < 0)
    {
      tracker_debug_error("%s: failed prepare string : %d\n",
                          __func__, ret);

      /* API unlock */

      sem_post(&g_nxfb_sem);

      return ret;
    }

  tracker_nxfb_print(tracker_nxfb.hbkgd, s);

  tracker_debug_info(s);

  /* API unlock */

  sem_post(&g_nxfb_sem);

  return 0;
}

/****************************************************************************
 * Name: tracker_nxfb_update
 ****************************************************************************/

int tracker_nxfb_flash(void)
{
  int len;

  /* API lock */

  sem_wait(&g_nxfb_sem);

  len = write(tracker_nxfb.drvfd, tracker_nxfb.planeinfo.fbmem,
              tracker_nxfb.planeinfo.fblen);
  if (len != tracker_nxfb.planeinfo.fblen)
    {
      tracker_debug_error("%s: failed to write display driver : %d\n",
                          __func__, len);
    }

  tracker_nxfb.pos.x = 0;
  tracker_nxfb.pos.y = 0;

  /* API unlock */

  sem_post(&g_nxfb_sem);

  return len;
}
