############################################################################
# test/sqa/singlefunction/multicpu/worker
#
#   Copyright (C) 2012, 2014 Gregory Nutt. All rights reserved.
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
include $(APPDIR)/Make.defs

ALL_SUBDIRS = hello
BUILD_SUBDIRS = hello

ASMP_DIR = $(TOPDIR)/../test/sqa/singlefunction/multicpu
WORKER_DIR = $(ASMP_DIR)/worker
ROMFS_DIR = $(WORKER_DIR)/romfs
ROMFS_IMG = $(WORKER_DIR)/romfs.img
ROMFS_HDR = $(WORKER_DIR)/romfs.h
DIRLIST_HDR = $(WORKER_DIR)/dirlist.h
SYMTAB_SRC = $(WORKER_DIR)/symtab.c

define DIR_template
$(1)_$(2):
	$(Q) $(MAKE) -C $(1) $(3) TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" ROMFS_DIR="$(ROMFS_DIR)" CROSSDEV=$(CROSSDEV)
endef

all: $(ROMFS_HDR) $(DIRLIST_HDR) $(SYMTAB_SRC)
.PHONY: all build clean install populate depend lib

$(foreach DIR, $(BUILD_SUBDIRS), $(eval $(call DIR_template,$(DIR),build, all)))
$(foreach DIR, $(ALL_SUBDIRS), $(eval $(call DIR_template,$(DIR),clean,clean)))
$(foreach DIR, $(BUILD_SUBDIRS), $(eval $(call DIR_template,$(DIR),install,install)))

# Build program(s) in each sud-directory

build: $(foreach DIR, $(BUILD_SUBDIRS), $(DIR)_build)

# Install each program in the romfs directory

install: $(foreach DIR, $(BUILD_SUBDIRS), $(DIR)_install)

# Create the romfs directory

$(ROMFS_DIR):
	$(Q) mkdir $(ROMFS_DIR)

# Populate the romfs directory

populate:  $(ROMFS_DIR) build install

# Create the romfs.img file from the populated romfs directory

$(ROMFS_IMG): populate
	$(Q) genromfs -f $@ -d $(ROMFS_DIR) -V "WORKER"

# Create the romfs.h header file from the romfs.img file

$(ROMFS_HDR) : $(ROMFS_IMG)
	$(Q) (cd $(WORKER_DIR); xxd -i romfs.img | sed -e "s/^unsigned/static const unsigned/g" >$@)

# Create the dirlist.h header file from the romfs directory

$(DIRLIST_HDR) : populate
	$(Q) $(WORKER_DIR)/mkdirlist.sh $(ROMFS_DIR) >$@

# Create the exported symbol table

$(SYMTAB_SRC): build
	$(Q) $(WORKER_DIR)/mksymtab.sh $(ROMFS_DIR) >$@

# Build ASMP worker library

lib/libasmpw.a:
	$(Q) $(MAKE) -C lib TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)

lib: lib/libasmpw.a

.depend:
	$(Q) $(MAKE) -C lib TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) depend
	@touch $@

depend: .depend

# Clean each subdirectory

lib_clean:
	$(Q) $(MAKE) -C lib TOPDIR="$(TOPDIR)" clean

clean: $(foreach DIR, $(ALL_SUBDIRS), $(DIR)_clean) lib_clean
	$(Q) rm -f $(ROMFS_HDR) $(DIRLIST_HDR) $(ROMFS_IMG) $(SYMTAB_SRC)
	$(Q) rm -rf $(ROMFS_DIR)
