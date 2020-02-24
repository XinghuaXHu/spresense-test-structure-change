/* 共通処理 */
/* powerOnAdonis(); powerOnAudio(); の確認用 */

#include <string.h>
#include <system/pm_cpufreqlock.h>
static PM_CpuFreqLock_t s_lockHighVoltage = PM_CPUFREQLOCK_INIT(0, 160, PM_CPUFREQLOCK_FLAG_HV);

void prepare_common_power_on(void)
{
	PM_AudioPowerOn_Called = 0; /* PM_AudioPowerOn()は2回呼ばれる */
	PM_AudioPowerOn_Sequence[0] = 0;
	PM_AudioPowerOn_Sequence[1] = 0;
	PM_AudioPowerOn_ReturnValue[0] = 0;
	PM_AudioPowerOn_ReturnValue[1] = 0;
	PM_PeriClockDisable_Called = 0;
	PM_AudioClockEnable_Called = 0;
// CpuFreqLock導入でPM_ChangeClockは呼ばなくなったので削除
//	PM_ChangeClock_Called = 0;
	setAudioIoMclk_Called = 0;
	PM_AcquireCpuFreqLock_Called = 0;
	PM_AcquireCpuFreqLock_ReturnValue = 0;
}
void check_common_power_on(void)
{
	CHECK(PM_AudioPowerOn_Called == 2);
	CHECK(PM_AudioPowerOn_id[0] == PM_ADONIS);
	CHECK(PM_AudioPowerOn_fp[0] == NULL);
	CHECK(PM_AudioPowerOn_id[1] == PM_AUDIO);
	CHECK(PM_AudioPowerOn_fp[1] == NULL);
	CHECK(PM_PeriClockDisable_Called == 1);
	CHECK(PM_PeriClockDisable_id == PM_AUDIO);
	CHECK(PM_PeriClockDisable_fp == NULL);
	CHECK(PM_AudioClockEnable_Called == 1);
	CHECK(PM_AudioClockEnable_source == PM_AUDIO_MODE_MCLK);
	CHECK(PM_AudioClockEnable_xdiv == 0);
	CHECK(PM_AudioClockEnable_fp == NULL);
	CHECK(PM_AcquireCpuFreqLock_Called == 1);
	CHECK(memcmp(&s_lockHighVoltage, &PM_AcquireCpuFreqLock_lock, sizeof(PM_CpuFreqLock_t)) == 0);
// CpuFreqLock導入でPM_ChangeClockは呼ばなくなったので削除
#if 0
	CHECK(PM_ChangeClock_Called == 1);
	CHECK(PM_ChangeClock_num == 5);
	/* 以下のtargetの中身のチェックはaudio_managerの実装に合わせてある */
	CHECK(PM_ChangeClock_target[0].target    == PM_CLOCK_SYS_CTRL_SEL);
	CHECK(PM_ChangeClock_target[0].param.sel == PM_CLOCK_MODE_SYSPLL_160M);
	CHECK(PM_ChangeClock_target[1].target    == PM_CLOCK_SYSPLL_DIV);
	CHECK(PM_ChangeClock_target[1].param.div == 2);
	CHECK(PM_ChangeClock_target[2].target    == PM_CLOCK_SYS_AHB_DIV);
	CHECK(PM_ChangeClock_target[2].param.div == 2);
	CHECK(PM_ChangeClock_target[3].target    == PM_CLOCK_APP_SEL);
	CHECK(PM_ChangeClock_target[3].param.sel == PM_CLOCKCHG_SYSPLL);
	CHECK(PM_ChangeClock_target[4].target    == PM_CLOCK_APP_SYSPLL_DIV);
	CHECK(PM_ChangeClock_target[4].param.div == 1);
#endif	
	CHECK(setAudioIoMclk_Called == 1);
}

void prepare_common_power_off(void)
{
	PM_AudioPowerOff_Called = 0; /* PM_AudioPowerOff()は2回呼ばれる */
	PM_AudioPowerOff_Sequence[0] = 0;
	PM_AudioPowerOff_Sequence[1] = 0;
	PM_AudioPowerOff_ReturnValue[0] = 0;
	PM_AudioPowerOff_ReturnValue[1] = 0;
	PM_PeriClockDisable_Called = 0;
	PM_ReleaseCpuFreqLock_Called = 0;
	PM_ReleaseCpuFreqLock_ReturnValue = 0;
}
void check_common_power_off(void)
{
	CHECK(PM_AudioPowerOff_Called == 2);
	CHECK(PM_AudioPowerOff_id[0] == PM_AUDIO);
	CHECK(PM_AudioPowerOff_fp[0] == NULL);
	CHECK(PM_PeriClockDisable_Called == 1);
	CHECK(PM_PeriClockDisable_id == PM_AUDIO);
	CHECK(PM_PeriClockDisable_fp == NULL);
	CHECK(PM_AudioPowerOff_id[1] == PM_ADONIS);
	CHECK(PM_AudioPowerOff_fp[1] == NULL);
	CHECK(PM_ReleaseCpuFreqLock_Called == 1);
	CHECK(memcmp(&s_lockHighVoltage, &PM_ReleaseCpuFreqLock_lock, sizeof(PM_CpuFreqLock_t)) == 0);
}


