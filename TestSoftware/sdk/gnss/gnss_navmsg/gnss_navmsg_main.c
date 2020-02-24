/****************************************************************************
 * test/gnss_navmsg_main.c
 *
 *   Copyright (C) 2018 Sony. All rights reserved.
 *   Author: Tomoyuki Takahashi <Tomoyuki.A.Takahashi@sony.com>
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
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arch/chip/gnss.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define SIG_CPHASE                18
#define SIG_GPSEPH                19
#define SIG_GLNEPH                20
#define SIG_SBAS                  21
#define RTK_TEST_INTERVAL         CXD56_GNSS_RTK_INTERVAL_1HZ

#define RTK_TEST_SATELLITE        (CXD56_GNSS_SAT_GPS | \
                                   CXD56_GNSS_SAT_GLONASS | \
                                   CXD56_GNSS_SAT_SBAS)
#define RTK_TEST_EPH_ENABLE       1
#define RTK_DATA_NUM_MAX          20
#define SAT_SELECT_GPS            (0x00000001)    /* GPS */
#define SAT_SELECT_GLN            (0x00000002)    /* GLONASS */
#define SBAS_SELECT               (0xFFFFFFFFFFFFFFFF)

/* Default differential seconds between UTC time and GPS time  */

#define GPS_DEFAULT_UTC_DIFF_SEC        (18)

#ifndef CONFIG_LIBC_FLOATINGPOINT
#error "Must set LIBC_FLOATINGPOINT on Kernel config for carrier phase print"
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/
static const char day_of_month[] 
/*    Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec */
  = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* RTK Functions */
static char asciidebug_gdsat2str(uint32_t sat)
{
  static const char prefix[]    = "PNSQIFBG";
  static const char unknown[]   = "?";
  int               i;
  int               prefix_size = sizeof(prefix) / sizeof(char);
  uint32_t          mask        = 1UL;

  for ( i = 0 ; i < prefix_size ; i++ )
    {
      if (sat & mask)
        {
          break;
        }
      mask <<= 1;
    }
  return (i < prefix_size) ? prefix[i] : unknown[0];
}

static void convert_from_utu2gpstime(struct cxd56_gnss_date_s *date,
                                     struct cxd56_gnss_time_s *time)
{
  time->sec += GPS_DEFAULT_UTC_DIFF_SEC;
  if (time->sec >= 60)
    {
      time->sec -= 60;
      if (time->minute < 59)
        {
          time->minute += 1;
        }
      else
        {
          time->minute = 0;
          if (time->hour < 23)
            {
              time->hour += 1;
            }
          else
            {
              time->hour = 0;
              if (date->day < day_of_month[date->month - 1])
                {
                  date->day += 1;
                }
              else
                {
                  date->day = 1;
                  if (date->month < 12)
                    {
                      date->month += 1;
                    }
                  else
                    {
                      date->month = 1;
                      date->year += 1;
                    }
                }
            }
        }
    }
}

/* Carrier phase */

static int read_cphase(int fd, struct cxd56_rtk_carrierphase_s *cphase)
{
  int ret;
  
  ret = lseek(fd, CXD56_GNSS_READ_OFFSET_RTK, SEEK_SET);
  if (ret < 0)
    {
      ret = errno;
      printf("lseek error %d\n", ret);
      goto _err;
    }

  ret = read(fd, cphase, sizeof(*cphase));
  if (ret < 0)
    {
      ret = errno;
      printf("read error CPHASE %d\n", ret);
    }
  
_err:
  return ret;
}

static int print_cphase(struct cxd56_rtk_carrierphase_s *cphase)
{
  int    ret = OK;
  int    i;
  double drift;
  int    dri;

  /* TimeStamp */
  printf("timestamp : 0x%08x%08x, now : 0x%08x%08x\n",
          (uint32_t)(0xffffffff & (cphase->infoout.timestamp >> 32)),
          (uint32_t)(0xffffffff & cphase->infoout.timestamp),
          (uint32_t)(0xffffffff & (cphase->infoout.timesnow  >> 32)),
          (uint32_t)(0xffffffff & cphase->infoout.timesnow ));
  
  /* header (1PPS status) */
  printf("Obs: %d\n", cphase->infoout.ppsstatus);
  
  /* Wntow */
  printf("Wntow: %d %d %d %d .%06d\n",
          cphase->infoout.wntow.weeknumber,
          cphase->infoout.wntow.tow,
          cphase->infoout.wntow.sec,
          cphase->infoout.wntow.rollover,
          cphase->infoout.wntow.frac);
  
  /* 1PPS time */
  convert_from_utu2gpstime(&cphase->infoout.date, &cphase->infoout.time);
  printf("PPS: %04hu %02d %02d %02d %02d   %2d.%06d\n",
          cphase->infoout.date.year,
          cphase->infoout.date.month,
          cphase->infoout.date.day,
          cphase->infoout.time.hour,
          cphase->infoout.time.minute,
          cphase->infoout.time.sec,
          cphase->infoout.time.usec);

  /* TimeTag */
  printf("Tag: %10u %6u %6u\n",
          cphase->infoout.tag.msec,
          cphase->infoout.tag.frac,
          cphase->infoout.tag.cycle);
  
  /* clock drift */
  if (cphase->infoout.cdvalidity == 1 )
    {
      drift = cphase->infoout.clockdrift;
      dri = (int)(drift * 100) % 100;
      dri = dri < 0 ? dri * -1 : dri;
    }
  else
    {
      drift = 0;
      dri = 0;
    }
  printf("Drift: %d.%02d\n", (long)drift, dri);

  for(i=0; i < cphase->infoout.svcount; i++)
    {
      printf("%02d %c%03d %3d %4d %0.02f %d %d %14.3f %8.2f %18.3f %d\n",
          cphase->svout[i].ch,
          asciidebug_gdsat2str(cphase->svout[i].gnss),
          cphase->svout[i].svid,
          cphase->svout[i].fdmch,
          cphase->svout[i].cn,
          cphase->svout[i].c2p,
          cphase->svout[i].polarity,
          cphase->svout[i].lastpreamble,
          cphase->svout[i].pseudorange,
          cphase->svout[i].doppler,
          cphase->svout[i].carrierphase,
          cphase->svout[i].lli
      );
    }

  printf("END\n");

  return ret;
}

/* GPS ephemeris */

static int read_gpseph(int fd, struct cxd56_rtk_gpsephemeris_s *gpseph)
{
  int ret;
  
  ret = lseek(fd, CXD56_GNSS_READ_OFFSET_GPSEPHEMERIS, SEEK_SET);
  if (ret < 0)
    {
      ret = errno;
      printf("lseek error %d\n", ret);
      goto _err;
    }

  ret = read(fd, gpseph, sizeof(*gpseph));
  if (ret < 0)
    {
      ret = errno;
      printf("read error %d\n", ret);
    }

_err:
  return ret;
}

static int print_gpseph(struct cxd56_rtk_gpsephemeris_s *gpseph)
{
  printf("[GPS EPH] PPSstatus:%d\n", gpseph->ppsstatus);
  printf("[GPS EPH] ID:%d, Tow:%d\n", gpseph->id, gpseph->tow);
  printf("[GPS EPH] y:%d m:%d d:%d\n", gpseph->tocdate.year,
          gpseph->tocdate.month, gpseph->tocdate.day);
  printf("[GPS EPH] h:%d m:%d s:%d\n", gpseph->toctime.hour,
          gpseph->toctime.minute, gpseph->toctime.sec);
  // Wntow
  printf("[GPS EPH] Wntow: %d %d %d %d .%06d\n",
          gpseph->tocwntow.weeknumber,
          gpseph->tocwntow.tow,
          gpseph->tocwntow.sec,
          gpseph->tocwntow.rollover,
          gpseph->tocwntow.frac);
  printf("END\n");
  
  return OK;
}

/* Glonass ephemeris */

static int read_glneph(int fd, struct cxd56_rtk_glonassephemeris_s *glneph)
{
  int ret;
  
  ret = lseek(fd, CXD56_GNSS_READ_OFFSET_GLNEPHEMERIS, SEEK_SET);
  if (ret < 0)
    {
      ret = errno;
      printf("lseek error %d\n", ret);
      goto _err;
    }

  ret = read(fd, glneph, sizeof(*glneph));
  if (ret < 0)
    {
      ret = errno;
      printf("read error %d\n", ret);
    }

_err:
  return ret;
}

static int print_glneph(struct cxd56_rtk_glonassephemeris_s *glneph)
{
  printf("[GLN EPH] valid:0x%x, slot:%d, ch:%d\n", glneph->valid,
          glneph->slot, glneph->ch);
  printf("[GLN EPH] time h:%d m:%d s:%d\n", glneph->tk_h,
          glneph->tk_m, glneph->tk_s);
  printf("[GLN EPH] Bn:%d, tb:%d, Hn_e:%d, En:%d, n:%d, M:%d\n",
          glneph->bn,
          glneph->tb, glneph->hn_e, glneph->en,
          glneph->n, glneph->m);
  printf("END\n");

  return OK;
}

/* sbas */
static int read_sbas(int fd, struct cxd56_gnss_sbasdata_s *sbasdat)
{
  int ret;
  
  ret = lseek(fd, CXD56_GNSS_READ_OFFSET_SBAS, SEEK_SET);
  if (ret < 0)
    {
      ret = errno;
      printf("lseek error %d\n", ret);
      goto _err;
    }

  ret = read(fd, sbasdat, sizeof(*sbasdat));
  if (ret < 0)
    {
      ret = errno;
      printf("read error %d\n", ret);
    }

_err:
  return ret;
}

static int print_sbas(struct cxd56_gnss_sbasdata_s *sbasdat)
{
  printf("[SBAS] timenow : 0x%08x%08x\n",
          (uint32_t)(0xffffffff & (sbasdat->timesnow  >> 32)),
          (uint32_t)(0xffffffff & sbasdat->timesnow ));
  printf("[SBAS] %d %6d", sbasdat->gpswn, sbasdat->gpstow);
  printf(" %d %2d : ", sbasdat->svid, sbasdat->msgid);
  for (int i = 0 ; i < CXD56_GNSS_SBAS_MESSAGE_DATA_LEN ; i++ )
    {
      printf("%02x", sbasdat->sbasmsg[i]);
    }
  printf("\n");

  return OK;
}


static int set_signal(int fd, uint8_t gnsssig, int signo, uint8_t enable)
{
  struct cxd56_gnss_signal_setting_s setting;

  setting.fd      = fd;
  setting.enable  = enable;
  setting.gnsssig = gnsssig;
  setting.signo   = signo;
  setting.data    = NULL;

  return ioctl(fd, CXD56_GNSS_IOCTL_SIGNAL_SET, (unsigned long)&setting);
}


