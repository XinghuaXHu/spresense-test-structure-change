/* 共通処理 */
/* MemMgrLiteのcreateStaticPools, destroyStaticPools確認用 */

void prepare_common_memmgr_create(void)
{
	MemMgrLite::translatePoolAddrToVa_Called = 0;
	int tmp;
	MemMgrLite::translatePoolAddrToVa_ReturnValue = (void *)&tmp; //適当なアドレス
	MemMgrLite::createStaticPools_Called = 0;
	MemMgrLite::destroyStaticPools_Called = 0; // 呼ばれないことcheck用
}

void check_common_memmgr_create(MemMgrLite::NumLayout layout_no)
{
	CHECK(MemMgrLite::translatePoolAddrToVa_Called == 1);
	CHECK(MemMgrLite::translatePoolAddrToVa_addr == MEMMGR_WORK_AREA_ADDR );
	CHECK(MemMgrLite::createStaticPools_Called == 1);
	CHECK(MemMgrLite::createStaticPools_layout_no == layout_no);
	CHECK(MemMgrLite::createStaticPools_work_area == MemMgrLite::translatePoolAddrToVa_ReturnValue);
	CHECK(MemMgrLite::createStaticPools_area_size == MEMMGR_MAX_WORK_SIZE);
	CHECK(MemMgrLite::destroyStaticPools_Called == 0); // 呼ばれないことcheck
}

void prepare_common_memmgr_destroy(void)
{
	MemMgrLite::createStaticPools_Called = 0; // 呼ばれないことcheck用
	MemMgrLite::destroyStaticPools_Called = 0;
}

void check_common_memmgr_destroy(void)
{
	CHECK(MemMgrLite::createStaticPools_Called == 0); // 呼ばれないことcheck
	CHECK(MemMgrLite::destroyStaticPools_Called == 1);
}


