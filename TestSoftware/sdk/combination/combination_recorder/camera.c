/****************************************************************************
 * camera/camera_main.c
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
#include <errno.h>
#include <debug.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <nuttx/fs/mkfatfs.h>
#include <nuttx/drivers/ramdisk.h>
#include "video/video.h"
#include "video_capture_thread.h"

//#include "board/spresense/include/common/cxd56_isx012.h"

#include <nuttx/video/isx012.h>

#include <sys/ioctl.h>
#include <sys/boardctl.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include <arch/chip/pm.h>
#include <arch/board/board.h>
#include <arch/chip/cisif.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/* Note: Buffer size must be multiple of 32. */

#define IMAGE_JPG_SIZE     (20*1024)  /* 150kB */
#define IMAGE_YUV_SIZE     (320*240*2) /* 150kB */

#define VIDEO_BUFNUM       (3)
#define STILL_BUFNUM       (1)

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

static struct v_buffer  *buffers_video;
static struct v_buffer  *buffers_still;
static unsigned int     n_buffers;

static uint8_t camera_main_file_count = 0;
static char    camera_main_filename[32];
bool initialize = false;


/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int  write_file(uint8_t *data, size_t len, int format);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static uint8_t __attribute__ ((aligned(32))) buffer[IMAGE_JPG_SIZE];

static char    g_camera_filename[32];

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/


static int write_file(
  uint8_t *data,
  size_t len,
  int format
)
{
  FILE *fp;
  int  fd;

  struct timespec curtime;   /* for clock_gettime */
  struct tm       *curgmt;   /* for gmtime        */

  clock_gettime(CLOCK_REALTIME, &curtime);
  curgmt = gmtime(&curtime.tv_sec);

  memset(g_camera_filename, 0, sizeof(g_camera_filename));

  mkdir("/mnt/sd0/CAMERA", 0777);

  if (format == V4L2_PIX_FMT_JPEG)
    {
      sprintf(g_camera_filename,
              "/mnt/sd0/CAMERA/%02d%02d%02d%02d.JPG",
              curgmt->tm_mday,
              curgmt->tm_hour,
              curgmt->tm_min,
              curgmt->tm_sec);
    }
  else
    {
      sprintf(g_camera_filename,
              "/mnt/sd0/CAMERA/%02d%02d%02d%02d.YUV",
              curgmt->tm_mday,
              curgmt->tm_hour,
              curgmt->tm_min,
              curgmt->tm_sec);
    }

  fp = fopen(g_camera_filename, "wb");
  if (NULL == fp)
    {
      printf("fopen error : %d\n", errno);
      return -1;
    }

  if (len != fwrite(data, 1, len, fp))
    {
      printf("fwrite error : %d\n", errno);
    }

  fflush(fp);
  fd = fileno(fp);
  fsync(fd);
  fclose(fp);
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
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
       printf("Failed to VIDIOC_S_FMT still : errno = %d\n", errno);
       return ret;
     }


     ret = test_vidioc_s_parm(fd, V4L2_BUF_TYPE_STILL_CAPTURE, numerator, denominator);

     if (ret < 0)
       {
         printf("Failed to VIDIOC_S_parm: errno = %d\n", errno);
         return ret;
       }

     /* VIDIOC_QBUF enqueue buffer */

     if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
       {
         buffers_video = malloc(sizeof(v_buffer_t) * buffernum);
         buffers = buffers_video;
       }
     else
       {
         buffers_still = malloc(sizeof(v_buffer_t) * buffernum);
         buffers = buffers_still;
       }
       printf("buff : %p\n", buffers_still);

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
         //buffers[n_buffers].start  =  buffer[IMAGE_JPG_SIZE];
         if (!buffers[n_buffers].start)
           {
             printf("Out of memory\n");
             return ret;
           }
           printf("start : %p\n", buffers[n_buffers].start);

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


 int still_shutter(int capture_num,
                   int hsize, int vsize,
                   int numerator,
                   int denominator,
                   int pixelformat)
 {
   int ret;
   int exitcode = ERROR;
   int v_fd;
   uint32_t loop;
   uint32_t buf_type;
   uint32_t format;
   struct v4l2_buffer         buf;


   buf_type = V4L2_BUF_TYPE_STILL_CAPTURE;
   format   = V4L2_PIX_FMT_JPEG;

   loop = capture_num;
/*
   ret = is_initialize();
   //if (ret != 0)
   if ( initialize == false )
     {
       printf("ERROR: Failed to initialize video: errno = %d\n", errno);
       goto errout_with_nx;
     }
*/

   v_fd = open("/dev/video", 0);
   if (v_fd < 0)
     {
       printf("ERROR: Failed to open video.errno = %d\n", errno);
       goto errout_with_isx;
     }

   /* Prepare STILL_CAPTURE */
   ret = camera_prepare(v_fd,
                        V4L2_BUF_TYPE_STILL_CAPTURE,
                        V4L2_BUF_MODE_FIFO,
                        //V4L2_PIX_FMT_JPEG,
                        pixelformat,
                         hsize,
                         vsize,
                        STILL_BUFNUM,
                         numerator,
                         denominator);
   if (ret < 0)
     {
       goto errout_with_buffer;
       return ret;
     }

   //test_streamoff(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE);
   //streamoffはない
   //ret = ioctl(v_fd, VIDIOC_STREAMOFF, V4L2_BUF_TYPE_STILL_CAPTURE);

   if (buf_type == V4L2_BUF_TYPE_STILL_CAPTURE)
     {
       ret = ioctl(v_fd, VIDIOC_TAKEPICT_START, 0);
       if (ret < 0)
         {
           printf("Failed to start taking picture\n");
         }
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

 printf("bytesused : %d\n", buf.bytesused);

       write_file((uint8_t *)buf.m.userptr, (size_t)buf.bytesused, pixelformat);

       /* Note: VIDIOC_QBUF reset released buffer pointer. */

       ret = ioctl(v_fd, VIDIOC_QBUF, (unsigned long)&buf);
       if (ret)
         {
           printf("Fail QBUF %d\n", errno);
           goto errout_with_buffer;
         }
     }

   if (buf_type == V4L2_BUF_TYPE_STILL_CAPTURE)
     {
       ret = ioctl(v_fd, VIDIOC_TAKEPICT_STOP, false);
       if (ret < 0)
         {
           printf("Failed to start taking picture\n");
         }
     }

   exitcode = OK;

 errout_with_buffer:
   close(v_fd);

   free_buffer(buffers_still, STILL_BUFNUM);

 errout_with_isx:

   //video_uninitialize();
   //initialize = false;


 errout_with_nx:

   //video_finalize();
   printf("Still Confirmation\n");
   return exitcode;
 }

