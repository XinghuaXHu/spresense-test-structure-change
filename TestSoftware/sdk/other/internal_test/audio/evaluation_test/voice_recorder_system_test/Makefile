##### Project Makefile #####

# Specify the relative path to the top of repository #
SPRITZER_TOP_REL := ../../../../..

# Project name
PROJECT_NAME := voice_recorder_system_test

# Target name
TARGET_NAME := spritzer_es_syseva
# for BDK-TS ES 
#TARGET_NAME := spritzer_es_spark

# Configuration data
CONFIG_DATA := test/audio/spritzer_es_syseva/developer_evaluation_test/$(PROJECT_NAME)/config/component/component.conf
CONFIG_DATA += test/audio/spritzer_es_syseva/developer_evaluation_test/$(PROJECT_NAME)/config/audio/audio.conf

##### Specify the source code for each cpu cluster #####
VPATH += src/

# C source code
APP_C_SRCS = 

# C++ source code
APP_CPP_SRCS = app_main.cpp as_voice_recorder_shell.cpp 

# assembler source code
APP_ASM_SRCS =

##### Additional modules and libraries #####
M0P_MODULES_EXT = dbg_shell modar_mgr dbg_cmd_os dbg_cmd_os_pm dbg_cmd_os_free dbg_cmd_os_top
GPS_MODULES_EXT =
APP_MODULES_EXT = dbg_log dbg_exception dbg_shell_sub
DSP_MODULES_EXT = cs_mod as_audio dbg_dump +fatfs +emmc
DSP_MODULES_EXT += +as_capture +as_decoder +as_encoder +as_filter +as_renderer +as_voice_command +as_voice_trigger

M0P_SW_OBJS_EXT = 
GPS_SW_OBJS_EXT = 
APP_SW_OBJS_EXT = libcommon.alf
DSP_SW_OBJS_EXT = libaudiocommon.alf libaudiobbdriver.alf libmediarecorder.alf libaudiomanager.alf libcommon.alf libsoundeffect.alf \
	stbaudioplayer.alf stbaudiovoicecommand.alf stbaudiovoicetrigger.alf stbaudiovoicerecorder.alf stbaudiovoicerecognition.alf 

M0P_SW_BLOCK_EXT = 
GPS_SW_BLOCK_EXT =
APP_SW_BLOCK_EXT = common
DSP_SW_BLOCK_EXT = audio storage common appfw sensors

##############################################################
########## Basically, should not need to edit below ##########
include $(SPRITZER_TOP_REL)/Project.mk

