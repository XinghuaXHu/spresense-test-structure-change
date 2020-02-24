/*
 *  recorder_wav_objIf.ino - Object I/F based sound recorder example application
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,  MA 02110-1301  USA
 */

#include <arch/chip/pm.h>
#include <SDHCI.h>
#include <MediaRecorder.h>
#include <MemoryUtil.h>

MediaRecorder *theRecorder;
SDClass theSD;

File s_myFile;

bool ErrEnd = false;

/**
 * @brief Audio attention callback
 *
 * When audio internal error occurc, this function will be called back.
 */

static void mediarecorder_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}

static const int32_t recoding_frames = 400;
static const int32_t buffer_size = 3072;  /*Now WAV is 768sample,16bit,stereo. so, One frame is 3072 bytes */
static const int32_t buffer_size_high = 3072 * 10;
static uint8_t       s_buffer[buffer_size_high];
bool is_hires = false;
/**
 * @brief Recorder done callback procedure
 *
 * @param [in] event        AsRecorderEvent type indicator
 * @param [in] result       Result
 * @param [in] sub_result   Sub result
 *
 * @return true on success, false otherwise
 */

static bool mediarecorder_done_callback(AsRecorderEvent event, uint32_t result, uint32_t sub_result)
{
  printf("mp cb %x %x %x\n", event, result, sub_result);

  return true;
}

/**
 * @brief Setup Recorder
 *
 * Set input device to Mic <br>
 * Initialize recorder to encode stereo wav stream with 48kHz sample rate <br>
 * System directory "/mnt/sd0/BIN" will be searched for SRC filter (SRC file)
 * Open "Sound.wav" file <br>
 */

void setup()
{
  if (!theSD.begin()) 
    {
      printf("SD card is not present\n");
    }
  printf("Please input case number:\n");
  char case_no[16];
  gets(case_no);
  if (strcmp(case_no, "usbmsc") == 0)
    {
      if (theSD.beginUsbMsc()) {
        printf("UsbMsc connect error\n");
        exit(1);
      }
      printf("<<< Begin USB Mass Storage Operation\n");
      
      gets(case_no);
      gets(case_no);
      
      if (theSD.endUsbMsc()) {
        printf("UsbMsc disconnect error\n");
        exit(1);
      }
      printf("<<< Finish USB Mass Storage Operation\n");
      delay(1000);
      up_pm_reboot();
    }
    
  /* Initialize memory pools and message libs */

  initMemoryPools();
  createStaticPools(MEM_LAYOUT_RECORDER);

  /* start audio system */

  theRecorder = MediaRecorder::getInstance();

  theRecorder->begin(mediarecorder_attention_cb);

  puts("initialization MediaRecorder");

  /* Set capture clock */

  if (strcmp(case_no, "16266") == 0
    || strcmp(case_no, "16267") == 0
    || strcmp(case_no, "16304") == 0
    || strcmp(case_no, "16305") == 0)
    {
      is_hires = true;
      theRecorder->setCapturingClkMode(MEDIARECORDER_CAPCLK_HIRESO);
      theRecorder->activate(AS_SETRECDR_STS_INPUTDEVICE_MIC, mediarecorder_done_callback, MEDIARECORDER_BUF_SIZE*2);
    }
  else
    {
      is_hires = false;
      theRecorder->setCapturingClkMode(MEDIARECORDER_CAPCLK_NORMAL);
      theRecorder->activate(AS_SETRECDR_STS_INPUTDEVICE_MIC, mediarecorder_done_callback);
    }

  usleep(100 * 1000); /* waiting for Mic startup */

  /*
   * Initialize recorder to decode stereo wav stream with 48kHz sample rate
   * Search for SRC filter in "/mnt/sd0/BIN" directory
   */

  if (strcmp(case_no, "16263") == 0)
    theRecorder->init(AS_CODECTYPE_WAV,
                    AS_CHANNEL_MONO,
                    AS_SAMPLINGRATE_48000,
                    AS_BITLENGTH_16,
                    AS_BITRATE_8000, /* Bitrate is effective only when mp3 recording */
                    "/mnt/sd0/BIN");
  else if (strcmp(case_no, "16264") == 0)
    theRecorder->init(AS_CODECTYPE_WAV,
                    AS_CHANNEL_STEREO,
                    AS_SAMPLINGRATE_48000,
                    AS_BITLENGTH_16,
                    AS_BITRATE_8000, /* Bitrate is effective only when mp3 recording */
                    "/mnt/sd0/BIN");
  else if (strcmp(case_no, "16265") == 0)
    theRecorder->init(AS_CODECTYPE_WAV,
                    AS_CHANNEL_4CH,
                    AS_SAMPLINGRATE_48000,
                    AS_BITLENGTH_16,
                    AS_BITRATE_8000, /* Bitrate is effective only when mp3 recording */
                    "/mnt/sd0/BIN");
  else if (strcmp(case_no, "16266") == 0)
    {
      theRecorder->init(AS_CODECTYPE_WAV,
                    AS_CHANNEL_MONO,
                    AS_SAMPLINGRATE_192000,
                    AS_BITLENGTH_16,
                    AS_BITRATE_8000, /* Bitrate is effective only when mp3 recording */
                    "/mnt/sd0/BIN");
    }
  else if (strcmp(case_no, "16267") == 0)
    {
      theRecorder->init(AS_CODECTYPE_WAV,
                    AS_CHANNEL_STEREO,
                    AS_SAMPLINGRATE_192000,
                    AS_BITLENGTH_16,
                    AS_BITRATE_8000, /* Bitrate is effective only when mp3 recording */
                    "/mnt/sd0/BIN");
    }
  else if (strcmp(case_no, "16304") == 0)
    {
      theRecorder->init(AS_CODECTYPE_WAV,
                    AS_CHANNEL_MONO,
                    AS_SAMPLINGRATE_192000,
                    AS_BITLENGTH_24,
                    AS_BITRATE_8000, /* Bitrate is effective only when mp3 recording */
                    "/mnt/sd0/BIN");
    }
  else if (strcmp(case_no, "16305") == 0)
    {
      theRecorder->init(AS_CODECTYPE_WAV,
                    AS_CHANNEL_STEREO,
                    AS_SAMPLINGRATE_192000,
                    AS_BITLENGTH_24,
                    AS_BITRATE_8000, /* Bitrate is effective only when mp3 recording */
                    "/mnt/sd0/BIN");
    }
  else if (strcmp(case_no, "16308") == 0)
    {
      theRecorder->init(AS_CODECTYPE_WAV,
                    AS_CHANNEL_MONO,
                    AS_SAMPLINGRATE_48000,
                    AS_BITLENGTH_24,
                    AS_BITRATE_8000, /* Bitrate is effective only when mp3 recording */
                    "/mnt/sd0/BIN");
    }
  else
    {
      printf("case number error\n");
      exit(1);
    }
  strcat(case_no, ".wav");
  s_myFile = theSD.open(case_no, FILE_WRITE);

  /* Verify file open */

  if (!s_myFile)
    {
      printf("File open error\n");
      exit(1);
    }

  printf("Open! %d\n", s_myFile);

  /* Write wav header (Write to top of file. File size is tentative.) */

  theRecorder->writeWavHeader(s_myFile);
  puts("Write Header!");
  
  /* Start Recorder */

  theRecorder->start();
  puts("Recording Start!");

}
/**
 * @brief Audio signal process (Modify for your application)
 */
