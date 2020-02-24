/****************************************************************************
 * camera/video_capture_thread.c
 *
 *   Copyright 2018 Sony Semiconductor Solutions Corporation
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
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
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
#include <errno.h>
#include <debug.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <nuttx/fs/mkfatfs.h>
#include "video/video.h"

#include "testapi.h"
#include "camera_testfunc.h"


#include <sys/ioctl.h>
#include <sys/boardctl.h>
#include <sys/mount.h>

#include <arch/chip/pm.h>
#include <arch/board/board.h>
#include <arch/chip/cisif.h>

#include <nuttx/lcd/lcd.h>
#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxglib.h>
#include "nximage.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define IMAGE_YUV_SIZE     (320*240*2) /* QVGA YUV422 */

#ifdef CONFIG_BUFNUM_NORMAL
	#define VIDEO_BUFNUM       (3)
#endif
#ifdef CONFIG_BUFNUM_TEST
	#define VIDEO_BUFNUM       (1)
#endif


#ifndef CONFIG_EXAMPLES_CAMERA_LCD_DEVNO
#  define CONFIG_EXAMPLES_CAMERA_LCD_DEVNO 0
#endif

#define itou8(v) ((v) < 0 ? 0 : ((v) > 255 ? 255 : (v)))

#define RGB565_RED      0xf800
#define RGB565_GREEN    0x07e0
#define RGB565_BLUE     0x001f

/****************************************************************************
 * Private Types
 ****************************************************************************/
struct uyvy_s
{
  uint8_t u0;
  uint8_t y0;
  uint8_t v0;
  uint8_t y1;
};

struct v_buffer {
  uint32_t             *start;
  uint32_t             length;
};
typedef struct v_buffer v_buffer_t;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/
static pthread_t g_video_capture_thid;
static bool      video_capture_enable;

static struct v_buffer  *buffers = NULL;
static unsigned int     n_buffers;

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

static int g_width = VIDEO_HSIZE_QVGA;
static int g_height = VIDEO_VSIZE_QVGA;
static int g_fps = 120;

static uint16_t camera_main_file_count = 0;
static char    camera_main_filename[32];

static bool    video_capture_start = false;
static uint8_t video_capture_count = 0;
static uint8_t video_capture_index = 0;

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int  write_file(uint8_t *data, size_t len, uint32_t format)
{
  FILE *fp;
  int  fd;

  camera_main_file_count++;
  if(camera_main_file_count >= 1000)
    {
      camera_main_file_count = 1;
    }
printf("%d\n", __LINE__);
//sleep(1);
  memset(camera_main_filename, 0, sizeof(camera_main_filename));

  if (format == V4L2_PIX_FMT_JPEG)
    {
      sprintf(camera_main_filename,
              "/mnt/sd0/VIDEO%03d.jpg",
              camera_main_file_count);
    }
  else if(format == V4L2_PIX_FMT_UYVY)
    {
      sprintf(camera_main_filename,
              "/mnt/sd0/VIDEO%03d.yuv",
              camera_main_file_count);
    }
    else 
    {
      sprintf(camera_main_filename,
              "/mnt/sd0/VIDEO%03d.rgb",
              camera_main_file_count);      
    }
printf("%d\n", __LINE__);
//sleep(1);

  printf("FILENAME:%s\n", camera_main_filename);

printf("%d\n", __LINE__);
//sleep(1);

  fp = fopen(camera_main_filename, "wb");
  if (NULL == fp)
    {
      printf("fopen error : %d\n", errno);
      return -1;
    }

  if (len != fwrite(data, 1, len, fp))
    {
      printf("fwrite error : %d\n", errno);
    }
    //sleep(1);
printf("%d\n", __LINE__);
  fflush(fp);
  fd = fileno(fp);
  fsync(fd);
  fclose(fp);
  return 0;
}

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

  dev = board_lcd_getdev(CONFIG_EXAMPLES_CAMERA_LCD_DEVNO);
  if (!dev)
    {
      printf("nximage_initialize: board_lcd_getdev failed, devno=%d\n",
             CONFIG_EXAMPLES_CAMERA_LCD_DEVNO);
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
      printf("nximage_initialize: nx_requestbkgd failed: %d\n", errno);
      nx_close(g_nximage.hnx);
      return ERROR;
    }

  while (!g_nximage.havepos)
    {
      (void) sem_wait(&g_nximage.sem);
    }
  printf("nximage_initialize: Screen resolution (%d,%d)\n",
         g_nximage.xres, g_nximage.yres);

  return 0;
}

