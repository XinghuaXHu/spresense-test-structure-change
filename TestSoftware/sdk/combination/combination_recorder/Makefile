############################################################################
# apps/examples/elf/Makefile
#
#   Copyright (C) 2012 Gregory Nutt. All rights reserved.
#   Author: Gregory Nutt <gnutt@nuttx.org>
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
# 3. Neither the name NuttX nor the names of its contributors may be
#    used to endorse or promote products derived from this software
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
-include $(SDKDIR)/Make.defs
include $(APPDIR)/Make.defs

# ELF Example

CONFIG_COMBINATION_RECORDER_APP_NAME ?= combination_rec
CONFIG_COMBINATION_RECORDER_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_COMBINATION_RECORDER_STACKSIZE ?= 4096

APPNAME   = $(CONFIG_COMBINATION_RECORDER_APP_NAME)
PRIORITY  = $(CONFIG_COMBINATION_RECORDER_PRIORITY)
STACKSIZE = $(CONFIG_COMBINATION_RECORDER_STACKSIZE)

AUDIODIR = $(SDKDIR)$(DELIM)modules$(DELIM)audio
LTEDIR = $(SDKDIR)$(DELIM)system$(DELIM)include
AESMDIR = $(SDKDIR)$(DELIM)modules$(DELIM)include
JPEGDIR = $(SDKDIR)$(DELIM)../externals$(DELIM)libjpeg
SENSINGDIR = $(SDKDIR)$(DELIM)modules$(DELIM)sensing

ifeq ($(CONFIG_COMBI_SET_REC_W_STEP), y)
CFLAGS   += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "./include_rec_W_step"}
else ifeq ($(CONFIG_COMBI_SET_REC_ONLY), y)
CFLAGS   += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "./include_rec_fix"}
else
CFLAGS   += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "./include"}
endif
CFLAGS   += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "$(LTEDIR)"}
CFLAGS   += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "$(AUDIODIR)"}

ifeq ($(CONFIG_COMBI_SET_REC_W_STEP), y)
CXXFLAGS   += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "./include_rec_W_step"}
else ifeq ($(CONFIG_COMBI_SET_REC_ONLY), y)
CXXFLAGS   += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "./include_rec_fix"}
else
CXXFLAGS += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "./include"}
endif
CXXFLAGS += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "$(AUDIODIR)"}
CXXFLAGS += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "$(AESMDIR)"}
CXXFLAGS += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "$(AUDIODIR)"}
CXXFLAGS += -I $(SENSINGDIR)

CFLAGS += -D_POSIX

CXXFLAGS += -D_POSIX
CXXFLAGS += -DUSE_MEMMGR_FENCE
CXXFLAGS += -DATTENTION_USE_FILENAME_LINE

# camera Example

ASRCS =
CSRCS = led.c gnss.c camera.c asmp.c spi_sensor.c uart2.c
# CSRCS += lte_http_get_main.c
CSRCS += video_capture_thread.c
CSRCS += camera_bkgd.c
CSRCS += test_cameraAPI.c

CSRCS += jpeg_decode_main.c
CSRCS += jpeg_decode_nx_bkgd.c
CSRCS += system_main.c
CSRCS += fat_main.c

ifeq ($(CONFIG_COMBI_SET_REC_ONLY), y)
CXXSRCS = audio_recorder.cxx
endif

ifeq ($(CONFIG_COMBI_SET_STEPCNT_ONLY), y)
CXXSRCS = step_counter_main.cxx
CXXSRCS += accel_sensor.cxx gnss_sensor.cxx
endif

ifeq ($(CONFIG_COMBI_SET_REC_W_STEP), y)
CXXSRCS = audio_recorder.cxx
CXXSRCS += step_counter_main.cxx
CXXSRCS += accel_sensor.cxx gnss_sensor.cxx
CXXSRCS += init.cxx
endif

MAINSRC = combination_recorder_main.c

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
CXXOBJS = $(CXXSRCS:$(CXXEXT)=$(OBJEXT))
MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(CXXSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS) $(CXXOBJS)

ifneq ($(CONFIG_BUILD_KERNEL),y)
  OBJS += $(MAINOBJ)
endif

ifeq ($(WINTOOL),y)
  BIN = "${shell cygpath -w $(APPDIR)$(DELIM)libtest$(LIBEXT)}"
  INSTALL_DIR = "${shell cygpath -w $(BIN_DIR)}"
else
  BIN = $(APPDIR)$(DELIM)libtest$(LIBEXT)
  INSTALL_DIR = $(BIN_DIR)
endif

CONFIG_COMBINATION_RECORDER_PROGNAME ?= combination_rec$(EXEEXT)
PROGNAME = $(CONFIG_COMBINATION_RECORDER_PROGNAME)

ROOTDEPPATH = --dep-path . --dep-path worker
VPATH = worker

# Build targets

all: .built
.PHONY: .built really_build clean_worker clean depend distclean

$(AOBJS): %$(OBJEXT): %.S
	$(call ASSEMBLE, $<, $@)

$(COBJS) $(MAINOBJ): %$(OBJEXT): %.c
	$(call COMPILE, $<, $@)

$(CXXOBJS): %$(OBJEXT): %$(CXXEXT)
	$(call COMPILEXX, $<, $@)

really_build: $(OBJS)
	$(call ARCHIVE, $(BIN), $(OBJS))
	@touch .built

.built:
	@$(MAKE) -C worker TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) lib
	@$(MAKE) -C worker TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)
	@$(MAKE) TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" really_build

ifeq ($(CONFIG_BUILD_KERNEL),y)
$(BIN_DIR)$(DELIM)$(PROGNAME): $(OBJS) $(MAINOBJ)
	@echo "LD: $(PROGNAME)"
	$(Q) $(LD) $(LDELFFLAGS) $(LDLIBPATH) -o $(INSTALL_DIR)$(DELIM)$(PROGNAME) $(ARCHCRT0OBJ) $(MAINOBJ) $(LDLIBS)
	$(Q) $(NM) -u  $(INSTALL_DIR)$(DELIM)$(PROGNAME)

install: $(BIN_DIR)$(DELIM)$(PROGNAME)

else
install:

endif

ifeq ($(CONFIG_NSH_BUILTIN_APPS),y)
$(BUILTIN_REGISTRY)$(DELIM)$(APPNAME)_main.bdat: $(DEPCONFIG) Makefile
	$(call REGISTER,$(APPNAME),$(PRIORITY),$(STACKSIZE),$(APPNAME)_main)

context: $(BUILTIN_REGISTRY)$(DELIM)$(APPNAME)_main.bdat
else
context:
endif

# Generate dummy romfs.h for avoid error on depend process

worker/romfs.h:
	@echo > $@

.depend: worker/romfs.h
	@$(MKDEP) $(ROOTDEPPATH) "$(CC)" -- $(CFLAGS) -- $(SRCS) >Make.dep
	@$(MAKE) -C worker TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) depend
	@touch $@

depend: .depend

clean_worker:
	@$(MAKE) -C worker TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) clean

clean: clean_worker
	$(call DELFILE, .built)
	$(call CLEAN)

distclean: clean
	$(call DELFILE, Make.dep)
	$(call DELFILE, .depend)

-include Make.dep
.PHONY: preconfig
preconfig:
