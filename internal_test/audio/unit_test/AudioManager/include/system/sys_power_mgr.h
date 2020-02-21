/*
stub version
本物はinclude/system/sys_power_mgr.h
*/
#ifndef SYS_POWER_MGR_H
#define SYS_POWER_MGR_H

#include <stdint.h>
#define CHIP_NAME_SPRITZER_ES
#include <system/pm_clock.h>

// エラーコード(SDK/include/sys_errno.h)
#define	EBUSY		16		/* Device busy */
#define	EINVAL		22		/* Invalid argument */

typedef void (*PM_CB)(int);

// 本物はinclude/system/pm_clock.hで定義
/*
typedef union param{
	uint32_t div;
	uint32_t sel;
	uint32_t gear;
}param_t;

typedef struct {
	uint32_t target;
	param_t param;
} PM_Clock_t;
*/

/* プロトタイプ宣言(本物は一部関数ではなくマクロ) */
#ifdef  __cplusplus
extern "C"{
#endif
int PM_AudioPowerOn(uint32_t id, PM_CB fp);
int PM_AudioPowerOff(uint32_t id, PM_CB fp);
int PM_PeriClockDisable(uint32_t id, PM_CB fp);
int PM_AudioClockEnable(uint32_t source, uint32_t xdiv, PM_CB fp);
int PM_ChangeClock (PM_Clock_t target[], int num);
int PM_RamControlByAddress (uint32_t address, uint32_t size, uint32_t status, PM_CB fp);
int PM_StopCpu(int cpuid);
#ifdef  __cplusplus
}
#endif

#define PM_AUDIO_MODE_RCOSC		2/**< Audio clock source = RCOSC */
#define PM_AUDIO_MODE_XOSC		1/**< Audio clock source = XOSC */
#define PM_AUDIO_MODE_MCLK		0/**< Audio clock source = external MCLK */

enum PM_ID {
    PM_KAKI = 1,	/**< Peripheral ID = KAKI ES:PWD_SYSIOP_SUB */
    PM_RTC1,		/**< Peripheral ID = RTC1 */
    PM_RTC2,		/**< Peripheral ID = RTC2 */
    PM_DMAC0,		/**< Peripheral ID = DMAC0 ES:uDMAC*/
    PM_DMAC1,		/**< Peripheral ID = DMAC1 */
    PM_HOSTIFC,		/**< Peripheral ID = HOSTIFC */
    PM_SPI_S,		/**< Peripheral ID = SPI Slave ES:HsotIF(SPI-Slave) */
    PM_I2C_S,		/**< Peripheral ID = I2C Slave ES:HsotIF(I2C-Slave) */
    PM_UART0,		/**< Peripheral ID = UART0 ES:HsotIF(UART) */
    PM_DMAC2,		/**< Peripheral ID = DMAC2 */
    PM_DMAC3,		/**< Peripheral ID = DMAC3 */
    PM_SPIF,		/**< Peripheral ID = SPI Flash I/F */
    PM_I2C_M,		/**< Peripheral ID = I2C Master ES:PWD_APP_SUB */
    PM_SPI_M,		/**< Peripheral ID = SPI Master ES:PWD_APP_SUB */
    PM_UART1,		/**< Peripheral ID = UART1 Debug UART*/
    PM_USB,			/**< Peripheral ID = USB ES:PWD_APP_SUB */
    PM_EMMC,		/**< Peripheral ID = eMMC ES:PWD_APP_SUB */
    PM_HSADC,		/**< Peripheral ID = HSADC ES:Elimination target */
    PM_SAKE,		/**< Peripheral ID = SAKE ES:PWD_CORE*/
    PM_ROM,			/* Don't use (internal specific id) */
    PM_AUDIO,		/**< Peripheral ID = AUDIO  ES:PWD_APP */
    PM_SLIMBUS,		/**< Peripheral ID = SLIMBUS ES:Elimination target */
    PM_SYSTEM,
    PM_SCU,			/**< Peripheral ID = SCU */
    PM_SCU_SPI,		/**< Peripheral ID = SCU_SPI ES:Integration PWD_SCU */
    PM_SCU_I2C0,	/**< Peripheral ID = SCU_I2C0 ES:Integration PWD_SCU */
    PM_SCU_I2C1,	/**< Peripheral ID = SCU_I2C1 ES:Integration PWD_SCU */
    PM_SCU_LPADC,	/**< Peripheral ID = SCU_LPADC */
    PM_SCU_HPADC0,	/**< Peripheral ID = SCU_HPADC0 */
    PM_GPS_ITP,		/**< Peripheral ID = GPS_ITP */
    PM_GPS_BB,		/**< Peripheral ID = GPS_BB ES:PWD_GNSS */
    PM_GPS_COP,		/**< Peripheral ID = GPS_COP ES:PWD_GNSS*/
    PM_ADONIS,		/**< Peripheral ID = ADONIS ES:Elimination target */
    PM_TIMER,		/**< Peripheral ID = TIMER */
    PM_WDT,			/**< Peripheral ID = WDT */
    PM_SCU_HPADC1,	/**< Peripheral ID = SCU_HPADC1 */
    PM_LNA,			/**< Peripheral ID = power supply input via PMIC */
    PM_BT,			/**< Peripheral ID = Bluetooth via PMIC */
    PM_XOSC,		/**< Peripheral ID = XOSC */
    PM_PMU,			/**< Peripheral ID = PMU */
#if defined(CHIP_NAME_SPRITZER_ES)
    PM_APP,			/**< Peripheral ID = PWD_APP */
    PM_APP_KAKI,	/**< Peripheral ID = PWD_APP KAKI */
    PM_APP_RAM,		/**< Peripheral ID = PWD_APP RAM */
    PM_APP_ADSP0,	/**< Peripheral ID = PWD_APP_DSP ADSP0 */
    PM_APP_ADSP1,	/**< Peripheral ID = PWD_APP_DSP ADSP1 */
    PM_APP_ADSP2,	/**< Peripheral ID = PWD_APP_DSP ADSP2 */
    PM_APP_ADSP3,	/**< Peripheral ID = PWD_APP_DSP ADSP3 */
    PM_APP_ADSP4,	/**< Peripheral ID = PWD_APP_DSP ADSP4 */
    PM_APP_ADSP5,	/**< Peripheral ID = PWD_APP_DSP ADSP5 */
    PM_APP_SAKE,	/**< Peripheral ID = PWD_APP_DSP SAKE */
    PM_APP_ADMAC,	/**< Peripheral ID = PWD_APP_DSP ADMAC */
    PM_APP_SKDMAC,	/**< Peripheral ID = PWD_APP_DSP SKDMAC */
    PM_APP_GE2D,	/**< Peripheral ID = PWD_APP_SUB GE2D */
    PM_APP_IDMAC,	/**< Peripheral ID = PWD_APP_SUB IDMAC */
    PM_APP_SPI,		/**< Peripheral ID = PWD_APP_SUB SPI */
    PM_APP_WSPI,	/**< Peripheral ID = PWD_APP_SUB WSPI(WiFi) */
    PM_APP_UART,	/**< Peripheral ID = PWD_APP_SUB UART */
    PM_APP_CISIF,	/**< Peripheral ID = PWD_APP_SUB CISIF */
    PM_APP_SDIO,	/**< Peripheral ID = PWD_APP_SUB SDIO */
#endif
};

