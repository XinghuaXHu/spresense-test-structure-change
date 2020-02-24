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
#include <Watchdog.h>

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

void pushed2(){ Serial.println("Pushed D02!"); }
void pushed3(){ Serial.println("Pushed D03!"); }
void pushed4(){ Serial.println("Pushed D04!"); }
void pushed5(){ Serial.println("Pushed D05!"); }
void pushed6(){ Serial.println("Pushed D06!"); }
void pushed7(){ Serial.println("Pushed D07!"); }
void pushed10(){ Serial.println("Pushed D10!"); }
void pushed11(){ Serial.println("Pushed D11!"); }
void pushed12(){ Serial.println("Pushed D12!"); }

struct pins_s {
  uint8_t pin;
  void (*isr)(void);
};

const pins_s pins[] = {
  { PIN_D02, pushed2  },
  { PIN_D03, pushed3  },
  { PIN_D04, pushed4  },
  { PIN_D05, pushed5  },
  { PIN_D06, pushed6  },
  { PIN_D07, pushed7  },
  { PIN_D10, pushed10 },
  { PIN_D11, pushed11 },
  { PIN_D12, pushed12 },
};

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

void enableBootCause(bootcause_e bc) {
  for (int i = 0; i < sizeof(bootcauses)/sizeof(bootcause_e); i++) {
    if (bootcauses[i] == bc) {
      LowPower.enableBootCause(bootcauses[i]);
    } else {
      LowPower.disableBootCause(bootcauses[i]);
    }
  }
}

void enableBootCause(uint8_t pin) {
  for (uint8_t i = PIN_D00; i <= PIN_D28; i++) {
    if (i == pin) {
      LowPower.enableBootCause(i);
    } else {
      LowPower.disableBootCause(i);
    }
  }
}

void printCurrentClock() {
  // Print the current clock
  RTC.begin();
  RtcTime now = RTC.getTime();
  printf("%04d/%02d/%02d %02d:%02d:%02d\n",
         now.year(), now.month(), now.day(),
         now.hour(), now.minute(), now.second());
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
  printCurrentClock();

  if ((bc == POR_SUPPLY) || (bc == POR_NORMAL)) {
    mySerial.println("spr_sdk-16355 start");
    EEPROM[0] = 0;
    EEPROM[1] = 0;
    EEPROM[2] = 0;
  } else if (bc == COLD_RTC_ALM0) {

    LowPower.disableBootCause(COLD_RTC_ALM0);
    if (LowPower.isEnabledBootCause(COLD_RTC_ALM0)) {
      mySerial.println("[fail] COLD_RTC_ALM0 is enabled");
      return;
    }
  } else if (bc == WDT_REBOOT) {
    LowPower.disableBootCause(WDT_REBOOT);
    if (!LowPower.isEnabledBootCause(WDT_REBOOT)) {
      mySerial.println("[fail] WDT_REBOOT is enabled");
      return;
    }
  } else if ((COLD_GPIO_IRQ36 <= bc) && (bc <= COLD_GPIO_IRQ47)) {
    uint8_t bc_pin = LowPower.getWakeupPin(bc);
    mySerial.print("bc_pin = ");
    mySerial.println(bc_pin);
    LowPower.disableBootCause(bc_pin);
    if (LowPower.isEnabledBootCause(bc_pin)) {
      mySerial.println("[fail] bc_pin is enabled");
      return;
    }
  } else {
    mySerial.println("[fail] unexpected boot cause");
    return;
  }

  begin_loop:

  const int count = EEPROM[0];
  EEPROM[0] += 1;
  mySerial.print("count = ");
  mySerial.println(count%13);

  if (count < 3) {

    enableBootCause(COLD_RTC_ALM0);
    mySerial.println("Goto cold sleep (RTC)");
    mySerial.print("sleeptime = ");
    mySerial.println(sleeptimes[count%3]);
    delay(5000);
    LowPower.coldSleep(sleeptimes[count%3]);
    LowPower.end();

  } else if (count == 3) {

    enableBootCause(WDT_REBOOT);
    mySerial.println("Start watch dog");
    Watchdog.begin();
    delay(5000);
    Watchdog.start(5000);

  } 
#if 0
  else if (count == 4) {
    enableBootCause(COLD_SCU_INT);
    mySerial.println("Goto cold sleep (SCU)");
    mySerial.print("sleeptime = ");
    mySerial.println(sleeptimes[count%3]);
    delay(5000);
    LowPower.coldSleep(sleeptimes[count%3]);
    LowPower.end();
  } 
  else if (count == 5) {
    enableBootCause(COLD_SEN_INT);
    mySerial.println("Goto cold sleep (SEN)");
    mySerial.print("sleeptime = ");
    mySerial.println(sleeptimes[count%3]);
    delay(5000);
    LowPower.coldSleep(sleeptimes[count%3]);
    LowPower.end();

  } 
  else if (count == 6) {
    enableBootCause(COLD_PMIC_INT);
    mySerial.println("Goto cold sleep (Battery)");
    mySerial.print("sleeptime = ");
    mySerial.println(sleeptimes[count%3]);
    delay(5000);
    LowPower.coldSleep(sleeptimes[count%3]);
    LowPower.end();
  } 
  else if (count == 7) {
    enableBootCause(COLD_SEN_INT);
    mySerial.println("Goto cold sleep (CXD5247)");
    mySerial.print("sleeptime = ");
    mySerial.println(sleeptimes[count%3]);
    delay(5000);
    LowPower.coldSleep(sleeptimes[count%3]);
    LowPower.end();
  }
#endif
  else {

    const int pin_count = EEPROM[1];
    EEPROM[1] += 1;
    if (pin_count < sizeof(pins) / sizeof(pins_s)) {
      enableBootCause(pins[pin_count].pin);
      pinMode(pins[pin_count].pin, INPUT_PULLUP);
      attachInterrupt(pins[pin_count].pin, pins[pin_count].isr, FALLING);
      mySerial.println("Goto cold sleep (GPIO)");
      mySerial.print("pin = ");
      mySerial.println(pins[pin_count].pin);
      delay(5000);
      LowPower.coldSleep();
      LowPower.end();
    } else {
      const int main_count = EEPROM[2];
      EEPROM[2] += 1;
      if (main_count < 10) {
        EEPROM[0] = 0;
        EEPROM[1] = 0;
        goto begin_loop;
      } else {
        mySerial.println("test end");
      }
    }
  }
}

void loop()
{
}

