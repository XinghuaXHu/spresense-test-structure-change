/****************************************************************************
 * demo/collet_box/tracker/tracker_nxfb_bkgd.c
 *
 *   Copyright (C) 2017 Sony Corporation. All rights reserved.
 *   Author: Kei Yamamoto <Kei.x.Yamamoto@sony.com>
 *           Tomonobu Hayakawa <Tomonobu.Hayakawa@sony.com>
 *           Tomoyuki Takahashi <Tomoyuki.A.Takahashi@sony.com>
 *           Yutaka Miyajima <Yutaka.Miyajima@sony.com>
 *
 * Based on examples/gnss_display/gnss_nxfb_bkgd.c
 *
 *   Copyright (C) 2011, 2013 Gregory Nutt. All rights reserved.
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

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>

#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxglib.h>
#include <nuttx/nx/nxfonts.h>

#include "tracker_debug.h"
#include "tracker_nxfb.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Select renderer -- Some additional logic would be required to support
 * pixel depths that are not directly addressable (1,2,4, and 24).
 */

#if CONFIG_EXAMPLES_TRACKER_BPP == 1
#  define RENDERER nxf_convert_1bpp
#elif CONFIG_EXAMPLES_TRACKER_BPP == 2
#  define RENDERER nxf_convert_2bpp
#elif CONFIG_EXAMPLES_TRACKER_BPP == 4
#  define RENDERER nxf_convert_4bpp
#elif CONFIG_EXAMPLES_TRACKER_BPP == 8
#  define RENDERER nxf_convert_8bpp
#elif CONFIG_EXAMPLES_TRACKER_BPP == 16
#  define RENDERER nxf_convert_16bpp
#elif CONFIG_EXAMPLES_TRACKER_BPP == 24
#  define RENDERER nxf_convert_24bpp
#elif  CONFIG_EXAMPLES_TRACKER_BPP == 32
#  define RENDERER nxf_convert_32bpp
#else
#  error "Unsupported CONFIG_EXAMPLES_TRACKER_BPP"
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void nxfb_redraw(NXWINDOW hwnd, FAR const struct nxgl_rect_s *rect,
                        bool morem, FAR void *arg);
static void nxfb_position(NXWINDOW hwnd, FAR const struct nxgl_size_s *size,
                          FAR const struct nxgl_point_s *pos,
                          FAR const struct nxgl_rect_s *bounds,
                          FAR void *arg);
#ifdef CONFIG_NX_XYINPUT
static void nxfb_mousein(NXWINDOW hwnd, FAR const struct nxgl_point_s *pos,
                         uint8_t buttons, FAR void *arg);
#endif

#ifdef CONFIG_NX_KBD
static void nxfb_kbdin(NXWINDOW hwnd, uint8_t nch, FAR const uint8_t *ch,
                       FAR void *arg);
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* Background window call table */