/****************************************************************************
 * gnss_navmsg_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int gnss_navmsg_main(int argc, char *argv[])
#endif
{
  int                        fd;
  int                        ret;
  sigset_t                   mask;
  int                        sig_id = -1;
  struct cxd56_rtk_setting_s rtk_setting;
  uint8_t                    gpseph_count = 0;
  uint8_t                    ppsstatus;
  union {
    struct cxd56_rtk_carrierphase_s     cphase;
    struct cxd56_rtk_gpsephemeris_s     gpseph;
    struct cxd56_rtk_glonassephemeris_s glneph;
    struct cxd56_gnss_sbasdata_s        sbasdat;
  } rbuf;

  printf("Hello, GNSS_NAVMSG!!\n");

  /* program start */

  printf("### NavMsg Output sample ###\n");

  fd = open("/dev/gps", O_RDONLY);
  if (fd <= 0)
    {
      printf("open error:%d\n", errno);
      return -ENODEV;
    }
  
  /* Set signals */

  sigemptyset(&mask);
  sigaddset(&mask, SIG_CPHASE);
  sigaddset(&mask, SIG_GPSEPH);
  sigaddset(&mask, SIG_GLNEPH);
  sigaddset(&mask, SIG_SBAS);
  ret = sigprocmask(SIG_BLOCK, &mask, NULL);
  if (ret != OK)
    {
      printf("sigprocmask failed. %d\n", ret);
      goto _err5;
    }

  ret = set_signal(fd, CXD56_GNSS_SIG_RTK, SIG_CPHASE, 1);
  if (ret != OK)
    {
      printf("set_signal CPHASE failed. %d\n", ret);
      goto _err5;
    }

  ret = set_signal(fd, CXD56_GNSS_SIG_SBAS, SIG_SBAS, 1);
  if (ret != OK)
    {
      printf("set_signal SBAS failed. %d\n", ret);
      goto _err4;
    }

  ret = set_signal(fd, CXD56_GNSS_SIG_GPSEPHEMERIS, SIG_GPSEPH, 1);
  if (ret != OK)
    {
      printf("set_signal GPSEPH failed. %d\n", ret);
      goto _err3;
    }

  ret = set_signal(fd, CXD56_GNSS_SIG_GLNEPHEMERIS, SIG_GLNEPH, 1);
  if (ret != OK)
    {
      printf("set_signal GLNEPH failed. %d\n", ret);
      goto _err2;
    }

  /* Select satellite */

  ret = ioctl(fd, CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM, RTK_TEST_SATELLITE);
  if (ret < 0)
    {
      printf("ioctl:CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM NG!!\n");
      goto _err1;
    }

  /* Start GNSS core */
  
  ret = ioctl(fd, CXD56_GNSS_IOCTL_START, CXD56_GNSS_STMOD_HOT);
  if (ret < 0)
    {
      printf("GNSS Start error\n");
      goto _err1;
    }
  
  /* Enable output of navigation Messages */

  printf("Start NavMsg Output \n");
  rtk_setting.interval  = RTK_TEST_INTERVAL;
  rtk_setting.gnss      = RTK_TEST_SATELLITE;
  rtk_setting.ephout    = RTK_TEST_EPH_ENABLE;
  rtk_setting.sbasout   = SBAS_SELECT;
  
  ret = ioctl(fd, CXD56_GNSS_IOCTL_NAVMSG_START, 
              (unsigned long)&rtk_setting);
  if (ret < 0)
    {
      printf("Start NavMsg output error\n");
      goto _err0;
    }
  
  do
    {
      /* Wait signal */

      sig_id = sigwaitinfo(&mask, NULL);

      switch (sig_id)
        {
        case SIG_CPHASE:
          ret = read_cphase(fd, &rbuf.cphase);
          if (ret < 0)
            {
              printf("read carrier phase error %d\n", ret);
              break;
            }
          print_cphase(&rbuf.cphase);
          ppsstatus = rbuf.cphase.infoout.ppsstatus;
#ifndef CONFIG_EXAMPLES_NAVMSG_REALTIME_GPSEPH
          while (gpseph_count > 0 && ppsstatus >= 7)
            {
                ret = read_gpseph(fd, &rbuf.gpseph);
                if (ret < 0)
                  {
                    break;
                  }
                print_gpseph(&rbuf.gpseph);
                gpseph_count--;
            }
#endif
          break;

        case SIG_GPSEPH:
          gpseph_count++;
#ifdef CONFIG_EXAMPLES_NAVMSG_REALTIME_GPSEPH
          ret = read_gpseph(fd, &rbuf.gpseph);
          if (ret < 0)
            {
              printf("read gps ephemeris error %d\n", ret);
              break;
            }
          print_gpseph(&rbuf.gpseph);
#endif
          break;

        case SIG_GLNEPH:
          ret = read_glneph(fd, &rbuf.glneph);
          if (ret < 0)
            {
              printf("read glonass ephemeris error %d\n", ret);
              break;
            }
          print_glneph(&rbuf.glneph);
          break;

       case SIG_SBAS:
          ret = read_sbas(fd, &rbuf.sbasdat);
          if (ret < 0)
            {
              printf("read sbas error %d\n", ret);
              break;
            }
          print_sbas(&rbuf.sbasdat);
          break;

        default:

          /* Unexpected signal */

          printf("sig_id %d\n", sig_id);
          ret = -EINTR;
          break;
        }
    }
  while (ret >= 0);
  
  /* Stop Log */

  printf("Stop NavMsg Output \n");

  ioctl(fd, CXD56_GNSS_IOCTL_RTK_STOP, 0);
_err0:
  ioctl(fd, CXD56_GNSS_IOCTL_STOP, 0);
_err1:
  set_signal(fd, CXD56_GNSS_SIG_GLNEPHEMERIS, SIG_GLNEPH, 0);
_err2:
  set_signal(fd, CXD56_GNSS_SIG_GPSEPHEMERIS, SIG_GPSEPH, 0);
_err3:
  set_signal(fd, CXD56_GNSS_SIG_SBAS, SIG_SBAS, 0);
_err4:
  set_signal(fd, CXD56_GNSS_SIG_RTK, SIG_CPHASE, 0);
_err5:
  ret = close(fd);
  if (ret < 0)
    {
      printf("device close error\n");
    }

  printf("[END] NavMsg Output sample \n");
  
  return ret;
}
