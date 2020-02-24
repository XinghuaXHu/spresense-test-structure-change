#ifndef MANAGER_H_INCLUDED
#define MANAGER_H_INCLUDED

namespace MemMgrLite {

typedef uint32_t	PoolAddr;
typedef uint8_t		NumLayout;		/* ���������C�A�E�g��(�ő�255) */

/* �ϊ��֐��B�ŏ����A�h���X�`�F�b�N�̂� */
void* translatePoolAddrToVa(PoolAddr addr);

class MemHandle {
};
	
class Manager {
public:
	/* MemoryManager�S�̂̏������B�P���CPU��1�񂾂��ďo������ */
	static void	initFirst(void* manager_area, uint32_t area_size);

	/* MemoryManager��CPU���̏������B�eCPU��1�񂾂��ďo������ */
	static void	initPerCpu(void* manager_area);

	/* �ÓI�������v�[���̐���/�j��/�֘A���擾 */
	static void	createStaticPools(NumLayout layout_no, void* work_area, uint32_t area_size);
	static void	destroyStaticPools();
};

} /* namespace MemMgrLite */

#endif /* MANAGER_H_INCLUDED */
