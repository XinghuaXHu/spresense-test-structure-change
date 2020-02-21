/*
 *  recorder.ino - Recorder example application
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
 * Initialize filetype to stereo mp3 with 48 Kb/s sampling rate <br>
 * Open "Sound.mp3" file in write mode
 */

/* Recording time[second] */

static const int32_t recoding_time = 10;

/* Recording bit rate
 * Set in bps.
 * Note: 96kbps fixed
 */

static const int32_t recoding_bitrate = 96000;

/* Bytes per second */

static const int32_t recoding_byte_per_second = (recoding_bitrate / 8);

/* Total recording size */

static const int32_t recoding_size = recoding_byte_per_second * recoding_time;

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
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);

  /*
   * Initialize filetype to stereo mp3 with 48 Kb/s sampling rate
   * Search for MP3 codec in "/mnt/sd0/BIN" directory
   */

  if (strcmp(cmd, "16344") == 0)
  {
    theAudio->initRecorder(AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, AS_CHANNEL_STEREO);
  }
  else if (strcmp(cmd, "16642") == 0)
  {
    theAudio->initRecorder(AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_16000, AS_CHANNEL_MONO);
  }
  else if (strcmp(cmd, "16643") == 0)
  {
    theAudio->initRecorder(AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_16000, AS_CHANNEL_STEREO);
  }
  else if (strcmp(cmd, "16644") == 0)
  {
    theAudio->initRecorder(AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, AS_CHANNEL_MONO);
  }
  else
  {
    printf("case number error\n");
    exit(1);
  }
 
  puts("Init Recorder!");

  /* Open file for data write on SD card */
  char filename[32] = {0};
  sprintf(filename, "%s.mp3", cmd);
  
    myFile = theSD.open(filename, FILE_WRITE);
  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }

  theAudio->startRecorder();
  puts("Recording Start!");
}

/**
 * @brief Record given frame number
 */
void loop() 
{
  err_t err;
  /* recording end condition */
  if (theAudio->getRecordingSize() > recoding_size)
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
  //exit(1);

  delay(1000);
  up_pm_reboot();
}
