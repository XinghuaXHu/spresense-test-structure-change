/*
power managerånÇÃstub
*/
#include "system/sys_power_mgr.h" // stub version
#include <stdlib.h>
#include "chateau_osal.h" // F_ASSERT
#include "stub/pm.h"

/* PM_AudioPowerOn, PM_AudioPowerOffÇÕï°êîâÒåƒÇŒÇÍÇÈÇ±Ç∆ÇëzíËÇ∑ÇÈ */
#define PM_AudioPower_Called_MAX (4)
int PM_AudioPowerOn_Called = 0;
uint32_t PM_AudioPowerOn_id[PM_AudioPower_Called_MAX] = {0,0,0,0};
PM_CB PM_AudioPowerOn_fp[PM_AudioPower_Called_MAX] = {NULL,NULL,NULL,NULL};
int PM_AudioPowerOn_Sequence[PM_AudioPower_Called_MAX] = {0,0,0,0};
int PM_AudioPowerOn_ReturnValue[PM_AudioPower_Called_MAX] = {0,0,0,0};
int PM_AudioPowerOn(uint32_t id, PM_CB fp)
{
	F_ASSERT(PM_AudioPowerOn_Called<PM_AudioPower_Called_MAX);
	PM_AudioPowerOn_id[PM_AudioPowerOn_Called] = id;
	PM_AudioPowerOn_fp[PM_AudioPowerOn_Called] = fp;
	PM_AudioPowerOn_Sequence[PM_AudioPowerOn_Called] = cpputest_call_sequence++;
	return PM_AudioPowerOn_ReturnValue[PM_AudioPowerOn_Called++];
}

int PM_AudioPowerOff_Called = 0;
uint32_t PM_AudioPowerOff_id[PM_AudioPower_Called_MAX] = {0,0,0,0};
PM_CB PM_AudioPowerOff_fp[PM_AudioPower_Called_MAX] = {NULL,NULL,NULL,NULL};
int PM_AudioPowerOff_Sequence[PM_AudioPower_Called_MAX] = {0,0,0,0};
int PM_AudioPowerOff_ReturnValue[PM_AudioPower_Called_MAX] = {0,0,0,0};
int PM_AudioPowerOff(uint32_t id, PM_CB fp)
{
	PM_AudioPowerOff_id[PM_AudioPowerOff_Called] = id;
	PM_AudioPowerOff_fp[PM_AudioPowerOff_Called] = fp;
	PM_AudioPowerOff_Sequence[PM_AudioPowerOff_Called] = cpputest_call_sequence++;
	return PM_AudioPowerOff_ReturnValue[PM_AudioPowerOff_Called++];
}

int PM_PeriClockDisable_Called = 0;
uint32_t PM_PeriClockDisable_id = 0;
PM_CB PM_PeriClockDisable_fp = NULL;
int PM_PeriClockDisable_Sequence = 0;
int PM_PeriClockDisable_ReturnValue = 0;
int PM_PeriClockDisable(uint32_t id, PM_CB fp)
{
	PM_PeriClockDisable_Called++;
	PM_PeriClockDisable_id = id;
	PM_PeriClockDisable_fp = fp;
	PM_PeriClockDisable_Sequence = cpputest_call_sequence++;
	return PM_PeriClockDisable_ReturnValue;
}

int PM_AudioClockEnable_Called = 0;
uint32_t PM_AudioClockEnable_source = 0;
uint32_t PM_AudioClockEnable_xdiv = 0;
PM_CB PM_AudioClockEnable_fp = NULL;
int PM_AudioClockEnable_Sequence = 0;
int PM_AudioClockEnable_ReturnValue = 0;
int PM_AudioClockEnable(uint32_t source, uint32_t xdiv, PM_CB fp)
{
	PM_AudioClockEnable_Called++;
	PM_AudioClockEnable_source = source;
	PM_AudioClockEnable_xdiv = xdiv;
	PM_AudioClockEnable_fp = fp;
	PM_AudioClockEnable_Sequence = cpputest_call_sequence++;
	return PM_AudioClockEnable_ReturnValue;
}

int PM_ChangeClock_Called = 0;
PM_Clock_t PM_ChangeClock_target[10]; // Ç∆ÇËÇ†Ç¶Ç∏ç≈ëÂ10å¬
int PM_ChangeClock_num = 0;
int PM_ChangeClock_Sequence = 0;
int PM_ChangeClock_ReturnValue = 0;
int PM_ChangeClock(PM_Clock_t target[], int num)
{
	int i;
	PM_ChangeClock_Called++;
	for(i=0;i<10&&i<num;i++) PM_ChangeClock_target[i] = target[i];
	PM_ChangeClock_num = num;
	PM_ChangeClock_Sequence = cpputest_call_sequence++;
	return PM_ChangeClock_ReturnValue;
}

int PM_RamControlByAddress_Called = 0;
int PM_RamControlByAddress_Sequence = 0;
uint32_t PM_RamControlByAddress_address = 0;
uint32_t PM_RamControlByAddress_size = 0;
uint32_t PM_RamControlByAddress_status = 0;
PM_CB PM_RamControlByAddress_fp = NULL;
int PM_RamControlByAddress_ReturnValue = 0;
int PM_RamControlByAddress(uint32_t address, uint32_t size, uint32_t status, PM_CB fp)
{
	PM_RamControlByAddress_Called++;
	PM_RamControlByAddress_Sequence = cpputest_call_sequence++;
	PM_RamControlByAddress_address = address;
	PM_RamControlByAddress_size = size;
	PM_RamControlByAddress_status = status;
	PM_RamControlByAddress_fp = fp;
	return PM_RamControlByAddress_ReturnValue;
}

int PM_StopCpu_Called = 0;
int PM_StopCpu_Sequence = 0;
int PM_StopCpu_cpuid;
int PM_StopCpu_ReturnValue = 0;
int PM_StopCpu(int cpuid)
{
	PM_StopCpu_Called++;
	PM_StopCpu_Sequence = cpputest_call_sequence++;
	PM_StopCpu_cpuid = cpuid;
	return PM_StopCpu_ReturnValue;
}


#include <system/pm_cpufreqlock.h>
#include <string.h>

int PM_AcquireCpuFreqLock_Called = 0;
PM_CpuFreqLock_t PM_AcquireCpuFreqLock_lock;
int PM_AcquireCpuFreqLock_ReturnValue = 0; // 0: Success
int PM_AcquireCpuFreqLock(PM_CpuFreqLock_t *lock)
{
	PM_AcquireCpuFreqLock_Called++;
	memcpy(&PM_AcquireCpuFreqLock_lock, lock, sizeof(PM_CpuFreqLock_t));
	return PM_AcquireCpuFreqLock_ReturnValue;
}

int PM_ReleaseCpuFreqLock_Called = 0;
PM_CpuFreqLock_t PM_ReleaseCpuFreqLock_lock;
int PM_ReleaseCpuFreqLock_ReturnValue = 0; // 0: success
int PM_ReleaseCpuFreqLock(PM_CpuFreqLock_t *lock)
{
	PM_ReleaseCpuFreqLock_Called++;
	memcpy(&PM_ReleaseCpuFreqLock_lock, lock, sizeof(PM_CpuFreqLock_t));
	return PM_ReleaseCpuFreqLock_ReturnValue;
}

