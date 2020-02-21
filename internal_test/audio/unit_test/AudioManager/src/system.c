/*
System系のstub
*/
#include <stdint.h>
#include "stub/system.h"

typedef int16_t			SYS_Id;
typedef uint32_t		SYS_FlagPattern;
typedef int				SYS_Timeout;
typedef unsigned int	SYS_WaitMode;

/* 関数名はos_wrapper_rename.hでnew付きにリネームされる */

int SYS_WaitFlag_Called = 0;
int SYS_WaitFlag_Sequence = 0;
int newSYS_WaitFlag(SYS_Id id, SYS_FlagPattern waitPattern, SYS_WaitMode waitMode, SYS_FlagPattern *flagPattern, SYS_Timeout timeout)
{
	SYS_WaitFlag_Called++;
	SYS_WaitFlag_Sequence = cpputest_call_sequence++;
	return 0; /* とりあえず、戻り値はF_ASSERTでしか評価されないので 0固定 */
}

int SYS_SetFlag_Called = 0;
int SYS_SetFlag_Sequence = 0;
int newSYS_SetFlag(SYS_Id id, SYS_FlagPattern setPattern)
{
	SYS_SetFlag_Called++;
	SYS_SetFlag_Sequence = cpputest_call_sequence++;
	return 0; /* とりあえず、戻り値はF_ASSERTでしか評価されないので 0固定 */
}

