/****************************************************************************
 * audio_recognizer/audio_recognizer_main.cxx
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
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <asmp/mpshm.h>
#include <sys/stat.h>

#include "memutils/os_utils/chateau_osal.h"
#include "memutils/simple_fifo/CMN_SimpleFifo.h"
#include "memutils/memory_manager/MemHandle.h"
#include "memutils/message/Message.h"
#include "audio/audio_high_level_api.h"
#include <audio/utilities/wav_containerformat.h>
#include "include/msgq_id.h"
#include "include/mem_layout.h"
#include "include/memory_layout.h"
#include "include/msgq_pool.h"
#include "include/pool_layout.h"
#include "include/fixed_fence.h"
#include "rcgproc_command.h"
#ifdef CONFIG_SQAT_AUDIO_RECOG_USEPREPROC
#include "userproc_command.h"
#endif /* CONFIG_SQAT_AUDIO_RECOG_USEPREPROC */

#include <arch/chip/cxd56_audio.h>
#include <audio_recog_def.h>
#include <audio_recog_proto.h>
#include <audio_recog_struct.h>

#define _USE_THREAD

#ifdef _USE_THREAD
static pthread_t monitor_th_tid;
#endif
#define DEBUG_LEVEL 2 

//#define  _TEST_NO_CREATE_1_ /* State Trans Error Check for RecognizerStatus */ 
//#define _TEST_DBG_
//#define _USE_CMDS_ITEM /* to dsplay with info of g_recog_cmds[].item */ 

using namespace MemMgrLite;

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define RECFILE_ROOTPATH "/mnt/sd0/REC"
#define DSPBIN_PATH "/mnt/sd0/BIN"

/* Use microphone channel number.
 *   [Analog microphone]
 *       Maximum number: 4
 *       The channel number is 1/2/4
 *   [Digital microphone]
 *       Maximum number: 8
 *       The channel number is 1/2/4/6/8
 */

/* Recognizing time(sec). */

#define RECOGNIZER_EXEC_TIME 10
#define TEST_TH_STACKSIZE 2048
#define TEST_TH_PRIORITY  SCHED_PRIORITY_DEFAULT

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/
//static int cmd_help(int no);
//static int cmd_quit(int no);

#ifdef _USE_THREAD
static int set_thread(recog_info_t *);
static FAR void test_monitor(FAR void *);
#endif
#define message(format, ...)    printf(format, ##__VA_ARGS__)
#define err(format, ...)        fprintf(stderr, format, ##__VA_ARGS__)
#define pr_debug(dbg,format, ...)   {if(dbg>=DEBUG_LEVEL) printf(format, ##__VA_ARGS__);}


/****************************************************************************
 * Private Data
 ****************************************************************************/

recog_info_t recog_info;

static recog_cmd_t g_recog_cmds[] =
{
    { "0", CRE_SMODE_0 },
    { "1", CRE_SMODE_1 },
    { "2", CRE_SMODE_2 },
    { "1_1_1", CRE_SMODE_1_1_1 },
    { "1_1_2_1", CRE_SMODE_1_1_2_1 },
    { "1_1_2_2", CRE_SMODE_1_1_2_2 },
    { "1_1_2_3", CRE_SMODE_1_1_2_3 },
    { "1_1_3", CRE_SMODE_1_1_3 },
    { "1_2_1", CRE_SMODE_1_2_1 },
    { "1_2_2", CRE_SMODE_1_2_2 },
    { "2_1_2", CRE_SMODE_2_1_2 },
    { "2_1_3", CRE_SMODE_2_1_3 },
    { "2_2_1", CRE_SMODE_2_2_1 },
    { "2_2_1_2", CRE_SMODE_2_2_1_2 },
    { "2_2_2", CRE_SMODE_2_2_2 },
    { "3_1_2", CRE_SMODE_3_1_2 },
    { "3_2_2", CRE_SMODE_3_2_2 },
    { "3_3_2", CRE_SMODE_3_3_2 },
    { "3_4_1", CRE_SMODE_3_4_1 },
    { "3_4_2", CRE_SMODE_3_4_2 },
    { "3_4_3", CRE_SMODE_3_4_3 }, /* invalid item */
};
static const int g_recog_cmd_count = sizeof(g_recog_cmds) / sizeof(recog_cmd_t);

#if 0
/*-------------------------------------------------------
    a command to show help  
 -------------------------------------------------------*/
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

/*-------------------------------------------------------
    a command to quit  
 -------------------------------------------------------*/
static int cmd_quit(int no)
{
  if (no >= 0)
    {
      return -1;
    }

  return 0;
}

static int cmd_help(int no);
static int cmd_quit(int no);
#endif


/****************************************************************************
 * Private Type Declarations
 ****************************************************************************/
#ifdef _USE_THREAD
/****************************************************************************
 * init_for_signal
 ****************************************************************************/
static int init_for_signal(recog_info_t *data)
{
  sigset_t *tsig;
  int ret;

  pr_debug(2, "==> %s \n",__func__);

  tsig = &(data->th_sig_d);

  pr_debug(3, "%s pid:%x \n",__func__, getpid());
  data->my_pid = getpid();

  sigemptyset(tsig);

  sigaddset(tsig, THREAD_SIG_NO);     /*  for pthread */
  sigaddset(tsig, THREAD_SIG_NO_ERR); /*  for pthread(err) */
  sigaddset(tsig, THREAD_SIG_ACT);    /*  for pthread(test) */
  ret = sigprocmask(SIG_UNBLOCK, tsig, NULL);
  if(ret){
     err("## Error sigprocmask :%d \n",ret);
     pr_debug(3, "<== %s error \n",__func__);
     return NG;
  }
  pr_debug(2, "<== %s ok \n",__func__);
  return 0;
}
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* For share memory. */

static mpshm_t s_shm;

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static bool printAudCmdResult(uint8_t command_code, AudioResult& result)
{
  pr_debug(1, "==> %s \n",__func__);

  if (AUDRLT_ERRORRESPONSE == result.header.result_code) {
    printf("Command code(0x%x): AUDRLT_ERRORRESPONSE:"
           "Module id(0x%x): Error code(0x%x)\n",
            command_code,
            result.error_response_param.module_id,
            result.error_response_param.error_code);

    pr_debug(3, "<== %s error 1 \n",__func__);
    return false;
  }
  else if (AUDRLT_ERRORATTENTION == result.header.result_code) {
    printf("Command code(0x%x): AUDRLT_ERRORATTENTION\n", command_code);
    pr_debug(3, "<== %s error 2 \n",__func__);
    return false;
  }
  else if (AUDRLT_NOTIFYSTATUS == result.header.result_code) {
   printf("NOTIFYSTATUS: substatus:%x status:%x \n",result.notify_status.sub_status_info
, result.notify_status.status_info); 
  }
  
  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}

/* Callback at Creating  */
static void app_attention_callback(const ErrorAttentionParam *attparam)
{
  union sigval value;
  int ret;
 
  pr_debug(2, "==> %s \n",__func__);
  
  printf("Attention!! %s L%d ecode %d subcode %d\n",
          attparam->error_filename,
          attparam->line_number,
          attparam->error_code,
          attparam->error_att_sub_code);

#if 1 
  /* setting for signal */
  value.sival_int = AUD_CL_01_ERR;
  ret = sigqueue(getpid(), THREAD_SIG_NO_ERR, value);
  if(ret<0)
    {
      pr_debug(3, "sigqueue error(ng):%x at %s \n",ret, __func__);
    }
  else
    {
       pr_debug(3, "sigqueue sent(ok) at %s \n", __func__);
    }
#endif
  pr_debug(2, "<== %s ok \n",__func__);
}

static void recognizer_find_callback(AsRecognitionInfo info)
{
  pr_debug(2, "==> %s \n",__func__);
  printf("app:recognizer_find_callback size %d\n", info.size);

  int16_t *param = (int16_t *)info.mh.getVa();

  printf(">> %d %d %d %d\n", param[0], param[1], param[2], param[3]); 
  pr_debug(2, "<== %s ok \n",__func__);
}


