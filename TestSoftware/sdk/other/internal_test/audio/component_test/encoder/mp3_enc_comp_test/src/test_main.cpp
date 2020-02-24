
#include <assert.h>
#include <drivers/peripheral/pd_gpio.h>
#include <system/sys_power_mgr.h>
#include <storage/fs_wrapper.h>
#include <debug/dbg_shell.h>
#include <debug/dbg_log.h>
#include "encoder_component.h"
#include "apus_drv.h"
#include "MemHandle.h"
#include "mem_layout.h"
#include "cpu_id.h"
#include "cpu_com.h"
#include "cpu_fifo_command.h"
#include "sdk_message_types.h"

#define DBG_MODULE DBG_MODULE_AS

void* manager_data_area = reinterpret_cast<void*>(MEMMGR_DATA_AREA_ADDR);

__USING_WIEN2

extern "C" {
	
void AS_WaitForAudioSubSystemActivated()
{
	int ret = SYS_WaitFlag(AS_ACTIVATED_FLG, 1, TWF_ORW, NULL, TMO_FEVR);
	F_ASSERT(ret == 0);
}

void AS_ReadyForAudioSubSystemActivated()
{
	int ret = SYS_SetFlag(AS_ACTIVATED_FLG, 1);
	F_ASSERT(ret == 0);
}

/*--------------------------------------------------------------------*/
void apu_assert(uint32_t cpu_id, char* filename, uint32_t line)
{
	/* Apu の Assert 時に DSP でキャッチする */
	DBG_LOGF_FATAL("!!APU ASSERTION ERROR!!! \n cpu_id = %d, \n filename = %s,\n",
		       cpu_id, filename);
	DBG_LOGF_FATAL("line = %d \n", line);
	F_ASSERT(0);
}

/*--------------------------------------------------------------------*/
void audio_recoder_cpu_intercom(ApuComPrm_t *p_param)
{
	ApuComPrmTemp_t com_param;

	com_param.cpu_id = p_param->cpu_id;
	com_param.type = p_param->type;

	F_ASSERT(p_param->command == CPU_CMD_SPU_AUDIO_APU_RESULT);

	if (APU_COM_DATA_TYPE_STRUCT_ADDRESS == com_param.type) {
		/* TODO(meguro): テンプレートがそのまま使用できれば不要 */
		com_param.data.pParam = p_param->data.pParam;
		Apu::Wien2ApuCmd* packet = reinterpret_cast<Apu::Wien2ApuCmd*>(p_param->data.pParam);
		com_param.process_mode = packet->header.process_mode;
	}
	else {
		com_param.data.value = p_param->data.value;
	}

	com_param.ex_data = p_param->ex_data;
	com_param.event_type = Apu::ExecEvent;

	MsgLib::sendIsr<ApuComPrmTemp_t>(MSGQ_AUD_APUS_DRV, MsgPriNormal, MSG_AUD_APUS_CMD_RES, MSGQ_NULL, com_param);
}

/*--------------------------------------------------------------------*/
void audio_cpu_dsp_intercom_callback(ApuComPrm_t *p_param)
{
	if (p_param->command == CPU_CMD_CORE6_AUDIO_APU_BOOT_DONE){
		/* Apu Coreのブート */
		switch( p_param->ex_data ){
		case CPUID_APU0:
			Chateau_SignalSemaphoreIsr(APU0_BOOT_SEM);
			break;
		case CPUID_APU1:
			Chateau_SignalSemaphoreIsr(APU1_BOOT_SEM);
			break;
		case CPUID_APU2:
			Chateau_SignalSemaphoreIsr(APU2_BOOT_SEM);
			break;
		default:
			F_ASSERT(0);
		}
		return;
	}
	if(p_param->command == CPU_CMD_AUDIO_APU_ERROR){
		apu_assert(p_param->cpu_id, reinterpret_cast<char *>(p_param->data.value),
			   p_param->ex_data);
	}
	audio_recoder_cpu_intercom(p_param);
}

extern int initfFatFs(void);
extern int finalizeFatFs(void);
extern bool createFileList();
extern bool sampleTest();

void test_main(void)
{
	ApuCpuComClass::init(audio_cpu_dsp_intercom_callback);
	
	/* MessageLibの初期化 */
	MsgLib::initFirst();
	MsgLib::initPerCpu();
	/* MemMgrLiteの初期化 */
	MemMgrLite::Manager::initFirst(manager_data_area, MEMMGR_DATA_AREA_SIZE);
	MemMgrLite::Manager::initPerCpu(manager_data_area);
	void* work_va = MemMgrLite::translatePoolAddrToVa(MEMMGR_WORK_AREA_ADDR);
	MemMgrLite::Manager::createStaticPools(1, work_va, MEMMGR_MAX_WORK_SIZE); /* layout no :1 for Voice Recorder */
	S_ASSERT(MEMMGR_WORK_AREA_SIZE >= MEMMGR_MAX_WORK_SIZE);
	AS_ReadyForAudioSubSystemActivated();

	DBG_LOG_DEBUG("mp3 encode component test WakeUp!\n");
	DBG_LogInit();

#ifdef ENABLE_FLASH_BOOT
	if (PM_LoadImage(PM_CPU_ADSP2, "MP3ENC") != 0) {
		DBG_LOGF_ERROR("APU0 LOAD ERROR\n");
	}
	/* ADSP2の電源ON */
	if (PM_StartCpu(PM_CPU_ADSP2) != 0) {
		DBG_LOGF_ERROR("APU0 START ERROR\n");
	}
	/* Apu起動待ち */
	if (Chateau_TimedWaitSemaphore(APU0_BOOT_SEM, 3000) == 0) {
		DBG_LOG_ERROR("Failed APU0 Boot\n");
		F_ASSERT(0);
	}
#else
	/* Apu起動待ち */
	/* ICE起動時は、Apu起動が手動になるため、待ち時間30秒としている。*/
	if (Chateau_TimedWaitSemaphore(APU0_BOOT_SEM, 30000) == 0) {
		DBG_LOG_ERROR("Failed APU0 Boot\n");
		F_ASSERT(0);
	}
#endif

	/* ここから下に、テストコードを記述する */
	if (initfFatFs() == 0) {
		if (createFileList()) {
			DBG_LOG_INFO("Test Start!\n");
			while(sampleTest());
			DBG_LOG_INFO("Test End!\n");
		}
	}
	finalizeFatFs();

	vTaskSuspend(NULL);
}

} /* extern "C" */

