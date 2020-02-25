#include <sdk/config.h> /* For Spresense Kconfig */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


#include <stdbool.h>
#include <stdlib.h>

#include "jpeglib.h"

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


#ifndef CONFIG_EXAMPLES_JPEG_DECODE_LCD_DEVNO
#  define CONFIG_EXAMPLES_JPEG_DECODE_LCD_DEVNO 0
#endif

#define itou8(v) ((v) < 0 ? 0 : ((v) > 255 ? 255 : (v)))

#define APP_FILENAME_LEN  128

#define APP_BYTES_PER_PIXEL 2  /* YUV4:2:2 has 2 bytes/pixel */

#define APP_QVGA_WIDTH    320
#define APP_QVGA_HEIGHT   240

static FILE *init_output_to_file(char *infile_name)
{
  int ret = OK;
  FILE *outfile;  /* file pointer of output file */
  char outfile_name[APP_FILENAME_LEN];

  /* Delete old file name */

  memset(outfile_name, 0, sizeof(outfile_name));

  /* Output file name = Input file name without extension + .YUV" */

  strncpy(outfile_name,
          infile_name,
          strlen(infile_name) - 3 /* 3 is extension length */);
  strncat(outfile_name, "YUV", 3);

  outfile = fopen(outfile_name, "wb");

  /* Initialize with the size of created YUV4:2:2 data */

  fseek(outfile,
        APP_QVGA_WIDTH * APP_QVGA_HEIGHT * APP_BYTES_PER_PIXEL,
        SEEK_SET);
  return outfile;
}

static void output_to_file(FILE *outfile,
                           JSAMPARRAY buffer,
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
            (APP_QVGA_WIDTH - width) * APP_BYTES_PER_PIXEL,
            SEEK_CUR);
    }

  /* Write last line */

  fwrite(buffer[height - 1],
         APP_BYTES_PER_PIXEL,
         width,
         outfile);
  return;
}

static void fin_output_to_file(FILE *outfile)
{
  fclose(outfile);
  return;
}

void decode_thread_00(void)
{
  int  infile;   /* file descriptor of input file */
  char infile_name[APP_FILENAME_LEN] = "/mnt/sd0/qvga.JPG";


  FILE *outfile;  /* file pointer of output file */

  struct jpeg_decompress_struct cinfo;

  struct jpeg_error_mgr jerr;

  JSAMPARRAY buffer;            /* Output row buffer */
  JDIMENSION output_position;   /* start position of output */
  JDIMENSION output_width_by_one_decode;
  JDIMENSION output_height_by_one_decode;
  bool       mcu = false;       /* True means "decode by the MCU" */

   if ((infile = open(infile_name, O_RDONLY)) < 0) {
     fprintf(stderr, "can't open %s\n", infile_name);
     return;
   }
   cinfo.err = jpeg_std_error(&jerr);
   jpeg_create_decompress(&cinfo);
   jpeg_fd_src(&cinfo, infile);    /* This is file descriptor API */
   (void) jpeg_read_header(&cinfo, TRUE);
   fprintf(stdout, "image width  = %d\n", cinfo.image_width);
   fprintf(stdout, "image height = %d\n", cinfo.image_height);
   cinfo.out_color_space = JCS_CbYCrY;
   cinfo.scale_num = APP_QVGA_WIDTH;
   cinfo.scale_denom = cinfo.image_width;
   (void) jpeg_start_decompress(&cinfo);
   if (mcu)
   {
     output_width_by_one_decode  = (cinfo.output_width  / cinfo.MCUs_per_row);
     output_height_by_one_decode = (cinfo.output_height / cinfo.MCU_rows_in_scan);
   }
   else
   {
     output_width_by_one_decode  = cinfo.output_width;
     output_height_by_one_decode = 1;
   }
   buffer = (*cinfo.mem->alloc_sarray)
   ((j_common_ptr) &cinfo,
   JPOOL_IMAGE,
   output_width_by_one_decode * APP_BYTES_PER_PIXEL,
   output_height_by_one_decode);
   outfile = init_output_to_file(infile_name);
   if (mcu)
   {
     while (cinfo.output_offset < (cinfo.output_width * cinfo.output_height))
     {
       jpeg_read_mcus(&cinfo,
         buffer,
         output_height_by_one_decode,
         &output_position);
           output_to_file(outfile, buffer, output_position,
             output_width_by_one_decode, output_height_by_one_decode);
     }
   }
   else
     {
        while (cinfo.output_scanline < cinfo.output_height)
        {
             jpeg_read_scanlines(&cinfo, buffer, 1);
             output_to_file(outfile, buffer, (cinfo.output_scanline - 1) * cinfo.output_width,
             output_width_by_one_decode, output_height_by_one_decode);
         }
     }
   fin_output_to_file(outfile);
   (void) jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);

   close(infile);
   printf("thread 00 END\n");
  pthread_exit(0);
}


