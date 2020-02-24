############################################################################
# test/sqa/combination/step_audio_lte/Makefile
#
#   Copyright 2018 Sony Semiconductor Solutions Corporation
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
# 3. Neither the name of Sony Semiconductor Solutions Corporation nor
#    the names of its contributors may be used to endorse or promote
#    products derived from this software without specific prior written
#    permission.
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
-include $(SDKDIR)/Make.defs
include $(APPDIR)/Make.defs

# Sensing application info

CONFIG_TEST_STEP_COUNTER_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_TEST_STEP_COUNTER_STACKSIZE ?= 4096

APPNAME   = step_audio_lte
PRIORITY  = $(CONFIG_TEST_STEP_COUNTER_PRIORITY)
STACKSIZE = $(CONFIG_TEST_STEP_COUNTER_STACKSIZE)

# Audio Example paths

AUDIODIR = $(SDKDIR)$(DELIM)modules$(DELIM)audio

# Sensor Example

MAINSRC = step_audio_lte_main.cxx
CXXSRCS = accel_sensor.cxx gnss_sensor.cxx
CXXSRCS += audio_player.cxx
CSRCS   = lte.c net_client.c

# Sensing Example paths

SENSINGDIR = $(SDKDIR)$(DELIM)modules$(DELIM)sensing

# Sensing Example flags

ifeq ($(WINTOOL),y)
CXXFLAGS += -I "${shell cygpath -w $(SENSINGDIR)}"
CXXFLAGS += -I "${shell cygpath -w $(AUDIODIR)}"
else
CXXFLAGS += -I $(SENSINGDIR)
CXXFLAGS += -I $(AUDIODIR)
CFLAGS += -I$(SDKDIR)$(DELIM)system$(DELIM)include
CXXFLAGS += -I$(SDKDIR)$(DELIM)system$(DELIM)include
endif

CXXFLAGS += -D_POSIX
CXXFLAGS += -DUSE_MEMMGR_FENCE
CXXFLAGS += -DATTENTION_USE_FILENAME_LINE

CONFIG_TEST_STEP_COUNTER_PROGNAME ?= step_audio_lte$(EXEEXT)
PROGNAME = $(CONFIG_TEST_STEP_COUNTER_PROGNAME)

include $(APPDIR)/Application.mk
