/* ���ʏ��� */
/* MemMgrLite��createStaticPools, destroyStaticPools�m�F�p */

void prepare_common_memmgr_create(void)
{
	MemMgrLite::translatePoolAddrToVa_Called = 0;
	int tmp;
	MemMgrLite::translatePoolAddrToVa_ReturnValue = (void *)&tmp; //�K���ȃA�h���X
	MemMgrLite::createStaticPools_Called = 0;
	MemMgrLite::destroyStaticPools_Called = 0; // �Ă΂�Ȃ�����check�p
}

void check_common_memmgr_create(MemMgrLite::NumLayout layout_no)
{
	CHECK(MemMgrLite::translatePoolAddrToVa_Called == 1);
	CHECK(MemMgrLite::translatePoolAddrToVa_addr == MEMMGR_WORK_AREA_ADDR );
	CHECK(MemMgrLite::createStaticPools_Called == 1);
	CHECK(MemMgrLite::createStaticPools_layout_no == layout_no);
	CHECK(MemMgrLite::createStaticPools_work_area == MemMgrLite::translatePoolAddrToVa_ReturnValue);
	CHECK(MemMgrLite::createStaticPools_area_size == MEMMGR_MAX_WORK_SIZE);
	CHECK(MemMgrLite::destroyStaticPools_Called == 0); // �Ă΂�Ȃ�����check
}

void prepare_common_memmgr_destroy(void)
{
	MemMgrLite::createStaticPools_Called = 0; // �Ă΂�Ȃ�����check�p
	MemMgrLite::destroyStaticPools_Called = 0;
}

void check_common_memmgr_destroy(void)
{
	CHECK(MemMgrLite::createStaticPools_Called == 0); // �Ă΂�Ȃ�����check
	CHECK(MemMgrLite::destroyStaticPools_Called == 1);
}


