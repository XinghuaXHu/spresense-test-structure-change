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

#include "testapi.h"
#include "camera_testfunc.h"

#define IMAGE_JPG_SIZE     (512*1024)  /* 512kB */
#define IMAGE_YUV_SIZE     (320*240*2) /* QVGA YUV422 */

#define VIDEO_BUFNUM       (3)
#define STILL_BUFNUM       (1)
#define DEFAULT_REPEAT_NUM (10)


struct v_buffer {
  uint32_t             *start;
  uint32_t             length;
};
typedef struct v_buffer v_buffer_t;
struct v4l2_buffer         buf;


static struct v_buffer  *buffers_video;
static struct v_buffer  *buffers_still;
static unsigned int     n_buffers;

static uint8_t camera_main_file_count = 0;
static char    camera_main_filename[32];

int v_fd;
bool initialize = false;

int camera_prepare_test_sub(int fd,
                          enum v4l2_buf_type type,
                          uint32_t           buf_mode,
                          uint32_t           pixformat,
                          uint16_t           hsize,
                          uint16_t           vsize,
                          uint8_t            buffernum,
                          int numerator,
                          int denominator,
                          uint16_t           sub_hsize,
                          uint16_t           sub_vsize)
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

  ret = test_vidioc_s_fmt(fd, type, hsize, vsize, pixformat, hsize, vsize, V4L2_PIX_FMT_UYVY);

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

int stillc(int num, int hsize, int vsize, int numerator, int denominator)
{
  if(hsize == 0 && vsize == 0 && numerator == 0 && denominator == 0)
  {
    still_shutter_close( num, VIDEO_HSIZE_FULLHD, VIDEO_VSIZE_FULLHD, 1, 15 , V4L2_PIX_FMT_JPEG);
  }
  else
  {
    still_shutter_close( num, hsize, vsize, numerator, denominator, V4L2_PIX_FMT_JPEG );
  }
  return 0;
  // > camera stillc 1 1280 710 1 15
}

int still_jpeg(int num, int hsize, int vsize, int numerator, int denominator)
{
  if(hsize == 0 && vsize == 0 && numerator == 0 && denominator == 0)
  {
    still_shutter( num, VIDEO_HSIZE_FULLHD, VIDEO_VSIZE_FULLHD, 1, 15 , V4L2_PIX_FMT_JPEG);
  }
  else
  {
    still_shutter( num, hsize, vsize, numerator, denominator, V4L2_PIX_FMT_JPEG );
  }
  return 0;
  // > camera stillc 1 1280 710 1 15
}

int shutter_plus(int num, int hsize, int vsize, int numerator, int denominator, int sub_hsize, int sub_vsize)
{
  sub_shutter(num,
              hsize,
              vsize,
              numerator,
              denominator,
              sub_hsize,
              sub_vsize);
  return 0;
  //camera sub 1 1280 720 1 15 360 240
}

int still_yuv(int num, int hsize, int vsize, int numerator, int denominator)
{
  if(hsize == 0 && vsize == 0 && numerator == 0 && denominator == 0)
  {
    still_shutter( 1, VIDEO_HSIZE_QVGA, VIDEO_VSIZE_QVGA, 1, 15 , V4L2_PIX_FMT_UYVY);

  }
  else
  {
    still_shutter( num, hsize, vsize, numerator, denominator, V4L2_PIX_FMT_UYVY );
  }
  return 0;
  // > camera yuv 1 1280 710 1 15
}

int videostart()
{
  int ret;
  ret = video_capture_thread_create();
  return ret;
}

int video_delete()
{
  int ret;
  ret = video_capture_thread_delete();
  return ret;
}