/*  */
static int set_sigque(int val, int mode)
{
  bool result = false;
  union sigval value;

  pr_debug(2, "==> %s \n",__func__);

  /* setting for signal */
  value.sival_int = val;

  result = sigqueue(getpid(), mode, value);
  if(result<0)
    {
      pr_debug(3, "sigqueue error(ng):%x for debug \n",result);
    }
  else
    {
      pr_debug(3, "sigqueue sent(ok) value:%d   \n", value);
    }

  pr_debug(2, "<== %s ok \n",__func__);
  return 1;

}
static bool set_delay_stop_recognizer(void)
{
  int ret;
 
  pr_debug(2, "==> %s \n",__func__);

  /* setting for signal */
  ret = set_sigque(AUD_CL_TEST3_4_1, THREAD_SIG_ACT);
  if(ret <0)
    {
      pr_debug(2, "<== %s err \n",__func__);
    }
  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}
/*-----------------------------------------------
  
 -----------------------------------------------*/

/*
  SetCreateAudioSubSystem

  Call AS_CreateAudioManager with some parameters
*/
static bool SetCreateAudioSubSystem(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  /* Create manager of AudioSubSystem. */
  AudioSubSystemIDs ids;
  ids.app         = MSGQ_AUD_APP;
  ids.mng         = MSGQ_AUD_MNG;
  ids.player_main = 0xFF;
  ids.player_sub  = 0xFF;
  ids.micfrontend = MSGQ_AUD_FRONTEND;
  ids.mixer       = 0xFF;
  ids.recorder    = 0xFF;
  ids.effector    = 0xFF;
  ids.recognizer  = MSGQ_AUD_RECOGNIZER;
  result = AS_CreateAudioManager(ids, app_attention_callback);
  if(result != 0)
  {
    printf("## Error at the API:AS_CreateAudioManager rslt:%d \n",result);
    pr_debug(3, "<== %s err 1 \n",__func__);
    return false;
  }
  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}

/*
  SetDeleteAudioManager

  Call AS_DeleteAudioManager
*/
static bool SetDeleteAudioManager(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  result = AS_DeleteAudioManager();
  if(result != 0)
  {
    printf("## Error at the API:AS_DeleteAudioManager rslt:%d \n",result);
    pr_debug(3, "<== %s err 2 \n",__func__);
    return false;
  }
  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}

/*
  SetCreateMicFrontend

  Call AS_CreateMicFrontend with some parameters
*/
static bool SetCreateMicFrontend(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  AsCreateMicFrontendParams_t frontend_create_param;
  frontend_create_param.msgq_id.micfrontend = MSGQ_AUD_FRONTEND;
  frontend_create_param.msgq_id.mng         = MSGQ_AUD_MNG;
  frontend_create_param.msgq_id.dsp         = MSGQ_AUD_PREDSP;
  frontend_create_param.pool_id.input       = S0_MIC_IN_BUF_POOL;
#ifdef CONFIG_SQAT_AUDIO_RECOG_USEPREPROC
  frontend_create_param.pool_id.output      = S0_PREPROC_BUF_POOL;
#else
  frontend_create_param.pool_id.output      = S0_NULL_POOL;
#endif
  frontend_create_param.pool_id.dsp         = S0_PRE_APU_CMD_POOL;

  result = AS_CreateMicFrontend(&frontend_create_param, NULL);
  if(!result)
  {
    printf("## Error at the API:AS_CreateMicFrontend rslt:%d \n",result);
    pr_debug(3, "<== %s err 2 \n",__func__);
    return false;
  }
  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}

/*
  SetDeleteMicFrontend

  Call AS_DeleteMicFrontend
*/
static bool SetDeleteMicFrontend(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  result = AS_DeleteMicFrontend();
  if(!result)
  {
    printf("## Error at the API:AS_DeleteMicFrontend rslt:%d \n",result);
    pr_debug(3, "<== %s err 2 \n",__func__);
    return false;
  }
  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}

/*
  SetCreateRecognizer

  Call AS_CreateRecognizer with some parameters
*/
static bool SetCreateRecognizer(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  AsCreateRecognizerParam_t recognizer_create_param;
  recognizer_create_param.msgq_id.recognizer = MSGQ_AUD_RECOGNIZER;
  recognizer_create_param.msgq_id.mng        = MSGQ_AUD_MNG;
  recognizer_create_param.msgq_id.dsp        = MSGQ_AUD_RCGDSP;
  recognizer_create_param.pool_id.out        = S0_OUTPUT_BUF_POOL;
  recognizer_create_param.pool_id.dsp        = S0_RCG_APU_CMD_POOL;

  result = AS_CreateRecognizer(&recognizer_create_param, NULL);
  if (!result)
    {
      printf("Error: AS_CreateRecognizer() failure. system memory insufficient!\n");
      pr_debug(3, "<== %s err 2 \n",__func__);
      return false;
    }
  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}

/*
  SetDeleteRecognizer

  Call AS_DeleteRecognizer
*/
static bool SetDeleteRecognizer(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  result = AS_DeleteRecognizer();
  if(!result)
  {
    printf("## Error at the API:AS_DeleteRecognizer rslt:%d \n",result);
    pr_debug(3, "<== %s err 2 \n",__func__);
    return false;
  }
  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}

/*
  SetCreateCapture

  Call AS_CreateCapture with some parameters
*/
static bool SetCreateCapture(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  AsCreateCaptureParam_t capture_create_param;
  capture_create_param.msgq_id.dev0_req  = MSGQ_AUD_CAP;
  capture_create_param.msgq_id.dev0_sync = MSGQ_AUD_CAP_SYNC;
  capture_create_param.msgq_id.dev1_req  = 0xFF;
  capture_create_param.msgq_id.dev1_sync = 0xFF;

  result = AS_CreateCapture(&capture_create_param);
  if (!result)
    {
      printf("Error: As_CreateCapture() failure. system memory insufficient!\n");
      pr_debug(3, "<== %s err 3 \n",__func__);
      return false;
    }
  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}

/*
  SetDeleteCapture

  Call AS_DeleteCapture
*/
static bool SetDeleteCapture(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  result = AS_DeleteCapture();
  if(!result)
  {
    printf("## Error at the API:AS_DeleteCapture rslt:%d \n",result);
    pr_debug(3, "<== %s err 2 \n",__func__);
    return false;
  }
  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}

/*--------------------------------------------------
   app_create_audio_sub_system_1_1_1

   basic case from origin
 -------------------------------------------------*/
static bool app_create_audio_sub_system_1_1_1(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  pr_debug(2, "AS_Create**  1st \n");

  result = SetCreateAudioSubSystem();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateAudioManager OK \n");

  /* Create Frontend. */
  result = SetCreateMicFrontend();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateMicFrontend OK \n");

  /* Create Capture feature. */
  result = SetCreateRecognizer();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateRecognizer OK \n");


  /* Create Capture feature. */
  result = SetCreateCapture();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateCapture OK \n");

  pr_debug(2, "<== %s OK \n",__func__);
  return true;
}

