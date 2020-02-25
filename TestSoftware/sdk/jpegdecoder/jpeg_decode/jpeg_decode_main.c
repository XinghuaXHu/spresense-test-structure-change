/****************************************************************************
 * examples/jpeg_decode/jpeg_decode_main.c
 *
 *   Copyright 2019 Sony Semiconductor Solutions Corporation
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

/*
 * This example is based on read_JPEG_file() function by IJG.
 *  base: example.c in http://www.ijg.org/files/jpegsrc.v9c.tar.gz
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sdk/config.h> /* For Spresense Kconfig */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


#include <stdbool.h>
#include <stdlib.h>

/*
 * Include file for users of JPEG library.
 * You will need to have included system headers that define at least
 * the typedefs FILE and size_t before you can include jpeglib.h.
 * (stdio.h is sufficient on ANSI-conforming systems.)
 * You may also wish to include "jerror.h".
 */

#include "jpeglib.h"

/*
 * In example.c by IJG, application use setjmp() and  longjmp().
 * Because NuttX OS do not support these functions, delete.
 */

/* #include <setjmp.h> */

/* For output to Spresense LCD.
 * imageproc has the color converter(YUV422 -> RGB565) function.
 */

#ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_LCD
#include <nuttx/board.h>
#include <nuttx/lcd/lcd.h>
#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxglib.h>

#  ifdef CONFIG_IMAGEPROC
#    include <imageproc/imageproc.h>
#  endif
#endif

#include "jpeg_decode.h"

#include "decode_thread.h"

#include <sdk/config.h>
#if defined(CONFIG_CODECS_HASH_MD5) && !defined(CONFIG_NSH_DISABLE_MD5)
#include "netutils/md5.h"
#define HAVE_CODECS_HASH_MD5 1
static MD5_CTX ctx_yuv;
static int md5bufferSize= 0;
static char md5buffer[5120*2];
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define APP_FILENAME_LEN  128

#define APP_BYTES_PER_PIXEL 2  /* YUV4:2:2 has 2 bytes/pixel */

#define APP_QVGA_WIDTH    320
#define APP_QVGA_HEIGHT   240

/* For output to Spresense LCD */

#ifndef CONFIG_EXAMPLES_JPEG_DECODE_LCD_DEVNO
#  define CONFIG_EXAMPLES_JPEG_DECODE_LCD_DEVNO 0
#endif

#define itou8(v) ((v) < 0 ? 0 : ((v) > 255 ? 255 : (v)))

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* For output to Spresense LCD */

