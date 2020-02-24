/*
 *  camera.ino - One minute interval time-lapse Camera
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
 *
 *  This is a test app for the camera library.
 *  This library can only be used on the Spresense with the FCBGA chip package.
 */

#include <SDHCI.h>
#include <stdio.h>  /* for sprintf */

#include <Camera.h>

#define BAUDRATE                (115200)

SDClass  theSD;
int take_picture_count = 0;

#include <LowPower.h>
#include <RTC.h>

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
 * Callback from Camera library when video frame is captured.
 */

void CamCB(CamImage img)
{

  /* Check the img instance is available or not. */

  if (img.isAvailable())
    {

      /* If you want RGB565 data, convert image data format to RGB565 */

      img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);

      /* You can use image data directly by using getImgSize() and getImgBuff().
       * for displaying image to a display, etc. */

      Serial.print("Image data size = ");
      Serial.print(img.getImgSize(), DEC);
      Serial.print(" , ");

      Serial.print("buff addr = ");
      Serial.print((unsigned long)img.getImgBuff(), HEX);
      Serial.println("");
    }
  else
    {
      Serial.print("Failed to get video stream image\n");
    }
}

/**
 * @brief Initialize camera
 */
void setup()
{

  /* Open serial communications and wait for port to open */

  Serial.begin(BAUDRATE);
  while (!Serial)
    {
      ; /* wait for serial port to connect. Needed for native USB port only */
    }

  LowPower.begin();
  // Get the boot cause
  bc = LowPower.bootCause();
  // Print the boot cause
  printBootCause(bc);

  /* begin() without parameters means that
   * number of buffers = 1, 30FPS, QVGA, YUV 4:2:2 format */

  Serial.println("Prepare camera");
  theCamera.begin();


  /* Start video stream.
   * If received video stream data from camera device,
   *  camera library call CamCB.
   */

  Serial.println("Start streaming");
  theCamera.startStreaming(true, CamCB);

  /* Auto white balance configuration */

  Serial.println("Set Auto white balance parameter");
  theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);
 
  /* Set parameters about still picture.
   * In the following case, QUADVGA and JPEG.
   */

  Serial.println("Start streaming");
  theCamera.setStillPictureImageFormat(
     CAM_IMGSIZE_QUADVGA_H,
     CAM_IMGSIZE_QUADVGA_V,
     CAM_IMAGE_PIX_FMT_JPG);
}

/**
 * @brief Take picture with format JPEG per second
 */

void loop()
{
  sleep(1); /* wait for one second to take still picture. */

  /* You can change the format of still picture at here also, if you want. */

  /* theCamera.setStillPictureImageFormat(
   *   CAM_IMGSIZE_HD_H,
   *   CAM_IMGSIZE_HD_V,
   *   CAM_IMAGE_PIX_FMT_JPG);
   */

  /* This sample code can take 10 pictures in every one second from starting. */

  if (take_picture_count < 10)
    {

      /* Take still picture.
      * Unlike video stream(startStreaming) , this API wait to receive image data
      *  from camera device.
      */
  
      Serial.println("call takePicture()");
      CamImage img = theCamera.takePicture();

      /* Check availability of the img instance. */
      /* If any error was occured, the img is not available. */

      if (img.isAvailable())
        {
          /* Create file name */
    
          char filename[16] = {0};
          sprintf(filename, "PICT%03d.JPG", take_picture_count);
    
          Serial.print("Save taken picture as ");
          Serial.print(filename);
          Serial.println("");

          /* Save to SD card as the finename */
    
          File myFile = theSD.open(filename, FILE_WRITE);
          myFile.write(img.getImgBuff(), img.getImgSize());
          myFile.close();
        }

      take_picture_count++;
      if (take_picture_count == 10) {
        Serial.println("test end");
        theCamera.startStreaming(false, NULL);
      }
    }

  if ((bc != DEEP_RTC) && (take_picture_count > 2))
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
    }
}

