#====================================================================#
#
#       File Name: rules.mk
#
#       Description: This is Make rules for Audio original.
#
#       Notes: (C) Copyright 2015 Sony Corporation
#
#       Author: Tetsuya Sakai
#
#====================================================================#

#--------------------------------------------------------------------#
# Create Fixlayout files.
#--------------------------------------------------------------------#
$(MEMLAYOUT_H) $(FIXEDFENCE_H) $(POOLLAYOUT_H): $(MEM_CONF) $(MAKE_MEMPOOL)
	ruby -I$(TOOL_MEMMGR_LITE_LIB_DIR) -I$(AUDIO_SDK_ROOT)/common/config $(MEM_CONF) $(MEMLAYOUT_H) $(FIXEDFENCE_H) $(POOLLAYOUT_H)

#--------------------------------------------------------------------#
# Create Dump files.
#--------------------------------------------------------------------#
DMP_ADDR = $(shell ruby -ne 'print $$_.split[2] if $$_ =~ /SPU_LOG_AREA_ADDR/' < $(MEMLAYOUT_H))
DMP_SIZE = $(shell ruby -ne 'print $$_.split[2] if $$_ =~ /SPU_LOG_AREA_SIZE/' < $(MEMLAYOUT_H))

$(DUMP_ID_H) $(DUMP_POOL_C): $(DUMP_CONF) $(MEMLAYOUT_H)
	ruby -I$(TOOL_DUMP_LIB_DIR) $(DUMP_CONF) $(DMP_ADDR) $(DMP_SIZE) $(DUMP_ID_H) $(DUMP_POOL_C)

#--------------------------------------------------------------------#
# Create SpinLock files.
#--------------------------------------------------------------------#
$(CPU_ID_H) $(SPL_ID_H) $(SPL_POOL_H): $(SPL_CONF) $(MEMLAYOUT_H)
	ruby -I$(INTER_CPU_LOCK_DIR)/tool $(SPL_CONF) $(MEMLAYOUT_H) SPL_MGR_AREA $(CPU_ID_H) $(SPL_ID_H) $(SPL_POOL_H)

#--------------------------------------------------------------------#
# Create MessageQue files.
#--------------------------------------------------------------------#
$(MSGQ_ID_H) $(MSGQ_POOL_H): $(MSGQ_CONF) $(MEMLAYOUT_H) $(SPL_ID_H)
	ruby -I$(MESSAGE_LIB_TOP)/tool -I$(COMMON_DIR)/config $(MSGQ_CONF) $(MEMLAYOUT_H) MSG_QUE_AREA $(MSGQ_ID_H) $(MSGQ_POOL_H)