struct uyvy_s
{
  uint8_t u0;
  uint8_t y0;
  uint8_t v0;
  uint8_t y1;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/


/****************************************************************************
 * Private Data
 ****************************************************************************/
/* In Spresense, the input file can be specified by file descriptor */

static int  infile;   /* file descriptor of input file */
static char infile_name[APP_FILENAME_LEN] = "/mnt/sd0/qvga.JPG";

#ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_FILE
static FILE *outfile;  /* file pointer of output file */
static char outfile_name[APP_FILENAME_LEN];
#endif  /* CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_FILE */

struct jpeg_decompress_struct cinfo;

struct jpeg_error_mgr jerr;

JSAMPARRAY buffer;            /* Output row buffer */
JDIMENSION output_position;   /* start position of output */
JDIMENSION output_width_by_one_decode;
JDIMENSION output_height_by_one_decode;
bool       mcu = false;       /* True means "decode by the MCU" */


#define DECODE_MQUEUE_PATH         ("jpeg_decode")
#define DECODE_MQUEUE_MODE               (0666)
#define DECODE_MAX_API_MQUEUE              (16)
#define DECODE_CMD_ARG                      (1)
#define DECODE_CMD_OPTION                   (2)
#define BUF_SIZE                     (512*1024)



struct decoder_command_s
{
  int   argc;
  FAR char *argv[40];
};

unsigned char my_buf[BUF_SIZE];
FILE  *fpi;
static bool g_taskrunning;

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_LCD
struct nximage_data_s g_jpeg_decode_nximage =
{
  NULL,          /* hnx */
  NULL,          /* hbkgd */
  0,             /* xres */
  0,             /* yres */
  false,         /* havpos */
  { 0 },         /* sem */
  0              /* exit code */
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/******************** JPEG DECOMPRESSION SAMPLE INTERFACE *******************/

/* This half of the example shows how to read data from the JPEG decompressor.
 * It's a bit more refined than the above, in that we show:
 *   (a) how to modify the JPEG library's standard error-reporting behavior;
 *   (b) how to allocate workspace using the library's memory manager.
 *
 * Just to make this example a little different from the first one, we'll
 * assume that we do not intend to put the whole image into an in-memory
 * buffer, but to send it line-by-line someplace else.  We need a one-
 * scanline-high JSAMPLE array as a work buffer, and we will let the JPEG
 * memory manager allocate it for us.  This approach is actually quite useful
 * because we don't need to remember to deallocate the buffer separately: it
 * will go away automatically when the JPEG object is cleaned up.
 */

#ifdef HAVE_CODECS_HASH_MD5
static void md5_callback(FAR char *src, int srclen, FAR char *dest,
                   FAR int *destlen, int mode)
{
  MD5Update((MD5_CTX *)dest, (unsigned char *)src, srclen);
}

static void tryMd5(char *buf, int size, bool final)
{
   memcpy(md5buffer+md5bufferSize,buf,size);
   md5bufferSize += size;
   if(final)
   {
       if(md5bufferSize>0)
       {
           FAR char srcbuf[128+2];
           memset(srcbuf, 0, 128+2);
           memcpy(srcbuf,md5buffer,md5bufferSize);
       
           int buflen = 33;
           md5_callback(srcbuf, md5bufferSize, (char *)&ctx_yuv, &buflen,0);
       }
       static const unsigned char hexchars[] = "0123456789abcdef";
       unsigned char mac[16];
       FAR char *src;
       FAR char *dest;
       
       FAR char destbuf[33];
       memset(destbuf, 0, 33);
       int i;
       MD5Final(mac, &ctx_yuv);
       src = (FAR char *)&mac;
       dest = destbuf;
       for (i = 0; i < 16; i++, src++)
         {
           *dest++ = hexchars[(*src) >> 4];
           *dest++ = hexchars[(*src) & 0x0f];
         }
       
       *dest = '\0';
       printf("\ncheckSum=%s\n", destbuf);
       return OK;

   }
   



   int useSize = 0;
   while(md5bufferSize>=128)
   {
       FAR char srcbuf[128+2];
       memset(srcbuf, 0, 128+2);
       memcpy(srcbuf,md5buffer + useSize,128);
       int buflen = 33;
       md5_callback(srcbuf, 128, (char *)&ctx_yuv, &buflen,0);
       useSize += 128;
       md5bufferSize -= 128;
   }

   if(md5bufferSize>0)
   {
       int temp = 0;
       while(temp<md5bufferSize)
       {
           md5buffer[temp] = md5buffer[useSize+temp];
           temp++;
       }
   }
   return OK;
    
}
#endif

#ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_LCD
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

  dev = board_lcd_getdev(CONFIG_EXAMPLES_JPEG_DECODE_LCD_DEVNO);
  if (!dev)
    {
      printf("nximage_initialize: board_lcd_getdev failed, devno=%d\n",
             CONFIG_EXAMPLES_JPEG_DECODE_LCD_DEVNO);
      return ERROR;
    }

  /* Turn the LCD on at 75% power */

  (void)dev->setpower(dev, ((3*CONFIG_LCD_MAXPOWER + 3)/4));

  /* Then open NX */

  printf("nximage_initialize: Open NX\n");
  g_jpeg_decode_nximage.hnx = nx_open(dev);
  if (!g_jpeg_decode_nximage.hnx)
    {
      printf("nximage_initialize: nx_open failed: %d\n", errno);
      return ERROR;
    }

  /* Set background color to black */