#if 0
static bool app_create_audio_sub_system_e2(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  /* Create manager of AudioSubSystem. */
  AudioSubSystemIDs ids;
  ids.app         = MSGQ_AUD_APP;
  ids.mng         = MSGQ_AUD_MNG;
  ids.player_main = 0xFF;
  ids.player_sub  = 0xFF;
  ids.micfrontend = MSGQ_AUD_FRONTEND;
  ids.mixer       = 0xFF;
  ids.recorder    = 0xFF;
  ids.effector    = 0xFF;
  ids.recognizer  = MSGQ_AUD_RECOGNIZER;
  result = AS_CreateAudioManager(ids, app_attention_callback);
  if(result != 0)
  {
    printf("## Error at the API:AS_CreateAudioManager rslt:%d \n",result);
    pr_debug(3, "<== %s err 1 \n",__func__);
    return false;
  }
  /* Create Frontend. */

  AsCreateMicFrontendParams_t frontend_create_param;
  frontend_create_param.msgq_id.micfrontend = MSGQ_AUD_FRONTEND;
  frontend_create_param.msgq_id.mng         = MSGQ_AUD_MNG;
  frontend_create_param.msgq_id.dsp         = MSGQ_AUD_PREDSP;
  frontend_create_param.pool_id.input       = S0_MIC_IN_BUF_POOL;
#ifdef CONFIG_SQAT_AUDIO_RECOG_USEPREPROC
  frontend_create_param.pool_id.output      = S0_PREPROC_BUF_POOL;
#else
  frontend_create_param.pool_id.output      = S0_NULL_POOL;
#endif
  frontend_create_param.pool_id.dsp         = S0_PRE_APU_CMD_POOL;

  result = AS_CreateMicFrontend(&frontend_create_param, NULL);
  if(!result)
  {
    printf("## Error at the API:AS_CreateMicFrontend rslt:%d \n",result);
    pr_debug(3, "<== %s err 2 \n",__func__);
    return false;
  }
#ifdef _TEST_NO_CREATE_1_  
  set_sigque(AUD_CL_OK, THREAD_SIG_NO);
  pr_debug(3, "<== %s temp return \n",__func__);
  return false;
#endif

  /* Create Recognizer. */

  AsCreateRecognizerParam_t recognizer_create_param;
  recognizer_create_param.msgq_id.recognizer = MSGQ_AUD_RECOGNIZER;
  recognizer_create_param.msgq_id.mng        = MSGQ_AUD_MNG;
  recognizer_create_param.msgq_id.dsp        = MSGQ_AUD_RCGDSP;
  recognizer_create_param.pool_id.out        = S0_OUTPUT_BUF_POOL;
  recognizer_create_param.pool_id.dsp        = S0_RCG_APU_CMD_POOL;

  result = AS_CreateRecognizer(&recognizer_create_param, NULL);
  if (!result)
    {
      printf("Error: AS_CreateRecognizer() failure. system memory insufficient!\n");
      pr_debug(3, "<== %s err 2 \n",__func__);
      return false;
    }

  /* Create Capture feature. */

  AsCreateCaptureParam_t capture_create_param;
  capture_create_param.msgq_id.dev0_req  = MSGQ_AUD_CAP;
  capture_create_param.msgq_id.dev0_sync = MSGQ_AUD_CAP_SYNC;
  capture_create_param.msgq_id.dev1_req  = 0xFF;
  capture_create_param.msgq_id.dev1_sync = 0xFF;

  result = AS_CreateCapture(&capture_create_param);
  if (!result)
    {
      printf("Error: As_CreateCapture() failure. system memory insufficient!\n");
      pr_debug(3, "<== %s err 3 \n",__func__);
      return false;
    }

  pr_debug(2, "<== %s OK \n",__func__);
  return true;
}
#endif
/*--------------------------------------------------
   app_create_audio_sub_system_1_2_1

   to check delete and create of each 
   projects/components 
 -------------------------------------------------*/
static bool app_create_audio_sub_system_1_2_1(int loop_cnt)
{
  bool result = false;
  int i=1;

  pr_debug(2, "==> %s \n",__func__);

  pr_debug(2, "AS_Create**  No.%d \n",i);

  result = SetCreateAudioSubSystem();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateAudioManager OK \n");

  /* Create Frontend. */
  result = SetCreateMicFrontend();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateMicFrontend OK \n");

  /* Create Capture feature. */
  result = SetCreateRecognizer();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateRecognizer OK \n");

  /* Create Capture feature. */
  result = SetCreateCapture();
  if (!result)
    {
      return false;
    }

  if(loop_cnt > LOOP_TIMES_1)
    {
      loop_cnt = LOOP_TIMES_1;
      pr_debug(3, "llop count changed to %d \n", LOOP_TIMES_1); 
    } 


  for(i=0; i< loop_cnt; i++)
    {
      pr_debug(2, "SetCreateCapture OK \n");

      result = SetDeleteAudioManager();
      if(!result) return false;
      result = SetDeleteMicFrontend();
      if(!result) return false;
      result = SetDeleteRecognizer();
      if(!result) return false;
      result = SetDeleteCapture();
      if(!result) return false;

      pr_debug(2, "AS_Delete**  No.%d OK \n",i+1);

      pr_debug(2, "AS_Create**  No.%d \n",i+1);

      result = SetCreateAudioSubSystem();
      if (!result)
        {
           return false;
        }
      pr_debug(2, "AS_CreateAudioManager OK \n");

      /* Create Frontend. */
      result = SetCreateMicFrontend();
      if (!result)
        {
          return false;
        }
      pr_debug(2, "AS_CreateMicFrontend OK \n");

      /* Create Capture feature. */
      result = SetCreateRecognizer();
      if (!result)
        {
           return false;
        }
        pr_debug(2, "SetCreateRecognizer OK \n");

      /* Create Capture feature. */
      result = SetCreateCapture();
      if (!result)
        {
          return false;
        }
    }

#ifdef _TEST_DBG_
  set_sigque(AUD_CL_OK, THREAD_SIG_NO);
  pr_debug(3, "<== %s temp return \n",__func__);
  return false;
#endif

  pr_debug(2, "<== %s OK \n",__func__);
  return true;
}

/*--------------------------------------------------
   app_create_audio_sub_system_1_1_2_1

   to check if it is possible for the execution 
   without a paticular order 
   case:
     CreateAudioManager
     CreateRecognizer
     CreateMicFrontend
     CreateCapture
 -------------------------------------------------*/
static bool app_create_audio_sub_system_1_1_2_1(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  pr_debug(2, "AS_Create**  1st \n");

  result = SetCreateAudioSubSystem();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateAudioManager OK \n");

  /* Create Capture feature. */
  result = SetCreateRecognizer();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateRecognizer OK \n");

  /* Create Frontend. */
  result = SetCreateMicFrontend();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateMicFrontend OK \n");

  /* Create Capture feature. */
  result = SetCreateCapture();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateCapture OK \n");

  pr_debug(2, "<== %s OK \n",__func__);
  return true;
}

/*--------------------------------------------------
   app_create_audio_sub_system_1_1_2_2

   to check if it is possible for the execution 
   without a paticular order 
   case:
     CreateAudioManager
     CreateMicFrontend
     CreateRecognizer
     CreateCapture
 -------------------------------------------------*/
static bool app_create_audio_sub_system_1_1_2_2(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  pr_debug(2, "AS_Create**  1st \n");


  result = SetCreateAudioSubSystem();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateAudioManager OK \n");

  /* Create Frontend. */
  result = SetCreateMicFrontend();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateMicFrontend OK \n");

  /* Create Recognizer feature */
  result = SetCreateRecognizer();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateRecognizer OK \n");

  /* Create Capture feature. */
  result = SetCreateCapture();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateCapture OK \n");

  pr_debug(2, "<== %s OK \n",__func__);
  return true;
}

/*--------------------------------------------------
   app_create_audio_sub_system_1_1_2_3

   to check if it is possible for the execution 
   without a paticular order 
   case:
     CreateAudioManager
     CreateMicFrontend
     CreateCapture
     CreateRecognizer
 -------------------------------------------------*/
static bool app_create_audio_sub_system_1_1_2_3(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  pr_debug(2, "AS_Create**  1st \n");


  result = SetCreateAudioSubSystem();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateAudioManager OK \n");

  /* Create Frontend. */
  result = SetCreateMicFrontend();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateMicFrontend OK \n");

  /* Create Capture feature. */
  result = SetCreateCapture();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateCapture OK \n");

  /* Create Recognizer feature */
  result = SetCreateRecognizer();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateRecognizer OK \n");

  pr_debug(2, "<== %s OK \n",__func__);
  return true;
}

/*--------------------------------------------------
   app_create_audio_sub_system_1_1_3

   to check if an error occured when called AS_Create**
   for Recognizer
 -------------------------------------------------*/
