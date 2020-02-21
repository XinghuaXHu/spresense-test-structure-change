############################################################################
# test/internal_test/audio/component_test/decoder/decoder_component_test/
# Makefile
#
#   Copyright (C) 2017 Sony Corporation
#   Author: Tomonobu Hayakawa<Tomonobu.Hayakawa@sony.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name NuttX nor Sony nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

-include $(TOPDIR)/.config
-include $(TOPDIR)/Make.defs
include $(APPDIR)/Make.defs

# Audio application info

CONFIG_TEST_DECODER_COMPONENT_APP_NAME ?= test_decoder_component
CONFIG_TEST_DECODER_COMPONENT_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_TEST_DECODER_COMPONENT_STACKSIZE ?= 4096

APPNAME = $(CONFIG_TEST_DECODER_COMPONENT_APP_NAME)
PRIORITY = $(CONFIG_TEST_DECODER_COMPONENT_PRIORITY)
STACKSIZE = $(CONFIG_TEST_DECODER_COMPONENT_STACKSIZE)

# Audio Example paths
AUDIO_TOPDIR  = $(SDKDIR)/modules/audio
AUDIO_INC_DIR = $(AUDIO_TOPDIR)/include
APPSDIR       = $(SDKDIR)/../apps

component_common.cxx:
	$(Q) cp $(AUDIO_TOPDIR)/components/common/component_common.cpp ./component_common.cxx
	$(Q) cp $(AUDIO_TOPDIR)/components/common/component_common.h ./component_common.h

decoder_component.cxx:
	$(Q) cp $(AUDIO_TOPDIR)/components/decoder/decoder_component.cpp ./decoder_component.cxx
	$(Q) cp $(AUDIO_TOPDIR)/components/decoder/decoder_component.h ./decoder_component.h

dsp_drv.cxx:
	$(Q) cp $(AUDIO_TOPDIR)/dsp_driver/src/dsp_drv.cpp ./dsp_drv.cxx
	$(Q) cp $(AUDIO_TOPDIR)/dsp_driver/include/dsp_drv.h ./dsp_drv.h

Mp3Parser.cxx:
	$(Q) cp $(AUDIO_TOPDIR)/stream_parser/mp3/Mp3Parser.cpp ./Mp3Parser.cxx

RamAdtsParser.cxx:
	$(Q) cp $(AUDIO_TOPDIR)/stream_parser/aaclc/RamAdtsParser.cpp ./RamAdtsParser.cxx

LatmAacLc.cxx:
	$(Q) cp $(AUDIO_TOPDIR)/stream_parser/aaclc/LatmAacLc.cpp ./LatmAacLc.cxx

playlist.cxx:
	$(Q) cp $(AUDIO_TOPDIR)/playlist/playlist.cpp ./playlist.cxx
	$(Q) cp $(AUDIO_TOPDIR)/playlist/playlist.h ./playlist.h

ASRCS =
CSRCS =
CXXSRCS = attention.cxx component_common.cxx decoder_component.cxx dsp_drv.cxx playlist.cxx Mp3Parser.cxx RamAdtsParser.cxx LatmAacLc.cxx
MAINSRC = test_decoder_component_main.cxx

ifeq ($(WINTOOL),y)
CXXFLAGS += -I"${shell cygpath -w $(APPSDIR)/include}"
CXXFLAGS += -I"${shell cygpath -w $(SDKDIR)/modules/include}"
CXXFLAGS += -I"${shell cygpath -w $(AUDIO_TOPDIR)}"
CXXFLAGS += -I"${shell cygpath -w $(AUDIO_INC_DIR)}"

CXXFLAGS += -I"${shell cygpath -w ./include}"
else
CXXFLAGS += -I$(APPSDIR)/include
CXXFLAGS += -I$(SDKDIR)/modules/include
CXXFLAGS += -I$(AUDIO_TOPDIR)
CXXFLAGS += -I$(AUDIO_INC_DIR)

CXXFLAGS += -I./include
endif
CXXFLAGS += -D_POSIX
CXXFLAGS += -DUSE_MEMMGR_FENCE
CXXFLAGS += -DATTENTION_USE_FILENAME_LINE
CXXFLAGS += -DCONFIG_AUDIOUTILS_PLAYER

CONFIG_TEST_DECODER_COMPONENT_PROGNAME ?= test_decoder_component$(EXEEXT)
PROGNAME = $(CONFIG_TEST_DECODER_COMPONENT_PROGNAME)

include $(APPDIR)/Application.mk

