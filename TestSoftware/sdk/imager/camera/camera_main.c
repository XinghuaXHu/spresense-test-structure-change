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
#include "video_capture_thread.h"

#include "testapi.h"
#include "camera_testfunc.h"

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


#ifdef CONFIG_BUFNUM_NORMAL
	#define VIDEO_BUFNUM       (2)
	#define STILL_BUFNUM       (2)
	#define IMAGE_JPG_SIZE     (512*1024)  /* 512kB */
	#define IMAGE_YUV_SIZE     (320*240*2) /* QVGA YUV422 */
#endif
#ifdef CONFIG_BUFNUM_TEST
	#define VIDEO_BUFNUM       (1)
	#define STILL_BUFNUM       (1)
	#define IMAGE_JPG_SIZE     (512*1024)  /* 512kB */
	#define IMAGE_YUV_SIZE     (480*360*2) /* QVGA YUV422 */
#endif

#define DEFAULT_REPEAT_NUM (10)
#define SD_CARD_PICTURE_PATH "/mnt/sd0"

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

static int  write_file(uint8_t *data, size_t len, uint32_t format, uint8_t *folderpath);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct v_buffer  *buffers_video;
static struct v_buffer  *buffers_still;
static unsigned int     n_buffers;

static uint16_t camera_main_file_count = 0;
static char    camera_main_filename[64];
bool initialize = false;
static int i_testno = 0;

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int  write_file(uint8_t *data, size_t len, uint32_t format, uint8_t *folderpath)
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
              "%s/VIDEO%03d.jpg",
              folderpath,camera_main_file_count);
    }
  else
    {
      sprintf(camera_main_filename,
              "%s/VIDEO%03d.yuv",
              folderpath,camera_main_file_count);
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

static int camera_prepare_sub(int                fd,
                          enum v4l2_buf_type type,
                          uint32_t           buf_mode,
                          uint32_t           pixformat,
                          uint16_t           hsize,
                          uint16_t           vsize,
                          uint8_t            buffernum,
                          int numerator,
                          int denominator,
                          uint16_t           sub_hsize,
                          uint16_t           sub_vsize
                          )
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
  printf("%d\n", __LINE__);

  ret = test_vidioc_s_fmt(fd, type, hsize, vsize, pixformat, sub_hsize, sub_vsize, V4L2_PIX_FMT_UYVY);

  printf("%d\n", __LINE__);

    if (ret < 0)
      {
        printf("Failed to VIDIOC_S_FMT: errno = %d\n", errno);
        return ret;
      }

      printf("%d\n", __LINE__);

  ret = test_vidioc_s_parm(fd, V4L2_BUF_TYPE_STILL_CAPTURE, numerator, denominator);

  if (ret < 0)
    {
      printf("Failed to VIDIOC_S_parm: errno = %d\n", errno);
      return ret;
    }

  /* VIDIOC_QBUF enqueue buffer */

  buffers_still = malloc(sizeof(v_buffer_t) * buffernum);
  buffers = buffers_still;

  printf("buff : %p\n", buffers_still);

  if (!buffers)
    {
      printf("Out of memory\n");
      return ret;
    }

  fsize = IMAGE_YUV_SIZE + IMAGE_JPG_SIZE;

  printf("fsize : %d\n", fsize);
  printf("%d\n", __LINE__);

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
        printf("start : %p\n", buffers[n_buffers].start);
    }

    printf("%d\n", __LINE__);
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
          return ret;
        }
    }

  /* VIDIOC_STREAMON start stream */
printf("%d\n", __LINE__);
  ret = ioctl(fd, VIDIOC_STREAMON, (unsigned long)&type);
  if (ret < 0)
    {
      printf("Failed to VIDIOC_STREAMON: errno = %d\n", errno);
      return ret;
    }

  return OK;
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
      printf("Failed to VIDIOC_S_FMT: errno = %d\n", errno);
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

      printf("fsize : %d\n", fsize);
      printf("%d\n", __LINE__);

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
          printf("start : %p\n", buffers[n_buffers].start);

      }
printf("%d\n", __LINE__);
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
            return ret;
          }
      }

    /* VIDIOC_STREAMON start stream */
  printf("%d\n", __LINE__);
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
printf("free_buffer bufnum %d\n", bufnum);
      for (cnt = 0; cnt < bufnum; cnt++)
        {
  printf("%d\n", __LINE__);
          if (buffers[cnt].start)
            {
printf("free_buffer cnt %d\n", cnt);
              free(buffers[cnt].start);
            }
        }
  printf("%d\n", __LINE__);
      free(buffers);
    }
}