const struct nx_callback_s tracker_nxfbcb =
{
  nxfb_redraw,   /* redraw */
  nxfb_position  /* position */
#ifdef CONFIG_NX_XYINPUT
  , nxfb_mousein /* mousein */
#endif
#ifdef CONFIG_NX_KBD
  , nxfb_kbdin   /* my kbdin */
#endif
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nxfb_redraw
 ****************************************************************************/

static void nxfb_redraw(NXWINDOW hwnd, FAR const struct nxgl_rect_s *rect,
                        bool more, FAR void *arg)
{
  tracker_debug_info("hwnd=%p rect={(%d,%d),(%d,%d)} more=%s\n",
                     hwnd, rect->pt1.x, rect->pt1.y, rect->pt2.x, rect->pt2.y,
                     more ? "true" : "false");
}

/****************************************************************************
 * Name: nxfb_position
 ****************************************************************************/

static void nxfb_position(NXWINDOW hwnd, FAR const struct nxgl_size_s *size,
                          FAR const struct nxgl_point_s *pos,
                          FAR const struct nxgl_rect_s *bounds,
                          FAR void *arg)
{
  /* Report the position */

  tracker_debug_info("hwnd=%p size=(%d,%d) pos=(%d,%d) bounds={(%d,%d),(%d,%d)}\n",
                     hwnd, size->w, size->h, pos->x, pos->y,
                     bounds->pt1.x, bounds->pt1.y,
                     bounds->pt2.x, bounds->pt2.y);

  /* Have we picked off the window bounds yet? */

  if (!tracker_nxfb.havepos)
    {
      /* Save the background window handle */

      tracker_nxfb.hbkgd = hwnd;

      /* Save the window limits */

      tracker_nxfb.xres = bounds->pt2.x + 1;
      tracker_nxfb.yres = bounds->pt2.y + 1;

      tracker_nxfb.havepos = true;
      sem_post(&tracker_nxfb.sem);
      tracker_debug_info("Have xres=%d yres=%d\n",
                         tracker_nxfb.xres, tracker_nxfb.yres);
    }
}

/****************************************************************************
 * Name: nxfb_mousein
 ****************************************************************************/

#ifdef CONFIG_NX_XYINPUT
static void nxfb_mousein(NXWINDOW hwnd, FAR const struct nxgl_point_s *pos,
                         uint8_t buttons, FAR void *arg)
{
  tracker_debug_info("nxfb_mousein: hwnd=%p pos=(%d,%d) button=%02x\n",
                     hwnd,  pos->x, pos->y, buttons);
}
#endif

/****************************************************************************
 * Name: nxfb_kbdin
 ****************************************************************************/

#ifdef CONFIG_NX_KBD
static void nxfb_kbdin(NXWINDOW hwnd, uint8_t nch, FAR const uint8_t *ch,
                       FAR void *arg)
{
  tracker_debug_info("hwnd=%p nch=%d\n", hwnd, nch);

   /* In this example, there is no keyboard so a keyboard event is not
    * expected.
    */

   tracker_debug_info("nxfb_kbdin: Unexpected keyboard callback\n");
}
#endif

/****************************************************************************
 * Name: nxfb_initglyph
 ****************************************************************************/

static void nxfb_initglyph(FAR uint8_t *glyph, uint8_t height,
                              uint8_t width, uint8_t stride)
{
  FAR nxgl_mxpixel_t *ptr;
#if CONFIG_EXAMPLES_TRACKER_BPP < 8
  nxgl_mxpixel_t pixel;
#endif
  unsigned int row;
  unsigned int col;

  /* Initialize the glyph memory to the background color */

#if CONFIG_EXAMPLES_TRACKER_BPP < 8

  pixel  = CONFIG_EXAMPLES_TRACKER_BGCOLOR;

#if CONFIG_NX_NPLANES > 1
# warning "More logic is needed for the case where CONFIG_NX_PLANES > 1"
#endif
#  if CONFIG_EXAMPLES_TRACKER_BPP == 1
  /* Pack 1-bit pixels into a 2-bits */

  pixel &= 0x01;
  pixel  = (pixel) << 1 |pixel;

#  endif
#  if CONFIG_EXAMPLES_TRACKER_BPP < 4

  /* Pack 2-bit pixels into a nibble */

  pixel &= 0x03;
  pixel  = (pixel) << 2 |pixel;

#  endif

  /* Pack 4-bit nibbles into a byte */

  pixel &= 0x0f;
  pixel  = (pixel) << 4 | pixel;

  ptr    = (FAR nxgl_mxpixel_t *)glyph;
  for (row = 0; row < height; row++)
    {
      for (col = 0; col < stride; col++)
        {
          /* Transfer the packed bytes into the buffer */

          *ptr++ = pixel;
        }
    }

#elif CONFIG_EXAMPLES_TRACKER_BPP == 24
# error "Additional logic is needed here for 24bpp support"

#else /* CONFIG_EXAMPLES_TRACKER_BPP = {8,16,32} */

  ptr = (FAR nxgl_mxpixel_t *)glyph;
  for (row = 0; row < height; row++)
    {
      /* Just copy the color value into the glyph memory */

      for (col = 0; col < width; col++)
        {
          *ptr++ = CONFIG_EXAMPLES_TRACKER_BGCOLOR;
#if CONFIG_NX_NPLANES > 1
# warning "More logic is needed for the case where CONFIG_NX_PLANES > 1"
#endif
        }
    }
#endif
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: tracker_nxfb_print
 *
 * Description:
 *   Print string in the center of the display with multi-line.
 *
 ****************************************************************************/

void tracker_nxfb_print(NXWINDOW hwnd, const char *str)
{
  FAR const struct nx_font_s *      fontset;
  FAR const struct nx_fontbitmap_s *fbm;
  FAR uint8_t *            glyph;
  FAR const char *         ptr;
  FAR struct nxgl_point_s *pos;
  FAR struct nxgl_rect_s   dest;
  FAR const void *         src[CONFIG_NX_NPLANES];
  unsigned int             glyphsize;
  unsigned int             mxstride;
  int                      ret;

  /* Get information about the font we are going to use */

  fontset = nxf_getfontset(tracker_nxfb.hfont);

  /* Allocate a bit of memory to hold the largest rendered font */

  mxstride  = (fontset->mxwidth * CONFIG_EXAMPLES_TRACKER_BPP + 7) >> 3;
  glyphsize = (unsigned int)fontset->mxheight * mxstride;
  glyph     = (FAR uint8_t*)malloc(glyphsize);
  if (glyph == NULL)
    {
      return;
    }

  /* Get a position on the display. */

  pos = &tracker_nxfb.pos;

  for (ptr = str; *ptr; ptr++)
    {

      /* Feed line if character is control code */

      if (*ptr == '\n')
        {
          pos->x = 0;
          pos->y += (nxgl_coord_t)fontset->mxheight - 6;
          continue;
        }

      /* Get the bitmap font for this ASCII code */

      fbm = nxf_getbitmap(tracker_nxfb.hfont, *ptr);
      if (fbm)
        {
          uint8_t fheight;      /* Height of this glyph (in rows) */
          uint8_t fwidth;       /* Width of this glyph (in pixels) */
          uint8_t fstride;      /* Width of the glyph row (in bytes) */

          /* Get information about the font bitmap */

          fwidth  = fbm->metric.width + fbm->metric.xoffset;
          fheight = fbm->metric.height + fbm->metric.yoffset;
          fstride = (fwidth * CONFIG_EXAMPLES_TRACKER_BPP + 7) >> 3;

          /* Initialize the glyph memory to the background color */

          memset(glyph, 0, glyphsize);
          nxfb_initglyph(glyph, fheight, fwidth, fstride);

          /* Then render the glyph into the allocated memory */

#if CONFIG_NX_NPLANES > 1
# warning "More logic is needed for the case where CONFIG_NX_PLANES > 1"
#endif
          (void)RENDERER((FAR nxgl_mxpixel_t*)glyph, fheight, fwidth,
                         fstride, fbm, CONFIG_EXAMPLES_TRACKER_FONTCOLOR);

          /* Describe the destination of the font with a rectangle */

          dest.pt1.x = pos->x;
          dest.pt1.y = pos->y;
          dest.pt2.x = pos->x + fwidth - 1;
          dest.pt2.y = pos->y + fheight - 1;

          /* Then put the font on the display */

          src[0] = (FAR const void *)glyph;
#if CONFIG_NX_NPLANES > 1
# warning "More logic is needed for the case where CONFIG_NX_PLANES > 1"
#endif
          ret = nx_bitmap((NXWINDOW)hwnd, &dest, src, pos, fstride);
          if (ret < 0)
            {
              tracker_debug_error("nxfb_write: nx_bitmapwindow failed: %d\n",
                                  errno);
            }

           /* Skip to the right the width of the font */

          pos->x += fwidth;
        }
      else
        {
           /* No bitmap (probably because the font is a space).  Skip to the
            * right the width of a space.
            */

          pos->x += fontset->spwidth;
        }
    }

  /* Free the allocated glyph */

  free(glyph);
}