int camera_main(unsigned long loop)
{
  int ret;
  int v_fd;
  uint32_t cnt;
  uint32_t fsize;
  enum   v4l2_buf_type       type;
  struct v4l2_format         fmt;
  struct v4l2_requestbuffers req;
  struct v4l2_buffer         buf;

  //ret = board_isx012_initialize("/dev/video", IMAGER_I2C);
  //ret = video_initialize("/dev/video");
  //ret = is_initialize();
  still_shutter( loop, VIDEO_HSIZE_QVGA, VIDEO_VSIZE_QVGA, 1, 15 , V4L2_PIX_FMT_JPEG);

#if 0
  printf("camera init ret=%d\n",ret);
  if (ret != 0)
    {
      printf("ERROR: Failed to init video. %d\n", errno);
      return -EPERM;
    }

  //v_fd = open("/dev/video0", O_CREAT);
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
    {
      printf("ERROR: Failed to open video. %d\n", errno);
      video_uninitialize();
      return -ENODEV;
    }

  /* Note: VIDIOC_S_FMT set buffer size. */
  /*       Currently, width and height are fixed. */

  memset(&fmt, 0, sizeof(v4l2_format_t));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  fsize = IMAGE_YUV_SIZE;
  fmt.fmt.pix.width       = VIDEO_HSIZE_QVGA;
  fmt.fmt.pix.height      = VIDEO_VSIZE_QVGA;

  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
  fmt.fmt.pix.field = V4L2_FIELD_ANY;
  ret = ioctl(v_fd, VIDIOC_S_FMT, (unsigned long)&fmt);
  if (ret)
    {
      printf("Fail set format %d\n", errno);
      return ERROR;
    }

  /* Note: VIDIOC_REQBUFS set buffer stages. */

  memset(&req, 0, sizeof(v4l2_requestbuffers_t));

  req.count  = 1;
  req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_USERPTR;

  ret = ioctl(v_fd, VIDIOC_REQBUFS, (unsigned long)&req);
  if (ret)
    {
      printf("Does not support user pointer i/o %d\n", errno);
      return ERROR;
    }

  /* Set constant parameter for VIDIOC_QBUF */
  memset(&buf, 0, sizeof(v4l2_buffer_t));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_USERPTR;
  buf.index = 0;
  buf.m.userptr = (uint32_t)buffer;

  /* Note: VIDIOC_STREAMON start video. */

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ret = ioctl(v_fd, VIDIOC_STREAMON, (unsigned long)&type);
  if (ret)
    {
      printf("Fail STREAMON %d\n", errno);
      return ERROR;
    }

  for (cnt = 0; cnt < loop; cnt++)
    {
      switch (cnt%2)
        {
          case 0:
            fsize                   = IMAGE_YUV_SIZE;
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
            fmt.fmt.pix.width       = VIDEO_HSIZE_QVGA;
            fmt.fmt.pix.height      = VIDEO_VSIZE_QVGA;

            break;

          case 1:
            fsize                   = IMAGE_JPG_SIZE;
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
            fmt.fmt.pix.width       = VIDEO_HSIZE_VGA;
            fmt.fmt.pix.height      = VIDEO_VSIZE_VGA;

            break;

          default:
            continue;
        }

      ret = ioctl(v_fd, VIDIOC_S_FMT, (unsigned long)&fmt);
      if (ret)
        {
          printf("Fail set format %d\n", errno);
          return ERROR;
        }

      buf.length = fsize;
      ret = ioctl(v_fd, VIDIOC_QBUF, (unsigned long)&buf);
      if (ret)
        {
          printf("Fail QBUF %d\n", errno);
          return ERROR;
        }

      /* Note: VIDIOC_DQBUF acquire capture data. */

      memset(&buf, 0, sizeof(v4l2_buffer_t));
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_USERPTR;

      ret = ioctl(v_fd, VIDIOC_DQBUF, (unsigned long)&buf);
      if (ret)
        {
          printf("Fail DQBUF %d\n", errno);
          return ERROR;
        }

      write_file((uint8_t *)buf.m.userptr,
                 (size_t)buf.bytesused,
                 fmt.fmt.pix.pixelformat);

      sleep(60);
    }

  close(v_fd);

  //board_isx012_uninitialize();
  //video_uninitialize();
  #endif
  return 0;
}

static void camera_task(void)
{
  while (1)
    {
      /* from open(device) to close() */

      camera_main(10);
      sleep(3);
      //break;
    }

  return;
}

int camera(void)
{
  int ret;

  ret = task_create("camera_task",
                    200,     /* task priority   */
                    2048,    /* task stack size */
                    (main_t)camera_task,
                    NULL);
  if (ret < 0)
    {
      printf("Failed to create task for camera still mode. ret=%d\n", ret);
      return ret;
    }

  return OK;
}