/*Test for open-close shutter*/
int still_shutter_close(int capture_num, int hsize, int vsize, int numerator, int denominator, int pixelformat)
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

  ret = video_initialize("/dev/video");

  printf("%d\n", __LINE__);
  if (ret != 0)
  //if (initialize == false )
    {
      printf("ERROR: Failed to initialize video: errno = %d\n", errno);
      goto errout_with_nx;
    }

  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }

  /* Prepare STILL_CAPTURE */
printf("%d\n", __LINE__);
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
printf("%d\n", __LINE__);
  if (ret < 0)
    {
      goto errout_with_buffer;
      return ret;
    }
printf("%d\n", __LINE__);

  test_streamoff(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE);
  //ret = ioctl(v_fd, VIDIOC_STREAMOFF, V4L2_BUF_TYPE_STILL_CAPTURE);

  if (buf_type == V4L2_BUF_TYPE_STILL_CAPTURE)
    {
      ret = ioctl(v_fd, VIDIOC_TAKEPICT_START, 0);
      ret = test_streamoff(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE);
      if (ret < 0)
        {
          printf("Failed to start taking picture\n");
        }
      ret = test_streamoff(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE);
      if (ret == 0)
        {
          printf("stream off stll capture\n");
        }
    }
printf("%d\n", __LINE__);
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
printf("%d\n", __LINE__);
      write_file((uint8_t *)buf.m.userptr, (size_t)buf.bytesused, pixelformat, SD_CARD_PICTURE_PATH);

      /* Note: VIDIOC_QBUF reset released buffer pointer. */

      ret = ioctl(v_fd, VIDIOC_QBUF, (unsigned long)&buf);
      if (ret)
        {
          printf("Fail QBUF %d\n", errno);
          goto errout_with_buffer;
        }
    }
printf("%d\n", __LINE__);

  exitcode = OK;

errout_with_buffer:
  close(v_fd);

  free_buffer(buffers_still, STILL_BUFNUM);

errout_with_isx:

  //video_uninitialize();
  initialize = false;

errout_with_nx:


  printf("Stillc Confirmation\n");
  return exitcode;
}


int sub_shutter(int capture_num,
                  int hsize,
                  int vsize,
                  int numerator,
                  int denominator,
                  int sub_hsize,
                  int sub_vsize)
{
  int ret;
  int exitcode = ERROR;
  int v_fd;
  uint32_t loop;
  uint32_t buf_type;
  uint32_t format;
  struct v4l2_buffer         buf;

  buf_type = V4L2_BUF_TYPE_STILL_CAPTURE;
  format   = V4L2_PIX_FMT_JPEG_WITH_SUBIMG;

  loop = capture_num;

  ret = video_initialize("/dev/video");

printf("%d\n", __LINE__);
//  if (initialize == false )
  if (ret != 0)
    {
      printf("ERROR: Failed to initialize video: errno = %d\n", errno);
      goto errout_with_nx;
    }

  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }

  /* Prepare STILL_CAPTURE */
  printf("%d\n", __LINE__);
  ret = camera_prepare_sub(v_fd,
                       V4L2_BUF_TYPE_STILL_CAPTURE,
                       V4L2_BUF_MODE_FIFO,
                       //V4L2_PIX_FMT_JPEG,
                       V4L2_PIX_FMT_JPEG_WITH_SUBIMG,
                        hsize,
                        vsize,
                       STILL_BUFNUM,
                        numerator,
                        denominator,
                      sub_hsize,
                      sub_vsize);
printf("%d\n", __LINE__);
  if (ret < 0)
    {
      goto errout_with_buffer;
      return ret;
    }
printf("%d\n", __LINE__);

  //ret = ioctl(v_fd, VIDIOC_STREAMOFF, V4L2_BUF_TYPE_STILL_CAPTURE);

  if (buf_type == V4L2_BUF_TYPE_STILL_CAPTURE)
    {
      ret = ioctl(v_fd, VIDIOC_TAKEPICT_START, 0);
      if (ret < 0)
        {
          printf("Failed to start taking picture\n");
        }
    }
printf("%d\n", __LINE__);
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
printf("%d\n", __LINE__);
printf("%d, %d, %d, %d, %d, %d, %d\n", capture_num, hsize, vsize, numerator, denominator, sub_hsize, sub_vsize);

printf("dq buff : %p\n", buf);

printf("usrptr : %p\n", buf.m.userptr);
printf("bytesused : %d\n", buf.bytesused);
sleep(10);

      write_file((uint8_t *)buf.m.userptr, (sub_vsize * sub_hsize * 2), V4L2_PIX_FMT_UYVY, SD_CARD_PICTURE_PATH);
      //write_file((uint8_t *)buf.m.userptr, 1,  V4L2_PIX_FMT_UYVY);
      write_file((uint8_t *)buf.m.userptr + (sub_vsize * sub_hsize * 2), (size_t)buf.bytesused, V4L2_PIX_FMT_JPEG, SD_CARD_PICTURE_PATH);
      /*Not support In v1.1.0*/

      /* Note: VIDIOC_QBUF reset released buffer pointer. */

      ret = ioctl(v_fd, VIDIOC_QBUF, (unsigned long)&buf);
      if (ret)
        {
          printf("Fail QBUF %d\n", errno);
          goto errout_with_buffer;
        }
    }
