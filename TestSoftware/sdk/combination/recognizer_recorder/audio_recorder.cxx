/****************************************************************************
 * audio_recorder/audio_recorder_main.cxx
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
#include <audio/utilities/frame_samples.h>
#ifdef CONFIG_SQA_COMBI_RECOG_RECORDER_USEPREPROC
#include "userproc_command.h"
#endif /* CONFIG_EXAMPLES_AUDIO_RECORDER_USEPREPROC */

#include <arch/chip/cxd56_audio.h>

/* Section number of memory layout to use */

#define AUDIO_SECTION   SECTION_NO0

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

#define USE_MIC_CHANNEL_NUM  2

/* For FIFO. */

#define READ_SIMPLE_FIFO_SIZE (3072 * USE_MIC_CHANNEL_NUM)
#define SIMPLE_FIFO_FRAME_NUM 60
#define SIMPLE_FIFO_BUF_SIZE  (READ_SIMPLE_FIFO_SIZE * SIMPLE_FIFO_FRAME_NUM)

/* For line buffer mode. */

#define STDIO_BUFFER_SIZE 4096

/* Length of recording file name */

#define MAX_PATH_LENGTH 128

/* Recording time(sec). */

#define RECORDER_REC_TIME 10

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Type Declarations
 ****************************************************************************/

/* For FIFO */

struct recorder_fifo_info_s
{
  CMN_SimpleFifoHandle        handle;
  AsRecorderOutputDeviceHdlr  output_device;
  uint32_t fifo_area[SIMPLE_FIFO_BUF_SIZE/sizeof(uint32_t)];
  uint8_t  write_buf[READ_SIMPLE_FIFO_SIZE];
};

struct recorder_file_info_s
{
  uint32_t  sampling_rate;
  uint8_t   channel_number;
  uint8_t   bitwidth;
  uint8_t   codec_type;
  uint16_t  format_type;
  uint32_t  size;
  DIR      *dirp;
  FILE     *fd;
};

struct recorder_info_s
{
  struct recorder_fifo_info_s  fifo;
  struct recorder_file_info_s  file;
};

/* Recording parameter. */

enum codec_type_e
{
  CODEC_TYPE_MP3 = 0,
  CODEC_TYPE_LPCM,
  CODEC_TYPE_OPUS,
  CODEC_TYPE_NUM
};

enum sampling_rate_e
{
  SAMPLING_RATE_8K = 0,
  SAMPLING_RATE_16K,
  SAMPLING_RATE_48K,
  SAMPLING_RATE_192K,
  SAMPLING_RATE_NUM
};

enum channel_type_e
{
  CHAN_TYPE_MONO = 0,
  CHAN_TYPE_STEREO,
  CHAN_TYPE_4CH,
  CHAN_TYPE_6CH,
  CHAN_TYPE_8CH,
  CHAN_TYPE_NUM
};

enum bitwidth_e
{
  BITWIDTH_16BIT = 0,
  BITWIDTH_24BIT,
  BITWIDTH_32BIT
};

enum microphone_device_e
{
  MICROPHONE_ANALOG = 0,
  MICROPHONE_DIGITAL,
  MICROPHONE_NUM
};

