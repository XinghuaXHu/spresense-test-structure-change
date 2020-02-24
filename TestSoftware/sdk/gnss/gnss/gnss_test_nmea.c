/****************************************************************************
 * test/sqa/singlefunction/gnss/gnss_test_nmea.c
 *
 *   Copyright (C) 2017 Sony. All rights reserved.
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
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
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

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "gpsutils/cxd56_gnss_nmea.h"
#include "gnss_test_main.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define GNSS_POLL_TIMEOUT_FOREVER -1
#define MY_GNSS_SIG1              19

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static char                   nmea_buf[NMEA_SENTENCE_MAX_LEN];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int print_nmea(int fd)
{
  int    ret;

  if (lseek(fd, CXD56_GNSS_READ_OFFSET_LAST_GNSS, SEEK_SET) < 0)
    {
      ret = errno;
      printf("lseek error %d\n", ret);
    }
  else if (read(fd, &gnss_posdat, sizeof(gnss_posdat)) < 0)
    {
      ret = errno;
      printf("read error %d\n", ret);
    }
  else
    {
      ret = OK;
      NMEA_Output(&gnss_posdat);
    }

  return ret;
}

/* output NMEA */

FAR static char *reqbuf(uint16_t size)
{
  if (size > sizeof(nmea_buf))
    {
      printf("reqbuf error: oversize %s\n", size);
      return NULL;
    }
  return nmea_buf;
}

static void freebuf(FAR char *buf)
{

  /* nop */

}

static int outnmea(FAR char *buf)
{
  printf("%s", buf);
  return 0;
}

static int outbin(FAR char *buf, uint32_t len)
{

  /* nop */

  return 0;
}

static int signal2own(int signo, void *data)
{
  int ret;

#ifdef CONFIG_CAN_PASS_STRUCTS
  union sigval value;
  value.sival_ptr = data;
  ret = sigqueue(getpid(), signo, value);
#else
  ret = sigqueue(getpid(), signo, data);
#endif

  return ret;
}

/****************************************************************************
 * gnss_atcmd_main
 ****************************************************************************/

int gnss_nmea(int argc, char *argv[])
{
  int               ret;
  int               fd;
  int               timecount = 60*3;
  struct pollfd     fds;
  NMEA_OUTPUT_CB    funcs;
  sem_t             syncsem;

  ret = sem_init(&syncsem, 0, 0);
  if (ret < 0)
    {
      _err("Failed to initialize syncsem!\n");
    }
  else
    {

      /* program start */

      fd = open("/dev/gps", O_RDONLY);
      if (fd < 0)
        {
          printf("open error:%d,%d\n", fd, errno);
          ret = -ENODEV;
        }
      else
        {

          /* Init NMEA library */

          NMEA_InitMask();
          funcs.bufReq  = reqbuf;
          funcs.out     = outnmea;
          funcs.outBin  = outbin;
          funcs.bufFree = freebuf;
          NMEA_RegistOutputFunc(&funcs);

          sem_post(&syncsem);

          NMEA_SetMask(0x001000ff);

          /* Set GNSS parameters. */

          gnss_setparams(fd);

          /* Start GNSS. */

          ret = gnss_start(fd, CXD56_GNSS_STMOD_COLD);
          if (ret == 0)
            {
              do
                {
                  memset(&fds, 0, sizeof(fds));
                  fds.fd     = fd;
                  fds.events = POLLIN;

                  ret = poll(&fds, 1, GNSS_POLL_TIMEOUT_FOREVER);
                  if (ret <= 0 && errno != EINTR)
                    {
                      printf("poll error %d,%d,%x,%x\n", ret, errno, fds.events, fds.revents);
                      break;
                    }

                  ret = sem_wait(&syncsem);
                  if (ret < 0)
                    {
                      printf("unexpected wait syncsem error%d\n", ret);
                      break;
                    }

                  if (fds.revents & POLLIN)
                    {
                      print_nmea(fd);

                      if (gnss_posdat.receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID)
                        {
                          timecount--;
                        }
                    }

                  sem_post(&syncsem);
                }
              while (timecount > 0);

              /* Stop GNSS. */

              gnss_stop(fd);
            }
          close(fd);
        }

      /* The pthread has returned */

      signal2own(MY_GNSS_SIG1, NULL);

      sem_destroy(&syncsem);

    }

  printf("%s() done\n", __func__);

  return ret;
}
