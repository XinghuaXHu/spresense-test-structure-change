############################################################################
# audio_recognizer/Makefile
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

SELECT = $(CONFIG_SQAT_AUDIO_RECOG2_SELECT)

# Audio application info

CONFIG_SQAT_AUDIO_RECOG2_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_SQAT_AUDIO_RECOG2_STACKSIZE ?= 2048

APPNAME   = audio_recognizer
PRIORITY  = $(CONFIG_SQAT_AUDIO_RECOG2_PRIORITY)
STACKSIZE = $(CONFIG_SQAT_AUDIO_RECOG2_STACKSIZE)

# Audio Example

MAINSRC = audio_recognizer_main.cxx

# Audio Example paths

AUDIODIR = $(SDKDIR)$(DELIM)modules$(DELIM)audio

# Audio Example flags

ifeq ($(WINTOOL),y)
CXXFLAGS += -I "${shell cygpath -w $(AUDIODIR)}"
CXXFLAGS += -I "${shell cygpath -w include}"

ifeq ($(SELECT),y)
 CXXFLAGS += -I "${shell cygpath -w worker_recognizer2/userproc/include}"
else
 CXXFLAGS += -I "${shell cygpath -w worker_recognizer/userproc/include}"
endif
ifeq ($(CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC),y)
CXXFLAGS1 += -I "${shell cygpath -w worker_preprocess/userproc/include}"
endif
else

ifeq ($(CONFIG_RECOG2_DSP1),y)
CXXFLAGS += -I worker_recognizer/userproc/include
endif
ifeq ($(CONFIG_RECOG2_DSP2),y)
CXXFLAGS += -I worker_recognizer2/userproc/include
endif
ifeq ($(CONFIG_RECOG2_DSP3),y)
CXXFLAGS += -I worker_recognizer_load/userproc/include
endif

CXXFLAGS += -I include 
CXXFLAGS += -I $(AUDIODIR)

#ifeq ($(SELECT),y)
#CXXFLAGS += -I worker_recognizer/userproc/include
#else
#CXXFLAGS += -I worker_recognizer2/userproc/include
#endif

ifeq ($(CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC),y)
CXXFLAGS += -I worker_preprocess/userproc/include
endif
endif

CXXFLAGS += -D_POSIX
CXXFLAGS += -DUSE_MEMMGR_FENCE
CXXFLAGS += -DATTENTION_USE_FILENAME_LINE

CXXFLAGS_ALL1 = $(CXXFLAGS1) $(CXXFLAGS)

CONFIG_SQAT_AUDIO_RECOG2_PROGNAME ?= audio_recognizer$(EXEEXT)
PROGNAME = $(CONFIG_SQAT_AUDIO_RECOG2_PROGNAME)

# Instead of including "Applicaion.mk", write "make rules" here.
# (Cannot use "Application.mk" because targe "clean" will duplicate and be ignored.)

CXXEXT ?= .cxx

CXXOBJS = $(CXXSRCS:$(CXXEXT)=$(OBJEXT))
MAINOBJ = $(MAINSRC:$(CXXEXT)=$(OBJEXT))

SRCS = $(CXXSRCS) $(MAINSRC)
OBJS = $(CXXOBJS)

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

ROOTDEPPATH = --dep-path .

all: .built
#.PHONY: .built worker_build worker2_build clean
.PHONY: .built worker_build clean

$(CXXOBJS): %$(OBJEXT): %$(CXXEXT)
	$(call COMPILEXX, $<, $@)

$(MAINOBJ): %$(OBJEXT): %$(CXXEXT)
	$(call COMPILEXX, $<, $@)

worker_build:
ifeq ($(CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC),y)
	$(Q) $(MAKE) -C worker_preprocess TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)
endif

ifeq ($(CONFIG_RECOG2_DSP1),y)
	$(Q) $(MAKE) -C worker_recognizer TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)
