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
#include "rcgproc_command.h"
#ifdef CONFIG_EXAMPLES_AUDIO_RECOGNIZER_USEPREPROC
#include "userproc_command.h"
#endif /* CONFIG_EXAMPLES_AUDIO_RECOGNIZER_USEPREPROC */

#include <arch/chip/cxd56_audio.h>


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

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Type Declarations
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static bool printAudCmdResult(uint8_t command_code, AudioResult& result)
{
  if (AUDRLT_ERRORRESPONSE == result.header.result_code) {
    printf("Command code(0x%x): AUDRLT_ERRORRESPONSE:"
           "Module id(0x%x): Error code(0x%x)\n",
            command_code,
            result.error_response_param.module_id,
            result.error_response_param.error_code);
    return false;
  }
  else if (AUDRLT_ERRORATTENTION == result.header.result_code) {
    printf("Command code(0x%x): AUDRLT_ERRORATTENTION\n", command_code);
    return false;
  }
  return true;
}

static void recognizer_find_callback(AsRecognitionInfo info)
{
  printf("app:recognizer_find_callback size %d\n", info.size);

  int16_t *param = (int16_t *)info.mh.getVa();

  printf(">> %d %d %d %d\n", param[0], param[1], param[2], param[3]); 
}

static bool app_set_ready(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_READY_STATUS;
  command.header.command_code  = AUDCMD_SETREADYSTATUS;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_init_micfrontend(uint32_t sampling_rate,
                                 uint8_t ch_num,
                                 uint8_t bitlength,
                                 uint8_t preproc_type,
                                 const char *dsp_name)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_INIT_MICFRONTEND;
  command.header.command_code  = AUDCMD_INIT_MICFRONTEND;
  command.header.sub_code      = 0x00;
  command.init_micfrontend_param.ch_num       = ch_num;
  command.init_micfrontend_param.bit_length   = bitlength;
  command.init_micfrontend_param.samples      = 384;
  command.init_micfrontend_param.out_fs       = AS_SAMPLINGRATE_16000;
  command.init_micfrontend_param.preproc_type = preproc_type;
  snprintf(command.init_micfrontend_param.preprocess_dsp_path,
           AS_RECOGNIZER_FILE_PATH_LEN,
           "%s", dsp_name);
  command.init_micfrontend_param.data_dest = AsMicFrontendDataToRecognizer;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_set_recognizer_status(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_RECOGNIZER_STATUS;
  command.header.command_code  = AUDCMD_SETRECOGNIZERSTATUS;
  command.header.sub_code      = 0x00;
  command.set_recognizer_status_param.input_device = AsMicFrontendDeviceMic;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_init_recognizer(const char *dsp_name)
{
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
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_start_recognizer(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_START_RECOGNIZER;
  command.header.command_code  = AUDCMD_START_RECOGNIZER;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_stop_recognizer(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_STOP_RECOGNIZER;
  command.header.command_code  = AUDCMD_STOP_RECOGNIZER;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  if (!printAudCmdResult(command.header.command_code, result))
    {
      return false;
    }

  return true;
}

#ifdef CONFIG_EXAMPLES_AUDIO_RECOGNIZER_USEPREPROC
static bool app_init_preproc_dsp(void)
{
  static InitParam s_initparam;

  AudioCommand command;
  command.header.packet_length = LENGTH_INIT_PREPROCESS_DSP;
  command.header.command_code  = AUDCMD_INIT_PREPROCESS_DSP;
  command.header.sub_code      = 0x00;
  command.init_preproc_param.packet_addr = reinterpret_cast<uint8_t *>(&s_initparam);
  command.init_preproc_param.packet_size = sizeof(s_initparam);
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_set_preproc_dsp(void)
{
  static SetParam s_setparam;

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
  return printAudCmdResult(command.header.command_code, result);
}
#endif /* CONFIG_EXAMPLES_AUDIO_RECOGNIZER_USEPREPROC */

static bool app_init_rcgproc(void)
{
  static InitRcgParam s_initrcgparam;
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
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_set_rcgproc(void)
{
  static SetRcgParam s_setrcgparam;
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
  return printAudCmdResult(command.header.command_code, result);
}

void app_recognizer_process(uint32_t rec_time)
{
  /* Timer Start */
  time_t start_time;
  time_t cur_time;

  time(&start_time);

  do
    {
      usleep(500 * 1000);
    } while((time(&cur_time) - start_time) < rec_time);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int audio_recognizer()
{
  printf("Start AudioRecognizer example\n");
  
  /* Set recognizer operation mode. */

  if (!app_set_recognizer_status())
    {
      printf("Error: app_set_recognizer_status() failure.\n");
      return 1;
    }

  /* Init MicFrontend. */

  if (!app_init_micfrontend(AS_SAMPLINGRATE_48000,
                            AS_CHANNEL_MONO,
                            AS_BITLENGTH_16,
#ifdef CONFIG_SQA_COMBI_RECOG_RECORDER_SAMPLERATECONV
                            AsMicFrontendPreProcSrc,
                            "/mnt/sd0/BIN/SRC"
#elif defined CONFIG_SQA_COMBI_RECOG_RECORDER_USEPREPROC
                            AsMicFrontendPreProcUserCustom,
                            "/mnt/sd0/BIN/PREPROC"
#else /* CONFIG_EXAMPLES_AUDIO_RECOGNIZER_THROUGH */
                            AsMicFrontendPreProcThrough,
                            "/mnt/sd0/BIN/PREPROC" /* Not effective */
#endif /* End */
                           ))
    {
      printf("Error: app_init_micfrontend() failure.\n");
      return 1;
    }

  /* Initialize recognizer. */

  if (!app_init_recognizer("/mnt/sd0/BIN/RCGPROC"))
    {
      printf("Error: app_init_recognizer() failure.\n");
      return 1;
    }

#ifdef CONFIG_SQA_COMBI_RECOG_RECORDER_USEPREPROC
  /* Init PREPROC */

  if (!app_init_preproc_dsp())
    {
      printf("Error: app_init_preproc_dsp() failure.\n");
      return 1;
    }

  /* Set PREPROC */

  if (!app_set_preproc_dsp())
    {
      printf("Error: app_set_preproc_dsp() failure.\n");
      return 1;
    }
#endif /* CONFIG_EXAMPLES_AUDIO_RECOGNIZER_USEPREPROC */

  /* Init RCGPROC */

  if (!app_init_rcgproc())
    {
      printf("Error: app_init_rcgproc() failure.\n");
      return 1;
    }

  /* Set RCGPROC */

  if (!app_set_rcgproc())
    {
      printf("Error: app_set_rcgproc() failure.\n");
      return 1;
    }

  /* Start recognizer operation. */

  if (!app_start_recognizer())
    {
      printf("Error: app_start_recognizer() failure.\n");
      return 1;
    }

  /* Running... */

  printf("Running time is %d sec\n", RECOGNIZER_EXEC_TIME);

  app_recognizer_process(RECOGNIZER_EXEC_TIME);

  /* Stop recognizer operation. */

  if (!app_stop_recognizer())
    {
      printf("Error: app_stop_recognizer() failure.\n");
      return 1;
    }

  /* Return the state of AudioSubSystem before voice_call operation. */

  if (!app_set_ready())
    {
      printf("Error: app_set_ready() failure.\n");
      return 1;
    }

  printf("Exit AudioRecognizer example\n");

  return 0;
}
