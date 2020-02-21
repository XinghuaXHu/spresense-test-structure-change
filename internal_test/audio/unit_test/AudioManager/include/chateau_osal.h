#ifndef CHATEAU_OSAL_H_INCLUDED
#define CHATEAU_OSAL_H_INCLUDED

#include "common_macro.h"
#include "common_assert.h"
extern int AssertionFailed;
extern const char* AssertionFailed_exp;
extern const char* AssertionFailed_filename;
extern int AssertionFailed_line;

//–{—ˆkernel.h->kernel_id.h‚Å’è‹`
#define AS_ACTIVATED_FLG (0)

#include <os_wrapper.h>
// begin os_wrapper.h
/* TMP tmout */
//#define TMO_POL		(0)
//#define TMO_FEVR	(-1)

/* UINT wfmode */
//#define TWF_ANDW	(0x0000)
//#define TWF_ORW		(0x0001)

//int SYS_WaitFlag(SYS_Id id, SYS_FlagPattern waitPattern, SYS_WaitMode waitMode, SYS_FlagPattern *flagPattern, SYS_Timeout timeout);
//end os_wrapper.h

#define TIME_POLLING	TMO_POL
#define TIME_FOREVER	(unsigned)TMO_FEVR

#define ATT_INI(...)
#define CRE_TSK(...)

#endif /* CHATEAU_OSAL_H_INCLUDED */
