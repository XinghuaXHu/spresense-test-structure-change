#====================================================================#
# 
#       File Name: Makefile
#       
#       Description: 
#           This is Makefile on CppUTest exe.
# 
#       Notes: (C) Copyright 2015 Sony Corporation
# 
#       Author: Tetsuya Sakai
# 
#====================================================================#
#--------------------------------------------------------------------#
# Configuration File Definition
#--------------------------------------------------------------------#
TOP_DIR		= ..

BASE_CONFIG	= $(TOP_DIR)/Config/base_config.mk

#------------------------------------------------#
# These files must do include first.
# Do in order of BASE_CONFIG, MAKE_CONFIG, and PATHS.
#------------------------------------------------#
include $(BASE_CONFIG)
include $(MAKE_CONFIG)
include $(PATHS)

#--------------------------------------------------------------------#
# Make Target
#--------------------------------------------------------------------#
all: build
	@echo $(TEST_EXE)
	@$(TEST_EXE) -ojunit || true

build:
	perl chg_test_code.pl
	cp $(ROOT_DIR)/audio/manager/attention.h ../include
	cp $(ROOT_DIR)/audio/manager/baseband_config.h ../include
	cp $(ROOT_DIR)/audio/manager/audio_sram_ctrl.h ../include
	cp $(ROOT_DIR)/audio/manager/audio_layout.h ../include
	cp $(ROOT_DIR)/audio/manager/audio_io_config.h ../include
	cp $(ROOT_DIR)/audio/manager/audio_sram_ctrl.h ../include
	cp $(ROOT_DIR)/audio/common/include/sdk_message_types.h ../include
	cp $(ROOT_DIR)/audio/common/include/wien2_common_defs.h ../include
	cp $(ROOT_DIR)/audio/common/include/wien2_internal_packet.h ../include
	cp $(ROOT_DIR)/audio/common/include/wien2_debug_print.h ../include
	cp $(ROOT_DIR)/audio/common/chateau/include/common_types.h ../include
	cp $(ROOT_DIR)/audio/common/chateau/include/common_ecode.h ../include
	cp $(ROOT_DIR)/audio/common/chateau/include/common_assert.h ../include
	cp $(ROOT_DIR)/audio/common/chateau/include/common_macro.h ../include
	cp $(ROOT_DIR)/audio/common/chateau/include/common_attention.h ../include
	cp $(ROOT_DIR)/audio/common/chateau/include/cpp_util.h ../include
	cp $(ROOT_DIR)/audio/common/chateau/include/chateau_assert.h ../include
	cp $(ROOT_DIR)/audio/common/chateau/include/chateau_macro.h ../include
	$(MAKE) -f Makefile.build
	$(RM) ../include/attention.h
	$(RM) ../include/baseband_config.h
	$(RM) ../include/audio_sram_ctrl.h
	$(RM) ../include/audio_layout.h
	$(RM) ../include/audio_io_config.h
	$(RM) ../include/audio_sram_ctrl.h
	$(RM) ../include/sdk_message_types.h
	$(RM) ../include/wien2_common_defs.h
	$(RM) ../include/wien2_internal_packet.h
	$(RM) ../include/wien2_debug_print.h
	$(RM) ../include/common_types.h
	$(RM) ../include/common_ecode.h
	$(RM) ../include/common_assert.h
	$(RM) ../include/common_macro.h
	$(RM) ../include/common_attention.h
	$(RM) ../include/cpp_util.h
	$(RM) ../include/chateau_assert.h
	$(RM) ../include/chateau_macro.h
	$(RM) ../include/audio_manager.h
	$(RM) ../src/audio_manager.cpp

#------------------------------------------------#
# Target clean
#------------------------------------------------#
clean allclean:
	$(RM) *.xml
ifeq ($(MAKECMDGOALS),allclean)
	$(RM) *.gcno
	$(RM) *.gcda
	$(RM) *.gcov
	$(RM) *.d
	$(RM) *.txt
	$(MAKE) -f Makefile.build $@
endif

#------------------------------------------------#
# other
#------------------------------------------------#
.PHONY: all build clean allclean

#--------------------------------------------------------------------#
# Do the include of these files at the last.
#--------------------------------------------------------------------#
include $(RULES)
