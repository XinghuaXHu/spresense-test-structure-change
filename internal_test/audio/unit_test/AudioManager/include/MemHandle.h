#ifndef MANAGER_H_INCLUDED
#define MANAGER_H_INCLUDED

namespace MemMgrLite {

typedef uint32_t	PoolAddr;
typedef uint8_t		NumLayout;		/* メモリレイアウト数(最大255) */

/* 変換関数。最小限アドレスチェックのみ */
void* translatePoolAddrToVa(PoolAddr addr);

class MemHandle {
};
	
class Manager {
public:
	/* MemoryManager全体の初期化。単一のCPUで1回だけ呼出すこと */
	static void	initFirst(void* manager_area, uint32_t area_size);

	/* MemoryManagerのCPU毎の初期化。各CPUで1回だけ呼出すこと */
	static void	initPerCpu(void* manager_area);

	/* 静的メモリプールの生成/破棄/関連情報取得 */
	static void	createStaticPools(NumLayout layout_no, void* work_area, uint32_t area_size);
	static void	destroyStaticPools();
};

} /* namespace MemMgrLite */

#endif /* MANAGER_H_INCLUDED */
