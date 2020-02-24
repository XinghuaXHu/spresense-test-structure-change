##### Specify the relative path to the top of repository #####
SPRITZER_TOP_REL := ../../../../../..
AUDIO_SDK_ROOT := $(SPRITZER_TOP_REL)/audio

#==== original config ====
CONFIG		= ./config/config.mk
PATHS		= ./config/paths.mk
RULES		= ./config/rules.mk

#==== Common DIR ====
AUDIO_CONFIG_DIR	= $(SPRITZER_TOP_REL)/audio/config
AUDIO_PATHS		= $(AUDIO_CONFIG_DIR)/paths.mk
AUDIO_CONFIG	= $(AUDIO_CONFIG_DIR)/config.mk

# Project name
PROJECT_NAME := mp3_enc_comp_test

# Target name
TARGET_NAME := spritzer_es_syseva

# Configuration data
CONFIG_DATA := /test/audio/spritzer_es_syseva/component_test/encoder/$(PROJECT_NAME)/config/component/component.conf
CONFIG_DATA +=	tool/config/pinconfig/spritzer_syseva/pin.conf \
				tool/config/pmicconfig/spritzer_syseva/pmic.conf


### 必要に応じてAPUのビルド対象となるフォルダを変更してください ###
#==== APU DIR ====
APU_TOP_DIR		= $(SPRITZER_TOP_REL)/audio/apus/
APU_DIR			= $(APU_TOP_DIR)/encoder/mp3enc
APU_BUILD_DIR	= $(APU_DIR)

#### PATHSは、SPRITZER_TOP_REL定義の後でインクルードすること                    ####
#### CONFIGは、PATHS内定義を参照しているので、PATHSより後にインクルードすること ####
include	$(PATHS)
include	$(CONFIG)

#override HEAP_MODEL = TWO_HEAP_MODEL

VPATH += ./src
VPATH += $(SPRITZER_TOP_REL)/audio/components/encoder
VPATH += $(SPRITZER_TOP_REL)/audio/objects/media_recorder
VPATH += $(SPRITZER_TOP_REL)/audio/objects/input_data_mng
VPATH += $(SPRITZER_TOP_REL)/audio/objects/player
VPATH += $(SPRITZER_TOP_REL)/audio/drivers/apu
VPATH += $(SRC_MEMMGR_LITE_LIB_DIR)
VPATH += $(SRC_MESSAGE_LIB_DIR)

##### Specify some customizable     #####
##### build options                 #####
# C Library type
# default: microlib
# export C_LIB_TYPE := standardlib
# OR
# export C_LIB_TYPE := microlib

export C_LIB_TYPE := standardlib

# Configuration type
export CONFIG_MULTICORE_ID_SUPPORT = y
export CONFIG_NEW_OS_WRAPPER := y

##### Debug Level Setting #####
#CDEFS += -DDBG_LEVEL_STATIC_DBG=DBG_LEVEL_DEBUG
#CDEFS += -DDBG_LEVEL_DYNAMIC_DBG=DBG_LEVEL_DEBUG

##### Specify the test-dependent source code for each cpu cluster #####
DSP_C_SRCS = assertion.c

#### 必要に応じて、ファイルを追加・削除 ###
DSP_CPP_SRCS = test_main.cpp sample_test.cpp encoder_component.cpp apus_drv.cpp playlist_manager.cpp wav_stream_mng.cpp

#### 以下のDSPのソースは、必須 ###
DSP_CPP_SRCS += MsgLib.cpp
DSP_CPP_SRCS += allocSeg.cpp createDynamicPool.cpp createPool.cpp createStaticPools.cpp destroyDynamicPool.cpp destroyPool.cpp destroyStaticPools.cpp
DSP_CPP_SRCS += fence.cpp freeSeg.cpp getSegAddr.cpp getSegSize.cpp getUsedSegs.cpp incSegRefCnt.cpp initFirst.cpp initPerCpu.cpp ScopedLock.cpp