void decode_thread_01(void)
{

  int  infile;   /* file descriptor of input file */
  char infile_name[APP_FILENAME_LEN] = "/mnt/sd0/vga.JPG";

  FILE *outfile;  /* file pointer of output file */

  struct jpeg_decompress_struct cinfo;

  struct jpeg_error_mgr jerr;

  JSAMPARRAY buffer;            /* Output row buffer */
  JDIMENSION output_position;   /* start position of output */
  JDIMENSION output_width_by_one_decode;
  JDIMENSION output_height_by_one_decode;
  bool       mcu = false;       /* True means "decode by the MCU" */

  if ((infile = open(infile_name, O_RDONLY)) < 0) {
    fprintf(stderr, "can't open %s\n", infile_name);
    return;
  }
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_fd_src(&cinfo, infile);    /* This is file descriptor API */
  (void) jpeg_read_header(&cinfo, TRUE);
  fprintf(stdout, "image width  = %d\n", cinfo.image_width);
  fprintf(stdout, "image height = %d\n", cinfo.image_height);
  cinfo.out_color_space = JCS_CbYCrY;
  cinfo.scale_num = APP_QVGA_WIDTH;
  cinfo.scale_denom = cinfo.image_width;
  (void) jpeg_start_decompress(&cinfo);
  if (mcu)
  {
    output_width_by_one_decode  = (cinfo.output_width  / cinfo.MCUs_per_row);
    output_height_by_one_decode = (cinfo.output_height / cinfo.MCU_rows_in_scan);
  }
  else
  {
    output_width_by_one_decode  = cinfo.output_width;
    output_height_by_one_decode = 1;
  }
  buffer = (*cinfo.mem->alloc_sarray)
  ((j_common_ptr) &cinfo,
  JPOOL_IMAGE,
  output_width_by_one_decode * APP_BYTES_PER_PIXEL,
  output_height_by_one_decode);
  outfile = init_output_to_file(infile_name);
  if (mcu)
  {
    while (cinfo.output_offset < (cinfo.output_width * cinfo.output_height))
    {
      jpeg_read_mcus(&cinfo,
        buffer,
        output_height_by_one_decode,
        &output_position);
          output_to_file(outfile, buffer, output_position,
            output_width_by_one_decode, output_height_by_one_decode);
    }
  }
  else
    {
      while (cinfo.output_scanline < cinfo.output_height)
      {
            jpeg_read_scanlines(&cinfo, buffer, 1);
            output_to_file(outfile,buffer, (cinfo.output_scanline - 1) * cinfo.output_width,
            output_width_by_one_decode, output_height_by_one_decode);
      }
    }
        fin_output_to_file(outfile);
        (void) jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        close(infile);
   printf("thread 01 END\n");
  pthread_exit(0);
}

void decode_thread_02(void)
{

  int  infile;   /* file descriptor of input file */
  char infile_name[APP_FILENAME_LEN] = "/mnt/sd0/orig1.JPG";

  FILE *outfile;  /* file pointer of output file */

  struct jpeg_decompress_struct cinfo;

  struct jpeg_error_mgr jerr;

  JSAMPARRAY buffer;            /* Output row buffer */
  JDIMENSION output_position;   /* start position of output */
  JDIMENSION output_width_by_one_decode;
  JDIMENSION output_height_by_one_decode;
  bool       mcu = false;       /* True means "decode by the MCU" */

  if ((infile = open(infile_name, O_RDONLY)) < 0) {
    fprintf(stderr, "can't open %s\n", infile_name);
    return;
  }
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_fd_src(&cinfo, infile);    /* This is file descriptor API */
  (void) jpeg_read_header(&cinfo, TRUE);
  fprintf(stdout, "image width  = %d\n", cinfo.image_width);
  fprintf(stdout, "image height = %d\n", cinfo.image_height);
  cinfo.out_color_space = JCS_CbYCrY;
  cinfo.scale_num = APP_QVGA_WIDTH;
  cinfo.scale_denom = cinfo.image_width;
  (void) jpeg_start_decompress(&cinfo);
  if (mcu)
  {
    output_width_by_one_decode  = (cinfo.output_width  / cinfo.MCUs_per_row);
    output_height_by_one_decode = (cinfo.output_height / cinfo.MCU_rows_in_scan);
  }
  else
  {
    output_width_by_one_decode  = cinfo.output_width;
    output_height_by_one_decode = 1;
  }
  buffer = (*cinfo.mem->alloc_sarray)
  ((j_common_ptr) &cinfo,
  JPOOL_IMAGE,
  output_width_by_one_decode * APP_BYTES_PER_PIXEL,
  output_height_by_one_decode);
  outfile = init_output_to_file(infile_name);
  if (mcu)
  {
    while (cinfo.output_offset < (cinfo.output_width * cinfo.output_height))
    {
      jpeg_read_mcus(&cinfo,
        buffer,
        output_height_by_one_decode,
        &output_position);
          output_to_file(outfile, buffer, output_position,
            output_width_by_one_decode, output_height_by_one_decode);
    }
  }
  else
    {
      while (cinfo.output_scanline < cinfo.output_height)
      {
            jpeg_read_scanlines(&cinfo, buffer, 1);
            output_to_file(outfile,buffer, (cinfo.output_scanline - 1) * cinfo.output_width,
            output_width_by_one_decode, output_height_by_one_decode);
      }
    }
        fin_output_to_file(outfile);
        (void) jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        close(infile);
   printf("thread 02 END\n");
  pthread_exit(0);
}

void decode_thread_03(void)
{

  int  infile;   /* file descriptor of input file */
  char infile_name[APP_FILENAME_LEN] = "/mnt/sd0/quad_vga.JPG";

  FILE *outfile;  /* file pointer of output file */

  struct jpeg_decompress_struct cinfo;

  struct jpeg_error_mgr jerr;

  JSAMPARRAY buffer;            /* Output row buffer */
  JDIMENSION output_position;   /* start position of output */
  JDIMENSION output_width_by_one_decode;
  JDIMENSION output_height_by_one_decode;
  bool       mcu = false;       /* True means "decode by the MCU" */

  if ((infile = open(infile_name, O_RDONLY)) < 0) {
    fprintf(stderr, "can't open %s\n", infile_name);
    return;
  }
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_fd_src(&cinfo, infile);    /* This is file descriptor API */
  (void) jpeg_read_header(&cinfo, TRUE);
  fprintf(stdout, "image width  = %d\n", cinfo.image_width);
  fprintf(stdout, "image height = %d\n", cinfo.image_height);
  cinfo.out_color_space = JCS_CbYCrY;
  cinfo.scale_num = APP_QVGA_WIDTH;
  cinfo.scale_denom = cinfo.image_width;
  (void) jpeg_start_decompress(&cinfo);
  if (mcu)
  {
    output_width_by_one_decode  = (cinfo.output_width  / cinfo.MCUs_per_row);
    output_height_by_one_decode = (cinfo.output_height / cinfo.MCU_rows_in_scan);
  }
  else
  {
    output_width_by_one_decode  = cinfo.output_width;
    output_height_by_one_decode = 1;
  }
  buffer = (*cinfo.mem->alloc_sarray)
  ((j_common_ptr) &cinfo,
  JPOOL_IMAGE,
  output_width_by_one_decode * APP_BYTES_PER_PIXEL,
  output_height_by_one_decode);
  outfile = init_output_to_file(infile_name);
  if (mcu)
  {
    while (cinfo.output_offset < (cinfo.output_width * cinfo.output_height))
    {
      jpeg_read_mcus(&cinfo,
        buffer,
        output_height_by_one_decode,
        &output_position);
          output_to_file(outfile, buffer, output_position,
            output_width_by_one_decode, output_height_by_one_decode);
    }
  }
  else
    {
      while (cinfo.output_scanline < cinfo.output_height)
      {
            jpeg_read_scanlines(&cinfo, buffer, 1);
            output_to_file(outfile,buffer, (cinfo.output_scanline - 1) * cinfo.output_width,
            output_width_by_one_decode, output_height_by_one_decode);
      }
    }
        fin_output_to_file(outfile);
        (void) jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        close(infile);
   printf("thread 03 END\n");
  pthread_exit(0);
}

