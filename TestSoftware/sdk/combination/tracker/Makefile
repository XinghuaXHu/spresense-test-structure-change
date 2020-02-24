############################################################################
# demo/collet_box/tracker/Makefile
#
#   Copyright (C) 2017 Sony Corporation. All rights reserved.
#   Author: Yutaka Miyajima <Yutaka.Miyajima@sony.com>
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
-include $(SDKDIR)/Make.defs

# Sensor Example paths

/* TODO: Change the path after moving the common_ecode.h */
AUD_INC_DIR	= $(APPDIR)/audioutils/include

ifeq ($(WINTOOL),y)
TRACKERINC += "${shell cygpath -w ./include}"
else
TRACKERINC += "./include"
endif

CXXFLAGS += -I$(TRACKERINC)
CFLAGS += -I$(TRACKERINC)

# Tracker Example

ASRCS =
#CXXSRCS = tracker_tram.cxx tracker_tram_sensor_control.cxx
#CXXSRCS += tracker_tram_accel_sensor.cxx tracker_tram_magnetometer_sensor.cxx
#CXXSRCS += tracker_tram_pressure_sensor.cxx tracker_tram_temperature_sensor.cxx
MAINSRC = tracker_main.c tracker_location.c tracker_net_client.c tracker_lte.c

ifeq ($(WINTOOL),y)
CXXFLAGS += -I./include

/* TODO: Change the path after moving the common_ecode.h */
CXXFLAGS += -I"${shell cygpath -w $(AUD_INC_DIR)}"
CFLAGS += -I"${shell cygpath -w $(AUD_INC_DIR)}"
else
CXXFLAGS += -I./include
CFLAGS += -I$(SDKDIR)$(DELIM)system$(DELIM)include
CXXFLAGS += -I$(SDKDIR)$(DELIM)system$(DELIM)include
/* TODO: Change the path after moving the common_ecode.h */
CXXFLAGS += -I$(AUD_INC_DIR)
CFLAGS += -I$(AUD_INC_DIR)
endif

ifeq ($(CONFIG_DEMO_COLLET_TRACKER_USING_NXFB),y)
#MAINSRC += tracker_nxfb.c tracker_nxfb_bkgd.c
endif

CXXFLAGS += -D_POSIX
CXXFLAGS += -DUSE_MEMMGR_FENCE

CONFIG_DEMO_COLLET_TRACKER_PROGNAME ?= tracker$(EXEEXT)
PROGNAME = $(CONFIG_DEMO_COLLET_TRACKER_PROGNAME)

# tracker built-in application info

APPNAME = tracker
PRIORITY = $(CONFIG_DEMO_COLLET_TRACKER_PRIORITY)
STACKSIZE = $(CONFIG_DEMO_COLLET_TRACKER_STACKSIZE)

include $(APPDIR)/Application.mk
