/****************************************************************************
 *
 *   Copyright 2018 Sony Semiconductor Solutions Corporation
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sdk/config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <arch/board/board.h>
#include <arch/chip/pm.h>
#include <arch/chip/cxd56_audio.h>

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Below is the same structure in:
 * - SpritzerPty/drivers/peripheral/aca/aca_drv.h
 * - spresense/sdk/bsp/src/audio/cxd56_audio_aca.c
 */
typedef struct
{
  uint32_t bank;
  uint32_t addr;
  uint32_t value;
} asAcaPulcoRegParam;

CXD56_AUDIO_ECODE cxd56_audio_analog_poweron(void);
CXD56_AUDIO_ECODE cxd56_audio_aca_read_reg(asAcaPulcoRegParam *param);
CXD56_AUDIO_ECODE cxd56_audio_aca_write_reg(asAcaPulcoRegParam *param);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void show_usage(FAR const char *progname)
{
  printf("\nUsage: %s [-h]\n", progname);
  printf("           [-b <bank>] [-r <addr>] [-w <addr> -v <value>]\n\n");
  printf("Description:\n");
  printf(" Aca utility tool\n");
  printf("Options:\n");
  printf(" -b <bank>: Bank number (default: 0)\n");
  printf(" -r <addr>: Single read from <addr>\n");
  printf(" -w <addr> -v <value>: Single write <value> to <addr>\n");
  printf(" -h: Show this message\n");
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * aca_main
 ****************************************************************************/

#include <sdk/config.h>
#include <stdio.h>

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int aca_main(int argc, char *argv[])
#endif
{
  int ret;
  int opt;
  uint8_t addr = 0;
  uint8_t value = 0;
  int rx = 0;
  int tx = 0;
  int txval = 0;
  int bank = 0;
  asAcaPulcoRegParam param;

  optind = -1;
  while ((opt = getopt(argc, argv, "r:w:v:b:h")) != -1)
    {
      switch (opt)
        {
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
          case 'b':
            bank = strtoul(optarg, NULL, 16);
            break;
          case 'h':
          case ':':
          case '?':
            show_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

  /* No operation */

  if (!tx && !rx)
    {
      show_usage(argv[0]);
      return EXIT_FAILURE;
    }

  /* read operation */

  if (rx)
    {
      cxd56_audio_analog_poweron();
      param.bank = bank;
      param.addr = addr;
      param.value = 0;
      ret = cxd56_audio_aca_read_reg(&param);
      value = param.value;

      if (ret)
        {
          printf("@(%d)[%02x]=>read error!\n", bank, addr);
        }
      else
        {
          printf("@(%d)[%02x]=>%02x\n", bank, addr, value);
        }
      return ret;
    }

  /* write operation */

  if (tx && txval)
    {
      cxd56_audio_analog_poweron();
      param.bank = bank;
      param.addr = addr;
      param.value = value;
      ret = cxd56_audio_aca_write_reg(&param);

      if (ret)
        {
          printf("@(%d)[%02x]<=write error!\n", bank, addr);
        }
      else
        {
          printf("@(%d)[%02x]<=%02x\n", bank, addr, value);
        }
      return ret;
    }

  return 0;
}
