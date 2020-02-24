/****************************************************************************
 * sqa/singlefunction/pmic/pmic_main.c
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

#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <nuttx/power/battery_charger.h>
#include <nuttx/power/battery_gauge.h>
#include <nuttx/power/battery_ioctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

int cxd56_pmic_read(uint8_t addr, void *buf, uint32_t size);
int cxd56_pmic_write(uint8_t addr, void *buf, uint32_t size);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct {
  const char *str;
  int target;
} list[] = {
  { "DDC_IO",       POWER_DDC_IO },
  { "LDO_EMMC",     POWER_LDO_EMMC },
  { "DDC_ANA",      POWER_DDC_ANA },
  { "LDO_ANA",      POWER_LDO_ANA },
  { "DDC_CORE",     POWER_DDC_CORE },
  { "LDO_PERI",     POWER_LDO_PERI },
  { "LSW2",         PMIC_LSW(2) },
  { "LSW3",         PMIC_LSW(3) },
  { "LSW4",         PMIC_LSW(4) },
  { "GPO0",         PMIC_GPO(0) },
  { "GPO1",         PMIC_GPO(1) },
  { "GPO2",         PMIC_GPO(2) },
  { "GPO3",         PMIC_GPO(3) },
  { "GPO4",         PMIC_GPO(4) },
  { "GPO5",         PMIC_GPO(5) },
  { "GPO6",         PMIC_GPO(6) },
  { "GPO7",         PMIC_GPO(7) },
#if defined(CONFIG_BOARD_SPRESENSE)
  { "AUDIO_DVDD",   POWER_AUDIO_DVDD },
  { "FLASH",        POWER_FLASH },
  { "TCXO",         POWER_TCXO },
  { "LNA",          POWER_LNA },
  { "AUDIO_AVDD",   POWER_AUDIO_AVDD },
  { "AUDIO_MUTE",   POWER_AUDIO_MUTE },
  { "IMAGE_SENSOR", POWER_IMAGE_SENSOR },
#elif defined(CONFIG_BOARD_CORVO)
  { "AUDIO_DVDD",   POWER_AUDIO_DVDD },
  { "FLASH",        POWER_FLASH },
  { "TCXO",         POWER_TCXO },
  { "LNA",          POWER_LNA },
  { "AUDIO_AVDD",   POWER_AUDIO_AVDD },
  { "SENSOR_18V",   POWER_SENSOR_18V },
  { "SENSOR_33V",   POWER_SENSOR_33V },
  { "BMI160",       POWER_BMI160 },
  { "SENSOR",       POWER_SENSOR },
  { "BTBLE",        POWER_BTBLE },
  { "EINK",         POWER_EINK },
  { "EMMC",         POWER_EMMC },
  { "LFOUR",        POWER_LFOUR },
  { "LTE",          POWER_LTE },
  { "IMAGE_SENSOR", POWER_IMAGE_SENSOR },
#elif defined(CONFIG_BOARD_COLLET)
  { "AUDIO_DVDD",   POWER_AUDIO_DVDD },
  { "FLASH",        POWER_FLASH },
  { "TCXO",         POWER_TCXO },
  { "LNA",          POWER_LNA },
  { "AUDIO_AVDD",   POWER_AUDIO_AVDD },
  { "SENSOR_18V",   POWER_SENSOR_18V },
  { "SENSOR_33V",   POWER_SENSOR_33V },
  { "BMI160",       POWER_BMI160 },
  { "SENSOR",       POWER_SENSOR },
  { "BTBLE",        POWER_BTBLE },
  { "EINK",         POWER_EINK },
  { "LFOUR",        POWER_LFOUR },
  { "LTE",          POWER_LTE },
#endif
};

static int g_intnum = 0;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifdef CONFIG_CXD56_PMIC_INT
static void pmic_notify_alarm(void *arg)
{
  printf("NOTIFY: ALARM\n");
  g_intnum++;
}

static void pmic_notify_wkupl(void *arg)
{
  printf("NOTIFY: WKUPL\n");
  g_intnum++;
}

static void pmic_notify_wkups(void *arg)
{
  printf("NOTIFY: WKUPS\n");
  g_intnum++;
}

static void pmic_notify_lowbatt(void *arg)
{
  printf("NOTIFY: LOWBATT\n");
  g_intnum++;
}
#endif /* CONFIG_CXD56_PMIC_INT */

#ifdef CONFIG_CXD5247_GAUGE
static void lowbatt_alarm_handler(int signo, siginfo_t *info, void *ctx)
{
  printf("Low Batt!\n");
}

