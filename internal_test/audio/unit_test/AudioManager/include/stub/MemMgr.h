#include "MemHandle.h"

/* �X�^�u����/�ϑ��p(���̂̓X�^�u��) */
namespace MemMgrLite {
extern int translatePoolAddrToVa_Called;
extern PoolAddr translatePoolAddrToVa_addr;
extern void *translatePoolAddrToVa_ReturnValue;
extern int initFirst_Called;
extern int initFirst_Sequence;
extern void *initFirst_manager_area;
extern uint32_t initFirst_area_size;
extern int initPerCpu_Called;
extern int initPerCpu_Sequence;
extern void *initPerCpu_manager_area;
extern int createStaticPools_Called;
extern NumLayout createStaticPools_layout_no;
extern void *createStaticPools_work_area;
extern uint32_t createStaticPools_area_size;
extern int destroyStaticPools_Called;
} /* namespace MemMgrLite */

/* �֐��Ăяo�����J�E���^(���̂̓e�X�g�R�[�h��) */
extern int cpputest_call_sequence;

