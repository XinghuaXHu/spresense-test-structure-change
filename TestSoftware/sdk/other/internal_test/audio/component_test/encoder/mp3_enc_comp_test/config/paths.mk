#====================================================================#
#
#       File Name: paths.mk
#
#       Description: Path information. Audio original.
#
#       Notes: (C) Copyright 2015 Sony Corporation
#
#       Author: Tetuya Sakai
#
#====================================================================#
include	$(AUDIO_PATHS)

#--------------------------------------------------------------------#
# Directory for auto create files
#--------------------------------------------------------------------#
OUTPUT_BASE		= $(SPRITZER_TOP_REL)/build/$(PROJECT_NAME)
INC_AUTOCREATE_DIR	= $(OUTPUT_BASE)/include
M0P_OUTPUT_DIR		= $(OUTPUT_BASE)/m0p
DSP_OUTPUT_DIR		= $(OUTPUT_BASE)/dsp
INC_M0P_AUTOCREATE_DIR	= $(OUTPUT_BASE)/m0p/include
INC_DSP_AUTOCREATE_DIR	= $(OUTPUT_BASE)/dsp/include
INC_DSP_AUD_AUTOCREATE_DIR	= $(OUTPUT_BASE)/dsp/audio

#--------------------------------------------------------------------#
# Conf/src files
#--------------------------------------------------------------------#
CONF_DIR		= ./config
MEM_CONF		:= $(CONF_DIR)/mem_layout.conf
DUMP_CONF		:= $(CONF_DIR)/dmp_layout.conf
SPL_CONF		:= $(CONF_DIR)/spl_layout.conf
MSGQ_CONF		:= $(CONF_DIR)/msgq_layout.conf

#--------------------------------------------------------------------#
# Auto create files
#--------------------------------------------------------------------#
MEMLAYOUT_H	:= $(INC_DSP_AUD_AUTOCREATE_DIR)/mem_layout.h
FIXEDFENCE_H	:= $(INC_DSP_AUD_AUTOCREATE_DIR)/fixed_fence.h
POOLLAYOUT_H	:= $(INC_DSP_AUD_AUTOCREATE_DIR)/pool_layout.h
DUMP_ID_H	:= $(INC_DSP_AUD_AUTOCREATE_DIR)/dmp_id.h	# ToDo: move to CPU folder
DUMP_POOL_C	:= $(INC_DSP_AUD_AUTOCREATE_DIR)/dmp_pool.c	# dummy, not used
CPU_ID_H	:= $(INC_DSP_AUD_AUTOCREATE_DIR)/cpu_id.h
SPL_ID_H	:= $(INC_DSP_AUD_AUTOCREATE_DIR)/spl_id.h
SPL_POOL_H	:= $(INC_DSP_AUD_AUTOCREATE_DIR)/spl_pool.h

MSGQ_ID_H	:= $(INC_DSP_AUD_AUTOCREATE_DIR)/msgq_id.h
MSGQ_POOL_H	:= $(INC_DSP_AUD_AUTOCREATE_DIR)/msgq_pool.h

#--------------------------------------------------------------------#
# Auto dir
#--------------------------------------------------------------------#
INC_APUS_DIR		=	$(SPRITZER_TOP_REL)/audio/apus/include