static bool app_create_audio_sub_system_1_1_3(void)
{
  bool result = false;

  pr_debug(2, "==> %s \n",__func__);

  pr_debug(2, "AS_Create**  1st \n");


  result = SetCreateAudioSubSystem();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateAudioManager OK \n");

  /* Create Frontend. */
  result = SetCreateMicFrontend();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateMicFrontend OK \n");

  /* Create Capture feature. */
  result = SetCreateCapture();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateCapture OK \n");

  /* Create Recognizer feature */
  result = SetCreateRecognizer();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateRecognizer OK \n");

  /* Create Recognizer feature */
  result = SetCreateRecognizer();
  if (result)
    {
      pr_debug(2, "Error SetCreateRecognizer 2nd OK \n");
      return false;
    }
  pr_debug(2, "SetCreateRecognizer Error \n");
  return false;
 
  pr_debug(2, "<== %s OK \n",__func__);
  return true;
}

/*--------------------------------------------------
   app_create_audio_sub_system_1_2_2

   to check if no dependency with a paticular order
   through deleting and creating of each 
   projects/components  
 -------------------------------------------------*/
static bool app_create_audio_sub_system_1_2_2(int loop_cnt)
{
  bool result = false;
  int i=1;
  int rem = 0;
  int cnt; 
  
  pr_debug(2, "==> %s \n",__func__);

  pr_debug(2, "AS_Create**  No.%d \n",i);

  result = SetCreateAudioSubSystem();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateAudioManager OK \n");

  /* Create Frontend. */
  result = SetCreateMicFrontend();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "AS_CreateMicFrontend OK \n");

  /* Create Capture feature. */
  result = SetCreateRecognizer();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateRecognizer OK \n");

  /* Create Capture feature. */
  result = SetCreateCapture();
  if (!result)
    {
      return false;
    }
  pr_debug(2, "SetCreateCapture OK \n");

  if(loop_cnt > LOOP_TIMES_1)
    {
      loop_cnt = LOOP_TIMES_1;
      cnt = loop_cnt * CASE_NUM;
      pr_debug(3, "llop count changed to %d \n", LOOP_TIMES_1); 
    }
  else
    {
      cnt = loop_cnt * CASE_NUM;
    } 

  for(i=0; i< cnt; i++)
    {
      rem = i%(CASE_NUM -1);
      pr_debug(2, "pattern case %d \n",rem);

      switch(rem)
        {
          case 0:
            result = SetDeleteAudioManager();
            if(!result) return false;

            result = SetDeleteMicFrontend();
            if(!result) return false;

            result = SetDeleteRecognizer();
            if(!result) return false;

            result = SetDeleteCapture();
            if(!result) return false;
            break;
          case 1:
            result = SetDeleteRecognizer();
            if(!result) return false;

            result = SetDeleteAudioManager();
            if(!result) return false;

            result = SetDeleteMicFrontend();
            if(!result) return false;

            result = SetDeleteCapture();
            if(!result) return false;
            break;
          case 2:
            result = SetDeleteAudioManager();
            if(!result) return false;

            result = SetDeleteRecognizer();
            if(!result) return false;

            result = SetDeleteMicFrontend();
            if(!result) return false;

            result = SetDeleteCapture();
            if(!result) return false;
            break;
          case 3:
            result = SetDeleteAudioManager();
            if(!result) return false;

            result = SetDeleteMicFrontend();
            if(!result) return false;

            result = SetDeleteCapture();
            if(!result) return false;

            result = SetDeleteRecognizer();
            if(!result) return false;
            break;
          default:
            err("Abnormal Error!! \n");
            return false;
        }
#if 1
      if(rem == 0)
        {
          pr_debug(2, "## No.%d \n",(i/CASE_NUM+1)); /* needed to check */
        }
#else
//          pr_debug(2, "## i:%d rem:%d (i/CASE_NUM):%d No:%d \n",i, rem, (i/CASE_NUM), (i/CASE_NUM+1)); /* needed to check */
#endif
      pr_debug(2, "AS_Delete**  OK \n");

      pr_debug(2, "AS_Create**  \n");

      result = SetCreateAudioSubSystem();
      if (!result)
        {
           return false;
        }
      pr_debug(2, "AS_CreateAudioManager OK \n");

      /* Create Frontend. */
      result = SetCreateMicFrontend();
      if (!result)
        {
          return false;
        }
      pr_debug(2, "AS_CreateMicFrontend OK \n");

      /* Create Capture feature. */
      result = SetCreateRecognizer();
      if (!result)
        {
           return false;
        }
        pr_debug(2, "SetCreateRecognizer OK \n");

      /* Create Capture feature. */
      result = SetCreateCapture();
      if (!result)
        {
          return false;
        }
    }

#ifdef _TEST_DBG_
  set_sigque(AUD_CL_OK, THREAD_SIG_NO);
  pr_debug(3, "<== %s temp return \n",__func__);
  return false;
#endif

  pr_debug(2, "<== %s OK \n",__func__);
  return true;
}

static void app_deact_audio_sub_system(void)
{
  AS_DeleteAudioManager();
  AS_DeleteRecognizer();
  AS_DeleteMicFrontend();
  AS_DeleteCapture();
}

static bool app_power_on(void)
{
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  AudioCommand command;
  command.header.packet_length = LENGTH_POWERON;
  command.header.command_code  = AUDCMD_POWERON;
  command.header.sub_code      = 0x00;
  command.power_on_param.enable_sound_effect = AS_DISABLE_SOUNDEFFECT;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);

  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}

static bool app_power_off(void)
{
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  AudioCommand command;
  command.header.packet_length = LENGTH_SET_POWEROFF_STATUS;
  command.header.command_code  = AUDCMD_SETPOWEROFFSTATUS;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);

  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}

static bool app_set_ready(void)
{
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  AudioCommand command;
  command.header.packet_length = LENGTH_SET_READY_STATUS;
  command.header.command_code  = AUDCMD_SETREADYSTATUS;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);
  
  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}
static bool app_get_status(void)
{
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  AudioCommand command;
  command.header.packet_length = LENGTH_GETSTATUS;
  command.header.command_code  = AUDCMD_GETSTATUS;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);
  
  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}

static bool app_init_mic_gain(void)
{
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  AudioCommand command;
  command.header.packet_length = LENGTH_INITMICGAIN;
  command.header.command_code  = AUDCMD_INITMICGAIN;
  command.header.sub_code      = 0;
  command.init_mic_gain_param.mic_gain[0] = 0;
  command.init_mic_gain_param.mic_gain[1] = 0;
  command.init_mic_gain_param.mic_gain[2] = 0;
  command.init_mic_gain_param.mic_gain[3] = 0;
  command.init_mic_gain_param.mic_gain[4] = 0;
  command.init_mic_gain_param.mic_gain[5] = 0;
  command.init_mic_gain_param.mic_gain[6] = 0;
  command.init_mic_gain_param.mic_gain[7] = 0;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);

  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}

static bool app_init_micfrontend(uint32_t sampling_rate,
                                 uint8_t ch_num,
                                 uint8_t bitlength,
                                 uint8_t preproc_type,
                                 const char *dsp_name)
{
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  AudioCommand command;
  command.header.packet_length = LENGTH_INIT_MICFRONTEND;
  command.header.command_code  = AUDCMD_INIT_MICFRONTEND;
  command.header.sub_code      = 0x00;
  command.init_micfrontend_param.ch_num       = ch_num;
  command.init_micfrontend_param.bit_length   = bitlength;
  command.init_micfrontend_param.samples      = 320;
  command.init_micfrontend_param.preproc_type = preproc_type;
  snprintf(command.init_micfrontend_param.preprocess_dsp_path,
           AS_RECOGNIZER_FILE_PATH_LEN,
           "%s", dsp_name);
  command.init_micfrontend_param.data_dest = AsMicFrontendDataToRecognizer;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);

  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}

