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
#include <asmp/mpmutex.h>
#include <asmp/mpmutex.h>
#include <asmp/mptask.h>
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
#ifdef CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC
#include "userproc_command.h"
#endif /* CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC */

#include <arch/chip/cxd56_audio.h>
#include <audio_recog_def.h>
#include <audio_recog_proto.h>
#include <audio_recog_struct.h>
#include "shm_def.h"

#define _USE_THREAD

#ifdef _USE_THREAD
static pthread_t monitor_th_tid;
#endif
#define DEBUG_LEVEL  2 

/* For Debug */
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
#ifdef _USE_THREAD
static int set_thread(recog_info_t *);
static FAR void test_monitor(FAR void *);
#endif
#define message(format, ...)    printf(format, ##__VA_ARGS__)
#define err(format, ...)        fprintf(stderr, format, ##__VA_ARGS__)
#define pr_debug(dbg,format, ...)   {if(dbg>=DEBUG_LEVEL){ printf(format, ##__VA_ARGS__);}}

/****************************************************************************
 * Private Type Declarations
 ****************************************************************************/
static int cmd_help(void);
static void set_rslt(recog_info_t *, bool);
static void set_subrslt(recog_info_t *, bool);
void app_recognizer_process(uint32_t );
static bool app_set_ready(void);
static bool app_get_status(void);
static void  pr_shm_info(recog_info_t *, bool);
static bool chk_shm_info(recog_info_t *, int);
static void set_rstmsg(recog_info_t *);
static void pr_info_reset(recog_info_t *);
static int sel_init_micfrontend(recog_info_t *);
static int sel_init_recog(recog_info_t *);

/****************************************************************************
 * Private Data
 ****************************************************************************/

recog_info_t recog_info;

static recog_cmd_t g_recog_cmds[] =
{
    { "1_1_1", CRE_SMODE_1_1_1 },
    { "1_1_2_1", CRE_SMODE_1_1_2_1 },
    { "1_1_2_2", CRE_SMODE_1_1_2_2 },
    { "1_1_2_3", CRE_SMODE_1_1_2_3 },
    { "1_1_3", CRE_SMODE_1_1_3 },
    { "1_2_1", CRE_SMODE_1_2_1 },
    { "1_2_2", CRE_SMODE_1_2_2 },
//    { "1_2_3", CRE_SMODE_1_2_3 },  /* no use */
    { "2_1_2", CRE_SMODE_2_1_2 },
    { "2_1_3", CRE_SMODE_2_1_3 },
    { "2_2_1", CRE_SMODE_2_2_1 },
    { "2_2_1_2", CRE_SMODE_2_2_1_2 },
    { "2_2_2", CRE_SMODE_2_2_2 },
    { "3_1_2", CRE_SMODE_3_1_2 },
    { "3_1_3", CRE_SMODE_3_1_3 },
    { "3_2_2", CRE_SMODE_3_2_2 },
    { "3_3_2", CRE_SMODE_3_3_2 },
    { "3_4_1", CRE_SMODE_3_4_1 },
    { "3_4_2", CRE_SMODE_3_4_2 },
    { "3_4_3", CRE_SMODE_3_4_3 }, /* invalid item */
    { "4_1_2", CRE_SMODE_4_1_2 }, 
    { "4_2_2", CRE_SMODE_4_2_2 }, 
    { "7_1", CRE_SMODE_7_1 }, 
    { "9_5_5", CRE_SMODE_9_5_5 }, 
};
static const int g_recog_cmd_count = sizeof(g_recog_cmds) / sizeof(recog_cmd_t);

/*-------------------------------------------------------
    a command to show help  
 -------------------------------------------------------*/
static int cmd_help(void)
{
  int len;
  int maxlen = 0;

  for (int i = 0; i < g_recog_cmd_count; i++)
    {
      len = strlen(g_recog_cmds[i].item);
      if (len > maxlen)
        {
          maxlen = len;
        }
    }

  message("AudioRecognizerTest commands\n");
  message("===Commands list=============\n");
  message("  No. Command  \n");
  for (int i = 0; i < g_recog_cmd_count; i++)
    {
      message("  %02d  %s  ", i, g_recog_cmds[i].item);
      len = maxlen - strlen(g_recog_cmds[i].item);
      for (int j = 0; j < len; j++)
        {
          message(" ");
        }
      message("\n");
    }
  message("=============================\n");
  return OK;
}

#if 0
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
#endif

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
#if 0
static mpmutex_t s_sem;
static mptask_t s_task;
#endif

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
static void pr_info_reset(recog_info_t *info)
{
  if(info->rstmsg)
    {
      message("## Push reset after this test if execute a next test!! \n");
    }
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
/*-----------------------------------------------
  
 -----------------------------------------------*/
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
#ifdef CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC
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
#ifdef CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC
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

      pr_debug(2, "AS_Create**  No.%d \n",i+2);

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
static bool app_create_audio_sub_system_1_1_3(recog_info_t *info)
{
  bool rslt = false;

  pr_debug(3, "==> %s \n",__func__);

  pr_debug(2, "AS_Create**  1st \n");

  rslt = SetCreateAudioSubSystem();
  if (!rslt)
    {
      return false;
    }
  pr_debug(2, "AS_CreateAudioManager OK \n");

  /* Create Frontend. */
  rslt = SetCreateMicFrontend();
  if (!rslt)
    {
      return false;
    }
  pr_debug(2, "AS_CreateMicFrontend OK \n");

  /* Create Capture feature. */
  rslt = SetCreateCapture();
  if (!rslt)
    {
      return false;
    }
  pr_debug(2, "SetCreateCapture OK \n");

  pr_debug(2, "AS_CreateRecognizer 1st \n");

  /* Create Recognizer feature */
  rslt = SetCreateRecognizer();
  if (!rslt)
    {
      return false;
    }
  pr_debug(2, "SetCreateRecognizer OK \n");

  pr_debug(2, "AS_CreateRecognizer 2nd \n");

  /* Create Recognizer feature */
  rslt = SetCreateRecognizer();
  if (rslt)
    {
      pr_debug(2, "Check OK : detected error at SetCreateRecognizer 2nd! \n");
      set_rslt(info, TEST_OK);
      rslt = true;
    }
  else
    {
      pr_debug(2, "SetCreateRecognizer Error \n");
      set_rslt(info, TEST_FAIL);
      rslt = false;
    } 
  pr_debug(3,"<== %s %s \n",__func__, rslt?"OK":"NG");
  return rslt;
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

/*--------------------------------------------------
   app_create_audio_sub_system_1_2_3(Experimental)

   to check if a Error occures when deleting and 
   creating of each projects/components in Ready
   state 
 -------------------------------------------------*/
static bool app_create_audio_sub_system_1_2_3(recog_info_t *info)
{
  bool rslt = false;
  int i=1;
  int chk=0;

   
  pr_debug(2, "==> %s \n",__func__);

  pr_debug(2, "AS_Create**  No.%d \n",i);

  rslt = SetCreateAudioSubSystem();
  if (!rslt)
    {
      set_subrslt(info, TEST_FAIL);
      return false;
    }
  pr_debug(2, "AS_CreateAudioManager OK \n");

  /* Create Frontend. */
  rslt = SetCreateMicFrontend();
  if (!rslt)
    {
      set_subrslt(info, TEST_FAIL);
      return false;
    }
  pr_debug(2, "AS_CreateMicFrontend OK \n");

  /* Create Capture feature. */
  rslt = SetCreateRecognizer();
  if (!rslt)
    {
      set_subrslt(info, TEST_FAIL);
      return false;
    }
  pr_debug(2, "SetCreateRecognizer OK \n");

  /* Create Capture feature. */
  rslt = SetCreateCapture();
  if (!rslt)
    {
      set_subrslt(info, TEST_FAIL);
      return false;
    }
  pr_debug(2, "SetCreateCapture OK \n");

  /* change to Ready State */
  rslt = app_set_ready();
  if(!rslt)
    {
       err("Error at set ready recognizer \n"); 
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = SetDeleteAudioManager();
  if(!rslt)
    {
      pr_debug(2, "AS_DeleteAudioManager  OK \n");
      chk++;
    }
  else
    {
      pr_debug(2,"## Error not a expected result in DeleteAudioManager \n");
      set_rslt(info, TEST_FAIL);
      return false;
    }
  rslt = SetDeleteMicFrontend();
  if(!rslt)
    {
      pr_debug(2, "AS_DeleteMicFrontend  OK \n");
      chk++;
    }
  else
    {
      pr_debug(2,"## Error not a expected result in DeleteMicFrontend \n");
      set_rslt(info, TEST_FAIL);
      return false;
    }

  rslt = SetDeleteRecognizer();
  if(!rslt)
    {
      pr_debug(2, "AS_DeleteRecognizer  OK \n");
      chk++;
    }
  else
    {
      pr_debug(2,"## Error not a expected result in DeleteRecognizer \n");
      set_rslt(info, TEST_FAIL);
      return false;
    }

  rslt = SetDeleteCapture();
  if(!rslt)
    {
      pr_debug(2, "AS_DeleteCapture  OK \n");
      chk++;
    }
  else
    {
      pr_debug(2,"## Error not a expected result in DeleteCapture \n");
      set_rslt(info, TEST_FAIL);
      return false;
    }

  if(chk != 4)
    {
      pr_debug(2, "<== %s NG \n",__func__);
      set_rslt(info, TEST_FAIL);
      return false;
    }
  pr_debug(2, "<== %s OK \n",__func__);
  set_rslt(info, TEST_OK);
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

static bool app_set_recog_sts_2_1_2(recog_info_t *info)
{
  bool rslt = false;

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       message("Test NG at preconditon \n"); 
       set_subrslt(info, TEST_FAIL);
       return false;
    }
  
  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       pr_debug(2,"Check OK \n");
       set_rslt(info, TEST_OK);
       return true;
    }
  else
    {
       pr_debug(2,"Check NG \n");
       set_rslt(info, TEST_FAIL);
       return false;
    }
}

static bool app_set_recog_sts_2_1_3(recog_info_t *info)
{
  bool rslt = false;

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }
 
  rslt = app_set_ready();
  if(!rslt)
    {
       err("Error at set ready recognizer \n"); 
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }
  rslt = app_get_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }
  return true;
}