/*
	PM_RamControlByTile(status);
	PM_RamControlByAddress(status);
*/
#define PM_SRAM_ON	3	/**< RAM status = Power On */
#define PM_SRAM_RET	1	/**< RAM status = Retention */
#define PM_SRAM_OFF	0	/**< RAM status = Power Off */

#define PM_CLOCK_MODE_SYSPLL_160M	(6) /**< Clock Change Mode = SYSPLL 160MHz/1 */

/*
	PM_BootCpu(cpuid);
	PM_BootCpuWithWaitMode(cpuid);
	PM_SleepCpu(cpuid);
	PM_WakeUpCpu(cpuid);
*/
#if defined(CHIP_NAME_SPRITZER_ES) || defined(CHIP_NAME_BECRUX)
	enum{
		PM_CPU_GDSP = 1,/*(1) < CPUID = GDSP */
		PM_CPU_ADSP0,/*(2) < CPUID = ADSP0 */
		PM_CPU_ADSP1,/*(3) < CPUID = ADSP1 */
		PM_CPU_ADSP2,/*(4) < CPUID = ADSP2 */
		PM_CPU_ADSP3,/*(5) < CPUID = ADSP3 */
		PM_CPU_ADSP4,/*(6) < CPUID = ADSP4 */
		PM_CPU_ADSP5,/*(7) < CPUID = ADSP5 */
		PM_CPU_ES_NUM/* MAX CPUID */
	};
#endif /* CHIP_NAME_SPRITZER_ES */
	enum{
		PM_CPU_M4F = 1,/*(1) < CPUID = M4F */
		PM_CPU_DSP00,/*(2) < CPUID = DSP0-0 */
		PM_CPU_DSP01,/*(3) < CPUID = DSP0-1 */
		PM_CPU_DSP02,/*(4) < CPUID = DSP0-2 */
		PM_CPU_DSP03,/*(5) < CPUID = DSP0-3 */
		PM_CPU_DSP10,/*(6) < CPUID = DSP1-0 */
		PM_CPU_DSP11,/*(7) < CPUID = DSP1-1 */
		PM_CPU_DSP12,/*(8) < CPUID = DSP1-2 */
		PM_CPU_DSP13,/*(9) < CPUID = DSP1-3 */
		PM_CPU_NUM/* MAX CPUID */
	};
#if defined(CHIP_NAME_SPRITZER_TS)
#define PM_CPU_MAXNUM	(10) // PM_CPU_NUM
#elif defined(CHIP_NAME_SPRITZER_ES)
#define PM_CPU_MAXNUM	(8) // PM_CPU_ES_NUM
#elif defined(CHIP_NAME_BECRUX)
#define PM_CPU_MAXNUM	(2)
#else
#error illegal CHIP_NAME error!!
#endif

#endif  /* SYS_POWER_MGR_H */