int streamoff()
{
  printf("video stream off test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  test_streamoff(v_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE);
  test_streamoff(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE);
  close(v_fd);
  return 0;
  // streamoff still mode is same as video mode

}

int change_brightness(int brightness)
{
  int ret;
  printf("brightness change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_BRIGHTNESS, brightness );
  close(v_fd);
  return ret;
}

int change_contrast(int contrast)
{
  int ret;
  printf("contrast change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_CONTRAST, contrast);
  close(v_fd);
  return ret;
}

int change_saturation(int saturation)
{
  int ret;
  printf("saturation change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_SATURATION, saturation);
  close(v_fd);
  return ret;
}


int change_hue(int hue)
{
  int ret;
  printf("hue change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_HUE, hue);
  close(v_fd);
  return ret;
}



int change_awb(int awb)
{
  int ret;
  printf("auto white balance change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret =       test_vidioc_s_ctrl(v_fd, V4L2_CID_AUTO_WHITE_BALANCE, awb);
  close(v_fd);
  return ret;
}


int change_redbalance(int balance)
{
  int ret;
  printf("redbalance change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_RED_BALANCE, balance);
  close(v_fd);
  return ret;
}

int change_bluebalance(int balance)
{
  int ret;
  printf("saturation change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_BLUE_BALANCE, balance);
  close(v_fd);
  return ret;
}

int change_gamma(int gamma)
{
  int ret;
  printf("gamma change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_GAMMA_CURVE, gamma);
  close(v_fd);
  return ret;
}


int change_exposure(int exposure)
{
  int ret;
  printf("exposure change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_EXPOSURE, exposure);
  close(v_fd);
  return ret;
}

int change_hflip(int flip)
{
  int ret;
  printf("hflip change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_HFLIP, flip);
  close(v_fd);
  return ret;
}

int change_vflip(int flip)
{
  int ret;
  printf("vflip change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_VFLIP, flip);
  close(v_fd);
  return ret;
}

int change_sharpness(int sharpness)
{
  int ret;
  printf("sharpness change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_SHARPNESS, sharpness);
  close(v_fd);
  return ret;
}

int change_colorkill(int color)
{
  int ret;
  printf("colorkill change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_COLOR_KILLER, color);
  close(v_fd);
  return ret;
}

int change_colorfx(int color)
{
  int ret;
  printf("colorfx change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ctrl(v_fd, V4L2_CID_COLORFX, color);
  close(v_fd);
  return ret;
}

int change_check(int check)
{
  int ret;
  printf("change check test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  switch ( check )
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
      printf("gamma curve: ");
      test_vidioc_g_ctrl(v_fd, V4L2_CID_GAMMA_CURVE);
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
  return ret;
}


int buffer_test(int testnum)
{
  int ret;

  printf("buffer test\n");

  struct v_buffer  *buffers;
  uint32_t fsize;
  uint32_t buf_type;

  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }

  if(testnum == 0)
  {
    ret = test_vidioc_reqbufs(v_fd, VIDEO_BUFNUM, V4L2_BUF_TYPE_VIDEO_CAPTURE, V4L2_MEMORY_USERPTR, V4L2_BUF_MODE_FIFO );
    if(ret == 0) printf("ok video, fifo\n");

    ret = test_vidioc_reqbufs(v_fd, VIDEO_BUFNUM, V4L2_BUF_TYPE_VIDEO_CAPTURE, V4L2_MEMORY_USERPTR, V4L2_BUF_MODE_RING );
    if(ret == 0) printf("ok, video ring\n");

    ret = test_vidioc_reqbufs(v_fd, STILL_BUFNUM, V4L2_BUF_TYPE_STILL_CAPTURE, V4L2_MEMORY_USERPTR, V4L2_BUF_MODE_FIFO );
    if(ret == 0) printf("ok still fifo\n");
    ret = test_vidioc_reqbufs(v_fd, STILL_BUFNUM, V4L2_BUF_TYPE_STILL_CAPTURE, V4L2_MEMORY_USERPTR, V4L2_BUF_MODE_RING );
    if(ret == 0) printf("ok still ring itiou dekiru ... :>\n");
  }

  if(testnum == 1)
  {
    int cnt;
    struct v4l2_requestbuffers req = {0};
    struct v_buffer  *buffers;
    //static struct v_buffer  *buffers_still;
    int buffernum = 2;


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
              return ret;;
            }
        }
      free_buffer(buffers_still, buffernum);
  }
  if(testnum == 2)
  {
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
      buf.type = buf_type;
      buf.memory = V4L2_MEMORY_USERPTR;

      ret = ioctl(v_fd, VIDIOC_DQBUF, (unsigned long)&buf);
      if (ret)
        {
          printf("Fail DQBUF %d\n", errno);
          return -1;
        }
  }
  if(testnum == 3)
  {
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
              return ret;;
            }
        }
      for (cnt = 0; cnt < n_buffers; cnt++)
      {
        buf.type = buf_type;
        buf.memory = V4L2_MEMORY_USERPTR;

        ret = ioctl(v_fd, VIDIOC_DQBUF, (unsigned long)&buf);
        if (ret)
        {
          printf("Fail DQBUF %d\n", errno);
          return -1;
        }
      }
  }

  close(v_fd);
  return ret;
}


int check_device()
{
  printf("device setting check test\n");
  int ret;
  int i, j;
  //int k = 0;
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  //test_vidioc_querymenu(v_fd, i, j, k);
  for(i = 0; i<4; i++)
  {
    for(j = 0; j< 17; j++)
    {
      test_vidioc_queryctrl(v_fd, i, j);
      test_vidioc_ext_queryctrl(v_fd, i, j);
    }
  }
  close(v_fd);
  return 0;
}

int check_format(int hsize, int vsize, int sub_hsize, int sub_vsize)
{
  int ret;
    printf("fortmat check test\n");
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      video_uninitialize();
    }
    if(sub_hsize == 0 && sub_vsize)
    {
      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, hsize, vsize, V4L2_PIX_FMT_JPEG, 0,0,0);
      if(ret == 0) printf("it can on videocapture, jpeg\n");
      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, hsize, vsize, V4L2_PIX_FMT_UYVY, 0,0,0);
      if(ret == 0) printf("it can on videocapture, yuv\n");

      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE, hsize, vsize, V4L2_PIX_FMT_JPEG, 0,0,0);
      if(ret == 0) printf("it can on stillcapture, jpeg\n");
      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE, hsize, vsize, V4L2_PIX_FMT_UYVY, 0,0,0);
      if(ret == 0) printf("it can on stillcapture, yuv\n");
    }
    else
    {
      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, hsize, vsize, V4L2_PIX_FMT_JPEG_WITH_SUBIMG, sub_hsize, sub_vsize, V4L2_PIX_FMT_JPEG);
      if(ret == 0) printf("it can on videocapture, jpeg+yuv in jpeg\n");
      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, hsize, vsize, V4L2_PIX_FMT_JPEG_WITH_SUBIMG, sub_hsize, sub_vsize, V4L2_PIX_FMT_UYVY);
      if(ret == 0) printf("it can on videocapture, jpeg+yuv in yuv\n");

      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE, hsize, vsize, V4L2_PIX_FMT_JPEG_WITH_SUBIMG, sub_hsize, sub_vsize, V4L2_PIX_FMT_JPEG);
      if(ret == 0) printf("it can on stillcapture, jpeg+yuv in jpeg\n");
      ret = test_vidoc_try_fmt(v_fd, V4L2_BUF_TYPE_STILL_CAPTURE, hsize, vsize, V4L2_PIX_FMT_JPEG_WITH_SUBIMG, sub_hsize, sub_vsize, V4L2_PIX_FMT_UYVY);
      if(ret == 0) printf("it can on stillcapture, jpeg+yuv in yuv\n");

    }
    close(v_fd);
    return 0;
}

int change_jpg_compress_quority(int quority)
{
  int ret;
  printf("jpeg compress quority change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_JPEG, V4L2_CID_JPEG_COMPRESSION_QUALITY, quority);
  close(v_fd);
  return ret;
}

int change_autoexpo(int boo)
{
  int ret;
  printf("autoexpo change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_JPEG, V4L2_CID_EXPOSURE_AUTO, boo);
  close(v_fd);
  return ret;
}

int change_expoabs(int boo)
{
  int ret;
  printf("expoabs change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_JPEG, V4L2_CID_EXPOSURE_ABSOLUTE, boo);
  close(v_fd);
  return ret;
}

int change_expomete(int boo)
{
  int ret;
  printf("expomete change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_JPEG, V4L2_CID_EXPOSURE_ABSOLUTE, boo);
  close(v_fd);
  return ret;
}

int change_whitebalace(int boo)
{
  int ret;
  printf("white balance change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_JPEG, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, boo);
  close(v_fd);
  return ret;
}

int change_zoom(int boo, int bairitu)
{
  int ret;
  printf("zoom change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }

  if ( boo == 1)
  {
    ret = test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_ZOOM_ABSOLUTE, bairitu );
  }
  if ( boo == 0)
  {
    ret = test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_ZOOM_RELATIVE, bairitu );
  }
  close(v_fd);
  return ret;
}

int change_widedinamicrange(int boo)
{
  int ret;
  printf("wide dinamic range change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_WIDE_DYNAMIC_RANGE , boo );
  close(v_fd);
  return ret;
}

int change_3a(int boo)
{
  int ret;
  printf("3a change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_3A_LOCK, boo );
  close(v_fd);
  return ret;
}

int change_iso(int boo)
{
  int ret;
  printf("iso change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_ISO_SENSITIVITY, boo );
  close(v_fd);
  return ret;
}

int change_isoauto(int boo)
{
  int ret;
  printf("iso auto change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
  ret = test_vidioc_s_ext_ctrl(v_fd, V4L2_CTRL_CLASS_CAMERA, V4L2_CID_ISO_SENSITIVITY_AUTO, boo );
  close(v_fd);
  return ret;
}

int change_af(int boo)
{
  int ret;
  printf("auto focus change test\n");
  v_fd = open("/dev/video", 0);
  if (v_fd < 0)
  {
    printf("ERROR: Failed to open video.errno = %d\n", errno);
    video_uninitialize();
  }
   ret = test_do_halfpush(v_fd);
  close(v_fd);
  return ret;
}

int repeat_create_delete()
{
    printf("open-close repeat test\n");
    int i;
    v_fd = open("/dev/video", 0);
    if (v_fd < 0)
    {
      printf("ERROR: Failed to open video.errno = %d\n", errno);
      video_uninitialize();
    }

    for(i = 0; i< 10; i++)
    {
      video_capture_thread_create();
      sleep(5);
      video_capture_thread_delete();
    }
    close(v_fd);
    return 0;
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

  ret = is_initialize();
  //if (ret != 0)
  printf("%d\n", __LINE__);
  if (initialize == false )
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
  ret = camera_prepare_test(v_fd,
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
printf("%d\n", __LINE__);

printf("dq buff : %p\n", buf);

printf("usrptr : %p\n", buf.m.userptr);
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

  //video_capture_thread_delete();
  printf("Still Confirmation\n");
  return exitcode;
}


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

  ret = is_initialize();
  //if (ret != 0)
  printf("%d\n", __LINE__);
  if (initialize == false )
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
  ret = camera_prepare_test(v_fd,
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
      write_file((uint8_t *)buf.m.userptr, (size_t)buf.bytesused, pixelformat);

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

  video_uninitialize();

errout_with_nx:

  //video_capture_thread_delete();
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

  ret = is_initialize();
  //if (ret != 0)
printf("%d\n", __LINE__);
  if (initialize == false )
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
  ret = camera_prepare_test_sub(v_fd,
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
printf("%d, %d, %d, %d, %d, %d, %d\n", capture_num, hsize, vsize, numerator, denominator, sub_vsize, sub_hsize);

printf("dq buff : %p\n", buf);

printf("usrptr : %p\n", buf.m.userptr);
printf("bytesused : %d\n", buf.bytesused);
sleep(10);

      write_file((uint8_t *)buf.m.userptr, (sub_vsize * sub_hsize * 2), V4L2_PIX_FMT_UYVY);
      //write_file((uint8_t *)buf.m.userptr, 1,  V4L2_PIX_FMT_UYVY);
      write_file((uint8_t *)buf.m.userptr + (sub_vsize * sub_hsize * 2), (size_t)buf.bytesused, V4L2_PIX_FMT_JPEG);
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

  //video_capture_thread_delete();
  printf("Still Confirmation\n");
  return exitcode;
}






int camera_prepare_test(int fd,
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
    printf("%d\n", __LINE__);
    ret = ioctl(fd, VIDIOC_STREAMON, (unsigned long)&type);
    if (ret < 0)
    {
      printf("Failed to VIDIOC_STREAMON: errno = %d\n", errno);
      return ret;
    }

    return OK;
  }
