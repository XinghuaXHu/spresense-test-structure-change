/****************************************************************************
 * sixaxis/sixaxis_main.c
 *
 *   Copyright (C) 2017 Sony Corporation
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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <arch/board/common/cxd56_bmi160.h>
#include <nuttx/sensors/bmi160.h>

#include "uart2.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ACC_DEVPATH      "/dev/accel0"

/****************************************************************************
 * Private Data
 ****************************************************************************/
static char g_spi_log[64];
static pthread_t g_spi_sensor_thid;
static int g_spi_fd_sd;
static int g_spi_fd_flash;


/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * sixaxis_main
 ****************************************************************************/

int spi_sensor_main(int loop)
{
  int fd;
  int cnt;
  struct accel_gyro_st_s data;

  fd = open(ACC_DEVPATH, O_RDONLY);
  if (fd < 0)
    {
      printf("Device %s open failure. %d\n", ACC_DEVPATH, fd);
      return -1;
    }

  for (cnt = 0; cnt < loop; cnt++)
    {
      int ret;
//      memset(&data, 0, sizeof(data));
//      memset(g_spi_log, 0, 64);

      ret = read(fd, &data, sizeof(struct accel_gyro_st_s));
      if (ret != sizeof(struct accel_gyro_st_s))
        {
          fprintf(stderr, "Read failed.\n");
          break;
        }

      printf("[%d] %d, %d, %d\n",
             data.sensor_time,
             data.accel.x, data.accel.y, data.accel.z);
      fflush(stdout);

      sprintf(g_spi_log, "spi sensor time:%d\n", data.sensor_time); 

      uart2_output(g_spi_log);

      sleep(1);
    }

  write(g_spi_fd_sd, g_spi_log, strlen(g_spi_log));
  fsync(g_spi_fd_sd);
  usleep(1);
  write(g_spi_fd_flash, g_spi_log, strlen(g_spi_log));
  fsync(g_spi_fd_flash);
  usleep(1);

  close(fd);

  return 0;
}

int spi_sensor_task(void)
{
  remove("/mnt/sd0/SPI.TXT");
  remove("/mnt/spif/SPI.TXT");

  sleep(1);

  g_spi_fd_sd = open("/mnt/sd0/SPI.TXT", O_RDWR | O_CREAT);
  if (g_spi_fd_sd < 0)
    {
      printf("SPI sensor log create error\n");
    }

  g_spi_fd_flash = open("/mnt/spif/SPI.TXT", O_RDWR | O_CREAT);
  if (g_spi_fd_flash < 0)
    {
      printf("SPI sensor log create error\n");
    }

  while (1)
    {
      spi_sensor_main(60);
    }

  return 0;
}

int spi_sensor(void)
{
  int ret;

  board_bmi160_initialize(5);

  ret = pthread_create(&g_spi_sensor_thid,
                       NULL,
                       (pthread_startroutine_t)spi_sensor_task,
                       NULL);
  if (ret != 0)
    {
      printf("Failed to create thread for SPI sensor. ret=%d\n", ret);
      return ret;
    }

  return OK;
}

