/*
 *  recorder_wav.ino - Recorder example application for WAV(PCM)
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
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <SDHCI.h>
#include <Audio.h>
#include <arch/chip/pm.h>
#include <arch/board/board.h>

SDClass theSD;
AudioClass *theAudio;

File myFile;

bool ErrEnd = false;

/**
 * @brief Audio attention callback
 *
 * When audio internal error occurc, this function will be called back.
 */

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}

/**
 * @brief Setup recording of mp3 stream to file
 *
 * Select input device as microphone <br>
 * Initialize filetype to stereo wav with 48 Kb/s sampling rate <br>
 * Open "Sound.wav" file in write mode
 */

static const int32_t recoding_frames = 400;
static const int32_t recoding_size = recoding_frames*3072; /* 2ch, 16bit, 768sample */
static const int32_t recoding_size_low = 100*3072;
static const int32_t recoding_size_high = 1000*3072;

char cmd[16] = {0};

void setup()
{
  if (!theSD.begin()) 
    {
      printf("SD card is not present\n");
    }
  printf("Please input cmd:\n");
  gets(cmd);
  if (strcmp(cmd, "usbmsc") == 0)
    {
      if (theSD.beginUsbMsc()) {
        printf("UsbMsc connect error\n");
        exit(1);
      }
      printf("<<< Begin USB Mass Storage Operation\n");
      
      gets(cmd);
      gets(cmd);
      
      if (theSD.endUsbMsc()) {
        printf("UsbMsc disconnect error\n");
        exit(1);
      }
      printf("<<< Finish USB Mass Storage Operation\n");
      delay(1000);
      up_pm_reboot();
    }

  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* Select input device as microphone */
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC, 0, SIMPLE_FIFO_BUF_SIZE*2, true);

  /*
   * Initialize filetype to stereo wav with 48 Kb/s sampling rate
   * Search for WAVDEC codec in "/mnt/sd0/BIN" directory
   */
  theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_48000,AS_BITLENGTH_16,AS_CHANNEL_8CH);

  puts("Init Recorder!");

  /* Open file for data write on SD card */
  char filename[32] = {0};
  sprintf(filename, "%s.wav", cmd);
  myFile = theSD.open(filename, FILE_WRITE);
  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }

  theAudio->writeWavHeader(myFile);
  puts("Write Header!");

  theAudio->startRecorder();
  puts("Recording Start!");

}

void loop() 
{
  err_t err;
  /* recording end condition */
  if ((strcmp(cmd, "16645") == 0 && theAudio->getRecordingSize() > recoding_size_low)
    || (strcmp(cmd, "16656") == 0 && theAudio->getRecordingSize() > recoding_size_high)
    || (strcmp(cmd, "16645") != 0 && strcmp(cmd, "16656") != 0 && theAudio->getRecordingSize() > recoding_size))
    {
      theAudio->stopRecorder();
      sleep(1);
      err = theAudio->readFrames(myFile);

      goto exitRecording;
    }


  /* Read frames to record in file */
  err = theAudio->readFrames(myFile);

  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("File End! =%d\n",err);
      theAudio->stopRecorder();
      goto exitRecording;
    }

  if (ErrEnd)
    {
      printf("Error End\n");
      theAudio->stopRecorder();
      goto exitRecording;
    }

  /* This sleep is adjusted by the time to write the audio stream file.
     Please adjust in according with the processing contents
     being processed at the same time by Application.
  */
//  usleep(10000);

  return;

exitRecording:

  theAudio->closeOutputFile(myFile);
  myFile.close();
  
  theAudio->setReadyMode();
  theAudio->end();
  
  puts("End Recording");
  delay(1000);
  up_pm_reboot();
}
