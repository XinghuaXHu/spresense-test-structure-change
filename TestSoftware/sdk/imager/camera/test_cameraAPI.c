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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>


int test_vidioc_enum_fmt(int fd, uint32_t index, uint32_t type)
{
  struct v4l2_fmtdesc data;
  int ret;
  data.index = index;
  data.type = type;

  ret = ioctl(fd, VIDIOC_ENUM_FMT, (unsigned long)&data);

  if (ret == 0) printf("description:%s, pixelformat:%d, subimg_pixelformat:%d \n",
                        data.description, data.pixelformat, data.subimg_pixelformat);

  return ret;
}


int test_vidioc_enum_framesizes(int fd,
                                uint32_t index,
                                uint32_t buf_type,
                                uint32_t pixel_format,
                                uint32_t subimg_type)
{
  struct v4l2_frmsizeenum data;
  int ret;
  data.index = index;
  data.buf_type = buf_type;
  data.pixel_format = pixel_format;
  data.subimg_type = subimg_type;

  ret = ioctl(fd, VIDIOC_ENUM_FRAMESIZES, (unsigned long)&data);

  if (ret == 0)
  {
    printf("type:%d, min_width:%d, max_width:%d, step_width:%d ",
            data.type, data.stepwise.min_width, data.stepwise.max_width, data.stepwise.step_width);
    printf("min_height:%d, max_height:%d, step_height:%d \n",
            data.stepwise.min_height, data.stepwise.max_height, data.stepwise.step_height);
  }

  return ret;
}


int test_vidioc_enum_frameintervals(int fd,
                                   uint32_t index,
                                   uint32_t buf_type,
                                   uint32_t pixel_format,
                                   uint32_t width,
                                   uint32_t height,
                                   uint32_t subimg_width,
                                   uint32_t subimg_height)
{
  struct v4l2_frmivalenum data;
  int ret;
  data.index = index;
  data.buf_type = buf_type;
  data.pixel_format = pixel_format;
  data.width = width;
  data.height = height;
  data.subimg_width = subimg_width;
  data.subimg_height = subimg_height;

  ret = ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, (unsigned long)&data);

  if (ret == 0)
  {
    printf("type:%d, numerator:%d, denominator:%d\n",
            data.type, data.discrete.numerator, data.discrete.denominator);
  }

  return ret;
}


int test_vidoc_try_fmt(int fd,
                       uint32_t type,
                       uint32_t pix_width,
                       uint32_t pix_height,
                       uint32_t pix_pixelformat,
                       uint32_t pix_subimg_width,
                       uint32_t pix_subimg_height,
                       uint32_t pix_subimg_pixelformat)
{
  struct v4l2_format data;
  int ret;
  data.type = type;
  data.fmt.pix.width = pix_width;
  data.fmt.pix.height = pix_height;
  data.fmt.pix.pixelformat = pix_pixelformat;
  data.fmt.pix.subimg_width = pix_subimg_width;
  data.fmt.pix.subimg_height = pix_subimg_height;
  data.fmt.pix.subimg_pixelformat = pix_subimg_pixelformat;

  ret = ioctl(fd, VIDIOC_TRY_FMT, (unsigned long)&data);

  return ret;
}


int test_vidioc_s_fmt(int fd,
                      uint32_t type,
                      uint32_t pix_width,
                      uint32_t pix_height,
                      uint32_t pix_pixelformat,
                      uint32_t pix_subimg_width,
                      uint32_t pix_subimg_height,
                      uint32_t pix_subimg_pixelformat)
{
  struct v4l2_format data;
  int ret;
  data.type = type;
  data.fmt.pix.width = pix_width;
  data.fmt.pix.height = pix_height;
  data.fmt.pix.pixelformat = pix_pixelformat;
  data.fmt.pix.subimg_width = pix_subimg_width;
  data.fmt.pix.subimg_height = pix_subimg_height;
  data.fmt.pix.subimg_pixelformat = pix_subimg_pixelformat;

  data.fmt.pix.field = V4L2_FIELD_ANY;
  //固定

  ret = ioctl(fd, VIDIOC_S_FMT, (unsigned long)&data);

  if (ret == 0){printf("test_vidioc_s_fmt\n");}else{printf("ret = %d\n",ret);}

  return ret;
}