  color = 0;
  nx_setbgcolor(g_jpeg_decode_nximage.hnx, &color);
  ret = nx_requestbkgd(g_jpeg_decode_nximage.hnx,
                       &g_jpeg_decode_nximagecb, NULL);
  if (ret < 0)
    {
      printf("nximage_initialize: nx_requestbkgd failed: %d\n", errno);
      nx_close(g_jpeg_decode_nximage.hnx);
      return ERROR;
    }

  while (!g_jpeg_decode_nximage.havepos)
    {
      (void) sem_wait(&g_jpeg_decode_nximage.sem);
    }
  printf("nximage_initialize: Screen resolution (%d,%d)\n",
         g_jpeg_decode_nximage.xres, g_jpeg_decode_nximage.yres);

  return 0;
}


#  ifndef CONFIG_IMAGEPROC
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
#  endif /* !CONFIG_IMAGEPROC */

/* init_output_to_lcd(), output_to_lcd() and fin_output_to_lcd()
 * are specific for this example.
 * These are for displaying to LCD.
 */

static int init_output_to_lcd(void)
{
  int ret = OK;
  static bool is_lcd_initialized = false;

  if (!is_lcd_initialized)
    {
      ret = nximage_initialize();
      if (ret < 0)
        {
          printf("camera_main: Failed to get NX handle: %d\n", errno);
          return ERROR;
        }
      is_lcd_initialized = true;
    }
#  ifdef CONFIG_IMAGEPROC
  imageproc_initialize();
#  endif /* CONFIG_IMAGEPROC */
  return ret;
}

static void output_to_lcd(JSAMPARRAY buffer,
                          JDIMENSION position,
                          JDIMENSION width,
                          JDIMENSION height)
{
  int y_cnt;
  /* Convert YUV4:2:2 to RGB565 */

  for (y_cnt = 0; y_cnt < height; y_cnt++)
    {
#  ifdef CONFIG_IMAGEPROC
      imageproc_convert_yuv2rgb((void *)buffer[y_cnt],
                                width,
                                1);
#  else
      yuv2rgb(buffer[y_cnt], width * APP_BYTES_PER_PIXEL);
#  endif /* CONFIG_IMAGEPROC */
    }

  /* Display RGB565 */

  nximage_image(g_jpeg_decode_nximage.hbkgd,
                buffer, position, width, height);
  return;
}

static void fin_output_to_lcd(void)
{
#  ifdef CONFIG_IMAGEPROC
  imageproc_finalize();
#  endif
  return;
}
#endif /* CONFIG_SQA_JPEG_DECODE_OUTPUT_LCD */
#ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_FILE

/* init_output_to_file(), output_to_file() and fin_output_to_file()
 * are specific for this example.
 * These are for saving to file.
 */

static int init_output_to_file(char* output_prefix)
{
  int ret = OK;

  /* Delete old file name */

  memset(outfile_name, 0, sizeof(outfile_name));

  /* Output file name = Input file name without extension + .YUV" */

  strncpy(outfile_name,
          infile_name,
          strlen(infile_name) - 4 /* 3 is extension length */);
  snprintf(outfile_name + strlen(outfile_name), (APP_FILENAME_LEN-1), "_%s.YUV", output_prefix);
  //strncat(outfile_name, "YUV", 3);

  outfile = fopen(outfile_name, "wb");

  /* Initialize with the size of created YUV4:2:2 data */

  int scale_n = output_prefix[0] - '0';
  fseek(outfile,
        cinfo.image_width * cinfo.image_height * scale_n * scale_n * APP_BYTES_PER_PIXEL / (8*8),
        SEEK_SET);
  return ret;
}

static void output_to_file(JSAMPARRAY buffer,
                           JDIMENSION position,
                           JDIMENSION width,
                           JDIMENSION height)
{
  int y_cnt;

  fseek(outfile, position * APP_BYTES_PER_PIXEL, SEEK_SET);
  for (y_cnt = 0; y_cnt < height - 1; y_cnt++)
    {
      fwrite(buffer[y_cnt],
             APP_BYTES_PER_PIXEL,
             width,
             outfile);

      /* Go to next line */
      fseek(outfile,
            (cinfo.output_width - width) * APP_BYTES_PER_PIXEL,
            SEEK_CUR);
    }

  /* Write last line */

  fwrite(buffer[height - 1],
         APP_BYTES_PER_PIXEL,
         width,
         outfile);
  return;
}

static void fin_output_to_file(void)
{
  fflush(outfile);
  fclose(outfile);
  return;
}
#endif /* CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_FILE */

#ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_CHECKSUM
static int init_output_to_checkSum(char* output_prefix)
{
  int ret = OK;
  memset(&ctx_yuv,0,sizeof(ctx_yuv));
  MD5Init(&ctx_yuv);
  md5bufferSize = 0;
  return ret;
}

static void output_to_checkSum(JSAMPARRAY buffer,
                           JDIMENSION position,
                           JDIMENSION width,
                           JDIMENSION height)
{
#ifdef HAVE_CODECS_HASH_MD5
  int y_cnt;
  for (y_cnt = 0; y_cnt < height - 1; y_cnt++)
  {
      tryMd5(buffer[y_cnt],APP_BYTES_PER_PIXEL*width, false);
  }

  /* Write last line */
  tryMd5(buffer[height-1],APP_BYTES_PER_PIXEL*width,false);
#endif
  return;
}
  
static void fin_output_to_checkSum(void)
{
#ifdef HAVE_CODECS_HASH_MD5
  tryMd5(NULL,0,true);
#endif
  return;
}
#endif /* CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_CHECKSUM */

/****************************************************************************
 * Public Functions
 ****************************************************************************/
/*
 * Sample routine for JPEG decompression to YUV4:2:2.
 * Assume that the source file name is passed in.
 */