printf("%d\n", __LINE__);

  if (buf_type == V4L2_BUF_TYPE_STILL_CAPTURE)
    {
      ret = ioctl(v_fd, VIDIOC_TAKEPICT_STOP, false);
      if (ret < 0)
        {
          printf("Failed to start taking picture\n");
        }
    }

printf("%d\n", __LINE__);
  exitcode = OK;

errout_with_buffer:
  close(v_fd);

  free_buffer(buffers_still, STILL_BUFNUM);

errout_with_isx:

  video_uninitialize();

errout_with_nx:

  printf("Still Confirmation\n");
  return exitcode;
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
  char    test_no_name[32] ={0};


  buf_type = V4L2_BUF_TYPE_STILL_CAPTURE;
  format   = V4L2_PIX_FMT_JPEG;

  loop = capture_num;

  ret = video_initialize("/dev/video");

  //if ( initialize == false )
  if (ret != 0)
    {
      printf("ERROR: Failed to initialize video: errno = %d\n", errno);
      goto errout_with_nx;
    }

  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }

  /* Prepare STILL_CAPTURE */
printf("%d\n", __LINE__);
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
printf("%d\n", __LINE__);
  if (ret < 0)
    {
      goto errout_with_buffer;
      return ret;
    }
printf("%d\n", __LINE__);

  //ret = ioctl(v_fd, VIDIOC_STREAMOFF, V4L2_BUF_TYPE_STILL_CAPTURE);

  if (buf_type == V4L2_BUF_TYPE_STILL_CAPTURE)
    {
      ret = ioctl(v_fd, VIDIOC_TAKEPICT_START, 0);
      if (ret < 0)
        {
          printf("Failed to start taking picture\n");
        }
    }
printf("%d\n", __LINE__);
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
        printf("buf flags =  %d\n", buf.flags);
printf("%d\n", __LINE__);

printf("dq buff : %p\n", buf);

printf("usrptr : %p\n", buf.m.userptr);
printf("bytesused : %d\n", buf.bytesused);
      
      if (0 != i_testno)
      {
         sprintf(test_no_name,"%s/autotest/Test_%d_%d",SD_CARD_PICTURE_PATH, i_testno, camera_main_file_count+1);
         //mkdir(test_no_name,0777);
      } 
      else
      {
         sprintf(test_no_name,"%s",SD_CARD_PICTURE_PATH);
      }
      write_file((uint8_t *)buf.m.userptr, (size_t)buf.bytesused, pixelformat, test_no_name);

      /* Note: VIDIOC_QBUF reset released buffer pointer. */

      ret = ioctl(v_fd, VIDIOC_QBUF, (unsigned long)&buf);
      if (ret)
        {
          printf("Fail QBUF %d\n", errno);
          goto errout_with_buffer;
        }
    }
printf("%d\n", __LINE__);

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

  printf("Still Confirmation\n");
  return exitcode;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
