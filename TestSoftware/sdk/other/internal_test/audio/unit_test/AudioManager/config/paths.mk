#====================================================================#
#
#       File Name: paths.mk
#
#       Description: CppUTest env local path information
#
#       Notes: (C) Copyright 2015 Sony Corporation
#
#====================================================================#
#--------------------------------------------------------------------#
# Directory or File
#--------------------------------------------------------------------#
#------------------------------------------------#
# User lib name.
#------------------------------------------------#
USER_LIB		= 
USER_LIB_ARC		= 


#------------------------------------------------#
# Top Dir.
#------------------------------------------------#
SRC_DIR			= $(TOP_DIR)/src
SRC_TEST_DIR		= $(TOP_DIR)/test
INC_DIR			= $(TOP_DIR)/include
INC_SYS_DIR		= $(TOP_DIR)/include/system
INC_DBG_DIR		= $(TOP_DIR)/include/debug
CFG_DIR			= $(TOP_DIR)/config
BUILD_DIR		= $(TOP_DIR)/build

#------------------------------------------------#
# exe File.
#------------------------------------------------#
TEST_EXE		= $(BUILD_DIR)/$(TARGET).exe

#--------------------------------------------------------------------#
# Root Path file
#--------------------------------------------------------------------#
#ROOT_PATHS		= $(ROOT_DIR)/Src/Config/paths.mk
#include $(ROOT_PATHS)

#------------------------------------------------#
# Tools definition
#------------------------------------------------#
override CC		= gcc
override CXX		= g++
override AR		= ar

COMPILE_ENV			= cygwin
CPPUTEST_DIR			= ../../lib/CppUTest/$(CPPUTEST_VERSION)
INC_CPPUTEST_DIR		= $(CPPUTEST_DIR)/include
CPPUTEST_LIB_DIR		= $(CPPUTEST_DIR)/lib/$(COMPILE_ENV)
CPPUTEST_LIB_ARC		= $(CPPUTEST_LIB_DIR)/libCppUTest.a $(CPPUTEST_LIB_DIR)/libCppUTestExt.a