static bool app_set_recog_sts_2_2_1_2(recog_info_t *info)
{
  bool rslt = false;

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }
 
  rslt = app_set_ready();
  if(!rslt)
    {
       err("Error at set ready recognizer \n"); 
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }
  rslt = app_get_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }
  return true;
}

static bool app_set_recog_sts_2_2_1(recog_info_t *info)
{
  bool rslt = false;

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }
 
  rslt = app_set_ready();
  if(!rslt)
    {
       err("Error at set ready recognizer \n"); 
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  return true;
}

static bool app_set_recog_sts_2_2_2(recog_info_t *info)
{
  bool rslt = false;

  rslt = app_set_recognizer_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }
 
  rslt = app_set_ready();
  if(!rslt)
    {
       err("Error at set ready recognizer \n"); 
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = app_get_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
       return false;
    }

  rslt = app_set_ready();
  if(rslt)
    {
       err("NG not detected an error at set ready recognizer \n"); 
       set_rslt(info, TEST_FAIL);
       return false;
    }
  else
   {
      message("OK detected an error at set ready recognizer \n"); 
      set_rslt(info, TEST_OK);
   }
  rslt = app_get_status();
  if(!rslt)
    {
       set_subrslt(info, TEST_FAIL);
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

static bool app_init_recognizer(const char *dsp_name, int mode)
{
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);

  AudioCommand command;
  command.header.packet_length = LENGTH_INIT_RECOGNIZER;
  command.header.command_code  = AUDCMD_INIT_RECOGNIZER;
  command.header.sub_code      = 0x00;
  if(mode == MODE_ERROR)
    { 
      command.init_recognizer.fcb = NULL;
    }
  else
    {
      command.init_recognizer.fcb = recognizer_find_callback;
    } 
  command.init_recognizer.recognizer_type = AsRecognizerTypeUserCustom;
  snprintf(command.init_recognizer.recognizer_dsp_path,
           AS_RECOGNIZER_FILE_PATH_LEN,
           "%s", dsp_name);

//  printf("###>>> dsp_name:%s len:%d \n", dsp_name, AS_RECOGNIZER_FILE_PATH_LEN);

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

#ifdef CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC
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
#endif /* CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC */

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

static bool app_set_rcgproc2(recog_info_t *info)
{
  static SetRcgParam s_setrcgparam;
  bool rslt;

  pr_debug(2, "==> %s \n",__func__);
  s_setrcgparam.enable = true;
  s_setrcgparam.offset = 0;
  s_setrcgparam.value1 = (info->tdata).val;
  s_setrcgparam.value2 = (info->tdata).val2;

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
#if 0
  ret = mpmutex_init(&s_sem, KEY_MUTEX);
  if(ret < 0)
    {
      pr_debug(3, "<== %s error mpmutex_init \n",__func__);
      return false;
    }
 
  ret = mptask_bindobj(&s_task, &s_sem);
  if(ret < 0)
    {
      pr_debug(3, "<== %s error mptask_bindobj \n",__func__);
      return false;
    }
#endif
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

void set_process_time(recog_info_t *info)
{
  uint32_t val;

  if(info->rand)
    {
      val = rand()%RAND_RANGE;
      
      pr_debug(2, "rand val:%d \n",val);
      if(val==0)
        {
          val = RAND_MIN_VAL;
          pr_debug(2, "reset rand val:%d \n",val);
        }
    }
  else
    {
      val = RECOGNIZER_EXEC_TIME;

    }
  info->rval = val;
  message("## rand value: 0x%X \n",info->rval);

  app_recognizer_process(val);
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
 
  pr_debug(3, "==> pthread start \n");
  
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

  pr_debug(3,"<== pthread end\n");
  return;
err:
  pr_debug(3, "<== pthread error end \n");
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

  pr_debug(3, "==> %s start\n",__func__);

  data = &(info->tdata);
 
  data->cmdnum = argc -1;

  if(argc > 1)
    {
      /* get a command number for test  */
      message("argv[1]:%s argc:%d \n",argv[1], argc);
      
      strncpy(data->cmddata, argv[1], CMD_SZ);
      len = strlen(data->cmddata);
      if(len >= CMD_SZ) 
        {
          data->cmddata[CMD_SZ-1] = '\0';
        }
      
      if(argc>2)
        {
          pr_debug(1,"argv[2]:%s \n",argv[2]);

          data->cnt = strtol(argv[2], NULL, 10);
          pr_debug(2,"data->cnt:%d(DEC) \n",data->cnt);
        }
      else
       {
         data->cnt = DEFAULT_CNT;
         pr_debug(2,"data->cnt:%d(DEC) \n",data->cnt);
       }
      
      if(argc>3)
        {
          pr_debug(1,"argv[3]:%s \n",argv[3]);

          data->val = strtol(argv[3], NULL, 10);
          pr_debug(2,"data->val:%d \n",data->val);
        }

      if(argc>4)
        {
          pr_debug(1,"argv[4]:%s \n",argv[4]);

          data->val2 = strtol(argv[4], NULL, 10);
          pr_debug(2,"data->val2:%x \n",data->val2);
        }
    }
  else
   {
      cmd_help();
      pr_debug(3, "<== %s error \n",__func__);
      return NG;
   }
  pr_debug(3, "<== %s end \n",__func__);
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
  
  pr_debug(3, "==> %s start\n",__func__);

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
  switch(info->selno)
  {
    case CRE_SMODE_7_1:
      if(tdata->cmdnum >= 4)
        {
          if(tdata->val2 != 0)
            { 
              info->pr_shm2 = true;
            }
          else
            {
              info->pr_shm2 = false;
            }
         
          pr_debug(2,"tdata->val2:%d info->pr_shm2:%d \n", tdata->val2, info->pr_shm2);
        }
      if(tdata->cmdnum >= 3)
        {
          if(tdata->val != 0)
            { 
              info->rand = true;
            }
          else
            {
              info->rand = false;
            }
         
          pr_debug(2,"tdata->val:%d info->rand:%d \n", tdata->val, info->rand);
        }
      if(tdata->cmdnum >= 2)
        { 
          info->loopcnt = tdata->cnt;
          pr_debug(2,"info->loopcnt:%d tdata->cnt:%d \n", info->loopcnt, tdata->cnt);
        }
      else
       {
          pr_debug(2,"info->loopcnt: %d <-- NO_SET\n", info->loopcnt);
       }
      info->pr_shm = true; 
      break;
  }

  if(flag)
    { 
      message("Detected a test number! :0x%x \n",*val); 
      pr_debug(3, "<== %s OK \n",__func__);
      return OK;
    }
  else
    {
      err("Not detected a test number! \n"); 
      pr_debug(3, "<== %s NG \n",__func__);
      return NG;
    } 
}

/****************************************************************************
 * set_loop_param 
****************************************************************************/
static int set_loop_param(recog_info_t *info)
{
  pr_debug(3, "==> %s start\n",__func__);

  switch(info->selno)
  {
    case CRE_SMODE_7_1:
      info->loop = LOOP_CONT;
      if(info->loopcnt == NOT_SET)
        { 
          info->loopcnt = LOOP_MAX_CNT1;
        }
      pr_debug(2,"set LOOP_CONT \n");
      break;
    default:
      pr_debug(2,"set LOOP_ONE \n");
      info->loop = LOOP_ONE;
  }
  pr_debug(3, "<== %s OK \n",__func__);
  return info->loopcnt;
}

/****************************************************************************
 * update loop_param 
****************************************************************************/
static bool update_loop_param(recog_info_t *info)
{
  pr_debug(3, "==> %s start\n",__func__);

  if(info->rslt == TEST_FAIL)
    {
      info->loop = LOOP_NONE;
      pr_debug(2,"set LOOP_NONE for error \n");
      return false; 
    }
  
  switch(info->loop)
  {
    case LOOP_ONE:
      info->loop = LOOP_NONE;
      pr_debug(2,"set LOOP_NONE \n");
      break;
    case LOOP_CONT:
      if(info->loopcnt == 1)
        {
          info->loop = LOOP_NONE;
          pr_debug(2,"set LOOP_NONE \n");
        }
      else
        { 
          --(info->loopcnt);
          pr_debug(2,"## loopcnt:%d \n",info->loopcnt);
        }
      break;
  }
  pr_debug(3, "<== %s OK \n",__func__);
  return true; 
}

/****************************************************************************
 * init_param 
****************************************************************************/
static void init_param(recog_info_t *info)
{
  pr_debug(3, "==> %s start\n",__func__);
 
  info->rslt    = TEST_NONE;
  info->subrslt = TEST_NONE;

  info->loopcnt = NOT_SET;
  info->pr_shm  = false; 
  info->pr_shm2 = false;
  info->rand    = false; 
  info->rstmsg  = false;
 
  info->mode = MODE_NOT_SET;

  info->tdata.cnt  = 0;
  info->tdata.val  = 0;
  info->tdata.val2 = 0;

  pr_debug(3, "<== %s OK \n",__func__);
}

int set_srand(void)
{
  pr_debug(3, "==> %s start\n",__func__);

  time_t Time = time(NULL);

  pr_debug(2,"Time for srand: 0x%X \n",Time);
  srand(Time);

  pr_debug(3, "<== %s OK \n",__func__);
  return OK; 
}

void init_testshm(recog_info_t *info)
{
  int m;
  char *buf;

  pr_debug(3, "==> %s start\n",__func__);
  buf = (char *)(info->buf);

  for(m = 0; m < RECOG_SZ_SHM; m++){
     buf[m] = 0x00;
   }
  pr_debug(3, "<== %s OK \n",__func__);
}
/****************************************************************************
 * init_for_mprsc 
****************************************************************************/
static bool init_for_mprsc(recog_info_t *info)
{
  int ret; 
  char *buf;

  pr_debug(3, "==> %s start\n",__func__);

  ret = mpshm_init(&(info->shm), KEY_SHM, RECOG_SZ_SHM);
  if(ret<0)
    {
      err("mpshm_init() failure. %d\n", ret);
      set_subrslt(info, TEST_FAIL);
      pr_debug(3, "<== %s mpshm_init error \n",__func__);
      return false;
    }
  pr_debug(2,"mpshm init ok \n");
  pr_debug(3,"paddr:%p \n",info->shm.paddr);

  info->buf = (char *)mpshm_attach(&info->shm, 0);
  if(info->buf == NULL)
    {
      err("mpshm_attach error!! \n");
      set_subrslt(info, TEST_FAIL);
      pr_debug(3, "<== %s mpshm_attach error \n",__func__);
      return ret;
    }

  buf = (char *)(info->buf);
  pr_debug(2, "init buf:%p \n",buf);

  /* initialize a shared memory */
  init_testshm(info);

  pr_debug(3, "<== %s OK \n",__func__);
  return true;
}

/****************************************************************************
 * finalize_for_mprsc 
****************************************************************************/
static bool finalize_for_mprsc(recog_info_t *info)
{
  int ret;
 
  pr_debug(3,"==> %s \n",__func__);

  ret = mpshm_detach(&info->shm);
  if(ret<0)
    {
      err("<== %s err %d \n",ret);
      return false;
    }
  ret = mpshm_destroy(&info->shm);
  if(ret<0)
    {
      err("<== %s err %d \n",ret);
      return false;
    }
  pr_debug(3,"<== %s OK \n",__func__);
  return true;
}
/****************************************************************************
 * set_rstmsg 
****************************************************************************/
static void set_rstmsg(recog_info_t *data)
{
  data->rstmsg = true;
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
static int sel_audiosubsystem(recog_info_t *info)
{
  int no = info->selno;
  bool rslt = false;
  recog_tdata_t *tdata;

  pr_debug(3,"==> %s \n",__func__);

  tdata = &(info->tdata);

  switch(no)
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
    case CRE_SMODE_1_2_3: /* with delete */
      rslt = app_create_audio_sub_system_1_2_3(info);
      break;
    case CRE_SMODE_1_1_3: /* expect an err indication */
      rslt = app_create_audio_sub_system_1_1_3(info);
      if(rslt)
        {
          set_rstmsg(info);
          return RET_SKIP2;
        }
      break;
    default:
      rslt = app_create_audio_sub_system_1_1_1();
  }
  if(!rslt)
    {
      err("Error: act_audiosubsystem() failure.\n");
      set_rslt(info, TEST_FAIL);
      pr_debug(3,"<== %s NG \n",__func__);
      return RET_NG; 
    }
  else
   {
      pr_debug(3,"<== %s OK \n",__func__);
      return RET_OK;
   } 
}
/****************************************************************************
 * sel_recog_op_mode 
 * to select a recognizer operation mode  
****************************************************************************/
static int sel_recog_op_mode(recog_info_t *info)
{
  int no = info->selno;
  int ret = RET_OK;
  bool rslt=false;

  pr_debug(3,"==> %s \n",__func__);

  /* Set recognizer operation mode. */

  switch(no)
  {
    case CRE_SMODE_2_1_2:
      app_set_recog_sts_2_1_2(info);
      set_rstmsg(info);
      ret = RET_SKIP1;
      break;
    case CRE_SMODE_2_1_3:
      rslt = app_set_recog_sts_2_1_3(info);
      if(!rslt)
        {
          set_rslt(&recog_info, TEST_FAIL); 
          ret = RET_SKIP3;
        }
      else
        { 
          set_rslt(&recog_info, TEST_OK); 
        }
      break;
    case CRE_SMODE_2_2_1:
      app_set_recog_sts_2_2_1(info);
      set_rslt(&recog_info, TEST_OK); 
      ret = RET_SKIP3;
      break;
    case CRE_SMODE_2_2_1_2:
      rslt = app_set_recog_sts_2_2_1_2(info);
      set_rslt(&recog_info, TEST_OK); 
      break;
    case CRE_SMODE_2_2_2:
      app_set_recog_sts_2_2_2(info);
      set_rstmsg(info);
      ret = RET_SKIP3;
    case CRE_SMODE_3_1_2:
    case CRE_SMODE_3_2_2:
      message("not changed to Recognizer State \n");
      break;
    default:
      rslt = app_set_recognizer_status();
      if(!rslt)
        {
          err("Error: app_set_recognizer_status() failure.\n");
          set_rslt(&recog_info, TEST_FAIL); 
          ret = RET_SKIP1;
       }
      break;
  }
  pr_debug(3,"<== %s %s \n",__func__, rslt?"OK":"NG");
  return ret;
}

/****************************************************************************
 * sel_init_micfrontend 
****************************************************************************/
static int sel_init_micfrontend(recog_info_t *info)
{
  int no = info->selno;
  int ret = RET_OK;

  /* Init MicFrontend. */
  switch(no)
    {
      case CRE_SMODE_3_1_2:
        break;
      case CRE_SMODE_3_2_2:
        if (!app_init_micfrontend(AS_SAMPLINGRATE_48000,
                            AS_CHANNEL_MONO,
                            AS_BITLENGTH_16,
#ifdef CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC
                            AsMicFrontendPreProcUserCustom,
#else /* CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC */
                            AsMicFrontendPreProcThrough,
#endif /* CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC */
                            "/mnt/sd0/BIN/PREPROC"))
          {
            pr_debug(3,"Check OK : detected an error at app_init_micfrontend() \n");
            set_rslt(info, TEST_OK); 
            ret = RET_END1; 
          }
        else
         {
            err("Check NG : Error not detected an error: app_init_micfrontend() \n");
            set_rslt(info, TEST_FAIL); 
            ret = RET_END1; 
         }

      default:
        if (!app_init_micfrontend(AS_SAMPLINGRATE_48000,
                            AS_CHANNEL_MONO,
                            AS_BITLENGTH_16,
#ifdef CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC
                            AsMicFrontendPreProcUserCustom,
#else /* CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC */
                            AsMicFrontendPreProcThrough,
#endif /* CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC */
                            "/mnt/sd0/BIN/PREPROC"))
        {
          err("Error: app_init_micfrontend() failure.\n");
          set_subrslt(info, TEST_FAIL); 
          ret = RET_END1;
        }
    }
  return ret;

}

/****************************************************************************
 * sel_init_recog 
****************************************************************************/
static int sel_init_recog(recog_info_t *info)
{
  int no = info->selno;
  int ret = RET_OK;
  char dspname[AS_RECOGNIZER_FILE_PATH_LEN];

  /* Initialize recognizer. */
  switch(no)
    {
      case CRE_SMODE_3_1_2:
        if (!app_init_recognizer("/mnt/sd0/BIN/RCGPROC", MODE_NORMAL))
          {
            message("Check OK : detected an error at app_init_recognizer() \n");
            set_rslt(&recog_info, TEST_OK);
            ret = RET_END4; 
          }
        else
         {
            err("Check NG : detected an error at app_init_recognizer() \n");
            set_rslt(&recog_info, TEST_FAIL); 
            ret = RET_END4; 
         }
        break;
      case CRE_SMODE_3_1_3:
        if (!app_init_recognizer("/mnt/sd0/BIN/RCGPROC", MODE_ERROR))
          {
            message("Detected an error at init_recognizer \n");
            set_rslt(&recog_info, TEST_OK);
            ret = RET_END4; 
          }
        else
         {
            err("Not an expected result : not detected an error at init_recognizer \n");
            set_rslt(&recog_info, TEST_FAIL);
            ret = RET_END4; 
         }
        break;
      case CRE_SMODE_7_1:
        if((recog_info.loopcnt)%2==0)
          {
             recog_info.mode = SET_MODE1;
             strncpy(dspname, "/mnt/sd0/BIN/RCGPROC", sizeof(dspname)/sizeof(char)-1);
             pr_debug(2, "##select RGPROC: %s \n",dspname);
          }
        else
          {
             recog_info.mode = SET_MODE2;
             strncpy(dspname, "/mnt/sd0/BIN/RCGPROC2", sizeof(dspname)/sizeof(char)-1);
             pr_debug(2, "##select RGPROC2: %s \n",dspname);
          } 
        if (!app_init_recognizer(dspname, MODE_NORMAL))
          {
            message("Check NG : Error not detected an error at app_init_recognizer()\n");
            set_subrslt(info, TEST_FAIL); 
            ret = RET_END3;
          }
        break;
      case CRE_SMODE_9_5_5:
        if (!app_init_recognizer("/mnt/sd0/BIN/RCGPROC3", MODE_NORMAL))
          {
            err("Error: app_init_recognizer() failure.\n");
            set_subrslt(info, TEST_FAIL); 
            ret = RET_END3;
          }
        break;
      default:
        if (!app_init_recognizer("/mnt/sd0/BIN/RCGPROC", MODE_NORMAL))
          {
            err("Error: app_init_recognizer() failure.\n");
            set_subrslt(info, TEST_FAIL); 
            ret = RET_END3;
          }
    }
  return ret; 
}

/****************************************************************************
 * sel_init_rcgproc 
****************************************************************************/
static int sel_init_rcgproc(recog_info_t *info)
{
  int no = info->selno;
  int ret = RET_OK;
  bool rslt = false;

  pr_debug(3,"==> %s \n",__func__);
  
  switch(no)
    {
      case CRE_SMODE_4_1_2:
        /* change to Ready State */
        rslt = app_set_ready();
        if(!rslt)
          {
            err("Error at set ready recognizer \n"); 
            set_subrslt(info, TEST_FAIL);
            ret = RET_END4;
          }

        rslt = app_get_status();
        if(!rslt)
          {
            err("Error at gete status \n"); 
            set_subrslt(info, TEST_FAIL);
            ret = RET_END3;
          }

        rslt = app_init_rcgproc();
        if(rslt)
          {
            set_rslt(info, TEST_FAIL);
            err("Error not an expected result(No error)!! \n");
            ret = RET_END3;  
          }
        else
          {
            set_rslt(info, TEST_OK);
            message("Check OK : received an expected result rslt:%d \n",rslt); 
            rslt = true;   
            ret = RET_END3;  
          } 
        break;
      default:
        /* Init RCGPROC */
        if (!app_init_rcgproc())
          {
            err("Error: app_init_rcgproc() failure.\n");
            set_rslt(info, TEST_FAIL);
            ret = RET_END4; 
          }
        else
          {
            rslt = true;   
          } 
    }

  switch(no)
    {
      case CRE_SMODE_7_1:
        pr_shm_info(&recog_info, recog_info.pr_shm2);

        rslt = chk_shm_info(&recog_info, POINT_1);
        if(!rslt)
          { 
            set_rslt(info, TEST_FAIL);
            ret = RET_END4;
          }
        else
          {
            rslt = true;   
         } 
       
        break;
    }

  pr_debug(3,"<== %s %s \n",__func__, rslt?"OK":"NG");
  return ret;
}

/****************************************************************************
 * sel_set_rcgproc 
****************************************************************************/
static int sel_set_rcgproc(recog_info_t *info)
{
  int no = info->selno;
  int ret = RET_OK;
  bool rslt = false;

  pr_debug(3,"==> %s \n",__func__);
  
  switch(no)
    {
      case CRE_SMODE_4_2_2:
        /* change to Ready State */
        rslt = app_set_ready();
        if(!rslt)
          {
            err("Error at set ready recognizer \n"); 
            set_subrslt(info, TEST_FAIL);
            ret = RET_END4;
          }

        rslt = app_get_status();
        if(!rslt)
          {
            err("Error at gete status \n"); 
            set_subrslt(info, TEST_FAIL);
            ret = RET_END3;
          }

        rslt = app_set_rcgproc();
        if(rslt)
          {
            set_rslt(info, TEST_FAIL);
            err("Error not an expected result(No error)!! \n");
            ret = RET_END3;  
          }
        else
          {
            set_rslt(info, TEST_OK);
            message("Check OK : received an expected result rslt:%d \n",rslt); 
            rslt = true;   
            ret = RET_END3;  
          } 
        break;
      case CRE_SMODE_9_5_5:
        if (!app_set_rcgproc2(info))
          {
            err("Error: app_set_rcgproc2() failure.\n");
            set_rslt(info, TEST_FAIL);
            ret = RET_END4; 
          }
        else
         {
            rslt = true;   
         }
        
        break;
      default:
        /* Set RCGPROC */

        if (!app_set_rcgproc())
          {
            err("Error: app_set_rcgproc() failure.\n");
            set_rslt(info, TEST_FAIL);
            ret = RET_END4; 
          }
        else
         {
            rslt = true;   
         }
    }

  switch(no)
    {
      case CRE_SMODE_7_1:
        pr_shm_info(&recog_info, recog_info.pr_shm2);

        rslt = chk_shm_info(&recog_info, POINT_1);
        if(!rslt)
          { 
            set_rslt(info, TEST_FAIL);
            ret = RET_END4;
          }
        else
          {
            rslt = true;   
         } 
        break;
    }
  pr_debug(3,"<== %s %s \n",__func__, rslt?"OK":"NG");
  return ret;
}

/****************************************************************************
 * pr_shm_info 
****************************************************************************/
static void  pr_shm_info(recog_info_t *info, bool flag)
{
  char *buf;
  int i;

  if(flag == true)
    { 
      buf = (char *)(info->buf); 
      for(i=0; i<SHM_DUMP_SZ; i++)
        {
           printf("%02X ",buf[i]); 
           if((i+1)%16 == 0)
             {
               printf("\n"); 
             }
        }
    } 
}

/****************************************************************************
 * chk_shm_info_p1 
****************************************************************************/
static bool chk_shm_info_p1(recog_info_t *info)
{
  int mode = info->mode;
  char *buf;
  bool rslt = false; 
  int cnt = 0;

  pr_debug(3,"==> %s \n",__func__);

  buf = (char *)(info->buf); 

  switch(mode)
    {
      case SET_MODE1:  
        if((buf[RCG_I_ADDR|RCG_OFFS0] == (char)RCG1_I_KEY)
          &&(buf[RCG_I_ADDR|RCG_OFFS3] == (char)RCG_INFO0))
          {
            cnt = buf[RCG_I_ADDR|RCG_OFFS1] << 8 | buf[RCG_I_ADDR|RCG_OFFS2]; 
            pr_debug(2,"## Check OK at MODE1 in POINT_1 : 0x%02x \n", cnt);
            rslt = true; 
          }
        else
          {
            pr_debug(2,"## Check NG no detected at MODE1 in POINT_1 \n");
            set_rslt(&recog_info, TEST_FAIL);
            rslt = false; 
          }
        break;
      case SET_MODE2:
        if((buf[RCG_I_ADDR|RCG_OFFS0] == (char)RCG2_I_KEY)
          &&(buf[RCG_I_ADDR|RCG_OFFS3] == (char)RCG_INFO0))
          {
            cnt = buf[RCG_I_ADDR|RCG_OFFS1] << 8 | buf[RCG_I_ADDR|RCG_OFFS2]; 
            pr_debug(2,"## Check OK at MODE2 in POINT_1 : 0x%02x \n", cnt);
            rslt = true; 
          }
        else
          {
            pr_debug(2,"## Check NG no detected at MODE2 in POINT_1 \n");
            set_rslt(&recog_info, TEST_FAIL);
            rslt = false; 
          }
        break;
      default:
        err("Error no set mode at POINT_1 \n");
        set_subrslt(&recog_info, TEST_FAIL);
        rslt = false; 
        break; 
    }
  pr_debug(3,"<== %s %s\n",__func__, rslt?"OK":"NG");
  return rslt;
}

/****************************************************************************
 * chk_shm_info_p2 
****************************************************************************/
static bool chk_shm_info_p2(recog_info_t *info)
{
  int mode = info->mode;
  char *buf;
  bool rslt = false; 
  int cnt = 0;

  pr_debug(3,"==> %s \n",__func__);

  buf = (char *)(info->buf); 

  switch(mode)
    {
      case SET_MODE1:  
        if((buf[RCG_S_ADDR|RCG_OFFS0] == (char)RCG1_S_KEY)
          &&(buf[RCG_S_ADDR|RCG_OFFS3] == (char)RCG_INFO0))
          {
            if((buf[RCG_S_ADDR|RCG_OFFS0|(FLD_OFFS*1)] == (char)RCG1_S_KEY)
              &&(buf[RCG_S_ADDR|RCG_OFFS3|(FLD_OFFS*1)] == (char)RCG_INFO1))
              {
                if((buf[RCG_S_ADDR|RCG_OFFS0|(FLD_OFFS*2)] == (char)RCG1_S_KEY)
                  &&(buf[RCG_S_ADDR|RCG_OFFS3|(FLD_OFFS*2)] == (char)RCG_INFO2))
                  {
                    if((buf[RCG_S_ADDR|RCG_OFFS0|(FLD_OFFS*3)] == (char)RCG1_S_KEY)
                      &&(buf[RCG_S_ADDR|RCG_OFFS3|(FLD_OFFS*3)] == (char)RCG_INFO3))
                      {
                        cnt = buf[RCG_S_ADDR|RCG_OFFS1] << 8 | buf[RCG_S_ADDR|RCG_OFFS2]; 
                        pr_debug(2,"## Check OK at MODE1 in POINT_2 : 0x%02x \n", cnt);
                        rslt = true;
                      }
                    else
                      {
                        pr_debug(2,"## Check NG no detected at MODE1 in POINT_2 no.4 \n");
                        set_rslt(&recog_info, TEST_FAIL);
                        rslt = false; 
                      }
                  }
                else
                  {
                     pr_debug(2,"## Check NG no detected at MODE1 in POINT_2 no.3 \n");
                     set_rslt(&recog_info, TEST_FAIL);
                     rslt = false; 
                  }
              }
            else
              {
                 pr_debug(2,"## Check NG no detected at MODE1 in POINT_2 no.2 \n");
                 set_rslt(&recog_info, TEST_FAIL);
                 rslt = false; 
              }
          }
        else
          {
            pr_debug(2,"## Check NG no detected at MODE1 in POINT_2 no.1 \n");
            set_rslt(&recog_info, TEST_FAIL);
            rslt = false; 
          }
        break;
      case SET_MODE2:
        if((buf[RCG_S_ADDR|RCG_OFFS0] == (char)RCG2_S_KEY)
          &&(buf[RCG_S_ADDR|RCG_OFFS3] == (char)RCG_INFO0))
          {
            if((buf[RCG_S_ADDR|RCG_OFFS0|(FLD_OFFS*1)] == (char)RCG2_S_KEY)
              &&(buf[RCG_S_ADDR|RCG_OFFS3|(FLD_OFFS*1)] == (char)RCG_INFO1))
              {
                if((buf[RCG_S_ADDR|RCG_OFFS0|(FLD_OFFS*2)] == (char)RCG2_S_KEY)
                  &&(buf[RCG_S_ADDR|RCG_OFFS3|(FLD_OFFS*2)] == (char)RCG_INFO2))
                  {
                    if((buf[RCG_S_ADDR|RCG_OFFS0|(FLD_OFFS*3)] == (char)RCG2_S_KEY)
                      &&(buf[RCG_S_ADDR|RCG_OFFS3|(FLD_OFFS*3)] == (char)RCG_INFO3))
                      {
                        cnt = buf[RCG_S_ADDR|RCG_OFFS1] << 8 | buf[RCG_S_ADDR|RCG_OFFS2]; 
                        pr_debug(2,"## Check OK at MODE2 in POINT_2 : 0x%02x \n", cnt);
                        rslt = true;
                      }
                    else
                      {
                        pr_debug(2,"## Check NG no detected at MODE2 in POINT_2 no.4 \n");
                        set_rslt(&recog_info, TEST_FAIL);
                        rslt = false; 
                      }
                  }
                else
                  {
                     pr_debug(2,"## Check NG no detected at MODE2 in POINT_2 no.3 \n");
                     set_rslt(&recog_info, TEST_FAIL);
                     rslt = false; 
                  }
              }
            else
              {
                 pr_debug(2,"## Check NG no detected at MODE2 in POINT_2 no.2 \n");
                 set_rslt(&recog_info, TEST_FAIL);
                 rslt = false; 
              }
          }
        else
          {
            pr_debug(2,"## Check NG no detected at MODE2 in POINT_2 no.1 \n");
            set_rslt(&recog_info, TEST_FAIL);
            rslt = false; 
          }
        break;
      default:
        err("Error no set mode at POINT_2 \n");
        set_subrslt(&recog_info, TEST_FAIL);
        rslt = false;
        break; 
    }  
  pr_debug(3,"<== %s %s\n",__func__, rslt?"OK":"NG");
  return rslt;
}

/****************************************************************************
 * chk_shm_info_p3
****************************************************************************/
static bool chk_shm_info_p3(recog_info_t *info)
{
  int mode = info->mode;
  char *buf;
  bool rslt = false; 
  int cnt = 0;
  int offs = 0;
  int val1 = 0;
  int val2 = 0;

  pr_debug(3,"==> %s \n",__func__);

  buf = (char *)(info->buf); 

  switch(mode)
    {
      case SET_MODE1:  
        if((buf[RCG_E_ADDR|RCG_OFFS0] == (char)RCG1_E_KEY)
          &&(buf[RCG_E_ADDR|RCG_OFFS3] == (char)RCG_INFO0))
          {
            if((buf[RCG_E_ADDR|RCG_OFFS0|(FLD_OFFS*1)] == (char)RCG1_E_KEY)
              &&(buf[RCG_E_ADDR|RCG_OFFS3|(FLD_OFFS*1)] == (char)RCG_INFO1))
              {
                if((buf[RCG_E_ADDR|RCG_OFFS0|(FLD_OFFS*2)] == (char)RCG1_E_KEY)
                  &&(buf[RCG_E_ADDR|RCG_OFFS3|(FLD_OFFS*2)] == (char)RCG_INFO2))
                  {
                    if((buf[RCG_E_ADDR|RCG_OFFS0|(FLD_OFFS*3)] == (char)RCG1_E_KEY)
                      &&(buf[RCG_E_ADDR|RCG_OFFS3|(FLD_OFFS*3)] == (char)RCG_INFO3))
                      {
                        cnt = buf[RCG_S_ADDR|RCG_OFFS1] << 8 | buf[RCG_S_ADDR|RCG_OFFS2]; 
                        offs = buf[RCG_S_ADDR|RCG_OFFS1|(FLD_OFFS*1)] << 8 | buf[RCG_S_ADDR|RCG_OFFS2|(FLD_OFFS*1)]; 
                        val1 = buf[RCG_S_ADDR|RCG_OFFS1|(FLD_OFFS*2)] << 8 | buf[RCG_S_ADDR|RCG_OFFS2|(FLD_OFFS*2)]; 
                        val2 = buf[RCG_S_ADDR|RCG_OFFS1|(FLD_OFFS*3)] << 8 | buf[RCG_S_ADDR|RCG_OFFS2|(FLD_OFFS*3)]; 
                        pr_debug(2,"## Check OK at MODE1 in POINT_3 exec1: 0x%04x offs:%x val1:%x val2:%x \n", cnt, offs, val1, val2);
                        rslt = true;
                      }
                    else
                      {
                        pr_debug(2,"## Check NG no detected at MODE1 in POINT_3 exec1 no.4 \n");
                        set_rslt(&recog_info, TEST_FAIL);
                        rslt = false; 
                      }
                  }
                else
                  {
                     pr_debug(2,"## Check NG no detected at MODE1 in POINT_3 exec1 no.3 \n");
                     set_rslt(&recog_info, TEST_FAIL);
                     rslt = false; 
                  }
              }
            else
              {
                 pr_debug(2,"## Check NG no detected at MODE1 in POINT_3 exec1 no.2 \n");
                 set_rslt(&recog_info, TEST_FAIL);
                 rslt = false; 
              }
          }
        else if((buf[RCG_E_ADDR2|RCG_OFFS0] == (char)RCG1_E_KEY)
          &&(buf[RCG_E_ADDR2|RCG_OFFS3] == (char)RCG_INFO0))
          {
            if((buf[RCG_E_ADDR2|RCG_OFFS0|(FLD_OFFS*1)] == (char)RCG1_E_KEY)
              &&(buf[RCG_E_ADDR2|RCG_OFFS3|(FLD_OFFS*1)] == (char)RCG_INFO1))
              {
                if((buf[RCG_E_ADDR2|RCG_OFFS0|(FLD_OFFS*2)] == (char)RCG1_E_KEY)
                  &&(buf[RCG_E_ADDR2|RCG_OFFS3|(FLD_OFFS*2)] == (char)RCG_INFO2))
                  {
                    if((buf[RCG_E_ADDR2|RCG_OFFS0|(FLD_OFFS*3)] == (char)RCG1_E_KEY)
                      &&(buf[RCG_E_ADDR2|RCG_OFFS3|(FLD_OFFS*3)] == (char)RCG_INFO3))
                      {
                        cnt = buf[RCG_E_ADDR2|RCG_OFFS1] << 8 | buf[RCG_E_ADDR2|RCG_OFFS2]; 
                        offs = buf[RCG_E_ADDR2|RCG_OFFS1|(FLD_OFFS*1)] << 8 | buf[RCG_E_ADDR2|RCG_OFFS2|(FLD_OFFS*1)]; 
                        val1 = buf[RCG_E_ADDR2|RCG_OFFS1|(FLD_OFFS*2)] << 8 | buf[RCG_E_ADDR2|RCG_OFFS2|(FLD_OFFS*2)]; 
                        val2 = buf[RCG_E_ADDR2|RCG_OFFS1|(FLD_OFFS*3)] << 8 | buf[RCG_E_ADDR2|RCG_OFFS2|(FLD_OFFS*3)]; 
                        pr_debug(2,"## Check OK at MODE1 in POINT_3 exec2: 0x%04x offs:%x val1:%x val2:%x \n", cnt, offs, val1, val2);
                        rslt = true;
                      }
                    else
                      {
                        pr_debug(2,"## Check NG no detected at MODE1 in POINT_3 exec2 no.4 \n");
                        set_rslt(&recog_info, TEST_FAIL);
                        rslt = false; 
                      }
                  }
                else
                  {
                     pr_debug(2,"## Check NG no detected at MODE1 in POINT_3 exec2 no.3 \n");
                     set_rslt(&recog_info, TEST_FAIL);
                     rslt = false; 
                  }
              }
            else
              {
                 pr_debug(2,"## Check NG no detected at MODE1 in POINT_3 exec2 no.2 \n");
                 set_rslt(&recog_info, TEST_FAIL);
                 rslt = false; 
              }
          }
        else
          {
            pr_debug(2,"## Check NG no detected at MODE1 in POINT_3 no.1 \n");
            set_rslt(&recog_info, TEST_FAIL);
            rslt = false; 
          }
        break;
      case SET_MODE2:
        if((buf[RCG_E_ADDR|RCG_OFFS0] == (char)RCG2_E_KEY)
          &&(buf[RCG_E_ADDR|RCG_OFFS3] == (char)RCG_INFO0))
          {
            if((buf[RCG_E_ADDR|RCG_OFFS0|(FLD_OFFS*1)] == (char)RCG2_E_KEY)
              &&(buf[RCG_E_ADDR|RCG_OFFS3|(FLD_OFFS*1)] == (char)RCG_INFO1))
              {
                if((buf[RCG_E_ADDR|RCG_OFFS0|(FLD_OFFS*2)] == (char)RCG2_E_KEY)
                  &&(buf[RCG_E_ADDR|RCG_OFFS3|(FLD_OFFS*2)] == (char)RCG_INFO2))
                  {
                    if((buf[RCG_E_ADDR|RCG_OFFS0|(FLD_OFFS*3)] == (char)RCG2_E_KEY)
                      &&(buf[RCG_E_ADDR|RCG_OFFS3|(FLD_OFFS*3)] == (char)RCG_INFO3))
                      {
                        cnt = buf[RCG_E_ADDR|RCG_OFFS1] << 8 | buf[RCG_E_ADDR|RCG_OFFS2]; 
                        offs = buf[RCG_E_ADDR|RCG_OFFS1|(FLD_OFFS*1)] << 8 | buf[RCG_E_ADDR|RCG_OFFS2|(FLD_OFFS*1)]; 
                        val1 = buf[RCG_E_ADDR|RCG_OFFS1|(FLD_OFFS*2)] << 8 | buf[RCG_E_ADDR|RCG_OFFS2|(FLD_OFFS*2)]; 
                        val2 = buf[RCG_E_ADDR|RCG_OFFS1|(FLD_OFFS*3)] << 8 | buf[RCG_E_ADDR|RCG_OFFS2|(FLD_OFFS*3)]; 
                        pr_debug(2,"## Check OK at MODE2 in POINT_3 exec1: 0x%04x offs:%x val1:%x val2:%x \n", cnt, offs, val1, val2);
                        rslt = true;
                      }
                    else
                      {
                        pr_debug(2,"## Check NG no detected at MODE2 in POINT_3 exec1 no.4 \n");
                        set_rslt(&recog_info, TEST_FAIL);
                        rslt = false; 
                      }
                  }
                else
                  {
                     pr_debug(2,"## Check NG no detected at MODE2 in POINT_3 exec1 no.3 \n");
                     set_rslt(&recog_info, TEST_FAIL);
                     rslt = false; 
                  }
              }
            else
              {
                 pr_debug(2,"## Check NG no detected at MODE2 in POINT_3 exec1 no.2 \n");
                 set_rslt(&recog_info, TEST_FAIL);
                 rslt = false; 
              }
          }
        else if((buf[RCG_E_ADDR2|RCG_OFFS0] == (char)RCG1_E_KEY)
          &&(buf[RCG_E_ADDR2|RCG_OFFS3] == (char)RCG_INFO0))
          {
            if((buf[RCG_E_ADDR2|RCG_OFFS0|(FLD_OFFS*1)] == (char)RCG1_E_KEY)
              &&(buf[RCG_E_ADDR2|RCG_OFFS3|(FLD_OFFS*1)] == (char)RCG_INFO1))
              {
                if((buf[RCG_E_ADDR2|RCG_OFFS0|(FLD_OFFS*2)] == (char)RCG1_E_KEY)
                  &&(buf[RCG_E_ADDR2|RCG_OFFS3|(FLD_OFFS*2)] == (char)RCG_INFO2))
                  {
                    if((buf[RCG_E_ADDR2|RCG_OFFS0|(FLD_OFFS*3)] == (char)RCG1_E_KEY)
                      &&(buf[RCG_E_ADDR2|RCG_OFFS3|(FLD_OFFS*3)] == (char)RCG_INFO3))
                      {
                        cnt = buf[RCG_E_ADDR2|RCG_OFFS1] << 8 | buf[RCG_E_ADDR2|RCG_OFFS2]; 
                        offs = buf[RCG_E_ADDR2|RCG_OFFS1|(FLD_OFFS*1)] << 8 | buf[RCG_E_ADDR2|RCG_OFFS2|(FLD_OFFS*1)]; 
                        val1 = buf[RCG_E_ADDR2|RCG_OFFS1|(FLD_OFFS*2)] << 8 | buf[RCG_E_ADDR2|RCG_OFFS2|(FLD_OFFS*2)]; 
                        val2 = buf[RCG_E_ADDR2|RCG_OFFS1|(FLD_OFFS*3)] << 8 | buf[RCG_E_ADDR2|RCG_OFFS2|(FLD_OFFS*3)]; 
                        pr_debug(2,"## Check OK at MODE2 in POINT_3 exec2: 0x%04x offs:%x val1:%x val2:%x \n", cnt, offs, val1, val2);
                        rslt = true;
                      }
                    else
                      {
                        pr_debug(2,"## Check NG no detected at MODE2 in POINT_3 exec2 no.4 \n");
                        set_rslt(&recog_info, TEST_FAIL);
                        rslt = false; 
                      }
                 set_rslt(&recog_info, TEST_FAIL);
                  }
                else
                  {
                     pr_debug(2,"## Check NG no detected at MODE2 in POINT_3 exec2 no.3 \n");
                     set_rslt(&recog_info, TEST_FAIL);
                     rslt = false; 
                  }
              }
            else
              {
                 pr_debug(2,"## Check NG no detected at MODE2 in POINT_3 exec2 no.2 \n");
                 set_rslt(&recog_info, TEST_FAIL);
                 rslt = false; 
              }
          }
        else
          {
            pr_debug(2,"## Check NG no detected at MODE2 in POINT_3 no.1 \n");
            set_rslt(&recog_info, TEST_FAIL);
            rslt = false; 
          }
        break;
      default:
        err("Error no set mode at POINT_3 \n");
        set_subrslt(&recog_info, TEST_FAIL);
        rslt = false;
    }
  pr_debug(3,"<== %s %s\n",__func__, rslt?"OK":"NG");
  return rslt;
}

/****************************************************************************
 * chk_shm_info_p4
****************************************************************************/
static bool chk_shm_info_p4(recog_info_t *info)
{
  int mode = info->mode;
  char *buf;
  bool rslt = false; 
  int cnt = 0;

  pr_debug(3,"==> %s \n",__func__);

  buf = (char *)(info->buf); 
 
  switch(mode)
    {
      case SET_MODE1:  
        if((buf[RCG_F_ADDR|RCG_OFFS0] == (char)RCG1_F_KEY)
          &&(buf[RCG_F_ADDR|RCG_OFFS3] == (char)RCG_INFO0))
          {
            cnt = buf[RCG_F_ADDR|RCG_OFFS1] << 8 | buf[RCG_F_ADDR|RCG_OFFS2]; 
            pr_debug(2,"## Check OK at MODE1 in POINT_4 : 0x%02x \n", cnt);
            rslt = true; 
          }
        else
          {
#if 0
            pr_debug(2,"## Check NG no detected at MODE1 in POINT_4 \n");
            set_rslt(&recog_info, TEST_FAIL);
            rslt = false; 
#else
            pr_debug(2,"## No detected at MODE1 in POINT_4 \n");
            rslt = true; 
#endif
          }
        break;
      case SET_MODE2:
        if((buf[RCG_F_ADDR|RCG_OFFS0] == (char)RCG2_F_KEY)
          &&(buf[RCG_F_ADDR|RCG_OFFS3] == (char)RCG_INFO0))
          {
            cnt = buf[RCG_F_ADDR|RCG_OFFS1] << 8 | buf[RCG_F_ADDR|RCG_OFFS2]; 
            pr_debug(2,"## Check OK at MODE2 in POINT_4 : 0x%02x \n", cnt);
            rslt = true; 
          }
        else
          {
#if 0
            pr_debug(2,"## Check NG no detected at MODE2 in POINT_4 \n");
            set_rslt(&recog_info, TEST_FAIL);
            rslt = false; 
#else
            pr_debug(2,"## No detected at MODE2 in POINT_4 \n");
            rslt = true; 
#endif
          }
        break;
      default:
        err("Error no set mode at POINT_4 \n");
        set_subrslt(&recog_info, TEST_FAIL);
        rslt = false;
        break;
    }
  pr_debug(3,"<== %s %s\n",__func__, rslt?"OK":"NG");
  return rslt; 
}

/****************************************************************************
 * chk_shm_info 
****************************************************************************/
static bool chk_shm_info(recog_info_t *info, int point)
{
  bool rslt = false; 

  pr_debug(3,"==> %s \n",__func__);

  switch(point)
    {
      /* init */
      case POINT_1:
        rslt = chk_shm_info_p1(info);
        break;
      /* set */ 
      case POINT_2:
        rslt = chk_shm_info_p2(info);
        break; 
      case POINT_3:
        rslt = chk_shm_info_p3(info);
        break;
      case POINT_4:
        rslt = chk_shm_info_p4(info);
        break;
      default:
        err("No support point:%d \n",point);
        rslt = false;
   }
  pr_debug(3,"<== %s %s\n",__func__, rslt?"OK":"NG");
  return rslt;
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
  int  cre_sel_mode = CRE_SMODE_0;
  bool rslt = false;
  int  *loop = &(recog_info.loop); 
  char dspname[AS_RECOGNIZER_FILE_PATH_LEN];
  int  tloop=0;
 
  message("Start AudioRecognizer example\n");

  /* initialize a data for test */
  init_param(&recog_info);


  set_srand();

  ret = get_cmd(argc, argv, &recog_info);
  if(ret)
    {
      goto notest;
    }
  ret = get_testnum(&cre_sel_mode, &recog_info);
  if(ret)
    {
      goto notest;
    }

  tloop = set_loop_param(&recog_info);

  rslt = init_for_mprsc(&recog_info);
  if(!rslt)
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
 
  /***********************************************
   * Test Loop 
   ***********************************************/
  while((*loop==LOOP_ONE)||(*loop==LOOP_CONT))
    {

    message("## Test loopcnt:%d/%d \n",recog_info.loopcnt, tloop ); 

    /* First, initialize the shared memory and memory utility used by AudioSubSystem. */

    if (!app_init_libraries())
      {
        err("Error: init_libraries() failure.\n");
        goto test_end;
      }

    /* Next, Create the features used by AudioSubSystem. */

    ret = sel_audiosubsystem(&recog_info);
    switch(ret)
      {
        case RET_SKIP2:
          goto test_end2;
          break;
        case RET_NG:
          err("Error: act_audiosubsystem() failure.\n");
          goto test_end2;
          break;
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

    ret = sel_recog_op_mode(&recog_info);
    switch(ret)
      {
        case RET_SKIP1:
          goto test_end;
          break;
        case RET_SKIP3:
          goto test_end3;
          break;
        case RET_NG:
          err("Error: app_set_recog_sts() failure.\n");
          set_rslt(&recog_info, TEST_FAIL); 
          goto test_end;
          break;
        default:
         set_rslt(&recog_info, TEST_OK); 
     }         
 
    /* Init MicFrontend. */
    ret = sel_init_micfrontend(&recog_info);
    switch(ret)
    {
      case RET_END1:
        goto test_end;
        break;
    }

#if 0
    ret = sel_init_recog(&recog_info)
      {
        
      }
#else
    /* Initialize recognizer. */
    switch(cre_sel_mode)
    {
      case CRE_SMODE_3_1_2:
        if (!app_init_recognizer("/mnt/sd0/BIN/RCGPROC", MODE_NORMAL))
          {
            message("Check OK : detected an error at app_init_recognizer() \n");
            set_rslt(&recog_info, TEST_OK); 
            goto test_end4;
          }
        else
         {
            err("Check NG : detected an error at app_init_recognizer() \n");
            set_rslt(&recog_info, TEST_FAIL); 
            goto test_end4;
         }
        break;
      case CRE_SMODE_3_1_3:
        if (!app_init_recognizer("/mnt/sd0/BIN/RCGPROC", MODE_ERROR))
          {
            message("Detected an error at init_recognizer \n");
            set_rslt(&recog_info, TEST_OK);
            goto test_end4;
          }
        else
         {
            err("Not an expected result : not detected an error at init_recognizer \n");
            set_rslt(&recog_info, TEST_FAIL);
            goto test_end4;
         }
        break;
      case CRE_SMODE_7_1:
        if((recog_info.loopcnt)%2==0)
          {
             recog_info.mode = SET_MODE1;
             strncpy(dspname, "/mnt/sd0/BIN/RCGPROC", sizeof(dspname)/sizeof(char)-1);
             pr_debug(2, "##select RGPROC: %s \n",dspname);
          }
        else
          {
             recog_info.mode = SET_MODE2;
             strncpy(dspname, "/mnt/sd0/BIN/RCGPROC2", sizeof(dspname)/sizeof(char)-1);
             pr_debug(2, "##select RGPROC2: %s \n",dspname);
          } 
        if (!app_init_recognizer(dspname, MODE_NORMAL))
          {
            message("Error not detected an error: app_init_recognizer() failure.\n");
            message("Test 7_1 finished NG !\n"); 
            goto test_end3;
          }
        break;
      case CRE_SMODE_9_5_5:
        if (!app_init_recognizer("/mnt/sd0/BIN/RCGPROC3", MODE_NORMAL))
          {
            err("Error: app_init_recognizer() failure.\n");
            goto test_end3;
          }
        break;
      default:
        if (!app_init_recognizer("/mnt/sd0/BIN/RCGPROC", MODE_NORMAL))
          {
            err("Error: app_init_recognizer() failure.\n");
            goto test_end3;
          }
    }
#endif

#ifdef CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC
    /* Init PREPROC */

    if (!app_init_preproc_dsp())
      {
        err("Error: app_init_preproc_dsp() failure.\n");
        goto_test_end3;
      }

    /* Set PREPROC */

    if (!app_set_preproc_dsp())
      {
        err("Error: app_set_preproc_dsp() failure.\n");
        goto_test_end3;
      }
#endif /* CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC */

    ret = sel_init_rcgproc(&recog_info);
    switch(ret)
      {
        case RET_END3:
          goto test_end3;
          break;
        case RET_END4:
          goto test_end4;
          break;
      }

    ret = sel_set_rcgproc(&recog_info);
    switch(ret)
      {
        case RET_END3:
          goto test_end3;
          break;
        case RET_END4:
          goto test_end4;
          break;
      }

    /* Start recognizer operation. */
    switch(cre_sel_mode)
    {
      case CRE_SMODE_3_3_2:
        if (!app_start_recognizer_on_ready())
          {
            message("detected an error: app_start_recognizer() failure.\n");
            message("Test 3_3_2 finished OK !\n"); 
            goto test_end4;
          }
        else
         {
           err("Error not detected an error: app_start_recognizer() failure.\n");
           message("Test 3_3_2 finished NG !\n"); 
           goto test_end4;
         }
        break;
      case CRE_SMODE_3_4_2:
        break;
      case CRE_SMODE_7_1:
        if (!app_start_recognizer())
          {
            err("Error: app_start_recognizer() failure.\n");
            goto test_end4;
          }

        up_udelay(1000000);  /*  wait for finishing flush */

  //    pr_shm_info(&recog_info, recog_info.pr_shm2);
        pr_shm_info(&recog_info, recog_info.pr_shm);

        rslt = chk_shm_info(&recog_info, POINT_3);
        if(!rslt)
          {
            if (!app_stop_recognizer())
              {
                err("Error: app_stop_recognizer() failure.\n");
                set_rslt(&recog_info, TEST_FAIL); 
                goto test_end;
              }
            goto test_end4; 
          }
        break;
      default:
        if (!app_start_recognizer())
          {
            err("Error: app_start_recognizer() failure.\n");
            goto test_end4;
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
            goto test_end3;
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
                goto test_end2;
              }
            goto test_end3;
          }
      default:
        /* Running... */

        message("Running time is %d sec\n", RECOGNIZER_EXEC_TIME);

        set_process_time(&recog_info);
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
                    goto test_end2;
                  }
              }
            goto test_end3;
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
      case CRE_SMODE_7_1:
        /* Stop recognizer operation. */

        if (!app_stop_recognizer())
          {
            err("Error: app_stop_recognizer() failure.\n");
            set_rslt(&recog_info, TEST_FAIL); 
            goto test_end2;
          }
        up_udelay(1000000);  /*  wait for finishing flush */
    
        pr_shm_info(&recog_info, recog_info.pr_shm2);

        rslt = chk_shm_info(&recog_info, POINT_4);
        if(!rslt) goto test_end4;
        break;
      default:

        /* Stop recognizer operation. */

        if (!app_stop_recognizer())
          {
            err("Error: app_stop_recognizer() failure.\n");
            set_rslt(&recog_info, TEST_FAIL); 
            goto test_end2;
          }
    }

    /* Return the state of AudioSubSystem before voice_call operation. */

test_end4:
    if (!app_set_ready())
      {
        err("Error: app_set_ready() failure.\n");
        set_rslt(&recog_info, TEST_FAIL); 
        goto test_end;
      }

test_end3:
    /* Change AudioSubsystem to PowerOff state. */
    if (!app_power_off())
      {
        err("Error: app_power_off() failure.\n");
        set_subrslt(&recog_info, TEST_FAIL); 
//        goto test_end;
      }

test_end2:
    /* Deactivate the features used by AudioSubSystem. */
    app_deact_audio_sub_system();

    /* finalize the shared memory and memory utility used by AudioSubSystem. */

    if (!app_finalize_libraries())
      {
        err("Error: finalize_libraries() failure.\n");
        set_subrslt(&recog_info, TEST_FAIL); 
      }


    pr_shm_info(&recog_info, recog_info.pr_shm);
 
   /* update loop parameter */
    rslt = update_loop_param(&recog_info);
    if(!rslt) goto test_end;

    init_testshm(&recog_info);

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
//  pr_shm_info(&recog_info);
  rslt = finalize_for_mprsc(&recog_info);
  if(!rslt)
    {
      set_subrslt(&recog_info, TEST_FAIL);
    }

  /**************************************
   *  Final judge 
   **************************************/
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
  pr_info_reset(&recog_info);
  message("Exit AudioRecognizer example\n");
notest:
  return 0;

}