int test_vidioc_s_parm(int fd,
                       uint32_t type,
                       uint32_t numerator,
                       uint32_t denominator)
{
  struct v4l2_streamparm data;
  int ret;
  data.type = type;
  data.parm.capture.timeperframe.numerator = numerator;
  data.parm.capture.timeperframe.denominator = denominator;

  ret = ioctl(fd, VIDIOC_S_PARM, (unsigned long)&data);
  if (ret == 0) printf("test_vidioc_s_param\n");

  return ret;
}

int test_vidioc_reqbufs(int fd,
                        uint32_t count,
                        uint32_t type,
                        uint32_t memory,
                        uint32_t mode)
{
    struct v4l2_requestbuffers data;
    int ret;
    data.count = count;
    data.type = type;
    data.memory = memory;
    data.mode = mode;

    ret = ioctl(fd, VIDIOC_REQBUFS, (unsigned long)&data);
    if (ret == 0) printf("test_vidioc_reqbufs\n");

    return ret;
}


int test_vidioc_qbuf(int fd,
                     uint32_t index,
                     uint32_t type,
                     unsigned long userptr,
                     unsigned long length)
{
  struct v4l2_buffer data;
  int ret;
  data.index = index;
  data.type = type;
  data.m.userptr = userptr;
  data.length = length;


  ret = ioctl(fd, VIDIOC_QBUF, (unsigned long)&data);
  if (ret == 0) printf("test_vidioc_qbuf\n");

  return ret;
}

int test_vidioc_dqbuf(int fd, uint32_t type)
{
  struct v4l2_buffer data;
  int ret;
  data.type = type;

  ret = ioctl(fd, VIDIOC_DQBUF, (unsigned long)&data);

  if (ret == 0)
  {
    printf("index:%d, bytesused:%d, userptr:%d\n",
            data.index, data.bytesused, data.m.userptr);
  }

  return ret;
}

int test_vidioc_queryctrl(int fd, uint16_t ctrl_class, uint16_t id)
{
  struct v4l2_queryctrl data;
  int ret;

  data.ctrl_class = ctrl_class;
  data.id = id;

  ret = ioctl(fd, VIDIOC_QUERYCTRL, (unsigned long)&data);

  if(ret == 0)
  {
    printf("type:%2d, name:%-26s, minimum:%4d, maximum:%4d, step:%2d, defalut_value:%4d, flags:%4d\n",
            data.type, data.name, data.minimum, data.maximum, data.step, data.default_value, data.flags);
  }

  return ret;
}

int test_vidioc_ext_queryctrl(int fd, uint16_t ctrl_class, uint16_t id)
{
  struct v4l2_query_ext_ctrl data;
  int ret;

  data.ctrl_class = ctrl_class;
  data.id = id;

  ret = ioctl(fd, VIDIOC_QUERY_EXT_CTRL, (unsigned long)&data);

  if(ret == 0)
  {
     printf("type:%2d,"
            " name:%-26s,"
            " minimum:%4PRId,"
            " maximum:%4PRId,"
            " step:%2PRId,"
            " defalut_value:%4PRId,"
            " flags:%4d,"
            " elem_size:%d,"
            " elems:%d,"
            " nr_of_dims:%d\n",
//            t_dims[V4L2_CTRL_MAX_DIMS]:%d\n",
            data.type,
            data.name,
            data.minimum,
            data.maximum,
            data.step,
            data.default_value,
            data.flags,
            data.elem_size,
            data.elems,
            data.nr_of_dims
        //    data.t_dims[V4L2_CTRL_MAX_DIMS];
          );
  }

  return ret;
}


