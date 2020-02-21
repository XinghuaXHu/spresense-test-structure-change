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

SDClass theSD;
AudioClass *theAudio;

File myFile;

bool ErrEnd = false;
bool TestEnd = false;

#include <LowPower.h>
#include <RTC.h>
int loop_count = 0;
const char* boot_cause_strings[] = {
  "Power On Reset with Power Supplied",
  "System WDT expired or Self Reboot",
  "Chip WDT expired",
  "WKUPL signal detected in deep sleep",
  "WKUPS signal detected in deep sleep",
  "RTC Alarm expired in deep sleep",
  "USB Connected in deep sleep",
  "Others in deep sleep",
  "SCU Interrupt detected in cold sleep",
  "RTC Alarm0 expired in cold sleep",
  "RTC Alarm1 expired in cold sleep",
  "RTC Alarm2 expired in cold sleep",
  "RTC Alarm Error occurred in cold sleep",
  "",
  "",
  "",
  "GPIO detected in cold sleep(irq36)",
  "GPIO detected in cold sleep(irq37)",
  "GPIO detected in cold sleep(irq38)",
  "GPIO detected in cold sleep(irq39)",
  "GPIO detected in cold sleep(irq40)",
  "GPIO detected in cold sleep(irq41)",
  "GPIO detected in cold sleep(irq42)",
  "GPIO detected in cold sleep(irq43)",
  "GPIO detected in cold sleep(irq44)",
  "GPIO detected in cold sleep(irq45)",
  "GPIO detected in cold sleep(irq46)",
  "GPIO detected in cold sleep(irq47)",
  "SEN_INT signal detected in cold sleep",
  "PMIC signal detected in cold sleep",
  "USB Disconnected in cold sleep",
  "USB Connected in cold sleep",
  "Power On Reset",
};

bootcause_e bc;

void printBootCause(bootcause_e bc)
{
  Serial.println("--------------------------------------------------");
  Serial.print("Boot Cause: (");
  Serial.print(bc);
  Serial.print(") ");
  Serial.print(boot_cause_strings[bc]);
  if ((COLD_GPIO_IRQ36 <= bc) && (bc <= COLD_GPIO_IRQ47)) {
    // Wakeup by GPIO
    int pin = LowPower.getWakeupPin(bc);
    Serial.print(" <- pin ");
    Serial.print(pin);
  }
  Serial.println();
  Serial.println("--------------------------------------------------");
}

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

void setup()
{
  if (!theSD.begin()) 
  {
    printf("SD card is not present\n");
  }
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

    if (theSD.endUsbMsc()) {
      printf("UsbMsc disconnect error\n");
      exit(1);
    }
    printf("<<< Finish USB Mass Storage Operation\n");
  }
  printf("Please input cmd:\n");
  gets(cmd);
#if 0
  Serial = Serial2;
  Serial.begin(9600);
#else
  Serial.begin(115200);
#endif

  LowPower.begin();
  // Get the boot cause
  bc = LowPower.bootCause();
  // Print the boot cause
  printBootCause(bc);

  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");
  if (strcmp(cmd, "16653") == 0
    || strcmp(cmd, "16654") == 0
    || strcmp(cmd, "16655") == 0
    || strcmp(cmd, "16656") == 0)
    theAudio->setRenderingClockMode(AS_CLKMODE_HIRES);
      
  /* Select input device as microphone */
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);

  /*
   * Initialize filetype to stereo wav with 48 Kb/s sampling rate
   * Search for WAVDEC codec in "/mnt/sd0/BIN" directory
   */
  if (strcmp(cmd, "16362") == 0)
    {
      theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_48000,AS_CHANNEL_STEREO);
    }
  else if (strcmp(cmd, "16645") == 0)
    {
      theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_16000,AS_CHANNEL_MONO);
    }
  else if (strcmp(cmd, "16646") == 0)
    {
      theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_16000,AS_CHANNEL_STEREO);
    }
  else if (strcmp(cmd, "16647") == 0)
    {
      theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_16000,AS_CHANNEL_4CH);
    }
  else if (strcmp(cmd, "16648") == 0)
    {
      theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_48000,AS_CHANNEL_MONO);
    }
  else if (strcmp(cmd, "16649") == 0)
    {
      theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_48000,AS_CHANNEL_4CH);
    }
  else if (strcmp(cmd, "16651") == 0)
    {
      theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_48000,AS_BITLENGTH_24,AS_CHANNEL_MONO);
    }
  else if (strcmp(cmd, "16652") == 0)
    {
      theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_48000,AS_BITLENGTH_24,AS_CHANNEL_STEREO);
    }
  else if (strcmp(cmd, "16653") == 0)
    {
      theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_192000,AS_BITLENGTH_16,AS_CHANNEL_MONO);
    }
  else if (strcmp(cmd, "16654") == 0)
    {
      theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_192000,AS_BITLENGTH_16,AS_CHANNEL_STEREO);
    }
  else if (strcmp(cmd, "16655") == 0)
    {
      theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_192000,AS_BITLENGTH_24,AS_CHANNEL_MONO);
    }
  else if (strcmp(cmd, "16656") == 0)
    {
      theAudio->initRecorder(AS_CODECTYPE_WAV,"/mnt/sd0/BIN",AS_SAMPLINGRATE_192000,AS_BITLENGTH_24,AS_CHANNEL_STEREO);
    }
  else
    {
      printf("case number error\n");
      exit(1);
    }

  puts("Init Recorder!");

  /* Open file for data write on SD card */
  strcat(cmd, ".wav");
  myFile = theSD.open(cmd, FILE_WRITE);
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

  loop_count++;
  if ((bc != DEEP_RTC) && (loop_count > 500))
    {
      // Print the current clock
      RTC.begin();
      RtcTime now = RTC.getTime();
      printf("%04d/%02d/%02d %02d:%02d:%02d\n",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());

      Serial.println("Go to deep sleep...");
      delay(1000);
      LowPower.deepSleep(10);
      LowPower.end();
      return;      
    }

  /* recording end condition */
  if (theAudio->getRecordingSize() > recoding_size)
    {
      theAudio->stopRecorder();
      sleep(1);
      err = theAudio->readFrames(myFile);
      TestEnd = true;
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
  if (TestEnd)
    {
      printf("test end\n");
    }
  else
    {
      printf("test fail\n");
    }   
  exit(1);
}