enum format_type_e
{
  FORMAT_TYPE_RAW = 0,
  FORMAT_TYPE_WAV,
  FORMAT_TYPE_OGG,
  FORMAT_TYPE_NUM
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static recorder_info_s s_recorder_info;

WavContainerFormat* s_container_format = NULL;
static WAVHEADER  s_wav_header;
/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void outputDeviceCallback(uint32_t size)
{
    /* do nothing */
}

static bool app_update_wav_file_size(void)
{
  fseek(s_recorder_info.file.fd, 0, SEEK_SET);

  s_container_format->getHeader(&s_wav_header, s_recorder_info.file.size);

  size_t ret = fwrite((const void *)&s_wav_header,
                      1,
                      sizeof(WAVHEADER),
                      s_recorder_info.file.fd);
  if (ret != sizeof(WAVHEADER))
    {
      printf("Fail to write file(wav header)\n");
      return false;
    }
  return true;
}

static bool app_write_wav_header(void)
{
  s_container_format->getHeader(&s_wav_header, 0);

  size_t ret = fwrite((const void *)&s_wav_header,
                      1,
                      sizeof(WAVHEADER),
                      s_recorder_info.file.fd);
  if (ret != sizeof(WAVHEADER))
    {
      printf("Fail to write file(wav header)\n");
      return false;
    }
  return true;
}

static bool app_init_wav_header(void)
{
  return s_container_format->init(FORMAT_ID_PCM,
                                 s_recorder_info.file.channel_number,
                                 s_recorder_info.file.sampling_rate,
                                 s_recorder_info.file.bitwidth);
}

static bool app_open_output_file(void)
{
  static char fname[MAX_PATH_LENGTH];

  struct tm *cur_time;
  struct timespec cur_sec;

  clock_gettime(CLOCK_REALTIME, &cur_sec);
  cur_time = gmtime(&cur_sec.tv_sec);

  if ((s_recorder_info.file.codec_type == AS_CODECTYPE_MP3) &&
      (s_recorder_info.file.format_type == FORMAT_TYPE_RAW))
    {
      snprintf(fname, MAX_PATH_LENGTH, "%s/%04d%02d%02d_%02d%02d%02d.mp3",
        RECFILE_ROOTPATH,
        cur_time->tm_year,
        cur_time->tm_mon,
        cur_time->tm_mday,
        cur_time->tm_hour,
        cur_time->tm_min,
        cur_time->tm_sec);
    }
  else if (s_recorder_info.file.codec_type == AS_CODECTYPE_LPCM)
    {
      if (s_recorder_info.file.format_type == FORMAT_TYPE_WAV)
        {
          snprintf(fname, MAX_PATH_LENGTH, "%s/%04d%02d%02d_%02d%02d%02d.wav",
            RECFILE_ROOTPATH,
            cur_time->tm_year,
            cur_time->tm_mon,
            cur_time->tm_mday,
            cur_time->tm_hour,
            cur_time->tm_min,
            cur_time->tm_sec);
        }
      else
        {
          snprintf(fname, MAX_PATH_LENGTH, "%s/%04d%02d%02d_%02d%02d%02d.pcm",
            RECFILE_ROOTPATH,
            cur_time->tm_year,
            cur_time->tm_mon,
            cur_time->tm_mday,
            cur_time->tm_hour,
            cur_time->tm_min,
            cur_time->tm_sec);
        }
    }
  else if ((s_recorder_info.file.codec_type == AS_CODECTYPE_OPUS) &&
           (s_recorder_info.file.format_type == FORMAT_TYPE_RAW))
    {
      snprintf(fname, MAX_PATH_LENGTH, "%s/%04d%02d%02d_%02d%02d%02d.raw",
        RECFILE_ROOTPATH,
        cur_time->tm_year,
        cur_time->tm_mon,
        cur_time->tm_mday,
        cur_time->tm_hour,
        cur_time->tm_min,
        cur_time->tm_sec);
    }
  else
    {
      printf("Unsupported format\n");
      return false;
    }

  s_recorder_info.file.fd = fopen(fname, "w");
  if (s_recorder_info.file.fd == 0)
    {
      printf("open err(%s)\n", fname);
      return false;
    }
  setvbuf(s_recorder_info.file.fd, NULL, _IOLBF, STDIO_BUFFER_SIZE);
  printf("Record data to %s.\n", &fname[0]);

  if (s_recorder_info.file.format_type == FORMAT_TYPE_WAV)
    {
      if (!app_init_wav_header())
      {
        printf("Error: app_init_wav_header() failure.\n");
        return false;
      }

      if (!app_write_wav_header())
      {
        printf("Error: app_write_wav_header() failure.\n");
        return false;
      }
  }
  return true;
}

static void app_close_output_file(void)
{
  fclose(s_recorder_info.file.fd);
}

static void app_write_output_file(uint32_t size)
{
  ssize_t ret;

  if (size == 0 || CMN_SimpleFifoGetOccupiedSize(&s_recorder_info.fifo.handle) == 0)
    {
      return;
    }

  if (CMN_SimpleFifoPoll(&s_recorder_info.fifo.handle,
                        (void*)s_recorder_info.fifo.write_buf,
                        size) == 0)
    {
      printf("ERROR: Fail to get data from simple FIFO.\n");
      return;
    }

  ret = fwrite(s_recorder_info.fifo.write_buf, 1, size, s_recorder_info.file.fd);
  if (ret < 0) {
    printf("ERROR: Cannot write recorded data to output file.\n");
    app_close_output_file();
    return;
  }
  s_recorder_info.file.size += size;
}

static bool app_init_simple_fifo(void)
{
  if (CMN_SimpleFifoInitialize(&s_recorder_info.fifo.handle,
                               s_recorder_info.fifo.fifo_area,
                               SIMPLE_FIFO_BUF_SIZE, NULL) != 0)
    {
      printf("Error: Fail to initialize simple FIFO.");
      return false;
    }
  CMN_SimpleFifoClear(&s_recorder_info.fifo.handle);

  s_recorder_info.fifo.output_device.simple_fifo_handler =
    (void*)(&s_recorder_info.fifo.handle);
  s_recorder_info.fifo.output_device.callback_function = outputDeviceCallback;

  return true;
}

static void app_pop_simple_fifo(void)
{
  size_t occupied_simple_fifo_size =
    CMN_SimpleFifoGetOccupiedSize(&s_recorder_info.fifo.handle);
  uint32_t output_size = 0;

  while (occupied_simple_fifo_size > 0)
    {
      output_size = (occupied_simple_fifo_size > READ_SIMPLE_FIFO_SIZE) ?
        READ_SIMPLE_FIFO_SIZE : occupied_simple_fifo_size;
      app_write_output_file(output_size);
      occupied_simple_fifo_size -= output_size;
    }
}

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

/*
static void app_attention_callback(const ErrorAttentionParam *attparam)
{
  printf("Attention!! %s L%d ecode %d subcode %d\n",
          attparam->error_filename,
          attparam->line_number,
          attparam->error_code,
          attparam->error_att_sub_code);
}
*/
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

static bool app_init_mic_gain(void)
{
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
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_set_recording_param(codec_type_e codec_type,
                                    sampling_rate_e sampling_rate,
                                    channel_type_e ch_type,
                                    bitwidth_e bitwidth)
{
  switch(codec_type)
    {
      case CODEC_TYPE_MP3:
        s_recorder_info.file.codec_type = AS_CODECTYPE_MP3;
        break;
      case CODEC_TYPE_LPCM:
        s_recorder_info.file.codec_type = AS_CODECTYPE_LPCM;
        break;
      case CODEC_TYPE_OPUS:
        s_recorder_info.file.codec_type = AS_CODECTYPE_OPUS;
        break;
      default:
        printf("Error: Invalid codec type(%d)\n", codec_type);
        return false;
    }

  switch(sampling_rate)
    {
      case SAMPLING_RATE_8K:
        s_recorder_info.file.sampling_rate = AS_SAMPLINGRATE_8000;
        break;
      case SAMPLING_RATE_16K:
        s_recorder_info.file.sampling_rate = AS_SAMPLINGRATE_16000;
        break;
      case SAMPLING_RATE_48K:
        s_recorder_info.file.sampling_rate = AS_SAMPLINGRATE_48000;
        break;
      case SAMPLING_RATE_192K:
        s_recorder_info.file.sampling_rate = AS_SAMPLINGRATE_192000;
        break;
      default:
        printf("Error: Invalid sampling rate(%d)\n", sampling_rate);
        return false;
    }

  switch(ch_type)
    {
      case CHAN_TYPE_MONO:
        s_recorder_info.file.channel_number = AS_CHANNEL_MONO;
        break;
      case CHAN_TYPE_STEREO:
        s_recorder_info.file.channel_number = AS_CHANNEL_STEREO;
        break;
      case CHAN_TYPE_4CH:
        s_recorder_info.file.channel_number = AS_CHANNEL_4CH;
        break;
      case CHAN_TYPE_6CH:
        s_recorder_info.file.channel_number = AS_CHANNEL_6CH;
        break;
      case CHAN_TYPE_8CH:
        s_recorder_info.file.channel_number = AS_CHANNEL_8CH;
        break;
      default:
        printf("Error: Invalid channel type(%d)\n", ch_type);
        return false;
    }

  switch (bitwidth)
    {
      case BITWIDTH_16BIT:
        s_recorder_info.file.bitwidth = AS_BITLENGTH_16;
        break;
      case BITWIDTH_24BIT:
        s_recorder_info.file.bitwidth = AS_BITLENGTH_24;
        break;
      case BITWIDTH_32BIT:
        s_recorder_info.file.bitwidth = AS_BITLENGTH_32;
        break;
      default:
        printf("Error: Invalid bit width(%d)\n", bitwidth);
        return false;
    }

  return true;
}

static bool app_set_recorder_status(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_RECORDER_STATUS;
  command.header.command_code  = AUDCMD_SETRECORDERSTATUS;
  command.header.sub_code      = 0x00;
  command.set_recorder_status_param.input_device          = AS_SETRECDR_STS_INPUTDEVICE_MIC;
  command.set_recorder_status_param.input_device_handler  = 0x00;
  command.set_recorder_status_param.output_device         = AS_SETRECDR_STS_OUTPUTDEVICE_RAM;
  command.set_recorder_status_param.output_device_handler = &s_recorder_info.fifo.output_device;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_init_micfrontend(uint8_t preproc_type,
                                 const char *dsp_name)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_INIT_MICFRONTEND;
  command.header.command_code  = AUDCMD_INIT_MICFRONTEND;
  command.header.sub_code      = 0x00;
  command.init_micfrontend_param.ch_num       = s_recorder_info.file.channel_number;
  command.init_micfrontend_param.bit_length   = s_recorder_info.file.bitwidth;
  command.init_micfrontend_param.samples      = getCapSampleNumPerFrame(s_recorder_info.file.codec_type,
                                                                        s_recorder_info.file.sampling_rate);
  command.init_micfrontend_param.preproc_type = preproc_type;
  snprintf(command.init_micfrontend_param.preprocess_dsp_path,
           AS_PREPROCESS_FILE_PATH_LEN,
           "%s", dsp_name);
  command.init_micfrontend_param.data_dest = AsMicFrontendDataToRecorder;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static void app_init_recorder_wav(AudioCommand* command)
{
  command->recorder.init_param.codec_type = AS_CODECTYPE_LPCM;
  s_recorder_info.file.format_type = FORMAT_TYPE_WAV;
}

static void app_init_recorder_mp3(AudioCommand* command)
{
  command->recorder.init_param.codec_type = AS_CODECTYPE_MP3;
  command->recorder.init_param.bitrate    = AS_BITRATE_96000;
  s_recorder_info.file.format_type = FORMAT_TYPE_RAW;
}

static void app_init_recorder_opus(AudioCommand* command)
{
  command->recorder.init_param.codec_type = AS_CODECTYPE_OPUS;
  command->recorder.init_param.bitrate    = AS_BITRATE_8000;
  command->recorder.init_param.computational_complexity = AS_INITREC_COMPLEXITY_0;
  s_recorder_info.file.format_type = FORMAT_TYPE_RAW;
}

static bool app_init_recorder(codec_type_e codec_type,
                              sampling_rate_e sampling_rate,
                              channel_type_e ch_type,
                              bitwidth_e bitwidth)
{
  if (!app_set_recording_param(codec_type,
                               sampling_rate,
                               ch_type,
                               bitwidth))
    {
      printf("Error: app_set_recording_param() failure.\n");
      return false;
    }

  AudioCommand command;
  command.header.packet_length = LENGTH_INIT_RECORDER;
  command.header.command_code  = AUDCMD_INITREC;
  command.header.sub_code      = 0x00;
  command.recorder.init_param.sampling_rate  = s_recorder_info.file.sampling_rate;
  command.recorder.init_param.channel_number = s_recorder_info.file.channel_number;
  command.recorder.init_param.bit_length     = s_recorder_info.file.bitwidth;
  snprintf(command.recorder.init_param.dsp_path, AS_AUDIO_DSP_PATH_LEN, "%s", DSPBIN_PATH);

  switch (s_recorder_info.file.codec_type)
    {
      case AS_CODECTYPE_LPCM:
        app_init_recorder_wav(&command);
        break;
      case AS_CODECTYPE_MP3:
        app_init_recorder_mp3(&command);
        break;
      case AS_CODECTYPE_OPUS:
        app_init_recorder_opus(&command);
        break;
      default:
        break;
  }

  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);

}

static bool app_start_recorder(void)
{
  s_recorder_info.file.size = 0;
  if (!app_open_output_file())
    {
      return false;
    }
  CMN_SimpleFifoClear(&s_recorder_info.fifo.handle);

  AudioCommand command;
  command.header.packet_length = LENGTH_START_RECORDER;
  command.header.command_code  = AUDCMD_STARTREC;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_stop_recorder(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_STOP_RECORDER;
  command.header.command_code  = AUDCMD_STOPREC;
  command.header.sub_code      = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  if (!printAudCmdResult(command.header.command_code, result))
    {
      return false;
    }

  size_t occupied_simple_fifo_size = CMN_SimpleFifoGetOccupiedSize(&s_recorder_info.fifo.handle);
  uint32_t output_size = 0;

  while (occupied_simple_fifo_size > 0)
    {
      output_size = (occupied_simple_fifo_size > READ_SIMPLE_FIFO_SIZE) ?
        READ_SIMPLE_FIFO_SIZE : occupied_simple_fifo_size;
      app_write_output_file(output_size);
      occupied_simple_fifo_size = CMN_SimpleFifoGetOccupiedSize(&s_recorder_info.fifo.handle);
    }

  if (s_recorder_info.file.format_type == FORMAT_TYPE_WAV)
    {
      if (!app_update_wav_file_size())
        {
          printf("Error: app_write_wav_header() failure.\n");
        }
    }

  app_close_output_file();
  return true;
}

#ifdef CONFIG_EXAMPLES_AUDIO_RECORDER_USEPREPROC
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
#endif /* CONFIG_EXAMPLES_AUDIO_RECORDER_USEPREPROC */

static bool app_set_clkmode(int clk_mode)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SETRENDERINGCLK;
  command.header.command_code  = AUDCMD_SETRENDERINGCLK;
  command.header.sub_code      = 0x00;
  command.set_renderingclk_param.clk_mode = clk_mode;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_open_file_dir(void)
{
  DIR *dirp;
  int ret;
  const char *name = RECFILE_ROOTPATH;

  dirp = opendir("/mnt");
  if (!dirp)
    {
      printf("opendir err(errno:%d)\n",errno);
      return false;
    }
  ret = mkdir(name, 0777);
  if (ret != 0)
    {
      if(errno != EEXIST)
        {
          printf("mkdir err(errno:%d)\n",errno);
          return false;
        }
    }

  s_recorder_info.file.dirp = dirp;
  return true;
}

static bool app_close_file_dir(void)
{
  closedir(s_recorder_info.file.dirp);

  return true;
}

void app_recorde_process(uint32_t rec_time)
{
  /* Timer Start */
  time_t start_time;
  time_t cur_time;

  time(&start_time);

  do
    {
      /* Check the FIFO every 5 ms and fill if there is space. */

      usleep(5 * 1000);
      app_pop_simple_fifo();

    } while((time(&cur_time) - start_time) < rec_time);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int audio_recorder()
{
  printf("Start AudioRecorder example\n");

  /* Open directory of recording file. */

  if (!app_open_file_dir())
    {
      printf("Error: app_open_file_dir() failure.\n");
      return 1;
    }
  
  /* Initialize simple fifo. */

  if (!app_init_simple_fifo())
    {
      printf("Error: app_init_simple_fifo() failure.\n");
      return false;
    }

  /* Set the initial gain of the microphone to be used. */

  if (!app_init_mic_gain())
    {
      printf("Error: app_init_mic_gain() failure.\n");
      return 1;
    }

  /* Set audio clock mode. */

  sampling_rate_e sampling_rate = SAMPLING_RATE_48K;

  if (!app_set_clkmode((sampling_rate == SAMPLING_RATE_192K) ? AS_CLKMODE_HIRES : AS_CLKMODE_NORMAL))
    {
      printf("Error: app_set_clkmode() failure.\n");
      return 1;
    }

  /* Set recorder operation mode. */

  if (!app_set_recorder_status())
    {
      printf("Error: app_set_recorder_status() failure.\n");
      return 1;
    }

  /* Initialize recorder. */

  if (!app_init_recorder(CODEC_TYPE_LPCM,
                         sampling_rate,
                         CHAN_TYPE_STEREO,
                         BITWIDTH_16BIT))
    {
      printf("Error: app_init_recorder() failure.\n");
      return 1;
    }

  /* Initialize MicFrontend. */

  if (!app_init_micfrontend(
#ifdef CONFIG_EXAMPLES_AUDIO_RECORDER_USEPREPROC
                            AsMicFrontendPreProcUserCustom,
#else /* CONFIG_EXAMPLES_AUDIO_RECORDER_USEPREPROC */
                            AsMicFrontendPreProcThrough,
#endif /* CONFIG_EXAMPLES_AUDIO_RECORDER_USEPREPROC */
                            "/mnt/sd0/BIN/PREPROC"))
    {
      printf("Error: app_init_micfrontend() failure.\n");
      return 1;
    }

#ifdef CONFIG_EXAMPLES_AUDIO_RECORDER_USEPREPROC
  if (!app_init_preproc_dsp())
    {
      printf("Error: app_init_preproc_dsp() failure.\n");
      return 1;
    }

  if (!app_set_preproc_dsp())
    {
      printf("Error: app_set_preproc_dsp() failure.\n");
      return 1;
    }
#endif /* CONFIG_EXAMPLES_AUDIO_RECORDER_USEPREPROC */

  /* Start recorder operation. */

  if (!app_start_recorder())
    {
      printf("Error: app_start_recorder() failure.\n");
      return 1;
    }

  /* Running... */

  printf("Running time is %d sec\n", RECORDER_REC_TIME);

  app_recorde_process(RECORDER_REC_TIME);

  /* Stop recorder operation. */

  if (!app_stop_recorder())
    {
      printf("Error: app_stop_recorder() failure.\n");
      return 1;
    }

  /* Close directory of recording file. */

  if (!app_close_file_dir())
    {
      printf("Error: app_close_contents_dir() failure.\n");
      return 1;
    }

  /* Return the state of AudioSubSystem before voice_call operation. */

  if (!app_set_ready())
    {
      printf("Error: app_set_ready() failure.\n");
      return 1;
    }

  printf("Exit AudioRecorder example\n");

  return 0;
}
