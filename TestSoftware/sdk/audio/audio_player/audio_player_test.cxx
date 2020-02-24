/****************************************************************************
 * test/sqa/singlefunction/audio_player/audio_player_test.c
 *
 *   Copyright (C) 2018 Sony Corporation
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "system/readline.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Public Type Declarations
 ****************************************************************************/

typedef int (*player_func)(int no);

struct player_cmd_s
{
  const char   *cmd;       /* The command text */
  player_func  func;       /* Pointer to command handler */
  const int    no;         /* Test number */
  const char   *help;      /* The help text */
};
typedef struct player_cmd_s player_cmd_t;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

extern int player_test(int no);

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int cmd_help(int no);
static int cmd_quit(int no);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static player_cmd_t g_player_cmds[] =
{
    { "h",     cmd_help,    -1,    "Display help for commands. ex)player> h" },
    { "help",  cmd_help,    -1,    "Display help for commands. ex)player> help" },
    { "1",     player_test,  0,    "Excute spr_sdk_14928.      ex)player> 1" },
    { "2",     player_test,  1,    "Excute spr_sdk_14929.      ex)player> 2" },
    { "3",     player_test,  2,    "Excute spr_sdk_15299.      ex)player> 3" },
    { "4",     player_test,  3,    "Excute spr_sdk_15454.      ex)player> 4" },
    { "5",     player_test,  4,    "Excute spr_sdk_15455.      ex)player> 5" },
    { "6",     player_test,  5,    "Excute spr_sdk_15317.      ex)player> 6" },
    { "7",     player_test,  6,    "Excute spr_sdk_15676.      ex)player> 7" },
    { "8",     player_test,  7,    "Excute spr_sdk_15451.      ex)player> 8" },
    { "q",     cmd_quit,    -1,    "Exit player test.          ex)player> q" },
    { "quit",  cmd_quit,    -1,    "Exit player test.          ex)player> quit" }
};
static const int g_player_cmd_count = sizeof(g_player_cmds) / sizeof(player_cmd_t);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int cmd_help(int no)
{
  int len;
  int maxlen = 0;

  if (no >= 0)
    {
      return -1;
    }

  for (int i = 0; i < g_player_cmd_count; i++)
    {
      len = strlen(g_player_cmds[i].cmd);
      if (len > maxlen)
        {
          maxlen = len;
        }
    }

  printf("AudioPlayerTest commands\n");
  printf("===Commands======+=Description=================\n");
  for (int i = 0; i < g_player_cmd_count; i++)
    {
      printf("  %s", g_player_cmds[i].cmd);
      len = maxlen - strlen(g_player_cmds[i].cmd);
      for (int j = 0; j < len; j++)
        {
          printf(" ");
        }
      printf("          : %s\n", g_player_cmds[i].help);
    }
  printf("===============================================\n");
  return 0;
}

static int cmd_quit(int no)
{
  if (no >= 0)
    {
      return -1;
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
extern "C" int main(int argc, FAR char *argv[])
#else
extern "C" int player_test_main(int argc, char *argv[])
#endif
{
  bool run_state;
  int  len;
  int  i;
  char buffer[64];

  printf("----Start AudioPlayer SQA test-------\n");

  run_state = true;
  while(run_state)
    {
      printf("player> ");
      fflush(stdout);

      len = readline(buffer, sizeof(buffer), stdin, stdout);
      buffer[len] = '\0';
      if (len > 0)
        {
          if (buffer[len-1] == '\n')
            {
              buffer[len-1] = '\0';
            }

          for (i = 0; i < g_player_cmd_count; i++)
            {
              if (strcmp(buffer, g_player_cmds[i].cmd) == 0)
                {
                  if (g_player_cmds[i].func != NULL)
                    {
                      g_player_cmds[i].func(g_player_cmds[i].no);
                    }
                  if (g_player_cmds[i].func == cmd_quit)
                    {
                      run_state = false;
                    }
                  break;
                }
            }

          if (i == g_player_cmd_count)
            {
              printf("%s:  unknown player command\n", buffer);
            }
        }
    }

  printf("----Exit AudioPlayer SQA test--------\n");

  return 0;
}