void decode_thread_04(void)
{

  int  infile;   /* file descriptor of input file */
  char infile_name[APP_FILENAME_LEN] = "/mnt/sd0/uxga.JPG";

  FILE *outfile;  /* file pointer of output file */

  struct jpeg_decompress_struct cinfo;

  struct jpeg_error_mgr jerr;

  JSAMPARRAY buffer;            /* Output row buffer */
  JDIMENSION output_position;   /* start position of output */
  JDIMENSION output_width_by_one_decode;
  JDIMENSION output_height_by_one_decode;
  bool       mcu = false;       /* True means "decode by the MCU" */

  if ((infile = open(infile_name, O_RDONLY)) < 0) {
    fprintf(stderr, "can't open %s\n", infile_name);
    return;
  }
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_fd_src(&cinfo, infile);    /* This is file descriptor API */
  (void) jpeg_read_header(&cinfo, TRUE);
  fprintf(stdout, "image width  = %d\n", cinfo.image_width);
  fprintf(stdout, "image height = %d\n", cinfo.image_height);
  cinfo.out_color_space = JCS_CbYCrY;
  cinfo.scale_num = APP_QVGA_WIDTH;
  cinfo.scale_denom = cinfo.image_width;
  (void) jpeg_start_decompress(&cinfo);
  if (mcu)
  {
    output_width_by_one_decode  = (cinfo.output_width  / cinfo.MCUs_per_row);
    output_height_by_one_decode = (cinfo.output_height / cinfo.MCU_rows_in_scan);
  }
  else
  {
    output_width_by_one_decode  = cinfo.output_width;
    output_height_by_one_decode = 1;
  }
  buffer = (*cinfo.mem->alloc_sarray)
  ((j_common_ptr) &cinfo,
  JPOOL_IMAGE,
  output_width_by_one_decode * APP_BYTES_PER_PIXEL,
  output_height_by_one_decode);
  outfile = init_output_to_file(infile_name);
  if (mcu)
  {
    while (cinfo.output_offset < (cinfo.output_width * cinfo.output_height))
    {
      jpeg_read_mcus(&cinfo,
        buffer,
        output_height_by_one_decode,
        &output_position);
          output_to_file(outfile, buffer, output_position,
            output_width_by_one_decode, output_height_by_one_decode);
    }
  }
  else
    {
      while (cinfo.output_scanline < cinfo.output_height)
      {
            jpeg_read_scanlines(&cinfo, buffer, 1);
            output_to_file(outfile,buffer, (cinfo.output_scanline - 1) * cinfo.output_width,
            output_width_by_one_decode, output_height_by_one_decode);
      }
    }
        fin_output_to_file(outfile);
        (void) jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        close(infile);
   printf("thread 04 END\n");
  pthread_exit(0);
}