 static int main_task(int argc, FAR char *argv[])
 {
   int cmp_res;
   int ret;
   mqd_t                   mqd;
   struct mq_attr          attr;
   struct decoder_command_s *command;
   mqd                     = mq_open(DECODE_MQUEUE_PATH, O_RDWR
                           , DECODE_MQUEUE_MODE, &attr);

   g_taskrunning           = true;


   while(g_taskrunning)
   {
     ret = mq_receive(mqd, (FAR char *)(&command)
       , sizeof(struct decoder_command_s *), NULL);
     if (0 > ret)
     {
       printf("receive fail[%d]\n", ret);
       continue;
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
            , "hello", strlen("hello"));
     if (cmp_res == 0)
     {
         printf("Hello jpeg decode world !\n");
         printf("command argv1 = %s\n", command->argv[DECODE_CMD_ARG]);
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
            , "std_error", strlen("std_error"));
     if (cmp_res == 0)
     {
       cinfo.err = jpeg_std_error(&jerr);
       printf("jpeg_str_error()\n");
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
            , "create_decompress", strlen("create_decompress"));
     if (cmp_res == 0)
     {
       jpeg_create_decompress(&cinfo);
       printf("jpeg_create_decompress()\n");
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
           , "fopen", strlen("fopen"));
     if (cmp_res == 0)
     {
       if ((infile = open(infile_name, O_RDONLY)) < 0) {
         fprintf(stderr, "can't open %s\n", infile_name);
       }
       printf("infilename: %s\n", infile_name);
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
           , "w_fopen", strlen("w_fopen"));
     if (cmp_res == 0)
     {
       if ((infile = open(infile_name, O_WRONLY)) < 0) {
         fprintf(stderr, "can't open %s\n", infile_name);
       }
       printf("infilename: %s\n", infile_name);
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
            , "stdio_src", strlen("stdio_src"));
     if (cmp_res == 0)
     {
       printf("infilename: %s\n", infile_name);
       printf("jpeg_stdio_src()\n");
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
            , "fd_src", strlen("fd_src"));
     if (cmp_res == 0)
     {
       printf("infilename: %s\n", infile_name);
       jpeg_fd_src(&cinfo, infile);
       printf("jpeg_fd_src()\n");
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
            , "mem_src", strlen("mem_src"));
     if (cmp_res == 0)
     {
      if( (fpi = fopen(infile_name, "rb")) == NULL)
      {
        fprintf(stderr, "input file open error\n");
      }
      fread(&my_buf, sizeof(unsigned char), BUF_SIZE, fpi);
      jpeg_mem_src(&cinfo, my_buf, BUF_SIZE);
      printf("jpeg_mem_src()\n");
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
            , "read_header", strlen("read_header"));
     if (cmp_res == 0)
     {
       (void) jpeg_read_header(&cinfo, TRUE);
       printf("jpeg_read_header()\n");
       printf("image width  = %d\n", cinfo.image_width);
       printf("image height = %d\n", cinfo.image_height);
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
            , "start_decomp", strlen("start_decomp"));
     if (cmp_res == 0)
     {
       int scale_n = atoi(command->argv[DECODE_CMD_OPTION]);
       cinfo.scale_num = cinfo.image_width * scale_n / 8;
       cinfo.scale_denom = cinfo.image_width;

       (void) jpeg_start_decompress(&cinfo);
       if (mcu)
         {
           /* Output size of 1 decode is the size of 1 MCU */

           output_width_by_one_decode  = (cinfo.output_width  / cinfo.MCUs_per_row);
           output_height_by_one_decode = (cinfo.output_height / cinfo.MCU_rows_in_scan);
         }
       else
         {
           /* Output size of 1 decode is the size of 1 line */

           output_width_by_one_decode  = cinfo.output_width;
           output_height_by_one_decode = 1;
         }
		printf("jpeg_start_decompress\n");
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
            , "output", strlen("output"));
     if (cmp_res == 0)
     {
       #ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_LCD
         init_output_to_lcd();
       #endif
         char* output_prefix = command->argv[DECODE_CMD_OPTION];
       #ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_FILE
         init_output_to_file(output_prefix);
       #endif
       #ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_CHECKSUM
         init_output_to_checkSum(output_prefix);
       #endif
         if (mcu)
           {
             while (cinfo.output_offset < (cinfo.output_width * cinfo.output_height))
               {
                 jpeg_read_mcus(&cinfo,
                                buffer,
                                output_height_by_one_decode,
                                &output_position);
       #ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_LCD
                 output_to_lcd(buffer, output_position,
                               output_width_by_one_decode, output_height_by_one_decode);
       #endif
       #ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_FILE
                 output_to_file(buffer, output_position,
                                output_width_by_one_decode, output_height_by_one_decode);
       #endif
       #ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_CHECKSUM
                 output_to_checkSum(buffer, output_position,
                                output_width_by_one_decode, output_height_by_one_decode);
       #endif
               }
           }
         else
         {
             while (cinfo.output_scanline < cinfo.output_height)
               {
                 jpeg_read_scanlines(&cinfo, buffer, 1);
       #ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_LCD
                 output_to_lcd(buffer, (cinfo.output_scanline - 1) * cinfo.output_width,
                               output_width_by_one_decode, output_height_by_one_decode);
       #endif
       #ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_FILE
                 output_to_file(buffer, (cinfo.output_scanline - 1) * cinfo.output_width,
                                output_width_by_one_decode, output_height_by_one_decode);
       #endif
       #ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_CHECKSUM
                 output_to_checkSum(buffer, (cinfo.output_scanline - 1) * cinfo.output_width,
                                output_width_by_one_decode, output_height_by_one_decode);
       #endif
               }
           }
       #ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_LCD
         fin_output_to_lcd();
       #endif
       #ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_FILE
         fin_output_to_file();
       #endif
       #ifdef CONFIG_SQA_JPEG_DECODE_OUTPUT_YUV_CHECKSUM
         fin_output_to_checkSum();
       #endif
         printf("output done\n");
     }



     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
            , "set_buf_mem", strlen("set_buf_mem"));
     if (cmp_res == 0)
     {
       buffer = (*cinfo.mem->alloc_sarray)
                     ((j_common_ptr) &cinfo,
                      JPOOL_IMAGE,
                      output_width_by_one_decode * APP_BYTES_PER_PIXEL,
                      output_height_by_one_decode);
       printf("setting buf_mem done\n");
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
        , "finish_decompress", strlen("finish_decompress"));
     if (cmp_res == 0)
     {
       (void) jpeg_finish_decompress(&cinfo);
       printf("jpeg_finish_decompress()\n");
       printf("jpeg_finish_decompress()\n");
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
        , "destroy_decompress", strlen("destroy_decompress"));
     if (cmp_res == 0)
     {
       jpeg_destroy_decompress(&cinfo);
       printf("jpeg_destroy_decompress()\n");
     }

     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
        , "fd_close", strlen("fd_close"));
     if (cmp_res == 0)
     {
       close(infile);
       printf("closed fd\n");
     }
     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
        , "mem_close", strlen("mem_close"));
     if (cmp_res == 0)
     {
       fclose(fpi);
       printf("closed fp\n");
     }


     cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
       , "select_decode_param", strlen("select_decode_param"));
      if (cmp_res == 0)
      {
        cmp_res = strncmp(command->argv[DECODE_CMD_OPTION]
          , "mcu", strlen("mcu"));
         if (cmp_res == 0)
         {
                 mcu = true;
                 printf("mcu decode\n");
         }
         cmp_res = strncmp(command->argv[DECODE_CMD_OPTION]
           , "line", strlen("line"));
          if (cmp_res == 0)
          {
            mcu = false;
            printf("line decode\n");
          }
       }

      cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
       , "filename", strlen("filename"));
      if (cmp_res == 0)
      {
        strncpy(infile_name, command->argv[DECODE_CMD_OPTION], APP_FILENAME_LEN);
        printf("infilename: %s\n", infile_name);
       }

      cmp_res = strncmp(command->argv[DECODE_CMD_ARG]
        , "color_space", strlen("color_space"));
      if (cmp_res == 0)
      {
        if ( strncmp(command->argv[DECODE_CMD_OPTION], "ycbcr", 20)==0 )
        {
          printf("set color_space : JCS_YCbCr\n");
          cinfo.out_color_space = JCS_YCbCr;
        }
        else if ( strncmp(command->argv[DECODE_CMD_OPTION], "cbycry", 20)==0 )
        {
          printf("set color_space : JCS_CbYCrY\n");
          cinfo.out_color_space = JCS_CbYCrY;
        }
        else if ( strncmp(command->argv[DECODE_CMD_OPTION], "grayscale", 20)==0 )
        {
          printf("set color_space : JCS_GRAYSCALE\n");
          cinfo.out_color_space = JCS_GRAYSCALE;
        }
        else if ( strncmp(command->argv[DECODE_CMD_OPTION], "rgb", 20)==0 )
        {
          printf("set color_space : JCS_RGB\n");
          cinfo.out_color_space = JCS_RGB;
        }
        else if ( strncmp(command->argv[DECODE_CMD_OPTION], "bg_ycc", 20)==0 )
        {
          printf("set color_space : JCS_BG_YCC\n");
          cinfo.out_color_space = JCS_BG_YCC;
        }
        else if ( strncmp(command->argv[DECODE_CMD_OPTION], "bg_rgb", 20)==0 )
        {
          printf("set color_space : JCS_BG_RGB\n");
          cinfo.out_color_space = JCS_BG_RGB;
        }
        else if ( strncmp(command->argv[DECODE_CMD_OPTION], "cmyk", 20)==0 )
        {
          printf("set color_space : JCS_CMYK\n");
          cinfo.out_color_space = JCS_CMYK;
        }
        else if ( strncmp(command->argv[DECODE_CMD_OPTION], "ycck", 20)==0 )
        {
          printf("set color_space : JCS_YCCK\n");
          cinfo.out_color_space = JCS_YCCK;
        }
        else
        {
          printf("set color_space as default: JCS_CbYCrY\n");
          cinfo.out_color_space = JCS_CbYCrY;
        }
      }

     }
   return 0;
 }