int test_vidioc_querymenu(int fd, uint16_t ctrl_class, uint16_t id, uint32_t index)
{
  struct v4l2_querymenu data;
  int ret;

  data.ctrl_class = ctrl_class;
  data.id = id;
  data.index = index;

  ret = ioctl(fd, VIDIOC_QUERYMENU, (unsigned long)&data);

  if(ret == 0)
  {
    printf("name:%s, value:%d\n",
            data.name,  data.value);
            /*union in struct v4l2_querymenu*/
  }
  return ret;
}


int test_vidioc_s_ctrl(int fd, uint16_t id, int32_t value)
{
  struct v4l2_control data;
  int ret;
  data.id = id;
  data.value = value;

  printf("ioctl calling \n");
  ret = ioctl(fd, VIDIOC_S_CTRL, (unsigned long)&data);

  //wait for 60 fps * 2
  usleep(1000*1000*2/60);

  if (ret != 0) printf("Failed to ioctl s_ctrl.errno:%d\n", errno);
  return ret;
}




int test_vidioc_g_ctrl_unmask(int fd, int id)
{
  struct v4l2_control data;
  int ret;
  int i;

  if (id == V4L2_CID_GAMMA_CURVE) return -1;

  data.id = id;
  ret = ioctl(fd, VIDIOC_G_CTRL, (unsigned long)&data);


  if (ret == 0)
  {
    switch(id)
    {
      // unsigned 2byte
      case V4L2_CID_RED_BALANCE:
      case V4L2_CID_BLUE_BALANCE:
        printf("%u\n", (data.value));
        break;
      // signed 1byte
      case V4L2_CID_BRIGHTNESS:
      case V4L2_CID_EXPOSURE:
        printf("%d\n", (data.value));
        break;
      // unsigned 1byte
      default:
        printf("%u\n", (data.value));
        break;
    }
  }
  else
  {
    printf("Failed! (ret = %d)\n", ret);
  }
  if (ret == 0) printf("returned check value \n");

  return ret;
}



int test_vidioc_g_ctrl(int fd, int id)
{
  struct v4l2_control data;
  int ret;
  int i;
  data.id = id;
  ret = ioctl(fd, VIDIOC_G_CTRL, (unsigned long)&data);


  if (ret == 0)
  {
    //switch(id)
    {
      printf("%d\n", data.value );
    }
  }
  else
  {
    printf("Failed! (ret = %d)\n", ret);
  }
  if (ret == 0) printf("returned check value \n");

  return ret;
}

