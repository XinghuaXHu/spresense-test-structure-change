/****************************************************************************
 * test/lte_socket/api/fdset_test.c
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

#ifndef HOST_MAKE
#  include <nuttx/config.h>
#  include <sdk/config.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "apitest.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define FDSET_TEST_FD_MIN         0
#define FDSET_TEST_FD_MAX         9

/* CONFIG_NFILE_DESCRIPTORS + CONFIG_NSOCKET_DESCRIPTORS in NuttX. */

#define FDSET_TEST_CHK_FDSET_SIZE 24

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/* Test No 32-01 */
static int fdset_test_fdsetsize(void)
{
  int size = 0;

  size = FD_SETSIZE;
  printf("FD_SETSIZE = %d\n", size);

  if (FDSET_TEST_CHK_FDSET_SIZE != size)
    {
      return -1;
    }
  return 0;
}

/* Test No 33-01 */
static int fdset_test_fdclr(void)
{
  const int setfd_1 = 1;
  const int setfd_2 = 2;
  const int clrfd = 1;
  const int chkfd = 2;
  fd_set fd;

  printf("START FD_CLR TEST\n");

  printf("set fd value %d and %d\n", setfd_1, setfd_2);
  FD_SET(setfd_1, &fd);
  FD_SET(setfd_2, &fd);

  printf("check set fd value\n");
  if (!FD_ISSET(setfd_1, &fd))
    {
      printf("fd value %d is not setted\n", setfd_1);
      return -1;
    }
  if (!FD_ISSET(setfd_2, &fd))
    {
      printf("fd value %d is not setted\n", setfd_2);
      return -1;
    }

  printf("clr fd value %d\n", setfd_1);
  FD_CLR(clrfd, &fd);

  printf("check set fd value\n");
  if (FD_ISSET(setfd_1, &fd))
    {
      printf("fd value %d clear failure\n", setfd_1);
      return -1;
    }

  if (!FD_ISSET(chkfd, &fd))
    {
      printf("fd value %d is not setted\n", setfd_2);
      return -1;
    }

  printf("FD_CLR TEST SUCCESS\n");
  return 0;
}

/* Test No 34-01 */
static int fdset_test_fdisset(void)
{
  const int setfd_1 = 1;
  const int setfd_2 = 2;
  const int clrfd = 1;
  const int chkfd = 2;
  fd_set fd;

  printf("START FD_ISSET TEST\n");
  printf("set fd value %d and %d\n", setfd_1, setfd_2);

  FD_SET(setfd_1, &fd);
  FD_SET(setfd_2, &fd);

  printf("clr fd value %d\n", clrfd);
  FD_CLR(clrfd, &fd);

  printf("check set fd value %d\n", setfd_2);
  if (!FD_ISSET(chkfd, &fd))
    {
      printf("fd value %d is not setted\n", setfd_2);
      return -1;
    }

  printf("FD_ISSET TEST SUCCESS\n");
  return 0;
}

/* Test No 35-01 */
static int fdset_test_fdset(void)
{
  const int setfd = 3;
  fd_set fd;
  printf("START FD_ISSET TEST\n");
  printf("set fd value %d\n", setfd);
  FD_SET(setfd, &fd);

  printf("check set fd value %d\n", setfd);
  if (!FD_ISSET(setfd, &fd))
    {
      printf("fd value %d is not setted\n", setfd);
      return -1;
    }

  printf("FD_SET TEST SUCCESS\n");
  return 0;
}

/* Test No 36-01 */
static int fdset_test_fdzero(void)
{
  int i;
  fd_set fd;
  printf("START FD_ZERO TEST\n");
  printf("set all fd value(%d - %d)\n",
    FDSET_TEST_FD_MIN, FDSET_TEST_FD_MAX);
  for (i = FDSET_TEST_FD_MIN; i != FDSET_TEST_FD_MAX; i++)
    {
      FD_SET(i, &fd);
    }

  printf("call FD_ZERO\n");
  FD_ZERO(&fd);

  printf("check cleared all fd value\n");
  for (i = FDSET_TEST_FD_MIN; i != FDSET_TEST_FD_MAX; i++)
    {
      if(FD_ISSET(i, &fd))
        {
          printf("fd value %d is not cleared\n", i);
          return -1;
        }
    }

  printf("FD_ZERO TEST SUCCESS\n");
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int apitest_fdset_main(void)
{
  printf("/*-----START FD_SET TEST-----*/\n");
  if (0 > fdset_test_fdsetsize())
    {
      printf("fdset_test_fdsetsize() is failure\n");
      return -1;
    }

  if (0 > fdset_test_fdclr())
    {
      printf("fdset_test_fdclr() is failure\n");
      return -1;
    }

  if (0 > fdset_test_fdisset())
    {
      printf("fdset_test_fdisset() is failure\n");
      return -1;
    }

  if (0 > fdset_test_fdset())
    {
      printf("fdset_test_fdset() is failure\n");
      return -1;
    }

  if (0 < fdset_test_fdzero())
    {
      printf("fdset_test_fdzero() is failure\n");
      return -1;
    }

  printf("/*-----FD_SET TEST ALL SUCCESS^v^-----*/\n");
  return 0;
}