endif
ifeq ($(CONFIG_RECOG2_DSP2),y)
	$(Q) $(MAKE) -C worker_recognizer2 TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)
endif
ifeq ($(CONFIG_RECOG2_DSP3),y)
	$(Q) $(MAKE) -C worker_recognizer_load TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)
endif

#ifeq ($(SELECT),y)
#	$(Q) $(MAKE) -C worker_recognizer2 TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)
#else
#	$(Q) $(MAKE) -C worker_recognizer TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)
#endif

.built: $(OBJS) worker_build

ifeq ($(CONFIG_BUILD_KERNEL),y)
$(BIN_DIR)$(DELIM)$(PROGNAME): $(OBJS) $(MAINOBJ)
#	@echo "LD: $(PROGNAME)"
	@echo "LD: sqa nuttx"
	$(Q) $(LD) $(LDELFFLAGS) $(LDLIBPATH) -o $(INSTALL_DIR)$(DELIM)$(PROGNAME) $(ARCHCRT0OBJ) $(MAINOBJ) $(LDLIBS)
	$(Q) $(NM) -u $(INSTALL_DIR)$(DELIM)$(PROGNAME)

install: $(BIN_DIR)$(DELIM)$(PROGNAME)
else
install: $(OBJS)
	$(call ARCHIVE, $(BIN), $(OBJS))
endif

ifeq ($(CONFIG_NSH_BUILTIN_APPS),y)
$(BUILTIN_REGISTRY)$(DELIM)$(APPNAME)_main.bdat: $(DEPCONFIG) Makefile
	$(call REGISTER,$(APPNAME),$(PRIORITY),$(STACKSIZE),$(APPNAME)_main)

context: $(BUILTIN_REGISTRY)$(DELIM)$(APPNAME)_main.bdat
else
context:
endif

.depend: Makefile $(SRCS)
	$(Q) $(MKDEP) $(ROOTDEPPATH) "$(CXX)" -- $(CXXFLAGS_ALL1) -- $(SRCS) >Make.dep
ifeq ($(CONFIG_SQAT_AUDIO_RECOG2_USEPREPROC),y)
	$(Q) $(MAKE) -C worker_preprocess TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) depend
endif
#ifeq ($(SELECT),y)
#	$(Q) $(MAKE) -C worker_recognizer2 TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) depend
#else
#	$(Q) $(MAKE) -C worker_recognizer TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) depend
#endif
ifeq ($(CONFIG_RECOG2_DSP1),y)
	$(Q) $(MAKE) -C worker_recognizer TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) depend
endif
ifeq ($(CONFIG_RECOG2_DSP2),y)
	$(Q) $(MAKE) -C worker_recognizer2 TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) depend
endif
ifeq ($(CONFIG_RECOG2_DSP3),y)
	$(Q) $(MAKE) -C worker_recognizer_load TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) depend
endif
	$(Q) touch $@
depend: .depend

clean_worker:
	@$(MAKE) -C worker_preprocess TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) clean
#ifeq ($(SELECT),y)
#	@$(MAKE) -C worker_recognizer2 TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) clean
#else
#	@$(MAKE) -C worker_recognizer TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) clean
#endif
ifeq ($(CONFIG_RECOG2_DSP1),y)
	@$(MAKE) -C worker_recognizer TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) clean
endif
ifeq ($(CONFIG_RECOG2_DSP2),y)
	@$(MAKE) -C worker_recognizer2 TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) clean
endif
ifeq ($(CONFIG_RECOG2_DSP3),y)
	@$(MAKE) -C worker_recognizer_load TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) clean
endif
	echo "##" $(CXXFLAGS_ALL1)

clean: clean_worker
	$(call DELFILE, .worker_preprocess)
	$(call DELFILE, .worker_recognizer)
	$(call CLEAN)

distclean: clean
	$(call DELFILE, Make.dep)
	$(call DELFILE, .depend)

-include Make.dep
.PHONY: preconfig
preconfig:

