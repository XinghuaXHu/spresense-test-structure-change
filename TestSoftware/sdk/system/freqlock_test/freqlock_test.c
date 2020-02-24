/****************************************************************************
 * sqa/singlefunction/freqlock_test/freqlock_test.c
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
#include <arch/chip/pm.h>
#include <errno.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define FREQLOCK_TEST_TASK_PRIORITY   150
#define FREQLOCK_TEST_TASK_STACK_SIZE 2048
#define FREQLOCK_TEST_NUM             100
#define FREQLOCK_TEST_TASK_NUM        8

#define PERFORMANCE_TEST_COUNT        10000000

extern uint64_t cxd56_rtc_count(void);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct pm_cpu_freqlock_s g_hvlock_0 =
  PM_CPUFREQLOCK_INIT(PM_CPUFREQLOCK_TAG('H', 'T', 0x1234),
                      PM_CPUFREQLOCK_FLAG_HV);

static struct pm_cpu_freqlock_s g_lvlock_0 =
  PM_CPUFREQLOCK_INIT(PM_CPUFREQLOCK_TAG('L', 'T', 0x5678),
                      PM_CPUFREQLOCK_FLAG_LV);

static struct pm_cpu_freqlock_s g_hvlock_1 =
  PM_CPUFREQLOCK_INIT(PM_CPUFREQLOCK_TAG('H', 'V', 0x4321),
                      PM_CPUFREQLOCK_FLAG_HV);

static struct pm_cpu_freqlock_s g_lvlock_1 =
  PM_CPUFREQLOCK_INIT(PM_CPUFREQLOCK_TAG('L', 'V', 0x8765),
                      PM_CPUFREQLOCK_FLAG_LV);

static struct pm_cpu_freqlock_s *g_lock[FREQLOCK_TEST_TASK_NUM] =
  {
    &g_hvlock_0, &g_hvlock_0, &g_hvlock_1, &g_hvlock_1,
    &g_lvlock_0, &g_lvlock_0, &g_lvlock_1, &g_lvlock_1
  };

#define LOCK_LV(str) \
{ \
  up_pm_acquire_freqlock(&g_lvlock_0); \
  printf("Lock LV %s\n", str); \
  sleep(CONFIG_FREQLOCK_TEST_SLEEP_TIME); \
}

#define UNLOCK_LV(str) \
{ \
  up_pm_release_freqlock(&g_lvlock_0); \
  printf("Unock LV %s\n", str); \
  sleep(CONFIG_FREQLOCK_TEST_SLEEP_TIME); \
}

#define LOCK_HV(str) \
{ \
  up_pm_acquire_freqlock(&g_hvlock_0); \
  printf("Lock HV %s\n", str); \
  sleep(CONFIG_FREQLOCK_TEST_SLEEP_TIME); \
}

#define UNLOCK_HV(str) \
{ \
  up_pm_release_freqlock(&g_hvlock_0); \
  printf("Unock HV %s\n", str); \
  sleep(CONFIG_FREQLOCK_TEST_SLEEP_TIME); \
}

#define PERFORMANCE_TEST(str) \
{ \
  rtc_0 = cxd56_rtc_count(); \
  for (i = 0 ; i < PERFORMANCE_TEST_COUNT ; i++) \
    { \
      cnt += 1; \
    } \
  rtc_1 = cxd56_rtc_count(); \
  printf("%s:RTC counter=%lld %dms\n", \
    str, rtc_1 - rtc_0, ((rtc_1 - rtc_0) * 1000) / 32768); \
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/


/****************************************************************************
 * Private Functions
 ****************************************************************************/
static void show_usage(FAR const char *progname)
{
  printf("\nUsage: %s [-c] [-h] [-t <test>]\n\n", progname);
  printf("Description:\n");
  printf(" Freq lock test operation\n");
  printf("Options:\n");
  printf(" -t <test>: <test> selects <random|chgvol|perfmc>.\n");
  printf("            random:random test\n");
  printf("            chgvol:change voltage test\n");
  printf("            perfmc:LV/HV performance test\n");
  printf(" -c: Show freq lock counter\n");
  printf(" -h: Show this message\n");
}

static void show_lockcount(void)
{
  int i;
  int cnt;

  for (i = 0 ; i < FREQLOCK_TEST_TASK_NUM ; i++)
    {
      cnt = up_pm_get_freqlock_count(g_lock[i]);
      printf("lock_id:%d freqlock count=%d\n", i, cnt);
    }
}

static int freqlock_random_test_task(int argc,  const char* argv[])
{
  int      i;
  int      cnt;
  uint32_t sleep_time;

  int lock_id = atoi(argv[1]);

  printf("freqlock random test start:lock_id=%d\n", lock_id);

  for (i = 0 ; i < FREQLOCK_TEST_NUM ; i++)
    {
      up_pm_acquire_freqlock(g_lock[lock_id]);
      sleep_time = (int)cxd56_rtc_count() % 5;
      printf("lock_id=%d: num=%d , sleep_time=%d\n", lock_id, i, sleep_time);
      sleep(sleep_time);
      up_pm_release_freqlock(g_lock[lock_id]);
    }

  cnt = up_pm_get_freqlock_count(g_lock[lock_id]);
  printf("lock_id=%d freqlock count=%d\n", lock_id, cnt);

  for (i = 0 ; i < FREQLOCK_TEST_NUM ; i++)
    {
      up_pm_acquire_freqlock(g_lock[lock_id]);
      usleep(10 * 1000);
    }
  for (i = 0 ; i < FREQLOCK_TEST_NUM ; i++)
    {
      up_pm_release_freqlock(g_lock[lock_id]);
      usleep(10 * 1000);
    }

  cnt = up_pm_get_freqlock_count(g_lock[lock_id]);
  printf("Exit freq lock test:lock_id=%d freqlock count=%d\n", lock_id, cnt);

  return 0;
}

static int freqlock_changevoltage_test_task(int argc,  const char* argv[])
{
  int      cnt;

  printf("Start freqlock changevoltage test\n");

  sleep(CONFIG_FREQLOCK_TEST_SLEEP_TIME);

  printf("RCOSC\n");

  /* LV */

  LOCK_LV("1-1");
  LOCK_LV("1-2");
  LOCK_LV("1-3");

  UNLOCK_LV("1-3");

  /* HV */

  LOCK_HV("1-1");
  LOCK_HV("1-2");
  LOCK_HV("1-3");

  UNLOCK_HV("1-3");
  UNLOCK_HV("1-2");

  LOCK_LV("1-4");

  /* LV */

  UNLOCK_HV("1-1");

  /* HV */

  LOCK_HV("2-1");
  LOCK_HV("2-2");

  UNLOCK_LV("1-4");
  UNLOCK_LV("1-2");
  UNLOCK_LV("1-1");

  UNLOCK_HV("2-2");

  /* RCOSC */

  UNLOCK_HV("2-1");

  /* HV */

  LOCK_HV("3-1");

  LOCK_LV("3-1");

  /* LV */

  UNLOCK_HV("3-1");

  /* RCOSC */

  UNLOCK_LV("3-1");

  cnt = up_pm_get_freqlock_count(&g_lvlock_0);
  printf("LV freqlock count=%d\n", cnt);
  cnt = up_pm_get_freqlock_count(&g_hvlock_0);
  printf("HV freqlock count=%d\n", cnt);

  printf("Exit freq lock test\n");

  return 0;
}

static int freqlock_performance_test_task(int argc,  const char* argv[])
{
  int      i;
  uint64_t rtc_0;
  uint64_t rtc_1;
  volatile int cnt = 0;

  printf("Start freqlock performance test\n");

  PERFORMANCE_TEST("RCOSC-1");

  up_pm_acquire_freqlock(&g_lvlock_0);

  PERFORMANCE_TEST("LV-1");

  up_pm_acquire_freqlock(&g_lvlock_0);

  PERFORMANCE_TEST("LV-2");

  up_pm_acquire_freqlock(&g_hvlock_0);

  PERFORMANCE_TEST("HV-1");

  up_pm_acquire_freqlock(&g_hvlock_0);

  PERFORMANCE_TEST("HV-2");

  up_pm_release_freqlock(&g_hvlock_0);

  PERFORMANCE_TEST("HV-3");

  up_pm_release_freqlock(&g_hvlock_0);

  PERFORMANCE_TEST("LV-3");

  up_pm_release_freqlock(&g_lvlock_0);

  PERFORMANCE_TEST("LV-4");

  up_pm_release_freqlock(&g_lvlock_0);

  PERFORMANCE_TEST("RCOSC-2");

  cnt = up_pm_get_freqlock_count(&g_lvlock_0);
  printf("LV freqlock count=%d\n", cnt);
  cnt = up_pm_get_freqlock_count(&g_hvlock_0);
  printf("HV freqlock count=%d\n", cnt);

  printf("Exit freqlock performance test\n");

  return 0;
}

/****************************************************************************
 * freq_test_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int freqlock_test_main(int argc, char *argv[])
#endif
{
  int   i;
  int   opt  = 0;
  int   topt = 0;
  pid_t pid  = -1;
  char  *targ = NULL;
  char  buffer[8];
  char  *param[2];

  if (argc <= 1)
    {
      show_usage(argv[0]);
      return EXIT_FAILURE;
    }

  optind = -1;
  while ((opt = getopt(argc, argv, ":cht:")) != -1)
    {
      switch (opt)
        {
        case 't':
          topt = 1;
          targ = optarg;
          break;

        case 'c':
          show_lockcount();
          return 0;

        case 'h':
        case ':':
        case '?':
          show_usage(argv[0]);
          return EXIT_FAILURE;
        }
    }

  if (topt && targ)
    {
      if (0 == strncmp("random", targ, 5))
        {
          for (i = 0 ; i < FREQLOCK_TEST_TASK_NUM ; i++)
            {
              sprintf(buffer, "%d", i);
              param[0] = buffer;
              param[1] = NULL;
              pid = task_create("FREQLOCK_TEST",
                                FREQLOCK_TEST_TASK_PRIORITY,
                                FREQLOCK_TEST_TASK_STACK_SIZE,
                                (main_t)freqlock_random_test_task,
                                (FAR char * const *)param);
            }
        }
      else if (0 == strncmp("chgvol", targ, 6))
        {
          pid = task_create("FREQLOCK_TEST",
                            FREQLOCK_TEST_TASK_PRIORITY,
                            FREQLOCK_TEST_TASK_STACK_SIZE,
                            (main_t)freqlock_changevoltage_test_task,
                            (FAR char * const *)NULL);
        }
      else if (0 == strncmp("perfmc", targ, 6))
        {
          pid = task_create("FREQLOCK_TEST",
                            FREQLOCK_TEST_TASK_PRIORITY,
                            FREQLOCK_TEST_TASK_STACK_SIZE,
                            (main_t)freqlock_performance_test_task,
                            (FAR char * const *)NULL);
        }
      else
        {
          show_usage(argv[0]);
          return EXIT_FAILURE;
        }
    }

  if (pid < 0)
    {
      printf("task_create() failure.\n");
    }

  return 0;
}
