/****************************************************************************
 * test/lte_socket/api/setsockopt_test.c
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
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "apitest.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define SOCKET_FD         0
#define INVALID_FD        -1
#define INVALID_LEVEL     -1
#define INVALID_OPTNAME   -1

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct apitest_setsockopt_s
{
  /* Input parameters */

  FAR const char *test_num;
  int            fd;
  int            level;
  int            optname;
  FAR const void *optval;
  int            optlen;

  /* verify parameters */

  struct apitest_verify_param_s verify_ret;
  bool                          verify_err_flag;
  struct apitest_verify_param_s verify_err;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const int            g_optval_int0       = 0;
static const int            g_optval_int1       = 1;
static const struct linger  g_optval_linger     = { 1, 10 };
static const struct timeval g_optval_tv_enable  = { 10, 10 };
static const struct timeval g_optval_tv_disable = { 0, 0 };

static struct apitest_setsockopt_s g_apitest_setsockopt_param[] =
{
  { "11-01", SOCKET_FD,  SOL_SOCKET,    SO_BROADCAST,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-02", INVALID_FD, SOL_SOCKET,    SO_BROADCAST,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, EBADF        }},
  { "11-03", SOCKET_FD,  SOL_SOCKET,    SO_BROADCAST,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-09", SOCKET_FD,  INVALID_LEVEL, SO_BROADCAST,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, ENOPROTOOPT  }},
  { "11-10", SOCKET_FD,  SOL_SOCKET,    SO_BROADCAST,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-11", SOCKET_FD,  SOL_SOCKET,    SO_REUSEADDR,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-12", SOCKET_FD,  SOL_SOCKET,    SO_KEEPALIVE,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-13", SOCKET_FD,  SOL_SOCKET,    SO_RCVBUF,       &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-14", SOCKET_FD,  SOL_SOCKET,    SO_LINGER,       &g_optval_linger,     sizeof(g_optval_linger),     { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-15", SOCKET_FD,  SOL_SOCKET,    SO_DEBUG,        &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, ENOPROTOOPT  }},
  { "11-16", SOCKET_FD,  SOL_SOCKET,    SO_OOBINLINE,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, ENOPROTOOPT  }},
  { "11-17", SOCKET_FD,  SOL_SOCKET,    SO_SNDBUF,       &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, ENOPROTOOPT  }},
  { "11-18", SOCKET_FD,  SOL_SOCKET,    SO_DONTROUTE,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, ENOPROTOOPT  }},
  { "11-19", SOCKET_FD,  SOL_SOCKET,    SO_RCVLOWAT,     &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, ENOPROTOOPT  }},
  { "11-20", SOCKET_FD,  SOL_SOCKET,    SO_SNDLOWAT,     &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, ENOPROTOOPT  }},
  { "11-21", SOCKET_FD,  SOL_SOCKET,    SO_RCVTIMEO,     &g_optval_tv_enable,  sizeof(g_optval_tv_enable),  { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-22", SOCKET_FD,  SOL_SOCKET,    SO_SNDTIMEO,     &g_optval_tv_enable,  sizeof(g_optval_tv_enable),  { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-23", SOCKET_FD,  SOL_SOCKET,    SO_ACCEPTCONN,   &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, ENOPROTOOPT  }},
  { "11-24", SOCKET_FD,  SOL_SOCKET,    SO_ERROR,        &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, ENOPROTOOPT  }},
  { "11-25", SOCKET_FD,  SOL_SOCKET,    SO_TYPE,         &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, ENOPROTOOPT  }},
  { "11-27", SOCKET_FD,  SOL_SOCKET,    INVALID_OPTNAME, &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, ENOPROTOOPT  }},
  { "11-46", SOCKET_FD,  SOL_SOCKET,    SO_BROADCAST,    NULL,                 sizeof(g_optval_int1),       { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, EINVAL       }},
  { "11-47", SOCKET_FD,  SOL_SOCKET,    SO_BROADCAST,    &g_optval_int0,       sizeof(g_optval_int0),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-48", SOCKET_FD,  SOL_SOCKET,    SO_BROADCAST,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-49", SOCKET_FD,  SOL_SOCKET,    SO_REUSEADDR,    &g_optval_int0,       sizeof(g_optval_int0),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-50", SOCKET_FD,  SOL_SOCKET,    SO_REUSEADDR,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-51", SOCKET_FD,  SOL_SOCKET,    SO_KEEPALIVE,    &g_optval_int0,       sizeof(g_optval_int0),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-52", SOCKET_FD,  SOL_SOCKET,    SO_KEEPALIVE,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-53", SOCKET_FD,  SOL_SOCKET,    SO_RCVBUF,       &g_optval_int0,       sizeof(g_optval_int0),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-54", SOCKET_FD,  SOL_SOCKET,    SO_RCVBUF,       &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-57", SOCKET_FD,  SOL_SOCKET,    SO_LINGER,       &g_optval_linger,     sizeof(g_optval_linger),     { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-58", SOCKET_FD,  SOL_SOCKET,    SO_RCVTIMEO,     &g_optval_tv_enable,  sizeof(g_optval_tv_enable),  { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-59", SOCKET_FD,  SOL_SOCKET,    SO_RCVTIMEO,     &g_optval_tv_disable, sizeof(g_optval_tv_disable), { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-60", SOCKET_FD,  SOL_SOCKET,    SO_SNDTIMEO,     &g_optval_tv_enable,  sizeof(g_optval_tv_enable),  { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-61", SOCKET_FD,  SOL_SOCKET,    SO_SNDTIMEO,     &g_optval_tv_disable, sizeof(g_optval_tv_disable), { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-91", SOCKET_FD,  SOL_SOCKET,    SO_BROADCAST,    &g_optval_int1,       sizeof(g_optval_int1),       { VERIFY_EQ, 0  }, false, { VERIFY_EQ, DONTCARE_VAL }},
  { "11-92", SOCKET_FD,  SOL_SOCKET,    SO_BROADCAST,    &g_optval_int1,       sizeof(g_optval_int1) - 1,   { VERIFY_EQ, -1 }, true,  { VERIFY_EQ, EINVAL       }},
};

static int g_setsockopt_fd = -1;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * setup_setsockopt
 ****************************************************************************/

static int setup_setsockopt(void)
{
  g_setsockopt_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (g_setsockopt_fd < 0)
    {
      printf("socket() error: %d\n", errno);
      return -1;
    }
  printf("socket success: sockfd = %d\n", g_setsockopt_fd);

  return 0;
}

/****************************************************************************
 * teardown_setsockopt
 ****************************************************************************/

static int teardown_setsockopt(void)
{
  if (g_setsockopt_fd != -1)
    {
      close(g_setsockopt_fd);
      g_setsockopt_fd = -1;
    }

  return 0;
}

/****************************************************************************
 * get_setsockopt_fd
 ****************************************************************************/

static int get_setsockopt_fd(int fd)
{
  int ret = -1;

  if (fd != INVALID_FD)
    {
      ret = g_setsockopt_fd;
    }

  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int apitest_setsockopt_main(void)
{
  int i;
  int ret;
  int result = 0;
  int err = 0;

  printf("+++++++++++++++++++++setsockopt test start+++++++++++++++++++++\n");

  for (i = 0; i < TABLE_NUM(g_apitest_setsockopt_param); i++)
    {
      setup_setsockopt();

      ret = setsockopt(get_setsockopt_fd(g_apitest_setsockopt_param[i].fd),
                       g_apitest_setsockopt_param[i].level,
                       g_apitest_setsockopt_param[i].optname,
                       g_apitest_setsockopt_param[i].optval,
                       g_apitest_setsockopt_param[i].optlen);

      err = errno;
      /* Check return code */

      switch(g_apitest_setsockopt_param[i].verify_ret.verify_flag)
        {
          case VERIFY_EQ:
            result = apitest_verify_eq(ret, &g_apitest_setsockopt_param[i].verify_ret);
            break;

          case VERIFY_NE:
            result = apitest_verify_ne(ret, &g_apitest_setsockopt_param[i].verify_ret);
            break;

          case VERIFY_LT:
            result = apitest_verify_lt(ret, &g_apitest_setsockopt_param[i].verify_ret);
            break;

          case VERIFY_LE:
            result = apitest_verify_le(ret, &g_apitest_setsockopt_param[i].verify_ret);
            break;

          case VERIFY_GT:
            result = apitest_verify_gt(ret, &g_apitest_setsockopt_param[i].verify_ret);
            break;

          case VERIFY_GE:
            result = apitest_verify_ge(ret, &g_apitest_setsockopt_param[i].verify_ret);
            break;

          default:
            printf("Unexpected verify_flag: %d\n",
                   g_apitest_setsockopt_param[i].verify_ret.verify_flag);
            result = -1;
            break;
        }

      if ((result == 0) && (g_apitest_setsockopt_param[i].verify_err_flag))
        {
          /* Check errno */

          switch(g_apitest_setsockopt_param[i].verify_err.verify_flag)
            {
              case VERIFY_EQ:
                result = apitest_verify_eq(err, &g_apitest_setsockopt_param[i].verify_err);
                break;

              default:
                printf("Unexpected verify_flag: %d\n",
                       g_apitest_setsockopt_param[i].verify_err.verify_flag);
                result = -1;
                break;
            }
        }

      if (-1 == result)
        {
          printf("[%s] error\n", g_apitest_setsockopt_param[i].test_num);
        }
      teardown_setsockopt();
    }

  printf("+++++++++++++++++++++setsockopt test end+++++++++++++++++++++\n");
  return result;
}
