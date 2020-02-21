/*
 *  rec_play.ino - Record and playback audio example application
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

#include <Audio.h>
#include <SDHCI.h>
#include <arch/chip/pm.h>

#define RecordLoopNum 800
#define RecordShortLoopNum 200

enum
{
  PlayReady = 0,
  Playing,
  RecordReady,
  Recording,
  CmdWaiting
};

SDClass theSD;
AudioClass *theAudio;

File myFile;
bool is_play_origin = false;
bool is_record_short = false;
static int s_state = CmdWaiting;
bool play_mode_wav = true;

/**
 * @brief Set up audio library to record and play
 *
 * Get instance of audio library and begin.
 * Set rendering clock to NORMAL(48kHz).
 */
void setup()
{
  if (!theSD.begin()) 
    {
      printf("SD card is not present\n");
    }
}

void cmdWait()
{
  printf("Please input cmd:\n");
  char cmd[16];
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
  if (strcmp(cmd, "begin_audio_wav") == 0)
    {
      // start audio system
      play_mode_wav = true;
      theAudio = AudioClass::getInstance();
    
      theAudio->begin();

      puts("initialization Audio Library");

      /* Set output device to speaker */
      theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
    }
  if (strcmp(cmd, "begin_audio_mp3") == 0)
    {
      // start audio system
      play_mode_wav = false;
      theAudio = AudioClass::getInstance();
    
      theAudio->begin();

      puts("initialization Audio Library");

      /* Set output device to speaker */
      theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
    }
  if (strcmp(cmd, "end_audio") == 0)
    {
      theAudio->end();
      up_pm_reboot();
    }
  if (strcmp(cmd, "play_origin") == 0)
    {
      is_play_origin = true;
      s_state = PlayReady;
    }
  if (strcmp(cmd, "record_short") == 0)
    {
      is_record_short = true;
      s_state = RecordReady;
    }
  if (strcmp(cmd, "play") == 0)
    {
      is_play_origin = false;
      s_state = PlayReady;
    }
  if (strcmp(cmd, "record") == 0)
    {
      is_record_short = false;
      s_state = RecordReady;
    }
  if (strcmp(cmd, "reboot") == 0)
    {
      up_pm_reboot();
    }
}
/**
 * @brief Audio player set up and start
 *
 * Set audio player to play mp3/Stereo audio.
 * Audio data is read from SD card.
 *
 */
void playerMode(char *fname)
{
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP);

  /*
   * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect"
   * Search for MP3 decoder in "/mnt/sd0/BIN" directory
   */
  err_t err;
  if (play_mode_wav)
  {
      err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_WAV, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, AS_CHANNEL_STEREO);
  }
  else
  {
      err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);
  }

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Player0 initialize error\n");
      exit(1);
    }

  /* Open file placed on SD card */
  myFile = theSD.open(fname);

  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }
  printf("Open! %d\n",myFile);

  /* Send first frames to be decoded */
  err = theAudio->writeFrames(AudioClass::Player0, myFile);

  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("File Read Error! =%d\n",err);
      myFile.close();
      exit(1);
    }

  puts("Play!");

  /* Main volume set to -16.0 dB, Main player and sub player set to 0 dB */
  theAudio->setVolume(0, 0, 0);
  theAudio->startPlayer(AudioClass::Player0);
}

/**
 * @brief Supply audio data to the buffer
 */
bool playStream()
{
  /* Send new frames to decode in a loop until file ends */
  err_t err = theAudio->writeFrames(AudioClass::Player0, myFile);

  /*  Tell when player file ends */
  if (err == AUDIOLIB_ECODE_FILEEND)
    {
      printf("Main player File End!\n");
    }

  /* Show error code from player and stop */
  if (err)
    {
      printf("Main player error code: %d\n", err);
      goto stop_player;
    }

  /* This sleep is adjusted by the time to read the audio stream file.
     Please adjust in according with the processing contents
     being processed at the same time by Application.
  */
  
  usleep(40000);
  
  /* Don't go further and continue play */
  return false;

stop_player:
//  sleep(1);
  theAudio->stopPlayer(AudioClass::Player0);
  myFile.close();

  /* return play end */
  return true;
}

/**
 * @brief Audio recorder set up and start
 *
 * Set audio recorder to record mp3/Stereo audio.
 * Audio data is written to SD card.
 *
 */
void recorderMode(char *fname)
{
  
  /* Select input device as analog microphone */
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);

  /*
   * Initialize filetype to stereo mp3 with 48 Kb/s sampling rate
   * Search for MP3 codec in "/mnt/sd0/BIN" directory
   */
  if (play_mode_wav)
  {
      theAudio->initRecorder(AS_CODECTYPE_WAV, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, AS_CHANNEL_STEREO);
  }
  else
  {
     theAudio->initRecorder(AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, AS_CHANNEL_STEREO);
  }


  /* Open file for data write on SD card */
  myFile = theSD.open(fname, FILE_WRITE);
  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }
  puts("Open!");

  puts("Rec!");

  theAudio->startRecorder();
}

/**
 * @brief Read audio data from the buffer
 */
bool recordStream()
{
  static int cnt = 0;
  err_t err = AUDIOLIB_ECODE_OK;

  /* recording end condition */
  if ((is_record_short && cnt > RecordShortLoopNum)
      || (!is_record_short && cnt > RecordLoopNum))
    {
      puts("End Recording");
      goto stop_recorder;
    }

  /* Read frames to record in file */
  err = theAudio->readFrames(myFile);

  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("File End! =%d\n",err);
      goto stop_recorder;
    }

  /* This sleep is adjusted by the time to write the audio stream file.
     Please adjust in according with the processing contents
     being processed at the same time by Application.
  */
  usleep(10000);
  cnt++;

  /* return still recording */
  return false;

stop_recorder:
//  sleep(1);
  theAudio->stopRecorder();
  theAudio->closeOutputFile(myFile);
  cnt = 0;

  /* return record end */
  return true;
}

/**
 * @brief Rec and play streams
 *
 * Repeat recording and playing stream files
 */
void loop()
{
  static int fcnt = 0;
  char fname[16] = "RecPlay";

  switch (s_state)
    {
      case CmdWaiting:
        cmdWait();
        break;

      case PlayReady:
        if (is_play_origin)
        {
          if (play_mode_wav)
          {
            snprintf(fname, sizeof(fname), "Origin.wav");
          }
          else
          {
            snprintf(fname, sizeof(fname), "Origin.mp3");
          }
        }
          
        else
        {
          if (play_mode_wav)
          {
            snprintf(fname, sizeof(fname), "RecPlay%d.wav", fcnt);
          }
          else
          {
            snprintf(fname, sizeof(fname), "RecPlay%d.mp3", fcnt);
          }
        }
          
        playerMode(fname);
        s_state = Playing;
        break;

      case Playing:
        if (playStream())
          {
            theAudio->setReadyMode();
            s_state = CmdWaiting;
          }
        break;
        
      case RecordReady:
        fcnt++;
        if (play_mode_wav)
        {
          snprintf(fname, sizeof(fname), "RecPlay%d.wav", fcnt);
        }
        else
        {
          snprintf(fname, sizeof(fname), "RecPlay%d.mp3", fcnt);
        }
        
        recorderMode(fname);
        s_state = Recording;
        break;
        
      case Recording:
        if (recordStream())
          {
            theAudio->setReadyMode();
            s_state = CmdWaiting;
          }
        break;

      default:
        break;
    }
}