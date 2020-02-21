/*
 *  TimedWakeupDeep.ino - Example for RTC wakeup from deep sleep
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

#include <LowPower.h>
#include <RTC.h>
#include <EEPROM.h>

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
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "SEN_INT signal detected in cold sleep",
  "PMIC signal detected in cold sleep",
  "USB Disconnected in cold sleep",
  "USB Connected in cold sleep",
  "Power On Reset",
};

const bootcause_e bootcauses[] = {
  POR_SUPPLY,
  WDT_REBOOT,
  WDT_RESET,
  DEEP_WKUPL,
  DEEP_WKUPS,
  DEEP_RTC,
  DEEP_USB_ATTACH,
  DEEP_OTHERS,
  COLD_SCU_INT,
  COLD_RTC_ALM0,
  COLD_RTC_ALM1,
  COLD_RTC_ALM2,
  COLD_RTC_ALMERR,
  COLD_GPIO_IRQ36,
  COLD_GPIO_IRQ37,
  COLD_GPIO_IRQ38,
  COLD_GPIO_IRQ39,
  COLD_GPIO_IRQ40,
  COLD_GPIO_IRQ41,
  COLD_GPIO_IRQ42,
  COLD_GPIO_IRQ43,
  COLD_GPIO_IRQ44,
  COLD_GPIO_IRQ45,
  COLD_GPIO_IRQ46,
  COLD_GPIO_IRQ47,
  COLD_SEN_INT,
  COLD_PMIC_INT,
  COLD_USB_DETACH,
  COLD_USB_ATTACH,
  POR_NORMAL,
};

#if 1
HardwareSerial &mySerial = Serial;
int myBaudrate = 115200;
#else
HardwareSerial &mySerial = Serial2;
int myBaudrate = 9600;
#endif

void printBootCause(bootcause_e bc)
{
  mySerial.println("--------------------------------------------------");
  mySerial.print("Boot Cause: (");
  mySerial.print(bc);
  mySerial.print(") ");
  mySerial.print(boot_cause_strings[bc]);
  if ((COLD_GPIO_IRQ36 <= bc) && (bc <= COLD_GPIO_IRQ47)) {
    // Wakeup by GPIO
    int pin = LowPower.getWakeupPin(bc);
    mySerial.print(" <- pin ");
    mySerial.print(pin);
  }
  mySerial.println();
  mySerial.println("--------------------------------------------------");
}

// Pin used to trigger a wakeup
const uint8_t button2 = PIN_D02;

void pushed2()
{
  mySerial.println("Pushed D02!");
}

void setup()
{
  mySerial.begin(myBaudrate);
  while (!mySerial);
  mySerial.println("boot =============================================");

  // step1
  // Initialize LowPower library
  LowPower.begin();

  // Get the boot cause
  bootcause_e bc = LowPower.bootCause();

  // Print the boot cause
  printBootCause(bc);

  // Print the current clock
  RTC.begin();
  RtcTime now = RTC.getTime();
  printf("%04d/%02d/%02d %02d:%02d:%02d\n",
         now.year(), now.month(), now.day(),
         now.hour(), now.minute(), now.second());

  if ((bc == POR_SUPPLY) || (bc == POR_NORMAL)) {
    mySerial.println("Example for RTC wakeup from deep sleep");

    // Button pin setting
    pinMode(button2, INPUT_PULLUP);
    attachInterrupt(button2, pushed2, FALLING);

    // step2
    LowPower.enableBootCause(button2);

    for (int i = 0; i < sizeof(bootcauses)/sizeof(bootcause_e); i++) {
      if (bootcauses[i] < COLD_GPIO_IRQ36 ||
          bootcauses[i] > COLD_GPIO_IRQ47) {
        LowPower.disableBootCause(bootcauses[i]);
      }
    }

    for (uint8_t i = PIN_D00; i <= PIN_D28; i++) {
      if (i != button2) {
        LowPower.disableBootCause(i);
      }
    }

    mySerial.println("Go to cold sleep...");
    delay(5000);
    // step3
    LowPower.coldSleep();
  // step5
  } else if (bc >= COLD_GPIO_IRQ36 && bc <= COLD_GPIO_IRQ47) {
    // step6
    LowPower.disableBootCause(button2);
    // step7
    if (LowPower.isEnabledBootCause(button2)) {
      mySerial.println("[fail] button2 is enabled");
    } else {
      mySerial.println("[pass] button2 is disabled");
    }
  } else {
    mySerial.println("[fail]unexpected boot cause");
  }
  // step8
  LowPower.end();
  delay(5000);
}

void loop()
{
}