static bool app_set_recognizer_status(void)
{
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  AudioCommand command;
  command.header.packet_length = LENGTH_SET_RECOGNIZER_STATUS;
  command.header.command_code  = AUDCMD_SETRECOGNIZERSTATUS;
  command.header.sub_code      = 0x00;
  command.set_recognizer_status_param.input_device = AsMicFrontendDeviceMic;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);

  if(!rslt)
    {
      pr_debug(2, "<== %s ng \n",__func__);
    }
   else
    {
      pr_debug(2, "<== %s ok \n",__func__);
    }
  return rslt;
}

static bool app_set_recog_sts_2_1_2(void)
{
  bool rslt = false;

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       message("Test NG at preconditon \n"); 
       return false;
    }
  
  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       message("Test OK \n"); 
       return true;
    }
  else
    {
       message("Test NG \n"); 
       return false;
    }
}

static bool app_set_recog_sts_2_1_3(void)
{
  bool rslt = false;

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }
 
  rslt = app_set_ready();
  if(!rslt)
    {
       err("Error at set ready recognizer \n"); 
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       return false;
    }
  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }
  return true;
}

#if 0
static bool app_set_recog_sts_2_2_1(void)
{
  bool rslt = false;

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }
 
  rslt = app_set_ready();
  if(!rslt)
    {
       err("Error at set ready recognizer \n"); 
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       return false;
    }
  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }
  return true;
}
#endif

static bool app_set_recog_sts_2_2_1(void)
{
  bool rslt = false;

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }
 
  rslt = app_set_ready();
  if(!rslt)
    {
       err("Error at set ready recognizer \n"); 
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }

  return true;
}

static bool app_set_recog_sts_2_2_2(void)
{
  bool rslt = false;

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }
 
  rslt = app_set_ready();
  if(!rslt)
    {
       err("Error at set ready recognizer \n"); 
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }

  rslt = app_set_ready();
  if(rslt)
    {
       err("NG not detected an error at set ready recognizer \n"); 
       return false;
    }
  message("OK detected an error at set ready recognizer \n"); 

  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }
  return true;
}

static bool app_recog_stop_3_4_3(void)
{
  bool rslt = false;

  rslt = app_set_ready();
  if(!rslt)
    {
       err("Error at set ready recognizer \n");
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }
  return true;
}

static bool app_init_recognizer(const char *dsp_name)
{
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  AudioCommand command;
  command.header.packet_length = LENGTH_INIT_RECOGNIZER;
  command.header.command_code  = AUDCMD_INIT_RECOGNIZER;
  command.header.sub_code      = 0x00;
  command.init_recognizer.fcb        = recognizer_find_callback;
  command.init_recognizer.recognizer_type = AsRecognizerTypeUserCustom;
  snprintf(command.init_recognizer.recognizer_dsp_path,
           AS_RECOGNIZER_FILE_PATH_LEN,
           "%s", dsp_name);

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);
 
  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}

static bool app_start_recognizer(void)
{
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  AudioCommand command;
  command.header.packet_length = LENGTH_START_RECOGNIZER;
  command.header.command_code  = AUDCMD_START_RECOGNIZER;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);
  
  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}

static bool app_start_recognizer_on_ready(void)
{
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  rslt = app_set_ready();
  if(!rslt)
    {
       err("Error at set ready recognizer \n"); 
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       return false;
    }

  AudioCommand command;
  command.header.packet_length = LENGTH_START_RECOGNIZER;
  command.header.command_code  = AUDCMD_START_RECOGNIZER;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);
 
  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}


static bool app_stop_recognizer(void)
{
  pr_debug(2, "==> %s \n",__func__);

  AudioCommand command;
  command.header.packet_length = LENGTH_STOP_RECOGNIZER;
  command.header.command_code  = AUDCMD_STOP_RECOGNIZER;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  if (!printAudCmdResult(command.header.command_code, result))
    {
      pr_debug(3, "<== %s error \n",__func__);
      return false;
    }

  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}

#ifdef CONFIG_SQAT_AUDIO_RECOG_USEPREPROC
static bool app_init_preproc_dsp(void)
{
  static InitParam s_initparam;
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  AudioCommand command;
  command.header.packet_length = LENGTH_INIT_PREPROCESS_DSP;
  command.header.command_code  = AUDCMD_INIT_PREPROCESS_DSP;
  command.header.sub_code      = 0x00;
  command.init_preproc_param.packet_addr = reinterpret_cast<uint8_t *>(&s_initparam);
  command.init_preproc_param.packet_size = sizeof(s_initparam);
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);
 
  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}

static bool app_set_preproc_dsp(void)
{
  static SetParam s_setparam;
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  s_setparam.enable = true;
  s_setparam.coef   = 99;

  AudioCommand command;
  command.header.packet_length = LENGTH_SET_PREPROCESS_DSP;
  command.header.command_code  = AUDCMD_SET_PREPROCESS_DSP;
  command.header.sub_code      = 0x00;
  command.set_preproc_param.packet_addr = reinterpret_cast<uint8_t *>(&s_setparam);
  command.set_preproc_param.packet_size = sizeof(s_setparam);
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);

  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}
#endif /* CONFIG_SQAT_AUDIO_RECOG_USEPREPROC */

static bool app_init_rcgproc(void)
{
  static InitRcgParam s_initrcgparam;
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);
  s_initrcgparam.ch_num       = 1;
  s_initrcgparam.sample_width = 2;

  AudioCommand command;
  command.header.packet_length = LENGTH_INIT_RECOGNIZER_DSP;
  command.header.command_code  = AUDCMD_INIT_RECOGNIZER_DSP;
  command.header.sub_code      = 0x00;
  command.init_rcg_param.packet_addr = reinterpret_cast<uint8_t *>(&s_initrcgparam);
  command.init_rcg_param.packet_size = sizeof(s_initrcgparam);
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);

  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}

static bool app_set_rcgproc(void)
{
  static SetRcgParam s_setrcgparam;
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);
  s_setrcgparam.enable = true;

  AudioCommand command;
  command.header.packet_length = LENGTH_SET_RECOGNIZER_DSP;
  command.header.command_code  = AUDCMD_SET_RECOGNIZER_DSP;
  command.header.sub_code      = 0x00;
  command.init_rcg_param.packet_addr = reinterpret_cast<uint8_t *>(&s_setrcgparam);
  command.init_rcg_param.packet_size = sizeof(s_setrcgparam);
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  rslt = printAudCmdResult(command.header.command_code, result);

  pr_debug(2, "<== %s ok \n",__func__);
  return rslt;
}

static bool app_init_libraries(void)
{
  int ret;
  uint32_t addr = AUD_SRAM_ADDR;

  pr_debug(2, "==> %s \n",__func__);

  /* Initialize shared memory.*/

  ret = mpshm_init(&s_shm, 1, 1024 * 128 * 2);
  if (ret < 0)
    {
      printf("Error: mpshm_init() failure. %d\n", ret);
  
      pr_debug(3, "<== %s error mpshm_init \n",__func__);
      return false;
    }

  ret = mpshm_remap(&s_shm, (void *)addr);
  if (ret < 0)
    {
      printf("Error: mpshm_remap() failure. %d\n", ret);
      pr_debug(3, "<== %s error mpshm_remap \n",__func__);
      return false;
    }

  /* Initalize MessageLib. */

  err_t err = MsgLib::initFirst(NUM_MSGQ_POOLS, MSGQ_TOP_DRM);
  if (err != ERR_OK)
    {
      printf("Error: MsgLib::initFirst() failure. 0x%x\n", err);
      pr_debug(3, "<== %s error initFirst:MsgLib \n",__func__);
      return false;
    }

  err = MsgLib::initPerCpu();
  if (err != ERR_OK)
    {
      printf("Error: MsgLib::initPerCpu() failure. 0x%x\n", err);
      pr_debug(3, "<== %s error initPerCpu:MsgLib \n",__func__);
      return false;
    }

  void* mml_data_area = translatePoolAddrToVa(MEMMGR_DATA_AREA_ADDR);
  err = Manager::initFirst(mml_data_area, MEMMGR_DATA_AREA_SIZE);
  if (err != ERR_OK)
    {
      printf("Error: Manager::initFirst() failure. 0x%x\n", err);
      pr_debug(3, "<== %s error initFirst:Manager \n",__func__);
      return false;
    }

  err = Manager::initPerCpu(mml_data_area, NUM_MEM_POOLS);
  if (err != ERR_OK)
    {
      printf("Error: Manager::initPerCpu() failure. 0x%x\n", err);
      pr_debug(3, "<== %s error initPerCpu:Manager \n",__func__);
      return false;
    }

  /* Create static memory pool of VoiceCall. */

  const NumLayout layout_no = MEM_LAYOUT_RECOGNIZER;
  void* work_va = translatePoolAddrToVa(S0_MEMMGR_WORK_AREA_ADDR);
  const PoolSectionAttr *ptr  = &MemoryPoolLayouts[SECTION_NO0][layout_no][0];
  err = Manager::createStaticPools(SECTION_NO0,
                                   layout_no,
                                   work_va,
                                   S0_MEMMGR_WORK_AREA_SIZE,
                                   ptr);
  if (err != ERR_OK)
    {
      printf("Error: Manager::createStaticPools() failure. %d\n", err);
      pr_debug(3, "<== %s error createStaticPools \n",__func__);
      return false;
    }
  
  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}