#ifdef CONFIG_BUILD_KERNEL
int camera_main(int argc, FAR char *argv[])
#else
int camera_main(int argc, char *argv[])
#endif
{
  int ret;
  int exitcode = ERROR;
  int v_fd;
  uint32_t loop;
  uint32_t buf_type;
  uint32_t format;
  struct v4l2_buffer         buf;


  video_initialize("/dev/video");

  if (argc>=2 && strncmp(argv[1], "cap", 4)==0)
  {
    buf_type = V4L2_BUF_TYPE_STILL_CAPTURE;
    format   = V4L2_PIX_FMT_JPEG;
  }
  else if(argc>=2 && strncmp(argv[1], "stillc", 7)==0)
  {
    if(argc==2) still_shutter_close( 1, VIDEO_HSIZE_FULLHD, VIDEO_VSIZE_FULLHD, 1, 15 , V4L2_PIX_FMT_JPEG);
    if(argc==6) still_shutter_close( atoi(argv[4]), atoi(argv[2]), atoi(argv[3]), 1, atoi(argv[5]), V4L2_PIX_FMT_JPEG );
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "init", 7)==0)
  {
    printf("Do free\n");
    return;
  }
  else if(argc>=2 && strncmp(argv[1], "open", 7)==0)
  {
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    return;
  }
  else if(argc>=2 && strncmp(argv[1], "close", 7)==0)
  {
    int ret;
    ret = close(v_fd);
    if(ret == 0) printf("closed\n");
    return;
  }
  else if(argc>=2 && strncmp(argv[1], "testno", 7)==0)
  {
    if(0 != atoi(argv[2]))
    {
      i_testno = atoi(argv[2]);
    }
    else 
    {
      i_testno =0;
    }
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "still", 6)==0)
  {
    if(argc==2) still_shutter( 1, VIDEO_HSIZE_FULLHD, VIDEO_VSIZE_FULLHD, 1, 15 , V4L2_PIX_FMT_JPEG);
    if(argc==6) still_shutter( atoi(argv[4]), atoi(argv[2]), atoi(argv[3]), 1, atoi(argv[5]), V4L2_PIX_FMT_JPEG );
    //video_uninitialize();
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "sub", 4)==0)
  {
    if( argc== 8)
    {
      sub_shutter(atoi(argv[6]),
                atoi(argv[2]),
                atoi(argv[3]),
                1,
                atoi(argv[7]),
                atoi(argv[4]),
                atoi(argv[5]) );
    }
    /*
    *argv number default is 6
    *change in ./tools/config.py
    */
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "yuv", 4)==0)
    {
      if(argc==2) still_shutter( 1, VIDEO_HSIZE_QVGA, VIDEO_VSIZE_QVGA, 1, 15 , V4L2_PIX_FMT_UYVY);
      if(argc==6) still_shutter( atoi(argv[4]), atoi(argv[2]), atoi(argv[3]), 1, atoi(argv[5]), V4L2_PIX_FMT_UYVY);
      return 0;
    }
  else if(argc>=2 && strncmp(argv[1], "video_capture_start", 19)==0)
  {
    // start video capture
    video_capture_flag(true);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "videostart", 11)==0)
  {
    if(argv[2] && argv[3] && argv[4])
    {
      video_capture_setting(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    }
    ret = video_capture_thread_create();
    //sleep(3);
    //ret = video_capture_thread_delete();
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "videodelete", 11)==0)
  {
    video_capture_thread_delete();
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "streamoff", 11)==0)
  {
    printf("video stream off test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    test_streamoff(v_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE);
    test_streamoff(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE);
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "streamon", 11)==0)
  {
    printf("video stream on test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    test_streamon(v_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE);
    test_streamon(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE);
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "brightness", 11)==0)
  {
    printf("brightness change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_BRIGHTNESS, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "contrast", 9)==0)
  {
    printf("contrast change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_CONTRAST, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "saturation", 12)==0)
  {
    printf("saturation change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_SATURATION, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "hue", 4)==0)
  {
    printf("hue change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_HUE, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "awb", 4)==0)
  {
    printf("auto white balance change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_AUTO_WHITE_BALANCE, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "redbalance", 12)==0)
  {
    printf("red balance change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_RED_BALANCE, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "bluebalance", 13)==0)
  {
    printf("blue balance change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_BLUE_BALANCE, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }

  else if(argc>=2 && strncmp(argv[1], "gamma", 6)==0)
  {
    printf("gamma change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_USER, V4L2_CID_GAMMA_CURVE, atoi(argv[2]) );
      //test_vidioc_s_ctrl(v_fd, V4L2_CID_GAMMA_CURVE, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }

  else if(argc>=2 && strncmp(argv[1], "exposure", 9)==0)
  {
    printf("exposure change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_EXPOSURE, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "hflip", 6)==0)
  {
    printf("hflip change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_HFLIP, atoi(argv[2]) );
      test_vidioc_s_ctrl(v_fd, V4L2_CID_HFLIP_STILL, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "vflip", 6)==0)
  {
    printf("vflip change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_VFLIP, atoi(argv[2]) );
      test_vidioc_s_ctrl(v_fd, V4L2_CID_VFLIP_STILL, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "sharpness", 10)==0)
  {
    printf("sharpness change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_SHARPNESS, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "colorkill", 10)==0)
  {
    printf("gray scale change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_COLOR_KILLER, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "colorfx", 8)==0)
  {
    printf("colorfx scale change test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ctrl(v_fd, V4L2_CID_COLORFX, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "check", 6)==0)
  {
    printf("check test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if(16 < atoi(argv[2]) || atoi(argv[2]) < 0)
    {
      printf("out of range num g_ctrl: ");

    }
    switch ( atoi(argv[2]) )
    {
      case 0:
        printf("brightness: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_BRIGHTNESS);
        break;
      case 1:
        printf("contrast: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_CONTRAST);
        break;
      case 2:
        printf("saturation: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_SATURATION);
        break;
      case 3:
        printf("hue: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_HUE);
        break;
      case 4:
        printf("auto white balance: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_AUTO_WHITE_BALANCE);
        break;
      case 5:
        printf("red balance: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_RED_BALANCE);
        break;
      case 6:
        printf("blue balance: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_BLUE_BALANCE);
        break;
      case 7:
        printf("gamma: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_GAMMA);
        break;
      case 8:
        printf("gamma curve: \n");
        test_vidioc_g_ext_ctrl(v_fd, V4L2_CID_GAMMA_CURVE);
//        test_vidioc_g_ctrl(v_fd, V4L2_CID_GAMMA_CURVE);
        break;
      case 9:
        printf("exposure: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_EXPOSURE);
        break;
      case 10:
        printf("hflip: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_HFLIP);
        break;
      case 11:
        printf("vflip: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_VFLIP);
        break;
      case 12:
        printf("hflip_still: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_HFLIP_STILL);
        break;
      case 13:
        printf("vflip_still: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_VFLIP_STILL);
        break;
      case 14:
        printf("sharpness: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_SHARPNESS);
        break;
      case 15:
        printf("color killer: ");
        test_vidioc_g_ctrl(v_fd, V4L2_CID_COLOR_KILLER);
        break;
      case 16:
      printf("color fx: ");
      test_vidioc_g_ctrl(v_fd, V4L2_CID_COLORFX);
      break;
      }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "unmask", 8)==0)
  {
    int i;
    printf("unmask test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    for(i = 0; i < 17; i++)
    {
      printf("check id = %d\n", i);
      test_vidioc_g_ctrl_unmask(v_fd, i);
    }
    return ret;
  }


  else if(argc>=2 && strncmp(argv[1], "buftest",8)==0)
  {
    printf("buffer test\n");

    struct v_buffer  *buffers;
    buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }

    /*initialize buffers(Generate Que)*/
    if(atoi(argv[2]) == 0)
    {
      printf("buftest 0\n");
      ret = test_vidioc_reqbufs(v_fd, VIDEO_BUFNUM, V4L2_BUF_TYPE_VIDEO_CAPTURE, V4L2_MEMORY_USERPTR, V4L2_BUF_MODE_FIFO );
      if(ret == 0) printf("ok video, fifo\n");

      ret = test_vidioc_reqbufs(v_fd, VIDEO_BUFNUM, V4L2_BUF_TYPE_VIDEO_CAPTURE, V4L2_MEMORY_USERPTR, V4L2_BUF_MODE_RING );
      if(ret == 0) printf("ok, video ring\n");

      ret = test_vidioc_reqbufs(v_fd, STILL_BUFNUM, V4L2_BUF_TYPE_STILL_CAPTURE, V4L2_MEMORY_USERPTR, V4L2_BUF_MODE_FIFO );
      if(ret == 0) printf("ok still fifo\n");
      ret = test_vidioc_reqbufs(v_fd, STILL_BUFNUM, V4L2_BUF_TYPE_STILL_CAPTURE, V4L2_MEMORY_USERPTR, V4L2_BUF_MODE_RING );
      if(ret == 0) printf("ok still ring\n");
      return;
    }

    /*Que more than Generate buffer returns error*/
    else if(atoi(argv[2]) == 1)
    {
      printf("buftest 1\n");
      int cnt;
      struct v4l2_requestbuffers req = {0};
      struct v_buffer  *buffers;
      //static struct v_buffer  *buffers_still;
      int buffernum = 2;
      uint32_t fsize;
      //static unsigned int     n_buffers;

      req.type   = V4L2_BUF_TYPE_STILL_CAPTURE;
      req.memory = V4L2_MEMORY_USERPTR;
      req.count  = buffernum;
      req.mode   = V4L2_BUF_MODE_FIFO;

      ret = ioctl(v_fd, VIDIOC_REQBUFS, (unsigned long)&req);
      if (ret < 0)
      {
        printf("Failed to VIDIOC_REQBUFS: errno = %d\n", errno);
        return ret;
      }
      buffers_still = malloc(sizeof(v_buffer_t) * buffernum);
      buffers = buffers_still;
      if (!buffers)
      {
        printf("Out of memory\n");
        return ret;
      }
      fsize = IMAGE_JPG_SIZE;
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
      for (cnt = 0; cnt < n_buffers+1; cnt++)
      {
        memset(&buf, 0, sizeof(v4l2_buffer_t));
        buf.type = V4L2_BUF_TYPE_STILL_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
        buf.index = cnt;
        buf.m.userptr = (unsigned long)buffers[cnt].start;
        buf.length = buffers[cnt].length;

        ret = ioctl(v_fd, VIDIOC_QBUF, (unsigned long)&buf);
        if (ret)
        {
          printf("Fail QBUF %d\n", errno);
          printf("Fail de ok\n");
          printf("ret = %d\n", ret);
          return ret;
        }
      }
      free_buffer(buffers_still, buffernum);
      printf("%d\n", __LINE__);
      return;
    }

    else if(atoi(argv[2]) == 2)
    {
      /*None buffer in que and Deque returns Error*/
      printf("deque with No que return error test\n");
      int cnt, ret;
      struct v4l2_requestbuffers req = {0};
      struct v_buffer  *buffers;
      struct v4l2_buffer         buf;
      //static struct v_buffer  *buffers_still;
      int buffernum = 2;
      uint32_t fsize;
      //static unsigned int     n_buffers;
      printf("%d\n", __LINE__);

      req.type   = V4L2_BUF_TYPE_STILL_CAPTURE;
      req.memory = V4L2_MEMORY_USERPTR;
      req.count  = buffernum;
      req.mode   = V4L2_BUF_MODE_FIFO;
      //printf("%d\n", __LINE__);

      ret = ioctl(v_fd, VIDIOC_REQBUFS, (unsigned long)&req);
      printf("%d\n", __LINE__);
      if (ret < 0)
      {
        printf("Failed to VIDIOC_REQBUFS: errno = %d\n", errno);
        return ret;
      }
      printf("%d\n", __LINE__);
      buffers_still = malloc(sizeof(v_buffer_t) * buffernum);
      buffers = buffers_still;
      printf("%d\n", __LINE__);
      if (!buffers)
      {
        printf("Out of memory\n");
        return ret;
      }
      printf("%d\n", __LINE__);

      fsize = IMAGE_JPG_SIZE;
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
      printf("%d\n", __LINE__);

      memset(&buf, 0, sizeof(v4l2_buffer_t));
      buf.type = V4L2_BUF_TYPE_STILL_CAPTURE;
      buf.memory = V4L2_MEMORY_USERPTR;
      printf("%d\n", __LINE__);
      printf("%d\n", &buf);

      ret = ioctl(v_fd, VIDIOC_DQBUF, (unsigned long)&buf);
      if (ret)
      {
        printf("Fail DQBUF %d\n", errno);
        return -1;
      }
      return;
    }
    else if(atoi(argv[2]) == 300)
    {
      struct v4l2_buffer         buf;

      memset(&buf, 0, sizeof(v4l2_buffer_t));
      buf.type = V4L2_BUF_TYPE_STILL_CAPTURE;
      buf.memory = V4L2_MEMORY_USERPTR;
      ret = ioctl(v_fd, VIDIOC_DQBUF, (unsigned long)&buf);

    }
    else if(atoi(argv[2]) == 3)
    {
      /*Can Deque that was Qued buffer */
      printf("qued buffer can deque buffer num\n");
      int cnt;
      struct v4l2_requestbuffers req = {0};
      struct v_buffer  *buffers;
      //static struct v_buffer  *buffers_still;
      int buffernum = 2;
      uint32_t fsize;
      //static unsigned int     n_buffers;

      req.type   = V4L2_BUF_TYPE_STILL_CAPTURE;
      req.memory = V4L2_MEMORY_USERPTR;
      req.count  = buffernum;
      req.mode   = V4L2_BUF_MODE_FIFO;

      printf("%d\n", __LINE__);
      ret = ioctl(v_fd, VIDIOC_REQBUFS, (unsigned long)&req);
      if (ret < 0)
      {
        printf("Failed to VIDIOC_REQBUFS: errno = %d\n", errno);
        return ret;
      }
      printf("%d\n", __LINE__);
      buffers_still = malloc(sizeof(v_buffer_t) * buffernum);
      buffers = buffers_still;
      if (!buffers)
      {
        printf("Out of memory\n");
        return ret;
      }
      fsize = IMAGE_JPG_SIZE;
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
      printf("%d\n", __LINE__);
      for (cnt = 0; cnt < n_buffers; cnt++)
      {
        memset(&buf, 0, sizeof(v4l2_buffer_t));
        buf.type = V4L2_BUF_TYPE_STILL_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
        buf.index = cnt;
        buf.m.userptr = (unsigned long)buffers[cnt].start;
        buf.length = buffers[cnt].length;

        ret = ioctl(v_fd, VIDIOC_QBUF, (unsigned long)&buf);
        if (ret)
        {
          printf("Fail QBUF %d\n", errno);
          return ret;
        }
      }
      printf("%d\n", __LINE__);

      for (cnt = 0; cnt < buffernum; cnt++)
      {
        printf("%d\n", __LINE__);
        memset(&buf, 0, sizeof(v4l2_buffer_t));
        buf.type = buf_type;
        buf.memory = V4L2_MEMORY_USERPTR;
        printf("%d\n", __LINE__);

        ret = ioctl(v_fd, VIDIOC_DQBUF, (unsigned long)&buf);
        printf("%d\n", __LINE__);
        if (ret != 0)
        {
          printf("Fail DQBUF %d\n", errno);
          return -1;
        }
      }
      printf("%d\n", __LINE__);
      return;
    }
    else
    {
      printf("need argv[2]\n");
      return;
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "device", 7)==0)
  {
    printf("device setting check test\n");
    int class = 0;
    int id = 0;
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    for(class = 0; class < 4; class++)
    {
      for(id = 0; id < 17; id++)
      {
        test_vidioc_queryctrl(v_fd, class, id);
      }
    }
    printf("-------\n");
    for(class = 0; class<4; class++)
    {
      for(id = 0; id< 17; id++)
      {
        test_vidioc_ext_queryctrl(v_fd, class, id);
      }
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "format", 7)==0)
  {
    printf("format check test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if(argc == 4)
    {
      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, atoi(argv[2]), atoi(argv[3]), V4L2_PIX_FMT_JPEG, 0,0,0);
      if(ret == 0){printf("it can on videocapture, jpeg\n");} else{printf("it can not videocapture, jpeg\n");}
      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, atoi(argv[2]), atoi(argv[3]), V4L2_PIX_FMT_UYVY, 0,0,0);
      if(ret == 0){printf("it can on videocapture, yuv\n");} else{printf("it can not videocapture, yuv\n");}

      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE, atoi(argv[2]), atoi(argv[3]), V4L2_PIX_FMT_JPEG, 0,0,0);
      if(ret == 0){printf("it can on stillcapture, jpeg\n");}else{printf("it can not stillcapture, jpeg\n");}
      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE, atoi(argv[2]), atoi(argv[3]), V4L2_PIX_FMT_UYVY, 0,0,0);
      if(ret == 0){printf("it can on stillcapture, yuv\n");}else{printf("it can not stillcapture, yuv\n");}
    }
    if(argc==6)
    {
      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, atoi(argv[2]), atoi(argv[3]), V4L2_PIX_FMT_JPEG_WITH_SUBIMG, atoi(argv[4]), atoi(argv[5]), V4L2_PIX_FMT_JPEG);
      if(ret == 0){printf("it can on videocapture, jpeg+yuv in jpeg\n");}else{printf("it can not videocapture, jpeg+yuv in jpeg\n");}
      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, atoi(argv[2]), atoi(argv[3]), V4L2_PIX_FMT_JPEG_WITH_SUBIMG, atoi(argv[4]), atoi(argv[5]), V4L2_PIX_FMT_UYVY);
      if(ret == 0){printf("it can on videocapture, jpeg+yuv in yuv\n");}else{printf("it can not videocapture, jpeg+yuv in yuv\n");}

      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE, atoi(argv[2]), atoi(argv[3]), V4L2_PIX_FMT_JPEG_WITH_SUBIMG, atoi(argv[4]), atoi(argv[5]), V4L2_PIX_FMT_JPEG);
      if(ret == 0){printf("it can on stillcapture, jpeg+yuv in jpeg\n");}else{printf("it can not stillcaputure jepg+yuv in jpeg\n");}
      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE, atoi(argv[2]), atoi(argv[3]), V4L2_PIX_FMT_JPEG_WITH_SUBIMG, atoi(argv[4]), atoi(argv[5]), V4L2_PIX_FMT_UYVY);
      if(ret == 0){printf("it can on stillcapture, jpeg+yuv in yuv\n");}else{printf("it can not stillcaputure jepg+yuv in yuv\n");}
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "jpg", 4)==0)
  {
    printf("jpg completion test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_JPEG, V4L2_CID_JPEG_COMPRESSION_QUALITY, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "autoexpo", 10)==0)
  {
    printf("auto exposure test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_EXPOSURE_AUTO, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "expoabs", 8)==0)
  {
    printf("absolute exposure test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_EXPOSURE_ABSOLUTE, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }

  else if(argc>=2 && strncmp(argv[1], "expomete", 9)==0)
  {
    printf("metering exposure test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_EXPOSURE_METERING, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }

  else if(argc>=2 && strncmp(argv[1], "whitebala", 10)==0)
  {
    printf("white balance test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }

  else if(argc>=2 && strncmp(argv[1], "zoom", 10)==0)
  {
    printf("zoom test\n");
    printf("camera zoom 1 is absolute\n");
    printf("camera zoom 0 is relative\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( atoi(argv[2])  == 1)
    {
      test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_ZOOM_ABSOLUTE, 0x0200 );
    }
    if ( atoi(argv[2])  == 0)
    {
      test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_ZOOM_RELATIVE, atoi(argv[3]) );
    }
    close(v_fd);
    return 0;
  }
else if(argc>=2 && strncmp(argv[1], "wide", 5)==0)
  {
    printf("V4L2_CID_WIDE_DYNAMIC_RANGE test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_WIDE_DYNAMIC_RANGE , atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }

  else if(argc>=2 && strncmp(argv[1], "3a", 5)==0)
  {
    printf("3A lock test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_3A_LOCK, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "iso", 4)==0)
  {
    printf("iso setting test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_ISO_SENSITIVITY, (atoi(argv[2]) * 1000) );
    }
    close(v_fd);
    return 0;
  }

  else if(argc>=2 && strncmp(argv[1], "af", 4)==0)
  {
    printf("auto focus test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    ret = test_do_halfpush(v_fd);
    printf("ret = %d\n", ret);
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "afnot", 6)==0)
  {
    printf("auto focus test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    ret = test_do_not_halfpush(v_fd);
    printf("ret = %d\n", ret);
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "isoauto", 8)==0)
  {
    printf("iso setting test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    if ( argv[2] )
    {
      test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_ISO_SENSITIVITY_AUTO, atoi(argv[2]) );
    }
    close(v_fd);
    return 0;
  }
else if(argc>=2 && strncmp(argv[1], "repeat", 7)==0)
  {
    printf("open-close repeat test\n");
    int i;
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    for(i = 0; i< 100; i++)
    {
      video_capture_thread_create();
      sleep(30);
      video_capture_thread_delete();
    }
    close(v_fd);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "num", 8)==0)
  {
    int64_t hoge = 530000;
    int64_t fuga = 108;
    int64_t baa = -273;
    printf("%d, %PRId\n", hoge, hoge);
    printf("%d, %PRId\n", fuga, fuga);
    printf("%d, %PRId\n", baa, baa);
    printf("-----\n");
    printf("%d, %PRId\n", (int32_t)hoge, (int32_t)hoge);
    printf("%d, %PRId\n", (int32_t)fuga, (int32_t)fuga);
    printf("%d, %PRId\n", (int32_t)baa, (int32_t)baa);
    printf("-----\n");
    printf("%d, %Id\n", hoge, hoge);
    printf("%d, %Id\n", fuga, fuga);
    printf("%d, %Id\n", baa, baa);

    printf("%d, %Id\n", (int32_t)hoge, (int32_t)hoge);
    printf("%d, %Id\n", (int32_t)fuga, (int32_t)fuga);
    printf("%d, %Id\n", (int32_t)baa, (int32_t)baa);
    return 0;
  }
  else if(argc>=2 && strncmp(argv[1], "dqcan", 8)==0)
  {
    printf("dqbuf cancel api test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }
    ret = ioctl(v_fd, VIDIOC_CANCEL_DQBUF, V4L2_BUF_TYPE_VIDEO_CAPTURE);
    printf("%d\n", ret);
    if (ret == 0) printf("cancel dqbuf ioctl succes\n");
    return ret;
  }
  else if(argc>=2 && strncmp(argv[1], "bufnumcheck", 20)==0)
  {
	printf("buf num video: %d\n", VIDEO_BUFNUM);
	printf("buf num still: %d\n", STILL_BUFNUM);
	return ;
  }
  else
  {
    buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format   = V4L2_PIX_FMT_UYVY;
  }

  if (argc==3)
    {
      loop = atoi(argv[2]);
    }
  else
    {
      loop = DEFAULT_REPEAT_NUM;
    }

  ret = video_initialize("/dev/video");

  if(ret != 0)
    {
      printf("ERROR: Failed to initialize video: errno = %d\n", errno);
      goto errout_with_nx;
    }

  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      goto errout_with_isx;
    }

  /* Prepare VIDEO_CAPTURE */

  ret = camera_prepare(v_fd,
                       V4L2_BUF_TYPE_VIDEO_CAPTURE,
                       V4L2_BUF_MODE_RING,
                       V4L2_PIX_FMT_UYVY,
                       VIDEO_HSIZE_QVGA,
                       VIDEO_VSIZE_QVGA,
                       VIDEO_BUFNUM,
                      1,
                      60);
  if (ret < 0)
    {
      return ret;
    }

  /* Prepare STILL_CAPTURE */

  ret = camera_prepare(v_fd,
                       V4L2_BUF_TYPE_STILL_CAPTURE,
                       V4L2_BUF_MODE_FIFO,
                       V4L2_PIX_FMT_JPEG,
                       VIDEO_HSIZE_FULLHD,
                       VIDEO_VSIZE_FULLHD,
                       STILL_BUFNUM,
                      1,
                    15);
  if (ret < 0)
    {
      goto errout_with_buffer;
      return ret;
    }

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
      /* Note:  acquire capture data. */

      memset(&buf, 0, sizeof(v4l2_buffer_t));
      buf.type = buf_type;
      buf.memory = V4L2_MEMORY_USERPTR;

      ret = ioctl(v_fd, VIDIOC_DQBUF, (unsigned long)&buf);
      if (ret)
        {
          printf("Fail DQBUF %d\n", errno);
          goto errout_with_buffer;
        }

      write_file((uint8_t *)buf.m.userptr, (size_t)buf.bytesused, format, SD_CARD_PICTURE_PATH);


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

  free_buffer(buffers_video, VIDEO_BUFNUM);
  free_buffer(buffers_still, STILL_BUFNUM);

errout_with_isx:

  video_uninitialize();

errout_with_nx:

  return exitcode;

}
