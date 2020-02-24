/****************************************************************************
 * test/sqa/singlefunction/gnss/gnss_test_main.c
 *
 *   Copyright (C) 2016,2017 Sony. All rights reserved.
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

#include <nuttx/config.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <arch/chip/gnss.h>
#include <string.h>
#include "gnss_test_main.h"

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
 * Name: gnss_main()
 *
 * Description:
 *   Set parameters and run positioning.
 *
 * Input Parameters:
 *   argc - Does not use.
 *   argv - Does not use.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int gnss_test_main(int argc, char *argv[])
#endif
{
  int ret;
  uint32_t stmod;

  const char *parameters[] = {
    "utility",      // 0
    "coldstart",    // 1
    "warmstart",    // 2
    "hotstart",     // 3
    "cycle",        // 4
    "hotrepeat",    // 5
    "signalonoff",  // 6
    "satellite",    // 7
    "nmea",         // 8
    "factory",      // 9
    "pvtloglong",   // 10
    "pvtlogstress", // 11
  };

  switch(argparse(parameters, argv[1]))
  {
    case 0:
      ret = gnss_testutility(argc, argv);
      break;

    case 1:
      stmod = CXD56_GNSS_STMOD_COLD;
      ret = gnss_starttest(argc, argv, stmod);
      break;

    case 2:
      stmod = CXD56_GNSS_STMOD_WARM;
      ret = gnss_starttest(argc, argv, stmod);
      break;

    case 3:
      stmod = CXD56_GNSS_STMOD_HOT;
      ret = gnss_starttest(argc, argv, stmod);
      break;

    case 4:
      ret = gnss_cycle(argc, argv);
      break;

    case 5:
      ret = gnss_hotrepeat(argc, argv);
      break;

    case 6:
      ret = gnss_signalonoff(argc, argv);
      break;

    case 7:
      ret = gnss_satellite(argc, argv);
      break;

    case 8:
      ret = gnss_nmea(argc, argv);
      break;

    case 9:
      ret = gnss_factory(argc, argv);
      break;

    case 10:
      ret = gnss_pvtloglong(argc, argv);
      break;

    case 11:
      ret = gnss_pvtlogstress(argc, argv);
      break;

    case ERROR:
    default:
      ret = ERROR;
      break;

  }
  printf("%s %s: %s\n", __func__, argv[1], ret != OK ? "FAILED" : "SUCCESS");

  return ret;
}