static bool app_finalize_libraries(void)
{
  pr_debug(2, "==> %s \n",__func__);

  /* Finalize MessageLib. */

  MsgLib::finalize();

  /* Destroy static pools. */

  MemMgrLite::Manager::destroyStaticPools();

  /* Finalize memory manager. */

  MemMgrLite::Manager::finalize();

  /* Destroy shared memory. */

  int ret;
  ret = mpshm_detach(&s_shm);
  if (ret < 0)
    {
      printf("Error: mpshm_detach() failure. %d\n", ret);
      pr_debug(3, "<== %s error mpshm_detach \n",__func__);
      return false;
    }

  ret = mpshm_destroy(&s_shm);
  if (ret < 0)
    {
      printf("Error: mpshm_destroy() failure. %d\n", ret);
      pr_debug(3, "<== %s error mpshm_destroy \n",__func__);
      return false;
    }

  pr_debug(2, "<== %s ok \n",__func__);
  return true;
}

void app_recognizer_process(uint32_t rec_time)
{
  pr_debug(2, "==> %s \n",__func__);

  /* Timer Start */
  time_t start_time;
  time_t cur_time;

  time(&start_time);

  do
    {
      usleep(500 * 1000);
    } while((time(&cur_time) - start_time) < rec_time);
  
  pr_debug(2, "<== %s ok \n",__func__);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
#ifdef _USE_THREAD
/****************************************************************************
 * Name: set_thread
 ****************************************************************************/
static int set_thread(recog_info_t *info)
{
  int ret;
  pthread_attr_t thattr;
  struct sched_param param;

  pr_debug(2, "==> %s \n",__func__);

  pthread_attr_init(&thattr);
  thattr.stacksize     = TEST_TH_STACKSIZE;
  param.sched_priority = TEST_TH_PRIORITY;
  pthread_attr_setschedparam(&thattr, &param);

  ret = pthread_create(&monitor_th_tid, &thattr, (pthread_startroutine_t)test_monitor,
                       (pthread_addr_t)(info));
  if (ret != 0)
    {
      ret = -ret; /* pthread_create does not modify errno. */
      goto err;
    }
   pr_debug(2, "<== %s ok \n",__func__);
   return 0;
err:
   pr_debug(3, "<== %s err:%d \n",__func__, ret);
   return ret;

}
#if 0 /* unused */
/****************************************************************************
 * Name: pr_chk_sig
 ****************************************************************************/
static int pr_chk_sig(siginfo_t *sinfo)
{
  int data;

  pr_debug(2, "==> %s \n",__func__);

  data = sinfo->si_value.sival_int;

  switch(data){
    case AUD_CL_OK: 
      printf("rcv AUD_CL_OK \n");
      pr_debug(2, "<== %s ok rcv AUD_CL_OK\n",__func__);
      return OK;
      break;
    default: 
      pr_debug(3, "rcv another signal \n");
  }
  pr_debug(3, "<== %s error no matched data \n",__func__);
  return NG;
}
#endif

/****************************************************************************
 * Name: pr_err_task
 ****************************************************************************/
static int pr_err_task(siginfo_t *sinfo)
{
  int data;

  pr_debug(2, "==> %s \n",__func__);

  data = sinfo->si_value.sival_int;

  switch(data){
    case AUD_CL_01_ERR: 
      printf("rcv AUD_CL_01_ERR \n");
      pr_debug(2, "<== %s ok rcv AUD_CL_01_ERR \n",__func__);
      return OK;
      break;
    case AUD_CL_02_ERR: 
      printf("rcv AUD_CL_02_ERR \n");
      pr_debug(2, "<== %s ok rcv AUD_CL_02_ERR \n",__func__);
      return OK;
      break;
    default: 
      pr_debug(2, "rcv another signal \n");
  }
  pr_debug(2, "<== %s error no matched data \n",__func__);

  return NG;
}

static bool sig_test_proc(siginfo_t *sinfo)
{
  int data;
 
  data = sinfo->si_value.sival_int;

  switch(data){
    case AUD_CL_TEST3_4_1:
      up_udelay(1000000);
      if (!app_stop_recognizer())
        {
          err("Error: app_stop_recognizer() failure.\n");
          return false;
        }
      message("succeeded in stopping recognizer. \n");
      break;
    default:
      message("Error no matched for receiving a signal \n");
  } 
  return true;
}
/****************************************************************************
 * Name: test_monitor 
 ****************************************************************************/
static FAR void test_monitor(FAR void *data)
{
  int ret;
  sigset_t *sig;
  siginfo_t sinfo;
  recog_info_t *pdata;
 
  pr_debug(3, "<== pthread start \n");
  
  pdata = (recog_info_t *)data; 
  pdata->th_pid = getpid(); /* set thread pid */

  sig = &(pdata->sig_d);

//  up_udelay(100000); 
  
  while(1){ 
       /* set a few mask for signal */
    sigemptyset(sig);
    sigaddset(sig, THREAD_SIG_NO);
    sigaddset(sig, THREAD_SIG_NO_ERR);
    sigaddset(sig, THREAD_SIG_ACT);

    pr_debug(3, "wait sig:%x \n",*sig);

    /* wait for a signal from all tasks */
    ret = sigwaitinfo(sig, &sinfo);
    if(ret>0)
      {
        switch (ret)
          {
            case THREAD_SIG_NO:
              message("Test Succeeded; code:%d val:%d \n",
                      sinfo.si_code, sinfo.si_value.sival_int);
              break;
            case THREAD_SIG_NO_ERR:
              ret = pr_err_task(&sinfo);
              if(ret >= 0)
                {
                  /* success */
                  message("Test Failed; code:%d val:%d at task%d \n",
                          sinfo.si_code, sinfo.si_value.sival_int, ret);
                  break;
                }
              else
                {
                  /* error  */
                  err("Test Failed but no task number: code:%d val:%d \n",
                       sinfo.si_code, sinfo.si_value.sival_int);
                }
              break;
            case THREAD_SIG_ACT:
              message("Call stopping: code:%d val:%d \n",
                      sinfo.si_code, sinfo.si_value.sival_int);
              ret = sig_test_proc(&sinfo); 
              if(!ret)
                {
                  err("Error at sig_test_proc on thread \n"); 
                  err("Test NG !! \n"); 
                }
              break;
            default:
              err("error not supported the number of task!:%d \n",ret);
          }
          pr_debug(1, "signal received now break! \n");
          break;
      }
    else
     {
       err("## Error siginfo \n");
       goto err;
     }
  }

  pr_debug(3,"==> pthread end\n");
  return;
err:
  pr_debug(3, "==> pthread error end \n");
  return;
}
#endif


/****************************************************************************
 * get_cmd 
 ****************************************************************************/
static int get_cmd(int argc, char *argv[], recog_info_t *info)
{
  size_t len;
  recog_tdata_t *data;

  data = &(info->tdata);
 
  if(argc > 1)
    {
      /* get a command number for test  */
      message("argv[1]:%s \n",argv[1]);
      
      strncpy(data->cmddata, argv[1], CMD_SZ);
      len = strlen(data->cmddata);
      if(len >= CMD_SZ) 
        {
          data->cmddata[CMD_SZ-1] = '\0';
        }
      
      if(argc>2)
        {
          message("argv[2]:%s \n",argv[2]);

          data->cnt = strtol(argv[2], NULL, 16);
          pr_debug(2,"data->cnt:%x \n",data->cnt);
        }
      else
       {
         data->cnt = DEFAULT_CNT;
         pr_debug(2,"data->cnt:%x \n",data->cnt);
       }
    }
  return OK;
}

/****************************************************************************
 * get_testnum 
 ****************************************************************************/
static int get_testnum(int *val, recog_info_t *info)
{
  recog_tdata_t *tdata; 
  int ret;
  bool flag = false;
  int list_max_sz = TEST_MAX_NO;
  int ind = 0;
  
  tdata = &(info->tdata);
 
  if(g_recog_cmd_count <TEST_MAX_NO)
    {
      list_max_sz = g_recog_cmd_count; 
    }
  while(ind<list_max_sz)
  {
    pr_debug(1, "ind:%d \n", ind);
    pr_debug(1, "item:%s cmddata:%s \n", g_recog_cmds[ind].item, tdata->cmddata);

    ret = strncmp(g_recog_cmds[ind].item, tdata->cmddata, CMD_SZ);
    if(!ret)
      {
        *val = g_recog_cmds[ind].tno;
        info->index = ind;
        info->selno = g_recog_cmds[ind].tno; 
#ifndef _USE_CMDS_ITEM 
        strncpy(info->test_no, g_recog_cmds[ind].item, CMD_SZ);  /* a redundant process */ 
#endif
        flag = true;
        break; 
      }
    ind++; 
  }
  if(flag)
    { 
      message("Detected a test number! :0x%x \n",*val); 
      return OK;
    }
  else
    {
      err("Not detected a test number! \n"); 
      return NG;
    } 
}

/****************************************************************************
 * init_param 
****************************************************************************/
static void init_param(recog_info_t *data)
{
  data->rslt    = TEST_NONE;
  data->subrslt = TEST_NONE;
}

/****************************************************************************
 * set_rslt 
****************************************************************************/
static void set_rslt(recog_info_t *data, bool val)
{
  data->rslt = val;
}

/****************************************************************************
 * set_subrslt 
****************************************************************************/
static void set_subrslt(recog_info_t *data, bool val)
{
  data->subrslt = val;
}

/****************************************************************************
 * sel_audiosubsystem 
 * to select a process around audio subsystem  
****************************************************************************/
static bool sel_audiosubsystem(recog_info_t *info)
{
  int mode = info->selno;
  bool rslt = false;
  recog_tdata_t *tdata;

  tdata = &(info->tdata);

  switch(mode)
  {
    case CRE_SMODE_1_1_1:
      rslt = app_create_audio_sub_system_1_1_1();
      break;
    case CRE_SMODE_1_1_2_1:
      rslt = app_create_audio_sub_system_1_1_2_1();
      break;
    case CRE_SMODE_1_1_2_2:
      rslt = app_create_audio_sub_system_1_1_2_2();
      break;
    case CRE_SMODE_1_1_2_3:
      rslt = app_create_audio_sub_system_1_1_2_3();
      break;
    case CRE_SMODE_1_2_1: /* with delete */
      rslt = app_create_audio_sub_system_1_2_1(tdata->cnt);
      break;
    case CRE_SMODE_1_2_2: /* with delete */
      rslt = app_create_audio_sub_system_1_2_2(tdata->cnt);
      break;
    case CRE_SMODE_1_1_3: /* expect an err indication */
      rslt = app_create_audio_sub_system_1_1_3();
      break;
    default:
      rslt = app_create_audio_sub_system_1_1_1();
  }
  if(!rslt)
    {
      err("Error: act_audiosubsystem() failure.\n");
      info->rslt = false;
      return false; 
    }
  return true; 
}

/****************************************************************************
 * main 
 ****************************************************************************/
#ifdef CONFIG_BUILD_KERNEL
extern "C" int main(int argc, FAR char *argv[])
#else
extern "C" int audio_recognizer_main(int argc, char *argv[])
#endif
{
  int  ret;
  int cre_sel_mode = CRE_SMODE_0;
  bool rslt = false;

  message("Start AudioRecognizer example\n");

  /* initialize a data for test */
  init_param(&recog_info);

  ret = get_cmd(argc, argv, &recog_info);
  if(ret)
    {
      goto end;
    }
  ret = get_testnum(&cre_sel_mode, &recog_info);
  if(ret)
    {
      goto end;
    }

#ifdef _USE_THREAD
  ret = init_for_signal(&recog_info);
  if(ret)
    {
      recog_info.rslt = false;
      goto test_end;
    }

  ret = set_thread(&recog_info);
  if (ret)
    {
      recog_info.rslt = false;
      goto test_end;
    }
#endif
 
  /* First, initialize the shared memory and memory utility used by AudioSubSystem. */

  if (!app_init_libraries())
    {
      err("Error: init_libraries() failure.\n");
      return 1;
    }

  /* Next, Create the features used by AudioSubSystem. */

  rslt = sel_audiosubsystem(&recog_info);
  if(!rslt)
    {
      err("Error: act_audiosubsystem() failure.\n");
      goto test_end; 
    }
  
  /* On and after this point, AudioSubSystem must be active.
   * Register the callback function to be notified when a problem occurs.
   */

  /* Change AudioSubsystem to Ready state so that I/O parameters can be changed. */

  if (!app_power_on())
    {
      err("Error: app_power_on() failure.\n");
      return 1;
    }

  /* Set the initial gain of the microphone to be used. */

  if (!app_init_mic_gain())
    {
      err("Error: app_init_mic_gain() failure.\n");
      return 1;
    }

  /* Set recognizer operation mode. */
  switch(cre_sel_mode)
  {
    case CRE_SMODE_2_1_2:
      app_set_recog_sts_2_1_2();
      set_rslt(&recog_info, TEST_OK); 
      goto test_end;
      break;
    case CRE_SMODE_2_1_3:
      rslt = app_set_recog_sts_2_1_3();
      if(!rslt)
        {
          set_rslt(&recog_info, TEST_FAIL); 
          goto test_end2;
        }
      else
        { 
          set_rslt(&recog_info, TEST_OK); 
        }
      break;
    case CRE_SMODE_2_2_1:
      app_set_recog_sts_2_2_1();
      set_rslt(&recog_info, TEST_OK); 
      goto test_end2;
      break;
    case CRE_SMODE_2_2_1_2:
      rslt = app_set_recog_sts_2_2_1();
      break;
    case CRE_SMODE_2_2_2:
      app_set_recog_sts_2_2_2();
      message("Test 2_2_2 finished !\n"); 
      goto test_end2;
    case CRE_SMODE_3_1_2:
    case CRE_SMODE_3_2_2:
      message("not changed to Recognizer State \n");
      rslt = true; 
      break;
    default:
      rslt = app_set_recognizer_status();
      if(!rslt)
        {
          err("Error: app_set_recognizer_status() failure.\n");
          set_rslt(&recog_info, TEST_FAIL); 
          goto test_end;
        }
      break;
  }
  if(!rslt)
    {
      err("Error: app_set_recog_sts() failure.\n");
      set_rslt(&recog_info, TEST_FAIL); 
      goto test_end;
    }
  else
    {
      set_rslt(&recog_info, TEST_OK); 
    }
  /* Init MicFrontend. */

  switch(cre_sel_mode)
  {
    case CRE_SMODE_3_1_2:
      break;
    case CRE_SMODE_3_2_2:
      if (!app_init_micfrontend(AS_SAMPLINGRATE_48000,
                            AS_CHANNEL_MONO,
                            AS_BITLENGTH_16,
#ifdef CONFIG_SQAT_AUDIO_RECOG_USEPREPROC
                            AsMicFrontendPreProcUserCustom,
#else /* CONFIG_SQAT_AUDIO_RECOG_USEPREPROC */
                            AsMicFrontendPreProcThrough,
#endif /* CONFIG_SQAT_AUDIO_RECOG_USEPREPROC */
                            "/mnt/sd0/BIN/PREPROC"))
        {
          err("detected an error : app_init_micfrontend() failure.\n");
          message("Test 3_2_2 finished OK !\n"); 
          goto test_end;
        }
      else
       {
          message("Error not detected an error: app_init_micfrontend() failure.\n");
          message("Test 3_2_2 finished NG !\n"); 
          goto test_end;
       }

    default:
      if (!app_init_micfrontend(AS_SAMPLINGRATE_48000,
                            AS_CHANNEL_MONO,
                            AS_BITLENGTH_16,
#ifdef CONFIG_SQAT_AUDIO_RECOG_USEPREPROC
                            AsMicFrontendPreProcUserCustom,
#else /* CONFIG_SQAT_AUDIO_RECOG_USEPREPROC */
                            AsMicFrontendPreProcThrough,
#endif /* CONFIG_SQAT_AUDIO_RECOG_USEPREPROC */
                            "/mnt/sd0/BIN/PREPROC"))
      {
        err("Error: app_init_micfrontend() failure.\n");
        return 1;
      }
  }

  /* Initialize recognizer. */
  switch(cre_sel_mode)
  {
    case CRE_SMODE_3_1_2:
      if (!app_init_recognizer("/mnt/sd0/BIN/RCGPROC"))
        {
          message("detected an error: app_init_recognizer() failure.\n");
          message("Test 3_1_2 finished OK !\n"); 
          goto test_end;
        }
      else
       {
          message("Error not detected an error: app_init_recognizer() failure.\n");
          message("Test 3_1_2 finished NG !\n"); 
          goto test_end;
       }
      break;
    default:
      if (!app_init_recognizer("/mnt/sd0/BIN/RCGPROC"))
        {
          err("Error: app_init_recognizer() failure.\n");
          return 1;
        }
  }

#ifdef CONFIG_SQAT_AUDIO_RECOG_USEPREPROC
  /* Init PREPROC */

  if (!app_init_preproc_dsp())
    {
      err("Error: app_init_preproc_dsp() failure.\n");
      return 1;
    }

  /* Set PREPROC */

  if (!app_set_preproc_dsp())
    {
      err("Error: app_set_preproc_dsp() failure.\n");
      return 1;
    }
#endif /* CONFIG_SQAT_AUDIO_RECOG_USEPREPROC */

  /* Init RCGPROC */

  if (!app_init_rcgproc())
    {
      err("Error: app_init_rcgproc() failure.\n");
      return 1;
    }

  /* Set RCGPROC */

  if (!app_set_rcgproc())
    {
      err("Error: app_set_rcgproc() failure.\n");
      return 1;
    }

  /* Start recognizer operation. */
  switch(cre_sel_mode)
  {
    case CRE_SMODE_3_3_2:
      if (!app_start_recognizer_on_ready())
       {
          message("detected an error: app_start_recognizer() failure.\n");
          message("Test 3_3_2 finished OK !\n"); 
          goto test_end;
        }
      else
       {
          err("Error not detected an error: app_start_recognizer() failure.\n");
          message("Test 3_3_2 finished NG !\n"); 
          goto test_end;
       }
      break;
    case CRE_SMODE_3_4_2:
      break;
    default:
      if (!app_start_recognizer())
        {
          err("Error: app_start_recognizer() failure.\n");
        return 1;
        }
      
  }
  
  switch(cre_sel_mode)
  {
    case CRE_SMODE_3_4_2:
      break;
    case CRE_SMODE_3_4_3:
      rslt = app_recog_stop_3_4_3();
      if(rslt)
        {
          err("Error at changed status to Ready\n" );
          goto test_end2;
        }
      break;
    case CRE_SMODE_3_4_1:
      rslt = set_delay_stop_recognizer();
      if(!rslt)
        {
          err("Error to issue a signal for stop recognizer \n" );
          rslt = app_set_ready();
          if(!rslt)
            {
               err("Test NG : error at set ready recognizer \n"); 
               goto test_end2_1;
            }
          goto test_end2;
        }
    default:
      /* Running... */

      message("Running time is %d sec\n", RECOGNIZER_EXEC_TIME);

      app_recognizer_process(RECOGNIZER_EXEC_TIME);

  }

  switch(cre_sel_mode)
  {
    case CRE_SMODE_3_4_2:
    case CRE_SMODE_3_4_3:

      if (!app_stop_recognizer())
        {
          message("Test OK detected an error: app_stop_recognizer() failure.\n");
          if(cre_sel_mode == CRE_SMODE_3_4_2) 
            {
              rslt = app_set_ready();
              if(!rslt)
                {
                  err("Error at set ready recognizer \n"); 
                  set_rslt(&recog_info, TEST_FAIL); 
                  goto test_end2_1;
                }
            }
          goto test_end2;
        }
      else
       {
          err("Error not detect an error:app_stop_recognizer() \n"); 
          set_rslt(&recog_info, TEST_FAIL); 
       }
      break;
    case CRE_SMODE_3_4_1:
      pr_debug(2,"test 3_4_1 skip stop_recognizer on main\n");
      break;
    default:

      /* Stop recognizer operation. */

      if (!app_stop_recognizer())
        {
          err("Error: app_stop_recognizer() failure.\n");
          set_rslt(&recog_info, TEST_FAIL); 
          goto test_end;
        }
  }

  /* Return the state of AudioSubSystem before voice_call operation. */

  if (!app_set_ready())
    {
      err("Error: app_set_ready() failure.\n");
      set_rslt(&recog_info, TEST_FAIL); 
      goto test_end;
    }

  /* Change AudioSubsystem to PowerOff state. */

test_end2:
  if (!app_power_off())
    {
      err("Error: app_power_off() failure.\n");
      set_subrslt(&recog_info, TEST_FAIL); 
      goto test_end;
    }
  /* Deactivate the features used by AudioSubSystem. */

test_end2_1:
  app_deact_audio_sub_system();

  /* finalize the shared memory and memory utility used by AudioSubSystem. */

  if (!app_finalize_libraries())
    {
      err("Error: finalize_libraries() failure.\n");
      set_subrslt(&recog_info, TEST_FAIL); 
      return 1;
    }

test_end:
#ifdef _USE_THREAD
#if 0
  pr_debug(3, "call pthread_join \n");
  ret = pthread_join(monitor_th_tid, NULL);
  pr_debug(3, "exit pthread_join \n");
#else
  pr_debug(3, "call pthread_cancel \n");
  ret = pthread_cancel(monitor_th_tid);
  pr_debug(3, "exit pthread_cancel \n");
#endif
#endif /* _USE_THREAD */

end:
  switch(recog_info.rslt)
  {
    case TEST_OK:
      switch(recog_info.subrslt)
      {
        case TEST_NONE:
        case TEST_OK:
#ifdef _USE_CMDS_ITEM 
          message("## Test OK for %s !!\n", g_recog_cmds[recog_info.index].item);
#else
          message("## Test OK for %s !\n", recog_info.test_no);
#endif
          break;
        case TEST_FAIL:
#ifdef _USE_CMDS_ITEM 
          message("## Test NG for %s 'cause other errors !!\n", g_recog_cmds[recog_info.index].item);
#else
          message("## Test NG for %s 'cause other errors !!\n", recog_info.test_no);
#endif
          break;
      }
      break; 
    case TEST_FAIL:
      message("## Test NG for %s !!\n", g_recog_cmds[recog_info.index].item);
      break;
    }
  message("Exit AudioRecognizer example\n");

  return 0;

}
