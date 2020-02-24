/****************************************************************************
 * test/sqa/combination/camera_jpgdec/camera_jpgdec_main.c
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
#include <sys/stat.h>
#include <errno.h>
#include <debug.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <nuttx/fs/mkfatfs.h>
#include "video/video.h"

#include <sys/ioctl.h>
#include <sys/boardctl.h>
#include <sys/mount.h>

#include <arch/chip/pm.h>
#include <arch/board/board.h>
#include <arch/chip/cisif.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/* Display of vsync timing */
/* #define CAMERA_MAIN_CISIF_INTRTRACE */

/* Note: Buffer size must be multiple of 32. */

//#define IMAGE_JPG_SIZE     (512*1024)  /* 512kB */
#define IMAGE_JPG_SIZE     (1024*1024)  /* 1024kB */
#define IMAGE_YUV_SIZE     (320*240*2) /* QVGA YUV422 */

#define STILL_BUFNUM       (1)

#define DEFAULT_REPEAT_NUM (65535)

#define IMAGE_FILENAME_LEN (32)

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

static int  write_file(uint8_t *data, size_t len, uint32_t format);
int decode_and_show_lcd(int argc,  const char* argv[]);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct v_buffer  *buffers_still;
static unsigned int     n_buffers;
static char       camera_main_filename[IMAGE_FILENAME_LEN];
static bool show_lcd_task_running = false;

extern bool lcd_show_running;
extern bool lcd_show_end;
extern uint32_t lcd_show_interval;

/****************************************************************************
 * Public Data
 ****************************************************************************/

uint32_t    camera_main_file_count = 0;
bool        mcu                    = false;
uint32_t    loop                   = DEFAULT_REPEAT_NUM;
const char *save_dir;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int  write_file(uint8_t *data, size_t len, uint32_t format)
{
  printf("%s start\n", __func__);
  FILE *fp;

  int tmp_count = camera_main_file_count;
  tmp_count++;
  if(tmp_count >= 100000)
    {
      camera_main_file_count = 1;
      tmp_count = 1;
    }

  memset(camera_main_filename, 0, sizeof(camera_main_filename));

  if (format == V4L2_PIX_FMT_JPEG)
    {
      snprintf(camera_main_filename,
               IMAGE_FILENAME_LEN,
               "%s/VIDEO%05d.JPG",
               save_dir, tmp_count);
    }
  else
    {
      snprintf(camera_main_filename,
               IMAGE_FILENAME_LEN,
               "%s/VIDEO%03d.YUV",
               save_dir, tmp_count);
    }

//  printf("FILENAME:%s\n", camera_main_filename);

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

  fclose(fp);
  camera_main_file_count++;
  printf("%s end\n", __func__);
  return 0;
}

