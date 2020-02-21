/*
 *  number_recognition.ino - hand written number recognition sample application
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

/**
 * @file number_recognition.ino
 * @author Sony Semiconductor Solutions Corporation
 * @brief DNNRT sample application.
 *
 * This sample uses the network model file (.nnb) and recognize image
 * in pgm (portable greyscale map) file. Both of requred files should be
 * placed at the SD card. And adjust file path (nnbfile and pgmfile) if
 * needed.
 */

#include <SDHCI.h>
#include <NetPBM.h>
#include <DNNRT.h>


DNNRT dnnrt;

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

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  LowPower.begin();
  // Get the boot cause
  bc = LowPower.bootCause();
  // Print the boot cause
  printBootCause(bc);

  File nnbfile("network.nnb");
  if (!nnbfile) {
    Serial.print("nnb not found");
    return;
  }
  int ret = dnnrt.begin(nnbfile);
  if (ret < 0) {
    Serial.print("Runtime initialization failure. ");
    Serial.print(ret);
    Serial.println();
    return;
  }

  // Image size for this network model is 28 x 28.

  File pgmfile("number4.pgm");
  NetPBM pgm(pgmfile);

  unsigned short width, height;
  pgm.size(&width, &height);

  DNNVariable input(width * height);
  float *buf = input.data();
  int i = 0;

  /*
   * Normalize pixel data into between 0.0 and 1.0.
   * PGM file is gray scale pixel map, so divide by 255.
   * This normalization depends on the network model.
   */

  for (int x = 0; x < height; x++) {
    for (int y = 0; y < width; y++) {
      buf[i] = float(pgm.getpixel(x, y)) / 255.0;
      i++;
    }
  }

  dnnrt.inputVariable(input, 0);
  dnnrt.forward();
  DNNVariable output = dnnrt.outputVariable(0);

  /*
   * Get index for maximum value.
   * In this example network model, this index represents a number,
   * so you can determine recognized number from this index.
   */

  int index = output.maxIndex();
  Serial.print("Image is ");
  Serial.print(index);
  Serial.println();
  Serial.print("value ");
  Serial.print(output[index]);
  Serial.println();
  if (bc != COLD_RTC_ALM0) {
    // Print the current clock
    RTC.begin();
    RtcTime now = RTC.getTime();
    printf("%04d/%02d/%02d %02d:%02d:%02d\n",
           now.year(), now.month(), now.day(),
           now.hour(), now.minute(), now.second());

    Serial.println("Go to cold sleep...");
    delay(1000);
    LowPower.coldSleep(10);
  } else {
    Serial.println("test end");
  }
  LowPower.end();
  dnnrt.end();
}

void loop() {
  // put your main code here, to run repeatedly:

}
