pmtest
^^^^^^

  This example executes some operations of power management.

    * CONFIG_TEST_PMTEST=y : Enable this example

    <Config Option>
    If you test FreqLock,
    * CONFIG_CPUFREQ_RELEASE_LOCK=y : Enable FreqLock
    If you test WakeLock,
    * CONFIG_CXD56_HOT_SLEEP=y : Enable WakeLock
    If you test wakeup by alarm,
    * CONFIG_CXD56_RTC=y : Enable Real Time Clock
    * CONFIG_RTC=y
    * CONFIG_RTC_HIRES=y
    * CONFIG_RTC_FREQUENCY=32768
    * CONFIG_RTC_ALARM
    * CONFIG_RTC_NALARMS=1
    * CONFIG_RTC_DRIVER=y
    * CONFIG_EXAMPLES_ALARM=y

  Example: Show help message

    nsh> pmtest -h

    Usage: pmtest [-s <sleep>] [-<e|d> <mask>] [-f <freq>] [-h]

    Description:
     Power Management operation
    Options:
     -s <sleep>: <sleep> selects <deep|cold|hot|run>.
     -e <mask>: Enable Boot Mask
     -d <mask>: Disable Boot Mask
     -f <freq>: <freq> selects <hv|lv|rcosc>.
     -h: Show this message

  Example: Display BootCause

    Below is that BootCause means Reboot.

    nsh> pmtest
    BootCause:
    (0x00000000) [ ] <POR> Power On Reset
    (0x00000001) [ ] <POR> DeadBattery
    (0x00000002) [*] <POR> Reboot or Watchdog
    (0x00000004) [ ] <Deep> Reset
    (0x00000008) [ ] <Deep> WKUPL signal
    (0x00000010) [ ] <Deep> WKUPS signal
    (0x00000020) [ ] <Deep> RTC Alarm
    (0x00000040) [ ] <Deep> USB Connected
    (0x00000080) [ ] <Deep> Reserved
    (0x00000100) [ ] <Cold> SCU Int
    (0x00000200) [ ] <Cold> RTC Alarm0
    (0x00000400) [ ] <Cold> RTC Alarm1
    (0x00000800) [ ] <Cold> RTC Alarm2
    (0x00001000) [ ] <Cold> RTC AlarmErr
    (0x00002000) [ ] <Cold> -
    (0x00004000) [ ] <Cold> -
    (0x00008000) [ ] <Cold> -
    (0x00010000) [ ] <Cold> GPIO0 Int
    (0x00020000) [ ] <Cold> GPIO1 Int
    (0x00040000) [ ] <Cold> GPIO2 Int
    (0x00080000) [ ] <Cold> GPIO3 Int
    (0x00100000) [ ] <Cold> GPIO4 Int
    (0x00200000) [ ] <Cold> GPIO5 Int
    (0x00400000) [ ] <Cold> GPIO6 Int
    (0x00800000) [ ] <Cold> GPIO7 Int
    (0x01000000) [ ] <Cold> GPIO8 Int
    (0x02000000) [ ] <Cold> GPIO9 Int
    (0x04000000) [ ] <Cold> GPIO10 Int
    (0x08000000) [ ] <Cold> GPIO11 Int
    (0x10000000) [ ] <Cold> Sensor Int
    (0x20000000) [ ] <Cold> PMIC Int
    (0x40000000) [ ] <Cold> USB Disconnected
    (0x80000000) [ ] <Cold> USB Connected

  Example: Control BootMask

    1. Display the current BootMask

    nsh> pmtest -e 0
    BootMask:
    (0x00000001) [*] <POR> DeadBattery
    (0x00000002) [*] <POR> Reboot or Watchdog
    (0x00000004) [*] <Deep> Reset
    (0x00000008) [*] <Deep> WKUPL signal
    (0x00000010) [*] <Deep> WKUPS signal
    (0x00000020) [*] <Deep> RTC Alarm
    (0x00000040) [*] <Deep> USB Connected
    (0x00000080) [*] <Deep> Reserved
    (0x00000100) [*] <Cold> SCU Int
    (0x00000200) [*] <Cold> RTC Alarm0
    (0x00000400) [*] <Cold> RTC Alarm1
    (0x00000800) [*] <Cold> RTC Alarm2
    (0x00001000) [*] <Cold> RTC AlarmErr
    (0x00002000) [ ] <Cold> -
    (0x00004000) [ ] <Cold> -
    (0x00008000) [ ] <Cold> -
    (0x00010000) [*] <Cold> GPIO0 Int
    (0x00020000) [*] <Cold> GPIO1 Int
    (0x00040000) [*] <Cold> GPIO2 Int
    (0x00080000) [*] <Cold> GPIO3 Int
    (0x00100000) [*] <Cold> GPIO4 Int
    (0x00200000) [*] <Cold> GPIO5 Int
    (0x00400000) [*] <Cold> GPIO6 Int
    (0x00800000) [*] <Cold> GPIO7 Int
    (0x01000000) [*] <Cold> GPIO8 Int
    (0x02000000) [*] <Cold> GPIO9 Int
    (0x04000000) [*] <Cold> GPIO10 Int
    (0x08000000) [*] <Cold> GPIO11 Int
    (0x10000000) [*] <Cold> Sensor Int
    (0x20000000) [*] <Cold> PMIC Int
    (0x40000000) [*] <Cold> USB Disconnected
    (0x80000000) [*] <Cold> USB Connected

    2. Disable Cold Boot by USB Connected

    nsh> pmtest -d 0x80000000
    BootMask:
    (0x00000001) [*] <POR> DeadBattery
    (0x00000002) [*] <POR> Reboot or Watchdog
    (0x00000004) [*] <Deep> Reset
    (0x00000008) [*] <Deep> WKUPL signal
    (0x00000010) [*] <Deep> WKUPS signal
    (0x00000020) [*] <Deep> RTC Alarm
    (0x00000040) [*] <Deep> USB Connected
    (0x00000080) [*] <Deep> Reserved
    (0x00000100) [*] <Cold> SCU Int
    (0x00000200) [*] <Cold> RTC Alarm0
    (0x00000400) [*] <Cold> RTC Alarm1
    (0x00000800) [*] <Cold> RTC Alarm2
    (0x00001000) [*] <Cold> RTC AlarmErr
    (0x00002000) [ ] <Cold> -
    (0x00004000) [ ] <Cold> -
    (0x00008000) [ ] <Cold> -
    (0x00010000) [*] <Cold> GPIO0 Int
    (0x00020000) [*] <Cold> GPIO1 Int
    (0x00040000) [*] <Cold> GPIO2 Int
    (0x00080000) [*] <Cold> GPIO3 Int
    (0x00100000) [*] <Cold> GPIO4 Int
    (0x00200000) [*] <Cold> GPIO5 Int
    (0x00400000) [*] <Cold> GPIO6 Int
    (0x00800000) [*] <Cold> GPIO7 Int
    (0x01000000) [*] <Cold> GPIO8 Int
    (0x02000000) [*] <Cold> GPIO9 Int
    (0x04000000) [*] <Cold> GPIO10 Int
    (0x08000000) [*] <Cold> GPIO11 Int
    (0x10000000) [*] <Cold> Sensor Int
    (0x20000000) [*] <Cold> PMIC Int
    (0x40000000) [*] <Cold> USB Disconnected
    (0x80000000) [ ] <Cold> USB Connected <===== Disabled

  Example: Enter Deep Sleep and resume by WKUPS on battery-powered environment

    nsh> pmtest -s deep
    Enter Deep Sleep...
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Sleep
    e.g.) push POWER_SW button on SP_YOC board
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Wakeup
    AEG

    NuttShell (NSH)
    nsh> pmtest
    BootCause:
    (0x00000000) [ ] <POR> Power On Reset
    (0x00000001) [ ] <POR> DeadBattery
    (0x00000002) [ ] <POR> Reboot or Watchdog
    (0x00000004) [ ] <Deep> Reset
    (0x00000008) [ ] <Deep> WKUPL signal
    (0x00000010) [*] <Deep> WKUPS signal
    (0x00000020) [ ] <Deep> RTC Alarm
    (0x00000040) [ ] <Deep> USB Connected
    (0x00000080) [ ] <Deep> Reserved
    (0x00000100) [ ] <Cold> SCU Int
    (0x00000200) [ ] <Cold> RTC Alarm0
    (0x00000400) [ ] <Cold> RTC Alarm1
    (0x00000800) [ ] <Cold> RTC Alarm2
    (0x00001000) [ ] <Cold> RTC AlarmErr
    (0x00002000) [ ] <Cold> -
    (0x00004000) [ ] <Cold> -
    (0x00008000) [ ] <Cold> -
    (0x00010000) [ ] <Cold> GPIO0 Int
    (0x00020000) [ ] <Cold> GPIO1 Int
    (0x00040000) [ ] <Cold> GPIO2 Int
    (0x00080000) [ ] <Cold> GPIO3 Int
    (0x00100000) [ ] <Cold> GPIO4 Int
    (0x00200000) [ ] <Cold> GPIO5 Int
    (0x00400000) [ ] <Cold> GPIO6 Int
    (0x00800000) [ ] <Cold> GPIO7 Int
    (0x01000000) [ ] <Cold> GPIO8 Int
    (0x02000000) [ ] <Cold> GPIO9 Int
    (0x04000000) [ ] <Cold> GPIO10 Int
    (0x08000000) [ ] <Cold> GPIO11 Int
    (0x10000000) [ ] <Cold> Sensor Int
    (0x20000000) [ ] <Cold> PMIC Int
    (0x40000000) [ ] <Cold> USB Disconnected
    (0x80000000) [ ] <Cold> USB Connected

  Example: Enter Cold Sleep and resume by RTC alarm

    nsh> alarm 10
    alarm_daemon started
    alarm_daemon: Running
    Opening /dev/rtc0
    Alarm 0 set in 10 seconds
    nsh> pmtest -s cold
    Enter Cold Sleep...
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Sleep
    (RTC Alarm expired after 10 seconds)
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Wakeup
    AEG

    NuttShell (NSH)
    nsh> pmtest
    BootCause:
    (0x00000000) [ ] <POR> Power On Reset
    (0x00000001) [ ] <POR> DeadBattery
    (0x00000002) [ ] <POR> Reboot or Watchdog
    (0x00000004) [ ] <Deep> Reset
    (0x00000008) [ ] <Deep> WKUPL signal
    (0x00000010) [ ] <Deep> WKUPS signal
    (0x00000020) [ ] <Deep> RTC Alarm
    (0x00000040) [ ] <Deep> USB Connected
    (0x00000080) [ ] <Deep> Reserved
    (0x00000100) [ ] <Cold> SCU Int
    (0x00000200) [*] <Cold> RTC Alarm0
    (0x00000400) [ ] <Cold> RTC Alarm1
    (0x00000800) [ ] <Cold> RTC Alarm2
    (0x00001000) [ ] <Cold> RTC AlarmErr
    (0x00002000) [ ] <Cold> -
    (0x00004000) [ ] <Cold> -
    (0x00008000) [ ] <Cold> -
    (0x00010000) [ ] <Cold> GPIO0 Int
    (0x00020000) [ ] <Cold> GPIO1 Int
    (0x00040000) [ ] <Cold> GPIO2 Int
    (0x00080000) [ ] <Cold> GPIO3 Int
    (0x00100000) [ ] <Cold> GPIO4 Int
    (0x00200000) [ ] <Cold> GPIO5 Int
    (0x00400000) [ ] <Cold> GPIO6 Int
    (0x00800000) [ ] <Cold> GPIO7 Int
    (0x01000000) [ ] <Cold> GPIO8 Int
    (0x02000000) [ ] <Cold> GPIO9 Int
    (0x04000000) [ ] <Cold> GPIO10 Int
    (0x08000000) [ ] <Cold> GPIO11 Int
    (0x10000000) [ ] <Cold> Sensor Int
    (0x20000000) [ ] <Cold> PMIC Int
    (0x40000000) [ ] <Cold> USB Disconnected
    (0x80000000) [ ] <Cold> USB Connected

  Example: Enable and disable Hot Sleep

    nsh> pmtest -s run <--- keep running state by WakeLock acquisition
    nsh> pmtest -s hot <--- enable hot sleep by WakeLock release

  Example: Frequency control

    nsh> pmtest -f hv    <--- HV PLL
    nsh> pmtest -f lv    <--- LV PLL
    nsh> pmtest -f rcosc <--- RCOSC


