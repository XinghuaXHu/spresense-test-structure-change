############################################################################
# sqa/singlefunction/audio_player/Makefile
#
#   Copyright (C) 2018 Sony Corporation
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
# 3. Neither the name Sony nor the names of its contributors
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

-include $(TOPDIR)/Make.defs

# Audio application info

CONFIG_SQA_TEST_AUDIO_PLAYER_APP_NAME ?= player_test
CONFIG_SQA_TEST_AUDIO_PLAYER_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_SQA_TEST_AUDIO_PLAYER_STACKSIZE ?= 4096

APPNAME   = $(CONFIG_SQA_TEST_AUDIO_PLAYER_APP_NAME)
PRIORITY  = $(CONFIG_SQA_TEST_AUDIO_PLAYER_PRIORITY)
STACKSIZE = $(CONFIG_SQA_TEST_AUDIO_PLAYER_STACKSIZE)

# Audio Example

MAINSRC = audio_player_test.cxx
CXXSRCS  = spr_sdk_audio_player.cxx spr_sdk_audio_player_common.cxx
CXXSRCS += spr_sdk_14928.cxx
CXXSRCS += spr_sdk_14929.cxx
CXXSRCS += spr_sdk_15299.cxx
CXXSRCS += spr_sdk_15454.cxx
CXXSRCS += spr_sdk_15455.cxx
CXXSRCS += spr_sdk_15317.cxx
CXXSRCS += spr_sdk_15676.cxx
CXXSRCS += spr_sdk_15451.cxx

# Audio SQA test paths

INCLUDEDIR = $(SDKDIR)$(DELIM)modules$(DELIM)include
AUDIODIR = $(SDKDIR)$(DELIM)modules$(DELIM)audio
APPSDIR = $(SDKDIR)/system/include

# Audio SQA test flags

ifeq ($(WINTOOL),y)
CXXFLAGS += -I "${shell cygpath -w $(APPSDIR)}"
CXXFLAGS += -I "${shell cygpath -w $(INCLUDEDIR)}"
CXXFLAGS += -I "${shell cygpath -w $(AUDIODIR)}"
else
CXXFLAGS += -I $(APPSDIR)
CXXFLAGS += -I $(INCLUDEDIR)
CXXFLAGS += -I $(AUDIODIR)
endif

CXXFLAGS += -D_POSIX
CXXFLAGS += -DUSE_MEMMGR_FENCE
CXXFLAGS += -DATTENTION_USE_FILENAME_LINE

CONFIG_SQA_TEST_AUDIO_PLAYER_PROGNAME ?= audio_player_test$(EXEEXT)
PROGNAME = $(CONFIG_SQA_TEST_AUDIO_PLAYER_PROGNAME)

include $(APPDIR)/Application.mk
