############################################################################
# asmp_extensions/Makefile
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

# ASMP Extensions Test

APPNAME = asmp_extensions
PRIORITY = SCHED_PRIORITY_DEFAULT
STACKSIZE = 40960

ASRCS =
CSRCS =
CXXSRCS = cmd_parser.cxx libgetopt.cxx test_api.cxx asmp_extensions_common.cxx
MAINSRC = asmp_extensions_main.cxx

APPSDIR = $(SDKDIR)/system/include

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
CXXOBJS += $(CXXSRCS:.cxx=$(OBJEXT))
MAINOBJ = $(MAINSRC:.cxx=$(OBJEXT))
SRCS = $(ASRCS) $(CSRCS) $(CXXSRCS) $(MAINSRC) 
OBJS = $(AOBJS) $(COBJS) $(CXXOBJS) 

ifneq ($(CONFIG_BUILD_KERNEL),y)
  OBJS += $(MAINOBJ)
endif

ifeq ($(WINTOOL),y)
  CXXFLAGS += -I "${shell cygpath -w $(APPSDIR)}"
  BIN = "${shell cygpath -w $(APPDIR)$(DELIM)libtest$(LIBEXT)}"
  INSTALL_DIR = "${shell cygpath -w $(BIN_DIR)}"
else
  CXXFLAGS += -I $(APPSDIR)
  BIN = $(APPDIR)$(DELIM)libtest$(LIBEXT)
  INSTALL_DIR = $(BIN_DIR)
endif
CXXFLAGS += -I $(SDKDIR)/system/include
CFLAGS += -I $(SDKDIR)/system/include

CONFIG_TEST_ASMP_EXTENSIONS_PROGNAME ?= asmp_extensions$(EXEEXT)
PROGNAME = $(CONFIG_TEST_ASMP_EXTENSIONS_PROGNAME)

ROOTDEPPATH = --dep-path . --dep-path worker
VPATH = worker

# Build targets

all: .built
.PHONY: .built really_build clean_worker clean depend distclean

$(AOBJS): %$(OBJEXT): %.S
	$(call ASSEMBLE, $<, $@)

$(COBJS): %$(OBJEXT): %.c
	$(call COMPILE, $<, $@)

$(CXXOBJS) $(MAINOBJ): %$(OBJEXT): %.cxx
	$(call COMPILEXX, $<, $@)

really_build: $(OBJS)
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
	$(call ARCHIVE, $(BIN), $(OBJS))
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