void signal_process(uint32_t size)
{
  /* Put any signal process */

  printf("Size %d [%02x %02x %02x %02x %02x %02x %02x %02x ...]\n",
         size,
         s_buffer[0],
         s_buffer[1],
         s_buffer[2],
         s_buffer[3],
         s_buffer[4],
         s_buffer[5],
         s_buffer[6],
         s_buffer[7]);
}

/**
 * @brief Execute one frame
 */
err_t execute_aframe(uint32_t* size)
{
  err_t err = theRecorder->readFrames(s_buffer, is_hires ? buffer_size_high : buffer_size, size);

  if(((err == MEDIARECORDER_ECODE_OK) || (err == MEDIARECORDER_ECODE_INSUFFICIENT_BUFFER_AREA)) && (*size > 0))
    {
      signal_process(*size);
    }
  int ret = s_myFile.write((uint8_t*)&s_buffer, *size);
  if (ret < 0)
    {
      puts("File write error.");
      err = MEDIARECORDER_ECODE_FILEACCESS_ERROR;
    }

  return err;
}

/**
 * @brief Execute frames for FIFO empty
 */
void execute_frames()
{
  uint32_t read_size = 0;
  do
    {
      err_t err = execute_aframe(&read_size);
      if ((err != MEDIARECORDER_ECODE_OK)
       && (err != MEDIARECORDER_ECODE_INSUFFICIENT_BUFFER_AREA))
        {
          break;
        }
    }
  while (read_size > 0);
}

/**
 * @brief Record audio frames
 */

void loop()
{
    static int32_t total_size = 0;
    uint32_t read_size = 0;

  /* Execute audio data */
  err_t err = execute_aframe(&read_size);
  if (err != MEDIARECORDER_ECODE_OK && err != MEDIARECORDER_ECODE_INSUFFICIENT_BUFFER_AREA)
    {
      puts("Recording Error!");
      theRecorder->stop();
      goto exitRecording;
    }
  else if (read_size>0)
    {
      total_size += read_size;
    }
  /* This sleep is adjusted by the time to write the audio stream file.
     Please adjust in according with the processing contents
     being processed at the same time by Application.
  */
//  usleep(10000);

  /* Stop Recording */
  if (total_size > (recoding_frames*(is_hires ? buffer_size_high : buffer_size)))
    {
      theRecorder->stop();

      /* Get ramaining data(flushing) */
      sleep(1); /* For data pipline stop */
      execute_frames();
      
      goto exitRecording;
    }

  if (ErrEnd)
    {
      printf("Error End\n");
      theRecorder->stop();
      goto exitRecording;
    }

  return;

exitRecording:

  theRecorder->writeWavHeader(s_myFile);
  puts("Update Header!");

  s_myFile.close();

  theRecorder->deactivate();
  theRecorder->end();
  
  puts("End Recording");
  delay(1000);
  up_pm_reboot();
}