static int set_lowbatt_voltage(int mvolt)
{
  int ret;
  sigset_t sigset;
  struct sigaction act;
  struct cxd5247_lowbatt_notify_s notify;

  int signo = 10;

  int fd = open("/dev/gauge", O_RDOK | O_WROK);

  /* signal setting */
  if ((ret = sigemptyset(&sigset)) < 0)
    {
      printf("sigemptyset failed. %d\n", ret);
      return ret;
    }

  if ((ret = sigaddset(&sigset, signo)) < 0)
    {
      printf("sigaddset failed. %d\n", ret);
      return ret;
    }

  memset(&act, 0, sizeof(act));
  act.sa_sigaction  = lowbatt_alarm_handler;
  act.sa_mask       = sigset;
  act.sa_flags      = SA_SIGINFO;

  if ((ret = sigaction(signo, &act, NULL)) < 0)
    {
      printf("sigaction failed. %d\n", ret);
      return ret;
    }

  notify.voltage = mvolt;
  notify.signo   = signo;

  ret = ioctl(fd, BATIOC_SET_LOWBATT, (uintptr_t)&notify);

  return ret;
}
#endif /* CONFIG_CXD5247_GAUGE */

static int getid(const char* str)
{
  int i;
  for (i = 0; i < sizeof(list)/sizeof(list[0]); i++)
    {
      if (!strncmp(str, list[i].str, strlen(list[i].str)))
        {
          return list[i].target;
        }
    }
  return 0;
}

static void show_usage(FAR const char *progname)
{
  printf("\nUsage: %s [-e <target>] [-d <target>] [-i <num>]", progname);
  printf("\n            [-l <mv>] [-h]");
  printf("\n            [-r <addr>] [-w <addr> -v <value>]\n\n");
  printf("Description:\n");
  printf(" Show the power status of each target device\n");
  printf("Options:\n");
  printf(" -e <target>: Enable power to the target\n");
  printf(" -d <target>: Disable power to the target\n");
  printf(" -i <num>: Receive the specified number of interrupts\n");
  printf(" -l <mv>: Set low battery notification at milli-volts\n");
  printf(" -r <addr>: Single read from <addr>\n");
  printf(" -w <addr> -v <value>: Single write <value> to <addr>\n");
  printf(" -h: Show this message\n");
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * pmic_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int pmic_main(int argc, char *argv[])
#endif
{
  int ret;
  int i;
  int opt;
  bool en;
  int id;
  uint8_t addr = 0;
  uint8_t value = 0;
  int rx = 0;
  int tx = 0;
  int txval = 0;
  int intnum = 0;
  int voltage = 0;

  optind = -1;
  while ((opt = getopt(argc, argv, "e:d:i:l:r:w:v:h")) != -1)
    {
      switch (opt)
        {
        case 'e':
          en = true;
          goto skip;
        case 'd':
          en = false;
        skip:
          id = getid(optarg);
          if (id)
            {
              printf("%s: %s\n", (en) ? "Enable ": "Disable", optarg);
              board_power_control(id, en);
            }
          else
            {
              printf("Invalid name: %s\n", optarg);
              return EXIT_FAILURE;
            }
          break;
        case 'r':
          rx = 1;
          addr = strtoul(optarg, NULL, 16);
          break;
        case 'w':
          tx = 1;
          addr = strtoul(optarg, NULL, 16);
          break;
        case 'v':
          txval = 1;
          value = strtoul(optarg, NULL, 16);
          break;
        case 'i':
          intnum = atoi(optarg);
          break;
        case 'l':
          voltage = atoi(optarg);
          break;
        case 'h':
        case ':':
        case '?':
          show_usage(argv[0]);
          return EXIT_FAILURE;
        }
    }

  /* interrupt operation */

  if (intnum)
    {
      up_pmic_set_notify(PMIC_NOTIFY_ALARM, pmic_notify_alarm);
      up_pmic_set_notify(PMIC_NOTIFY_WKUPL, pmic_notify_wkupl);
      up_pmic_set_notify(PMIC_NOTIFY_WKUPS, pmic_notify_wkups);
      up_pmic_set_notify(PMIC_NOTIFY_LOWBATT, pmic_notify_lowbatt);
      while (g_intnum < intnum)
        {
          sleep(1);
        }
    }

  if (voltage)
    {
#ifdef CONFIG_CXD5247_GAUGE
      set_lowbatt_voltage(voltage);
#endif /* CONFIG_CXD5247_GAUGE */
    }

  /* read operation */

  if (rx)
    {
      ret = cxd56_pmic_read(addr, &value, sizeof(value));
      if (ret)
        {
          printf("@[%02x]=>read error!\n", addr);
        }
      else
        {
          printf("@[%02x]=>%02x\n", addr, value);
        }
      return ret;
    }

  /* write operation */

  if (tx && txval)
    {
      ret = cxd56_pmic_write(addr, &value, sizeof(value));
      if (ret)
        {
          printf("@[%02x]<=write error!\n", addr);
        }
      else
        {
          printf("@[%02x]<=%02x\n", addr, value);
        }
      return ret;
    }

  /* show power status */

  printf("\n%16s : %s\n", "Target Name", "on/off");
  for (i = 0; i < sizeof(list)/sizeof(list[0]); i++)
    {
      en = board_power_monitor(list[i].target);
      printf("%16s : %s\n", list[i].str, (en) ? "on" : "off");
    }

  return 0;
}
