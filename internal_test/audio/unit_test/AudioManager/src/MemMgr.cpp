/*
MemMgrLiteŒn‚Ìstub
*/
#include <stdint.h>
#include <stdlib.h>
#include "MemHandle.h"
#include "stub/MemMgr.h"

extern int cpputest_call_sequence;

namespace MemMgrLite {

int translatePoolAddrToVa_Called = 0;
PoolAddr translatePoolAddrToVa_addr;
void *translatePoolAddrToVa_ReturnValue = NULL;

int initFirst_Called = 0;
int initFirst_Sequence = 0;
void *initFirst_manager_area;
uint32_t initFirst_area_size;

int initPerCpu_Called = 0;
int initPerCpu_Sequence = 0;
void *initPerCpu_manager_area;

int createStaticPools_Called = 0;
int createStaticPools_Sequence = 0;
NumLayout createStaticPools_layout_no;
void *createStaticPools_work_area;
uint32_t createStaticPools_area_size;

int destroyStaticPools_Called = 0;
int destroyStaticPools_Sequence = 0;

//namespace MemMgrLite {

void* translatePoolAddrToVa(PoolAddr addr)
{
	translatePoolAddrToVa_Called++;
	translatePoolAddrToVa_addr = addr;
	return translatePoolAddrToVa_ReturnValue;
}

void Manager::initFirst(void* manager_area, uint32_t area_size)
{
	initFirst_Called++;
	initFirst_Sequence = cpputest_call_sequence++;
	initFirst_manager_area = manager_area;
	initFirst_area_size = area_size;
}

void Manager::initPerCpu(void* manager_area)
{
	initPerCpu_Called++;
	initPerCpu_Sequence = cpputest_call_sequence++;
	initPerCpu_manager_area = manager_area;
}

void Manager::createStaticPools(NumLayout layout_no, void* work_area, uint32_t area_size)
{
	createStaticPools_Called++;
	createStaticPools_layout_no = layout_no;
	createStaticPools_work_area = work_area;
	createStaticPools_area_size = area_size;
	createStaticPools_Sequence = cpputest_call_sequence++;
}

void Manager::destroyStaticPools()
{
	destroyStaticPools_Called++;
	destroyStaticPools_Sequence = cpputest_call_sequence++;
}

} /* end of namespace MemMgrLite */