static inline void ycbcr2rgb(uint8_t y,  uint8_t cb, uint8_t cr,
                             uint8_t *r, uint8_t *g, uint8_t *b)
{
  int _r;
  int _g;
  int _b;
  _r = (128 * (y-16) +                  202 * (cr-128) + 64) / 128;
  _g = (128 * (y-16) -  24 * (cb-128) -  60 * (cr-128) + 64) / 128;
  _b = (128 * (y-16) + 238 * (cb-128)                  + 64) / 128;
  *r = itou8(_r);
  *g = itou8(_g);
  *b = itou8(_b);
}

static inline uint16_t ycbcrtorgb565(uint8_t y, uint8_t cb, uint8_t cr)
{
  uint8_t r;
  uint8_t g;
  uint8_t b;

  ycbcr2rgb(y, cb, cr, &r, &g, &b);
  r = (r >> 3) & 0x1f;
  g = (g >> 2) & 0x3f;
  b = (b >> 3) & 0x1f;
  return (uint16_t)(((uint16_t)r << 11) | ((uint16_t)g << 5) | (uint16_t)b);
}

/* Color conversion to show on display devices. */

static void yuv2rgb(void *buf, uint32_t size)
{
  struct uyvy_s *ptr;
  struct uyvy_s uyvy;
  uint16_t *dest;
  uint32_t i;

  ptr = buf;
  dest = buf;
  for (i = 0; i < size / 4; i++)
    {
      /* Save packed YCbCr elements due to it will be replaced with
       * converted color data.
       */

      uyvy = *ptr++;

      /* Convert color format to packed RGB565 */

      *dest++ = ycbcrtorgb565(uyvy.y0, uyvy.u0, uyvy.v0);
      *dest++ = ycbcrtorgb565(uyvy.y1, uyvy.u0, uyvy.v0);
    }
}

 
static void RGB565ToRGB888(void *RGB565buf, void *RGB888buf, uint32_t size)
{
  unsigned short *pn565Color = RGB565buf;
  unsigned int *pn888Color = RGB888buf;
  unsigned char cRed;
  unsigned char cGreen;
  unsigned char cBlue;
  unsigned short n565Color = 0;
  uint32_t i;
  
  for (i = 0; i < size / 4; i++)
  {
      n565Color = *pn565Color++;
      unsigned char cRed   = (n565Color & RGB565_RED)    >> 8;
      unsigned char cGreen = (n565Color & RGB565_GREEN)  >> 3;
      unsigned char cBlue  = (n565Color & RGB565_BLUE)   << 3;
      *pn888Color++ = (cRed << 16) + (cGreen << 8) + (cBlue << 0);
  }

}

