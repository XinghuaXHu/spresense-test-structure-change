/*
 *  player.ino - Simple sound player example application
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
 * @brief Setup audio player to play mp3 file
 *
 * Set clock mode to normal <br>
 * Set output device to speaker <br>
 * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect" <br>
 * System directory "/mnt/sd0/BIN" will be searched for MP3 decoder (MP3DEC file)
 * Open "Sound.mp3" file <br>
 * Set master volume to -16.0 dB
 */
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
  strcat(cmd, ".mp3");
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

  // start audio system
  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* Set clock mode to normal */
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);

  /* Set output device to speaker with first argument.
   * If you want to change the output device to I2S,
   * specify "AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT" as an argument.
   * Set speaker driver mode to LineOut with second argument.
   * If you want to change the speaker driver mode to other,
   * specify "AS_SP_DRV_MODE_1DRIVER" or "AS_SP_DRV_MODE_2DRIVER" or "AS_SP_DRV_MODE_4DRIVER"
   * as an argument.
   */
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);

  /*
   * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect"
   * Search for MP3 decoder in "/mnt/sd0/BIN" directory
   */
  err_t err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Player0 initialize error\n");
      exit(1);
    }

  /* Open file placed on SD card */
  myFile = theSD.open(cmd);

  /* Verify file open */
  if (!myFile)
    {
      printf("File open error\n");
      exit(1);
    }
  printf("Open! %d\n",myFile);

  /* Send first frames to be decoded */
  err = theAudio->writeFrames(AudioClass::Player0, myFile);

  if ((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND))
    {
      printf("File Read Error! =%d\n",err);
      myFile.close();
      exit(1);
    }

  puts("Play!");

  /* Main volume set to -16.0 dB */
  theAudio->setVolume(-160);
  theAudio->startPlayer(AudioClass::Player0);
}

/**
 * @brief Play stream
 *
 * Send new frames to decode in a loop until file ends
 */
void loop()
{
  puts("loop!!");
  loop_count++;
  if ((bc != DEEP_RTC) && (loop_count > 0))
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

  /* Send new frames to decode in a loop until file ends */
  int err = theAudio->writeFrames(AudioClass::Player0, myFile);

  /*  Tell when player file ends */
  if (err == AUDIOLIB_ECODE_FILEEND)
    {
      printf("test end\n");
      goto stop_player;
    }

  /* Show error code from player and stop */
  if (err)
    {
      printf("Main player error code: %d\n", err);
      goto stop_player;
    }

  if (ErrEnd)
    {
      printf("Error End\n");
      goto stop_player;
    }

  /* This sleep is adjusted by the time to read the audio stream file.
     Please adjust in according with the processing contents
     being processed at the same time by Application.
  */

  usleep(40000);


  /* Don't go further and continue play */
  return;

stop_player:
  sleep(1);
  theAudio->stopPlayer(AudioClass::Player0);
  myFile.close();


      

 
  
  exit(1);
}
