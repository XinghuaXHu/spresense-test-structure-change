/*##############################################################################
指示付きの初期化子を使わないよう修正(c++では使えないので)
 * Copyright 2015 Sony Corporation.
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sony Corporation.
 * No part of this file may be copied, modified, sold, and distributed in any
 * form or by any means without prior explicit permission in writing from
 * Sony Corporation.
 */
/**
 * @file       pm_cpufreqlock.h
 * @note       power manager cpu freq lock
 * @attention  
 */
/*############################################################################*/
#ifndef __PM_CPUFREQLOCK_H__
#define __PM_CPUFREQLOCK_H__

/**
 * @defgroup sys_power_mgr Power Manager Cpu FreqLock
 *
 * Power Manager Cpu FreqLock API.
 * Actual usage and more details are see other documents.
 *
 * <pre>\#include <system/pm_cpufreqlock.h></pre>
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include <stdint.h>
#include <sys_queue.h>

#include <system/pm_locktag.h>

//#define CONFIG_PM_DEBUG_CPUFREQLOCK

/**
 * CPU FreqLock structure
 */
typedef struct PM_CpuFreqLock {
	SLIST_ENTRY(PM_CpuFreqLock) entry;
	int			count;
	uint32_t	info;
	int			mips;
	int			flag;
#ifdef CONFIG_PM_DEBUG_CPUFREQLOCK
	uint32_t	elapse;
	uint32_t	ra;
	uint32_t	sp;
#endif
} PM_CpuFreqLock_t;

/* flags */
/// flag to request HV state
#define PM_CPUFREQLOCK_FLAG_HV				0x0001
/// (INTERNAL USE ONLY) initialized
#define PM_CPUFREQLOCK_FLAG_INITIALIZED		0x8000
/// (INTERNAL USE ONLY) use PLL
#define PM_CPUFREQLOCK_FLAG_PLL				0x4000

/**
 * for debug (TBD)
 */
#ifdef CONFIG_PM_DEBUG_CPUFREQLOCK
# define PM_DEBUG_CPUFREQLOCK() \
	/*.elapse = */ 0, \
	/*.ra = */ 0, \
	/*.sp = */ 0,
#else
# define PM_DEBUG_CPUFREQLOCK()
#endif

/**
 * Initialization macro of a CPU FreqLock variable 
 *
 *  @param[in] _tag: identifier of this PM_CpuFreqLock_t instance.
 *  @param[in] _mips: estimated mips which this lock requires.
 *  @param[in] _flag: flag values which defines the requested behavior.
 */
#define PM_CPUFREQLOCK_INIT(_tag, _mips, _flag) \
{ \
	/*.entry = */ NULL, \
	/*.count = */ 0, \
	/*.info = */ _tag, \
	/*.mips = */ _mips, \
	/*.flag = */ _flag, \
	PM_DEBUG_CPUFREQLOCK() \
}

/**
 * Declaration macro of a CPU FreqLock variable 
 */
#define DEFINE_PM_CPUFREQLOCK(x, tag)	PM_CpuFreqLock_t x = PM_CPUFREQLOCK_INIT(tag)

/*---------------------------------------------------------------------------*/
/**
 *  CPU FreqLock System Initialization
 *
 *  @param[in] head: head of freqlock list
 *  @return 0 on success, otherwise error
 *
 *  @par Blocking
 *      Yes
 *  @par Context
 *      Task / Interrupt
 *  @par Reentrant
 *      No
 *
 *  @attention This API is called from Power Manager only once at system boot,
 *  and so there is no need to call from a user client.
 */
/*---------------------------------------------------------------------------*/
int PM_InitCpuFreqLockSystem(void *head);
/*---------------------------------------------------------------------------*/
/**
 *  Acquire CPU FreqLock
 *
 *  @param[in] lock: a pointer to PM_CpuFreqLock_t structure
 *  @return 0 on success, otherwise error
 *  @retval -EINVAL: invalid argument
 *
 *  @par Blocking
 *      Yes
 *  @par Context
 *      Task / Interrupt
 *  @par Reentrant
 *      Yes
 */
/*---------------------------------------------------------------------------*/
int PM_AcquireCpuFreqLock(PM_CpuFreqLock_t *lock);
/*---------------------------------------------------------------------------*/
/**
 *  Acquire CPU FreqLock with timeout
 *
 *  @param[in] lock: a pointer to PM_CpuFreqLock_t structure
 *  @param[in] timeout: timeout value in millisecond
 *  @return 0 on success, otherwise error
 *  @retval -EINVAL: invalid argument
 *  @retval -ENOTSUP: not supported
 *
 *  @par Blocking
 *      Yes
 *  @par Context
 *      Task / Interrupt
 *  @par Reentrant
 *      Yes
 *
 *  @attention This API is not implemented yet, return -ENOTSUP
 */
/*---------------------------------------------------------------------------*/
int PM_AcquireCpuFreqLockTimed(PM_CpuFreqLock_t *lock, int timeout);
/*---------------------------------------------------------------------------*/
/**
 *  Release CPU FreqLock
 *
 *  @param[in] lock: a pointer to PM_CpuFreqLock_t structure
 *  @return 0 on success, otherwise error
 *  @retval -EINVAL: invalid argument
 *
 *  @par Blocking
 *      Yes
 *  @par Context
 *      Task / Interrupt
 *  @par Reentrant
 *      Yes
 */
/*---------------------------------------------------------------------------*/
int PM_ReleaseCpuFreqLock(PM_CpuFreqLock_t *lock);
/*---------------------------------------------------------------------------*/
/**
 *  Get CPU FreqLock status
 *
 *  @param[in] lock: a pointer to PM_CpuFreqLock_t structure
 *  @return > 0 on success, otherwise error
 *    - if locked,  return > 0
 *    - if unlocked, return 0
 *  @retval -EINVAL: invalid argument
 *
 *  @par Blocking
 *      Yes
 *  @par Context
 *      Task / Interrupt
 *  @par Reentrant
 *      Yes
 */
/*---------------------------------------------------------------------------*/
int PM_GetCpuFreqLockStatus(PM_CpuFreqLock_t *lock);
/*---------------------------------------------------------------------------*/
/**
 *  Count the number of CPU FreqLocked
 *
 *  @return the number of CPU FreqLocked
 *
 *  @par Blocking
 *      Yes
 *  @par Context
 *      Task / Interrupt
 *  @par Reentrant
 *      Yes
 */
/*---------------------------------------------------------------------------*/
int PM_CountAcquiredCpuFreqLock(void);
/*---------------------------------------------------------------------------*/
/**
 *  Dump CPU FreqLocked status for debug
 *
 *  @return 0 on success, otherwise error
 *
 *  @par Blocking
 *      Yes
 *  @par Context
 *      Task / Interrupt
 *  @par Reentrant
 *      Yes
 */
/*---------------------------------------------------------------------------*/
int PM_DumpCpuFreqLock(void);

#ifdef __cplusplus
}
#endif

#endif /* __PM_CPUFREQLOCK_H__ */