GPS_ASM_SRCS =
DSP_ASM_SRCS =
DSP1_ASM_SRCS =

##### Specify the module names of each SW blocks to link #####
M0P_MODULES = dbg_log dbg_exception dbg_shell dbg_driver i2c power_mgr flash_mgr rtc_mgr sc_driver dbg_trace modar_mgr dbg_cmd_os dbg_cmd_os_pm dbg_cmd_os_free dbg_cmd_os_top
DSP_MODULES = dbg_log dbg_exception dbg_shell_sub dbg_trace_sub +usb +emmc +fatfs cs_mod

##### Specify the object files of each SW blocks to link #####
M0P_SW_OBJS = $(KERNEL_LIB) libdrivers.alf libsecmgr.alf
DSP_SW_OBJS = $(KERNEL_LIB) libdrivers.alf libpowermgr.alf libcommon.alf

##### Specify the directory name of SW block to build #####
M0P_SW_BLOCK = os drivers system debug
DSP_SW_BLOCK = os drivers system debug storage common appfw

##### Specifty the name of clusters to build #####
##### ex. m0 m4 m0p gps dsp0 dsp1            #####
all : create_dir
	$(MAKE) $(MEMLAYOUT_H)
	$(MAKE) $(DUMP_ID_H)
	$(MAKE) $(CPU_ID_H)
	$(MAKE) $(MSGQ_ID_H)
	$(MAKE) m0p
	$(MAKE) dsp
	$(MAKE) apu

#### Dirの自動生成はmake m0p/dsp1 でも行われるが、生成後リンクまで一気に行ってしまう。 ####
#### そのため、ビルドに必要な自動生成ファイルと、自動ファイル配置用のDir生成は         ####
#### make m0p/dsp1を実行前に行う必要がある。                                           ####
create_dir:
	@mkdir -p $(OUTPUT_BASE)
	@mkdir -p $(M0P_OUTPUT_DIR)
	@mkdir -p $(DSP_OUTPUT_DIR)
	@mkdir -p $(INC_M0P_AUTOCREATE_DIR)
	@mkdir -p $(INC_DSP_AUTOCREATE_DIR)
	@mkdir -p $(INC_DSP_AUD_AUTOCREATE_DIR)

##############################################################
########## Basically, should not need to edit below ##########

.PHONY : all m0p gps dsp0 dsp1 clean apu

include $(SPRITZER_TOP_REL)/Common.mk
include $(MK_PATH)/Build.mk


apu:
	cd $(APU_BUILD_DIR) ; $(MAKE) BUILD_MODULE_NAME=$(PROJECT_NAME)

include $(RULES)

CDEFS += $(DEFS)

CDEFS += -D_SPRITZER_OS
INCLUDES += -I../include
INCLUDES += $(INC_COMMON_LIB)
INCLUDES += -I$(SPRITZER_TOP_REL)/audio/manager
INCLUDES += -I$(SPRITZER_TOP_REL)/audio/components/encoder
INCLUDES += -I$(SPRITZER_TOP_REL)/audio/objects/media_recorder
INCLUDES += -I$(SPRITZER_TOP_REL)/audio/objects/input_data_mng
INCLUDES += -I$(SPRITZER_TOP_REL)/audio/objects/player
INCLUDES += -I$(SPRITZER_TOP_REL)/audio/drivers/apu
INCLUDES += -I$(SPRITZER_TOP_REL)/audio/drivers/baseband/include
INCLUDES += -I$(SPRITZER_TOP_REL)/audio/common/include
INCLUDES += -I$(SPRITZER_TOP_REL)/include
INCLUDES += -I$(INC_SDK_AUTOCREATE_DIR)
INCLUDES += -I$(INC_DSP_AUD_AUTOCREATE_DIR)
INCLUDES += -I$(INC_APUS_DIR)
