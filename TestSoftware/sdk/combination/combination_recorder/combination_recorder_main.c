/****************************************************************************
 * sqa/combination/audio_gnss_led_camera_asmp/app_main.c
 *
 *   Copyright (C) 2017 Sony Corporation.
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
#include <sys/types.h>
#include <stdio.h>
#include <sys/boardctl.h>

#include "led.h"
#include "gnss.h"
#include "audio_recorder.h"
#include "camera.h"
#include "asmp.h"
#include "spi_sensor.h"
#include "uart2.h"

#include "video_capture_thread.h"

#if (defined CONFIG_COMBI_SET_REC_W_STEP) || (defined CONFIG_COMBI_SET_STEPCNT_ONLY)
#include "include_rec_W_step/stepcounter.h"
#include "init.h"
#endif

#include "jpeg_decode.h"

#include "system.h"
#include "fat.h"


#include <string.h>
#include <stdlib.h>


/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/
#define CMD_TBL_NUM 13

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int combination_rec_main(int argc, char *argv[])
#endif
{
  int ret;
  int i, j;
  typedef struct table_s
  {
      char *name;
      int flag;
  } table;

  table command[CMD_TBL_NUM] =
  {
      {"lte", 0},
      {"led", 0},
      {"gnss", 0},
      {"audio", 0},
      {"video", 0},
      {"camera", 0},
      {"asmp", 0},
      {"decode", 0},
      {"step", 0},
      {"sh_init", 0},
      {"flash", 0},
      {"fat", 0}
  };

  if(argc >1)
  {
      for(j = 0; j <argc; j++)
      {
          for(i = 0; i < CMD_TBL_NUM; i++)
          {
              if(strcmp(argv[j], command[i].name)==0)
              {
                  command[i].flag = 1;
              }
          }
      }
  }


  /* Prepare UART2 output */

  uart2_init();


#ifdef CONFIG_COMBI_SET_LTE
  if(command[0].flag == 1)
  {
    ret = lte(argv[2]);
    if (ret != OK)
    {
      printf("Failed to start lte\n");
      return ret;
    }
  }
#endif

    /* Start LED flickering */
#ifdef CONFIG_COMBI_SET_LED
  if(command[1].flag == 1)
  {
    ret = led();
    if (ret != OK)
    {
      printf("Failed to start LED\n");
      return ret;
    }
  }
#endif

    /* Start GNSS */
#ifdef CONFIG_COMBI_SET_GNSS
  if(command[2].flag == 1)
  {
    ret = gnss();
    if (ret != OK)
      {
        printf("Failed to start gnss\n");
        return ret;
      }
  }
#endif

    /* Start audio revcorder */
#if defined(CONFIG_COMBI_SET_AUDIO) || defined(CONFIG_COMBI_SET_REC_W_STEP)
  if(command[3].flag == 1)
  {
    ret = audio_recorder(argv[1]);
    if (ret != OK)
    {
      printf("Failed to start audio recorder\n");
      return ret;
    }
  }
#endif

    /*Start Video capture*/
    ret = video_initialize("/dev/video");
    if (ret < 0)
    {
      printf("Failed to video initialize. ret=%d\n", ret);
      return ret;
    }

#ifdef CONFIG_COMBI_SET_VIDEO
  if(command[4].flag == 1)
  {
    ret = video_capture_thread_create();
    if (ret < 0)
    {
      printf("Failed to create task for camera video mode. ret=%d\n", ret);
      return ret;
    }
  }
#endif

   sleep(1);

    /* Start camera */
#ifdef CONFIG_COMBI_SET_CAMERA
  if(command[5].flag == 1)
  {
    ret = camera();
    if (ret != OK)
    {
      printf("Failed to start camera\n");
      return ret;
    }
  }
#endif


    /* Start CPU load program in each CPU */
#ifdef CONFIG_COMBI_SET_ASMP
  if(command[6].flag == 1)
  {
    ret = asmp(argv[1]);
    if (ret != OK)
    {
      printf("Failed to start asmp\n");
      return ret;
    }
  }
#endif

#ifdef CONFIG_COMBI_SET_DECODE
  if(command[7].flag == 1)
  {
    ret = decode();
    if (ret != OK)
    {
      printf("Failed to start jpeg decode\n");
      return ret;
    }
  }
#endif

#if defined(CONFIG_COMBI_SET_STEP) || defined(CONFIG_COMBI_SET_REC_W_STEP)
  if(command[8].flag == 1)
  {
    ret = stepcounter();
    if (ret != OK)
    {
      printf("Failed to start step counter\n");
      return ret;
    }
  }
#endif

#ifdef CONFIG_COMBI_SET_REC_W_STEP
  if(command[9].flag == 1)
  {
    printf("app_init_libraries()\n");
    if (!app_init_libraries())
      {
        printf("Error: init_libraries() failure.\n");
        return 1;
      }
  }
#endif

#ifdef CONFIG_COMBI_SET_FLASH
  if(command[10].flag == 1)
  {
    ret = system_flash();
    if (ret != OK)
      {
        printf("Error: system_flash() failure.\n");
        return ret;
      }
  }
#endif

#ifdef CONFIG_COMBI_SET_FAT
  if(command[11].flag == 1)
  {
    ret = fat();
    if (ret != OK)
      {
        printf("Error: fat() failure.\n");
        return ret;
      }
  }
#endif



    /* Start SPI sensor */
  /*
    ret = spi_sensor();
    if (ret != OK)
      {
        printf("Failed to start SPI sensor\n");
        return ret;
      }
  */

    return OK;
}
