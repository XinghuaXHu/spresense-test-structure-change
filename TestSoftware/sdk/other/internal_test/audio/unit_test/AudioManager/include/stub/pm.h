#include "system/sys_power_mgr.h" // stub version
#include <system/pm_cpufreqlock.h>

/* スタブ制御/観測用(実体はスタブ側) */
extern int PM_AudioPowerOn_Called;
extern uint32_t PM_AudioPowerOn_id[];
extern PM_CB PM_AudioPowerOn_fp[];
extern int PM_AudioPowerOn_Sequence[];
extern int PM_AudioPowerOn_ReturnValue[];
extern int PM_AudioPowerOff_Called;
extern uint32_t PM_AudioPowerOff_id[];
extern PM_CB PM_AudioPowerOff_fp[];
extern int PM_AudioPowerOff_Sequence[];
extern int PM_AudioPowerOff_ReturnValue[];
extern int PM_PeriClockDisable_Called;
extern uint32_t PM_PeriClockDisable_id;
extern PM_CB PM_PeriClockDisable_fp;
extern int PM_PeriClockDisable_Sequence;
extern int PM_PeriClockDisable_ReturnValue;
extern int PM_AudioClockEnable_Called;
extern uint32_t PM_AudioClockEnable_source;
extern uint32_t PM_AudioClockEnable_xdiv;
extern PM_CB PM_AudioClockEnable_fp;
extern int PM_AudioClockEnable_Sequence;
extern int PM_AudioClockEnable_ReturnValue;
extern int PM_ChangeClock_Called;
extern PM_Clock_t PM_ChangeClock_target[];
extern int PM_ChangeClock_num;
extern int PM_ChangeClock_Sequence;
extern int PM_ChangeClock_ReturnValue;
extern int PM_RamControlByAddress_Called;
extern int PM_RamControlByAddress_Sequence;
extern uint32_t PM_RamControlByAddress_address;
extern uint32_t PM_RamControlByAddress_size;
extern uint32_t PM_RamControlByAddress_status;
extern PM_CB PM_RamControlByAddress_fp;
extern int PM_RamControlByAddress_ReturnValue;
extern int PM_StopCpu_Called;
extern int PM_StopCpu_Sequence;
extern int PM_StopCpu_cpuid;
extern int PM_StopCpu_ReturnValue;
extern int PM_AcquireCpuFreqLock_Called;
extern PM_CpuFreqLock_t PM_AcquireCpuFreqLock_lock;
extern int PM_AcquireCpuFreqLock_ReturnValue;
extern int PM_ReleaseCpuFreqLock_Called;
extern PM_CpuFreqLock_t PM_ReleaseCpuFreqLock_lock;
extern int PM_ReleaseCpuFreqLock_ReturnValue;

/* 関数呼び出し順カウンタ(実体はテストコード側) */
extern int cpputest_call_sequence;

