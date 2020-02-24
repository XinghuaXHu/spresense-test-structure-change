/****************************************************************************
 * demo/collet_box/tracker/tracker_nxfb.h
 *
 *   Copyright (C) 2017 Sony Corporation. All rights reserved.
 *   Author: Kei Yamamoto <Kei.x.Yamamoto@sony.com>
 *           Tomonobu Hayakawa <Tomonobu.Hayakawa@sony.com>
 *           Tomoyuki Takahashi <Tomoyuki.A.Takahashi@sony.com>
 *           Yutaka Miyajima <Yutaka.Miyajima@sony.com>
 *
 * Based on examples/gnss_display/gnss_nxfb.h
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

#ifndef __APPS_DEMO_COLLET_BOX_TRACKER_TRACKER_NXFB_H
#define __APPS_DEMO_COLLET_BOX_TRACKER_TRACKER_NXFB_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/boardctl.h>

#include <nuttx/nx/nxglib.h>
#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxfonts.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

#ifndef CONFIG_NX
#  error "NX is not enabled (CONFIG_NX)"
#endif

#define CONFIG_EXAMPLES_TRACKER_BPP        8
#define CONFIG_EXAMPLES_TRACKER_FONTCOLOR  3
#define CONFIG_EXAMPLES_TRACKER_BGCOLOR    0
#define CONFIG_EXAMPLES_TRACKER_FONTID     FONTID_DEFAULT
#define CONFIG_EXAMPLES_TRACKER_VPLANE     0
#define CONFIG_EXAMPLES_TRACKER_DEVNO      0

/****************************************************************************
 * Public Types
 ****************************************************************************/

enum exitcode_e
{
  NXEXIT_SUCCESS = 0,
  NXEXIT_EXTINITIALIZE,
  NXEXIT_FBINITIALIZE,
  NXEXIT_FBGETVPLANE,
  NXEXIT_LCDINITIALIZE,
  NXEXIT_LCDGETDEV,
  NXEXIT_NXOPEN,
  NXEXIT_FONTOPEN,
  NXEXIT_NXREQUESTBKGD,
  NXEXIT_NXSETBGCOLOR
};

/* Describes one cached glyph bitmap */

struct nxfb_glyph_s
{
  uint8_t code;                        /* Character code */
  uint8_t height;                      /* Height of this glyph (in rows) */
  uint8_t width;                       /* Width of this glyph (in pixels) */
  uint8_t stride;                      /* Width of the glyph row (in bytes) */
  uint8_t usecnt;                      /* Use count */
  FAR uint8_t *bitmap;                 /* Allocated bitmap memory */
};

/* Describes on character on the display */

struct nxfb_bitmap_s
{
  uint8_t code;                        /* Character code */
  uint8_t flags;                       /* See BMFLAGS_* */
  struct nxgl_point_s pos;             /* Character position */
};

struct nxfb_data_s
{
  /* The NX handles */

  NXHANDLE hnx;
  NXHANDLE hbkgd;
  NXHANDLE hfont;

  /* The screen resolution */

  nxgl_coord_t xres;
  nxgl_coord_t yres;

  volatile bool havepos;
  sem_t sem;
  volatile int code;

  int drvfd;
  struct boardioc_graphics_s devinfo;
  struct fb_planeinfo_s planeinfo;
  struct fb_videoinfo_s videoinfo;

  /* starting postion */

  FAR struct nxgl_point_s pos;
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* nxfb state data */

extern struct nxfb_data_s tracker_nxfb;

/* NX callback vtables */

extern const struct nx_callback_s tracker_nxfbcb;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

extern int tracker_nxfb_initialize(void);
extern int tracker_nxfb_terminate(void);
extern int tracker_nxfb_printf(const char *f, ...);
extern int tracker_nxfb_flash(void);

extern void tracker_nxfb_print(NXWINDOW hwnd, const char *str);

#endif /* __APPS_DEMO_COLLET_BOX_TRACKER_TRACKER_NXFB_H */