#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int jpeg_decode_main(int argc, char *argv[])
#endif
{
  static FAR struct decoder_command_s *command = NULL;

  struct mq_attr          attr;
  mqd_t                   mqd;
  int                     itr = 0;

  if(argc>=2 && strncmp(argv[1], "thread_create", 20)==0)
  {
    int rc;
    pthread_attr_t attr_A, attr_B, attr_C, attr_D, attr_E;
    size_t s1;
    pthread_t thread_A, thread_B,  thread_C, thread_D, thread_E;

    rc = pthread_attr_init(&attr_A);
    if (rc == -1) {
       perror("error in pthread_attr_init");
       exit(1);
    }
    rc = pthread_attr_init(&attr_B);
    if (rc == -1) {
       perror("error in pthread_attr_init");
       exit(1);
    }
    rc = pthread_attr_init(&attr_C);
    if (rc == -1) {
       perror("error in pthread_attr_init");
       exit(1);
    }
    rc = pthread_attr_init(&attr_D);
    if (rc == -1) {
       perror("error in pthread_attr_init");
       exit(1);
    }
    rc = pthread_attr_init(&attr_E);
    if (rc == -1) {
       perror("error in pthread_attr_init");
       exit(1);
    }


    s1 = 4096;
    rc = pthread_attr_setstacksize(&attr_A, s1);
    if (rc == -1) {
       perror("error in pthread_attr_setstacksize");
       exit(2);
    }
    rc = pthread_attr_setstacksize(&attr_B, s1);
    if (rc == -1) {
       perror("error in pthread_attr_setstacksize");
       exit(2);
    }
    rc = pthread_attr_setstacksize(&attr_C, s1);
    if (rc == -1) {
       perror("error in pthread_attr_setstacksize");
       exit(2);
    }
    rc = pthread_attr_setstacksize(&attr_D, s1);
    if (rc == -1) {
       perror("error in pthread_attr_setstacksize");
       exit(2);
    }
    rc = pthread_attr_setstacksize(&attr_E, s1);
    if (rc == -1) {
       perror("error in pthread_attr_setstacksize");
       exit(2);
    }

    pthread_create(&thread_A, &attr_A, decode_thread_00, NULL);
    pthread_create(&thread_B, &attr_B, decode_thread_01, NULL);
    pthread_create(&thread_C, &attr_C, decode_thread_02, NULL);
    pthread_create(&thread_D, &attr_D, decode_thread_03, NULL);
    pthread_create(&thread_E, &attr_E, decode_thread_04, NULL);

    pthread_join(thread_A, NULL);
    pthread_join(thread_B, NULL);
    pthread_join(thread_C, NULL);
    pthread_join(thread_D, NULL);
    pthread_join(thread_E, NULL);
    return 0;
  }




  if ( 2 <= argc)
  {
    if (command == NULL)
    {
      command = malloc(sizeof(struct decoder_command_s));
      memset(command, 0, sizeof(struct decoder_command_s));
    }
    command->argc = argc;

    for (itr = 0; itr < argc; itr++)
    {
      if (command->argv[itr] == NULL)
        command->argv[itr] = malloc(strlen(argv[itr]) + 1);
      memset(command->argv[itr], '\0', (strlen(argv[itr]) + 1));
      strncpy((command->argv[itr]), argv[itr],  strlen(argv[itr]));
    }

    if (!g_taskrunning)
    {
      task_create("hoge_task", 100, 2048, main_task, NULL);
    }

    attr.mq_maxmsg  = DECODE_MAX_API_MQUEUE;
    attr.mq_msgsize = (sizeof(struct decoder_command_s *));
    mqd = mq_open(DECODE_MQUEUE_PATH, O_RDWR | O_CREAT
        , DECODE_MQUEUE_MODE, &attr);

    if (mq_send(mqd, (FAR const char *)(&command)
        , sizeof(struct decoder_command_s *), 0) != 0)
    {
      printf("mq_send to task_main Failed!!\n");
      for (itr = 0; itr < (command->argc); itr++)
      {
        free(command->argv[itr]);
        command->argv[itr] = NULL;
      }
      free(command);
      command = NULL;
    }

    usleep(500*1000);
  }
  return 0;

}
