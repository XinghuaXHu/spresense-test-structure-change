/****************************************************************************
 * sqa/singlefunction/pmtest/pmtest.c
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arch/board/board.h>
#include <arch/chip/pm.h>
#include <nuttx/board.h>

#include "nshlib/nshlib.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define BOOT_TIME_PROFILE
#if defined(BOOT_TIME_PROFILE) && \
  defined(CONFIG_RTC) && defined(CONFIG_RTC_ALARM)

extern uint64_t cxd56_rtc_count(void);
extern uint64_t cxd56_rtc_almcount(void);

#define BOOT_TIME() do \
  { printf("B:%d= %lld\n", __LINE__, cxd56_rtc_count()); } while (0)

#define ALARM_TIME() do \
  { printf("A:%d= %lld\n", __LINE__, cxd56_rtc_almcount()); } while (0)

#else

#define BOOT_TIME()
#define ALARM_TIME()

#endif  /* BOOT_TIME_PROFILE */

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct pm_cpu_wakelock_s g_pmtest_wlock = {
  .count = 0,
  .info = PM_CPUWAKELOCK_TAG('W', 'T', 0x1234),
};

static struct pm_cpu_freqlock_s g_pmtest_hvlock =
  PM_CPUFREQLOCK_INIT(PM_CPUFREQLOCK_TAG('H', 'T', 0x1234),
                      PM_CPUFREQLOCK_FLAG_HV);

static struct pm_cpu_freqlock_s g_pmtest_lvlock =
  PM_CPUFREQLOCK_INIT(PM_CPUFREQLOCK_TAG('L', 'T', 0x5678),
                      PM_CPUFREQLOCK_FLAG_LV);

static char *g_bootcause_string[] =
{
  "<POR> DeadBattery",
  "<POR> Reboot or Watchdog",
  "<Deep> Reset",
  "<Deep> WKUPL signal",
  "<Deep> WKUPS signal",
  "<Deep> RTC Alarm",
  "<Deep> USB Connected",
  "<Deep> Reserved",
  "<Cold> SCU Int",
  "<Cold> RTC Alarm0",
  "<Cold> RTC Alarm1",
  "<Cold> RTC Alarm2",
  "<Cold> RTC AlarmErr",
  "<Cold> -",
  "<Cold> -",
  "<Cold> -",
  "<Cold> GPIO0 Int",
  "<Cold> GPIO1 Int",
  "<Cold> GPIO2 Int",
  "<Cold> GPIO3 Int",
  "<Cold> GPIO4 Int",
  "<Cold> GPIO5 Int",
  "<Cold> GPIO6 Int",
  "<Cold> GPIO7 Int",
  "<Cold> GPIO8 Int",
  "<Cold> GPIO9 Int",
  "<Cold> GPIO10 Int",
  "<Cold> GPIO11 Int",
  "<Cold> Sensor Int",
  "<Cold> PMIC Int",
  "<Cold> USB Disconnected",
  "<Cold> USB Connected",
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void show_usage(FAR const char *progname)
{
  printf("\nUsage: %s [-s <sleep>] [-<e|d> <mask>] [-f <freq>] [-p] [-h]\n\n",
         progname);
  printf("Description:\n");
  printf(" Power Management operation\n");
  printf("Options:\n");
  printf(" -s <sleep>: <sleep> selects <deep|cold|hot|run>.\n");
  printf(" -e <mask>: Enable Boot Mask\n");
  printf(" -d <mask>: Disable Boot Mask\n");
  printf(" -f <freq>: <freq> selects <hv|lv|rcosc>.\n");
  printf(" -p: TCXO,FLASH power off\n");
  printf(" -h: Show this message\n");
}

static void show_bootcause(void)
{
  int i;
  uint32_t bootcause = up_pm_get_bootcause();

  /* Boot Cause */

  printf("BootCause:\n");

  printf("(0x%08x) [%c] %s\n", 0,
         (bootcause == 0) ? '*' : ' ',
         "<POR> Power On Reset");

  for (i = 0; i < 32; i++)
    {
      printf("(0x%08x) [%c] %s\n", (1 << i),
             (bootcause & (1 << i)) ? '*' : ' ',
             g_bootcause_string[i]);
    }
}

static void show_bootmask(void)
{
  int i;
  uint32_t bootmask = up_pm_get_bootmask();

  /* Boot Mask */

  printf("BootMask:\n");

  for (i = 0; i < 32; i++)
    {
      printf("(0x%08x) [%c] %s\n", (1 << i),
             (bootmask & (1 << i)) ? '*' : ' ',
             g_bootcause_string[i]);
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * pmtest_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int pmtest_main(int argc, char *argv[])
#endif
{
  int opt = 0;
  uint32_t mask;
  int sopt = 0;
  char *sarg = NULL;
  int fopt = 0;
  char *farg = NULL;
#ifdef CONFIG_TEST_PMTEST_AUTO_LAUNCH
  static int isfirst = 1;
#endif

  BOOT_TIME();

  optind = -1;
  while ((opt = getopt(argc, argv, ":s:e:d:f:ph")) != -1)
    {
      switch (opt)
        {
        case 's':
          sopt = 1;
          sarg = optarg;
          break;
        case 'e':
          mask = strtoul(optarg, NULL, 0);
          up_pm_set_bootmask(mask);
          show_bootmask();
          return 0;
        case 'd':
          mask = strtoul(optarg, NULL, 0);
          up_pm_clr_bootmask(mask);
          show_bootmask();
          return 0;
        case 'f':
          fopt = 1;
          farg = optarg;
          break;
        case 'p':
#if defined(CONFIG_BOARD_SPRESENSE)
          board_xtal_power_control(false);
          board_lna_power_control(false);
#elif defined(CONFIG_BOARD_CORVO)
          board_xtal_power_control(false);
          board_flash_power_control(false);
#endif
          return 0;
        case 'h':
        case ':':
        case '?':
          show_usage(argv[0]);
          return EXIT_FAILURE;
        }
    }

  /* sleep option */

  if (sopt && sarg)
    {
      if (0 == strncmp("deep", sarg, 4))
        {
          printf("Enter Deep Sleep...\n");
          ALARM_TIME();
          board_power_off(BOARD_POWEROFF_DEEP);
        }
      else if (0 == strncmp("cold", sarg, 4))
        {
          printf("Enter Cold Sleep...\n");
          ALARM_TIME();
          board_power_off(BOARD_POWEROFF_COLD);
        }
      else if (0 == strncmp("hot", sarg, 3))
        {
          printf("WakeLock: Unlock\n");
          while (up_pm_count_acquire_wakelock() > 0)
            {
              up_pm_release_wakelock(&g_pmtest_wlock);
            }
        }
      else if (0 == strncmp("run", sarg, 3))
        {
          printf("WakeLock: Lock\n");
          up_pm_acquire_wakelock(&g_pmtest_wlock);
        }
      else
        {
          show_usage(argv[0]);
          return EXIT_FAILURE;
        }
      return 0;
    }

  /* freq option */

  if (fopt && farg)
    {
      if (0 == strncmp("hv", farg, 2))
        {
          printf("FreqLock: HV\n");
          up_pm_acquire_freqlock(&g_pmtest_hvlock);
        }
      else if (0 == strncmp("lv", farg, 2))
        {
          printf("FreqLock: LV\n");
          up_pm_acquire_freqlock(&g_pmtest_lvlock);
          while (up_pm_get_freqlock_count(&g_pmtest_hvlock) > 0)
            {
              up_pm_release_freqlock(&g_pmtest_hvlock);
            }
        }
      else if (0 == strncmp("rcosc", farg, 5))
        {
          printf("FreqLock: RCOSC\n");
          while (up_pm_get_freqlock_count(&g_pmtest_hvlock) > 0)
            {
              up_pm_release_freqlock(&g_pmtest_hvlock);
            }
          while (up_pm_get_freqlock_count(&g_pmtest_lvlock) > 0)
            {
              up_pm_release_freqlock(&g_pmtest_lvlock);
            }
        }
      else
        {
          show_usage(argv[0]);
          return EXIT_FAILURE;
        }
      return 0;
    }

  /* just show boot cause */

  show_bootcause();

#ifdef CONFIG_TEST_PMTEST_AUTO_LAUNCH
  if (isfirst)
    {
      isfirst = 0;

      /* boardctl(BOARDIOC_INIT, 0) is called from nsh_initialize() */

      nsh_initialize();

      return nsh_consolemain(0, NULL);
    }
#endif

  return 0;
}
