############################################################################
# test/voice_call/Makefile
#
#   Copyright (C) 2017 Sony Corporation. All rights reserved.
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

-include $(TOPDIR)/Make.defs
include $(APPDIR)/Make.defs

# Audio application info

CONFIG_TEST_VOICE_CALL_APP_NAME ?= voice_call
CONFIG_TEST_VOICE_CALL_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_TEST_VOICE_CALL_STACKSIZE ?= 2048

APPNAME = $(CONFIG_TEST_VOICE_CALL_APP_NAME)
PRIORITY = $(CONFIG_TEST_VOICE_CALL_PRIORITY)
STACKSIZE = $(CONFIG_TEST_VOICE_CALL_STACKSIZE)

# Audio Example paths

AUDIO_INC_DIR = $(APPDIR)/audio/include
APPSDIR       = $(SDKDIR)/system

# Audio Example

ASRCS =
CXXSRCS =
MAINSRC = voice_call_main.cxx

ifeq ($(WINTOOL),y)
CXXFLAGS += -I"${shell cygpath -w ./include}"
CXXFLAGS += -I"${shell cygpath -w $(AUDIO_INC_DIR)}"
CXXFLAGS += -I"${shell cygpath -w $(APPSDIR)/include}"
CXXFLAGS += -I"${shell cygpath -w $(SDKDIR)/modules/include}"
else
CXXFLAGS += -I./include
CXXFLAGS += -I$(AUDIO_INC_DIR)
CXXFLAGS += -I$(APPSDIR)/include
CXXFLAGS += -I$(SDKDIR)/modules/include
endif #($(WINTOOL),y)

CXXFLAGS += -D_POSIX
CXXFLAGS += -DUSE_MEMMGR_FENCE
CXXFLAGS += -DATTENTION_USE_FILENAME_LINE

CONFIG_TEST_VOICE_CALL_PROGNAME ?= voice_call$(EXEEXT)
PROGNAME = $(CONFIG_TEST_VOICE_CALL_PROGNAME)

include $(APPDIR)/Application.mk