static int camera_prepare(int                fd,
                          enum v4l2_buf_type type,
                          uint32_t           buf_mode,
                          uint32_t           pixformat,
                          uint16_t           hsize,
                          uint16_t           vsize,
                          uint8_t            buffernum,
                        int numerator,
                      int denominator)
{
  int ret;
  int cnt;
  uint32_t fsize;
  struct v4l2_format         fmt = {0};
  struct v4l2_requestbuffers req = {0};
  struct v4l2_buffer         buf = {0};

  /* VIDIOC_REQBUFS initiate user pointer I/O */

  req.type   = type;
  req.memory = V4L2_MEMORY_USERPTR;
  req.count  = buffernum;
  req.mode   = buf_mode;

  ret = ioctl(fd, VIDIOC_REQBUFS, (unsigned long)&req);
  if (ret < 0)
    {
      printf("Failed to VIDIOC_REQBUFS: errno = %d\n", errno);
      return ret;
    }

  /* VIDIOC_S_FMT set format */
/*
  fmt.type                = type;
  fmt.fmt.pix.width       = hsize;
  fmt.fmt.pix.height      = vsize;
  fmt.fmt.pix.field       = V4L2_FIELD_ANY;
  fmt.fmt.pix.pixelformat = pixformat;
*/

  //ret = ioctl(fd, VIDIOC_S_FMT, (unsigned long)&fmt);
  ret = test_vidioc_s_fmt(fd, type, hsize, vsize, pixformat, 0, 0, 0);
  if (ret < 0)
    {
      printf("Failed to VIDIOC_S_FMT: errno = %d\n", errno);
      return ret;
    }

    ret = test_vidioc_s_parm(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, numerator, denominator);
    if (ret < 0)
      {
        printf("Failed to VIDIOC_S_parm: errno = %d\n", errno);
        return ret;
      }

  /* VIDIOC_QBUF enqueue buffer */

  if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {
      buffers = malloc(sizeof(v_buffer_t) * buffernum);
    }

  if (!buffers)
    {
      printf("Out of memory\n");
      return ret;
    }

  if (pixformat == V4L2_PIX_FMT_UYVY)
    {
      fsize = hsize * vsize * 2;
    }

  for (n_buffers = 0; n_buffers < buffernum; ++n_buffers)
    {
      buffers[n_buffers].length = fsize;

      /* Note: VIDIOC_QBUF set buffer pointer. */
      /*       Buffer pointer must be 32bytes aligned. */

      buffers[n_buffers].start  = memalign(32, fsize);
      if (!buffers[n_buffers].start)
        {
          printf("Out of memory\n");
          return ret;
        }
    }

  for (cnt = 0; cnt < n_buffers; cnt++)
    {
      memset(&buf, 0, sizeof(v4l2_buffer_t));
      buf.type = type;
      buf.memory = V4L2_MEMORY_USERPTR;
      buf.index = cnt;
      buf.m.userptr = (unsigned long)buffers[cnt].start;
      buf.length = buffers[cnt].length;

      ret = ioctl(fd, VIDIOC_QBUF, (unsigned long)&buf);
      if (ret)
        {
          printf("Fail QBUF %d\n", errno);
          return ret;;
        }
    }

  /* VIDIOC_STREAMON start stream */

  ret = ioctl(fd, VIDIOC_STREAMON, (unsigned long)&type);
  if (ret < 0)
    {
      printf("Failed to VIDIOC_STREAMON: errno = %d\n", errno);
      return ret;
    }

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void video_capture_thread(void)
{
  int ret;
  int v_fd;
  struct v4l2_requestbuffers req = {0};


  struct v4l2_buffer         buf;
  unsigned int *yuvbuf = NULL;

  /* Prepare display for preview */

  ret = nximage_initialize();
  if (ret < 0)
    {
      printf("video_capture_thread: Failed to get NX handle: %d\n", errno);
      return;
    }

  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
    {
      printf("video_capture_thread: Failed to open video.errno = %d\n", errno);
      return;
    }

  /* Prepare VIDEO_CAPTURE */
  printf("video_capture_thread: width = %d height = %d fps = %d\n", g_width, g_height, g_fps);
  ret = camera_prepare(v_fd,
                       V4L2_BUF_TYPE_VIDEO_CAPTURE,
                       //V4L2_BUF_MODE_RING,
                       V4L2_BUF_MODE_RING,
                       V4L2_PIX_FMT_UYVY,
                       g_width, g_height,
                       //480, 360,
                       //96, 64,
                       VIDEO_BUFNUM,
                      1 , g_fps);
  if (ret < 0)
    {
      return;
    }

  video_capture_enable = true;

  do
    {
      /* Note: VIDIOC_DQBUF acquire capture data. */

      memset(&buf, 0, sizeof(v4l2_buffer_t));
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_USERPTR;

      ret = ioctl(v_fd, VIDIOC_DQBUF, (unsigned long)&buf);
      if (ret)
        {
          printf("video_capture_thread: Fail DQBUF %d\n", errno);
          return;
        }

      if(video_capture_start)
      {
        if ( 10 > video_capture_index)
        {
		video_capture_index++;
        }
        else
        {
		video_capture_count++;
		
		yuvbuf = malloc(buf.bytesused*2);
		if (!yuvbuf)
		{
		  printf("yuvbuf memory error\n");
		  return ret;
		}

		write_file((uint8_t *)buf.m.userptr, (size_t)buf.bytesused, V4L2_PIX_FMT_UYVY);
		
		free(yuvbuf);
		yuvbuf = NULL;

		if(3 == video_capture_count)
		{
		  video_capture_count = 0;
                  video_capture_index = 0;
		  video_capture_start = false;
		  printf("video capture finish\n");
		}
        }

      }

      /* Convert YUV color format to RGB565 and display */

      yuv2rgb((void *)buf.m.userptr, buf.bytesused);

      nximage_image(g_nximage.hbkgd, (void *)buf.m.userptr);

      /* Note: VIDIOC_QBUF reset released buffer pointer. */

      ret = ioctl(v_fd, VIDIOC_QBUF, (unsigned long)&buf);
      if (ret)
        {
          printf("video_capture_thread: Fail QBUF %d\n", errno);
          return;
        }
    }
  while (video_capture_enable);

  req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_USERPTR;
  req.count  = 0;
  req.mode   = V4L2_BUF_MODE_RING;

  ret = ioctl(v_fd, VIDIOC_REQBUFS, (unsigned long)&req);
  if (ret < 0)
    {
      printf("Failed to VIDIOC_REQBUFS: errno = %d\n", errno);
    }
  close(v_fd);


  task_delete(0);
  return;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int video_capture_thread_create(void)
{
  if (!task_create("VideoCaptureThread", 100, 2048, video_capture_thread, NULL) )
    {
      return -1;
    }

  return OK;
}

int video_capture_thread_delete(void)
{
  video_capture_enable = false;
  return OK;
}

void video_capture_setting(int width, int height, int fps)
{
  g_width = width;
  g_height = height;
  g_fps = fps;
}

void video_capture_flag(bool flag)
{
  video_capture_start = flag;
}