int test_vidioc_s_ext_ctrl(int fd,
                           uint16_t class,
//                           uint16_t count,
                           uint16_t id,
                           //uint16_t size,
                           //int index,
                           int32_t value)
{

  struct v4l2_ext_controls data;
  struct v4l2_ext_control local_data;
  int ret;
  int i;

  local_data.id = id;
  data.controls = &local_data;

  data.ctrl_class = class;
  data.count = 1;

  //local_data.size = size;
  local_data.value = value;

  uint16_t fuga[][19] = {
    /* GAMMA0 ISX012 ROM Default */
    { 0, 7, 29, 47, 61, 74, 81, 90, 97, 106,
       73, 130, 173, 204, 225, 237, 246, 262, 268},
    /* Basic for IoTCamera */
    { 0, 7, 30, 48, 62, 77, 88, 100, 108, 118,
     82, 144, 188, 214, 230, 241, 250, 259, 258},
    /* Bright for IoTCamera */
    { 0, 7, 50, 84, 104, 122, 135, 146, 154, 161,
     133, 183, 206, 221, 232, 241, 250, 259, 258},
    /* High-Bright for IoTCamera */
    { 0, 111, 128, 138, 146, 153, 159, 164, 168, 172,
      157, 183, 206, 221, 232, 241, 250, 259, 258},
     /*user defined*/
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25}
  };

  if(id == V4L2_CID_GAMMA_CURVE)
  {
    //struct v4l2_cid_gamma_curve data;
    //data.num = V4L2_CID_GAMMA_CURVE_NUM_ISX012;
    //data.value = &fuga[0];

    if(value >= 0 && value <= 5)
    {
      data.controls->p_u16 = fuga[value];
    }
    else
    {
      data.controls->p_u16 = fuga[3];

    }
    //test_cameraAPI.c:420:27: warning: assignment from incompatible pointer type
    ret = ioctl(fd, VIDIOC_S_EXT_CTRLS, (unsigned long)&data);
    printf("test_vidioc_s_ext_ctrl call\n");
    printf("ret = %d\n", ret);
    for(i = 0; i <19; i++)
    {
      printf("%d\n", (data.controls->p_u16[i]) );
    }
    if (ret != 0) printf("Failed to ioctl s_ext_ctrl.errno:%d\n", errno);
    if (ret != 0) printf("error.idx:%d\n", data.error_idx);
    return ret ;
  }
  ret = ioctl(fd, VIDIOC_S_EXT_CTRLS, (unsigned long)&data);

  //wait for 60 fps * 2
  usleep(1000*1000*2/60);

  printf("test_vidioc_s_ext_ctrl call\n");
  printf("ret = %d\n", ret);
  if (ret != 0) printf("error.idx:%d\n", data.error_idx);
  if (ret != 0) printf("Failed to ioctl s_ext_ctrl.errno:%d\n", errno);
  return ret;
}

int test_vidioc_g_ext_ctrl(int fd, int id)
{
  struct v4l2_ext_controls data;
  struct v4l2_ext_control local_data;
  int i;


  local_data.id = id;
  data.controls = &local_data;

  data.ctrl_class = V4L2_CTRL_CLASS_USER;
  data.count = 1;


  int ret;
  ret = ioctl(fd, VIDIOC_G_EXT_CTRLS, (unsigned long)&data);

/*todo: write special processing for gamma*/
  if(ret == 0)
  {
    switch(id)
    {
      case V4L2_CID_GAMMA_CURVE:
        printf("data.controls->id = %d\n", data.controls->id);
        printf("----------\n");
        for(i = 0; i <19; i++)
        {
          printf("%d\n", data.controls->p_u16[i]);
        }

        break;
      default:
        printf("gamma only\n");
        break;

    }
  }
  else
  {
    printf("Failed! (ret = %d)\n", ret);
  }
  if (ret == 0) printf("returned check value \n");
  return ret;
}


int test_do_halfpush(int fd)
{
  int ret;
  ret = ioctl(fd, VIDIOC_DO_HALFPUSH, true);
  if (ret == 0) printf("do half push\n");
  return ret;
}
int test_do_not_halfpush(int fd)
{
  int ret;
  ret = ioctl(fd, VIDIOC_DO_HALFPUSH, false);
  if (ret == 0) printf("release half push\n");
  return ret;
}


int test_streamon(int fd, const int type)
{
  int ret;
  ret = ioctl(fd, VIDIOC_STREAMON, (unsigned long)&type);
  /*type - Video or Still*/
  if (ret == 0) printf("stream on\n");
  return ret;
}

int test_streamoff(int fd, const int type)
{
  int ret;
  ret = ioctl(fd, VIDIOC_STREAMOFF, (unsigned long)&type);
  if(ret == 0)printf("stream off\n");
  return ret;
}

int test_takepict_start(int fd, uint32_t interval)
{
  int ret;
  ret = ioctl(fd, VIDIOC_TAKEPICT_START, interval);
  if (ret == 0) printf("ioctl called \n");
  return ret;
}

int test_takepict_stop(int fd, bool halfpush)
{
  int ret;
  ret = ioctl(fd, VIDIOC_TAKEPICT_STOP, halfpush);
  if (ret == 0) printf("ioctl called \n");
  return ret;
}