static int camera_prepare(int                fd,
                          enum v4l2_buf_type type,
                          uint32_t           buf_mode,
                          uint32_t           pixformat,
                          uint16_t           hsize,
                          uint16_t           vsize,
                          uint8_t            buffernum)
{
  int ret;
  int cnt;
  uint32_t fsize;
  struct v4l2_format         fmt = {0};
  struct v4l2_requestbuffers req = {0};
  struct v4l2_buffer         buf = {0};
  struct v_buffer  *buffers;

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

  fmt.type                = type;
  fmt.fmt.pix.width       = hsize;
  fmt.fmt.pix.height      = vsize;
  fmt.fmt.pix.field       = V4L2_FIELD_ANY;
  fmt.fmt.pix.pixelformat = pixformat;

  ret = ioctl(fd, VIDIOC_S_FMT, (unsigned long)&fmt);
  if (ret < 0)
    {
      printf("Failed to VIDIOC_S_FMT: errno = %d\n", errno);
      return ret;
    }

  /* VIDIOC_QBUF enqueue buffer */

  buffers_still = malloc(sizeof(v_buffer_t) * buffernum);
  buffers = buffers_still;

  if (!buffers)
    {
      printf("Out of memory\n");
      return ret;
    }

  if (pixformat == V4L2_PIX_FMT_UYVY)
    {
      fsize = IMAGE_YUV_SIZE;
    }
  else
    {
      fsize = IMAGE_JPG_SIZE;
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

static void free_buffer(struct v_buffer  *buffers, uint8_t bufnum)
{
  uint8_t cnt;
  if (buffers)
    {
      for (cnt = 0; cnt < bufnum; cnt++)
        {
          if (buffers[cnt].start)
            {
              free(buffers[cnt].start);
            }
        }

      free(buffers);
    }
}

static void show_usage()
{
  printf("Options:\n");
  printf(" -w <width>  : Picture width.\n");
  printf(" -h <height> : Picture height.\n");
  printf(" -c <count>  : Repeat count.\n");
  printf(" -m          : Use mcu.\n");
  printf(" -i          : Lcd show interval (sec)\n");
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int camera_jpgdec_main(int argc, char *argv[])
#endif
{
  int ret;
  int exitcode      = ERROR;
  int v_fd;
  int pid;
  struct stat stat_buf;
  uint32_t buf_type = V4L2_BUF_TYPE_STILL_CAPTURE;
  uint32_t format   = V4L2_PIX_FMT_JPEG;
  struct v4l2_buffer buf;
  
  uint16_t width    = VIDEO_HSIZE_5M;
  uint16_t height   = VIDEO_VSIZE_5M;

  camera_main_file_count = 0;
  mcu                    = false;
  loop                   = DEFAULT_REPEAT_NUM;
  show_lcd_task_running  = false;
  
  int opt;
  /* select capture mode */

  /* In SD card is available, use SD card.
   * Otherwise, use SPI flash.
   */

  while ((opt = getopt(argc, argv, "w:h:c:mi:")) != -1)
    {
      switch (opt)
        {
        case 'w':
          width = (int)strtol(optarg, NULL, 10);
          break;

        case 'h':
          height = (int)strtol(optarg, NULL, 10);
          break;

        case 'c':
          loop = (int)strtol(optarg, NULL, 10);
          break;

        case 'm':
          mcu = true;
          break;

        case 'i':
          lcd_show_interval = (int)strtol(optarg, NULL, 10);
          break;

        case '?':
        default:
          show_usage();
          return -1;
          }
      }

  ret = stat("/mnt/sd0", &stat_buf);
  if (ret < 0)
    {
      save_dir = "/mnt/spif";
    }
  else
    {
      save_dir = "/mnt/sd0";
    }

  printf("size : %dx%d / count : %d / mcu : %s / save_dir : \"%s\" / show_interval : %d\n",
         width, height, loop, (mcu)?"true":"false", save_dir, lcd_show_interval);

  ret = video_initialize("/dev/video");
  if (ret != 0)
    {
      printf("ERROR: Failed to initialize video: errno = %d\n", errno);
      goto errout_with_nx;
    }

  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_video_init;
    }

  /* Prepare STILL_CAPTURE */

  ret = camera_prepare(v_fd,
                       V4L2_BUF_TYPE_STILL_CAPTURE,
                       V4L2_BUF_MODE_FIFO,
                       V4L2_PIX_FMT_JPEG,
                       width,
                       height,
                       STILL_BUFNUM);
  if (ret < 0)
    {
      goto errout_with_buffer;
    }

  ret = ioctl(v_fd, VIDIOC_TAKEPICT_START, 0);
  if (ret < 0)
    {
      printf("Failed to start taking picture\n");
    }

  while (loop-- > 0)
    {
      /* Note: VIDIOC_DQBUF acquire capture data. */

      memset(&buf, 0, sizeof(v4l2_buffer_t));
      buf.type = buf_type;
      buf.memory = V4L2_MEMORY_USERPTR;

      ret = ioctl(v_fd, VIDIOC_DQBUF, (unsigned long)&buf);
      if (ret)
        {
          printf("Fail DQBUF %d\n", errno);
          goto errout_with_buffer;
        }

      ret = write_file((uint8_t *)buf.m.userptr, (size_t)buf.bytesused, format);
      if (ret)
        {
          printf("Fail write_file %d\n", errno);
          goto errout_with_buffer;
        }

      /* Note: VIDIOC_QBUF reset released buffer pointer. */

      ret = ioctl(v_fd, VIDIOC_QBUF, (unsigned long)&buf);
      if (ret)
        {
          printf("Fail QBUF %d\n", errno);
          goto errout_with_buffer;
        }

      if (!show_lcd_task_running)
        {
          show_lcd_task_running = true;
          pid = task_create("decode_and_show_lcd_task",
                      100,
                      3072,
                      (main_t)decode_and_show_lcd,
                      (FAR char * const *)NULL);
        }
      usleep(10000);
//      while (lcd_show_running)
//        {
//          usleep(10000);
//        }
    }

  printf("\ntake pict completed\n\n");

  ret = ioctl(v_fd, VIDIOC_TAKEPICT_STOP, false);
  if (ret < 0)
    {
      printf("Failed to start taking picture\n");
    }

  while(!lcd_show_end)
    {
      sleep(1);
    }
  task_delete(pid);
  printf("take pict and show lcd completed\n");

  exitcode = OK;

errout_with_buffer:
  close(v_fd);

  free_buffer(buffers_still, STILL_BUFNUM);

errout_with_video_init:

  video_uninitialize();

errout_with_nx:
  return exitcode;
}
