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

const int sleeptimes[] = { 1, 10, 30 };

#if 0
HardwareSerial &mySerial = Serial;
int myBaudrate = 115200;
#else
HardwareSerial &mySerial = Serial2;
int myBaudrate = 115200;
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

  if ((bc == POR_SUPPLY) || (bc == POR_NORMAL)) {
    mySerial.println("Example for RTC wakeup from deep sleep");
    EEPROM[0] = 0;
  // step4
  } else if (bc == DEEP_RTC) {
    EEPROM[0] += 1;
    // step5
    LowPower.disableBootCause(DEEP_RTC);
    // step6
    if (LowPower.isEnabledBootCause(DEEP_RTC)) {
      mySerial.println("[fail] DEEP_RTC is enabled");
      return;
    }
  } else {
    mySerial.println("[fail]unexpected boot cause");
    return;
  }

  const int count = EEPROM[0];
  if (count >= 10) {
    mySerial.println("loop end");
    return;
  } else {
    mySerial.print("count = ");
    mySerial.println(count);
  }

  // Print the boot cause
  printBootCause(bc);

  // step2
  LowPower.enableBootCause(DEEP_RTC);
  for (int i = 0; i < sizeof(bootcauses)/sizeof(bootcause_e); i++) {
    if (bootcauses[i] != DEEP_RTC) {
      LowPower.disableBootCause(bootcauses[i]);
    }
  }

  int stime = sleeptimes[count % (sizeof(sleeptimes) / sizeof(int))];
  mySerial.print("sleep time = ");
  mySerial.println(stime);

  // Print the current clock
  RTC.begin();
  RtcTime now = RTC.getTime();
  printf("%04d/%02d/%02d %02d:%02d:%02d\n",
         now.year(), now.month(), now.day(),
         now.hour(), now.minute(), now.second());

  // Go to deep sleep during about 10 seconds
  mySerial.println("Go to deep sleep...");
  // step3
  LowPower.deepSleep(stime);
  LowPower.end();
}

void loop()
{
}
