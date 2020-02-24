############################################################################
# worker/asmptest/Makefile
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

include ../Make.def2

-include $(TOPDIR)/Make.defs
-include $(SDKDIR)/Make.defs

ifeq ($(WINTOOL),y)
LIB_DIR = "${shell cygpath -w ../lib}"
else
LIB_DIR = "../lib"
endif

LDLIBPATH +=  -L $(LIB_DIR)

LDLIBS += -lasmpw

BIN = asmptest

CSRCS = $(BIN).c

CELFFLAGS += -Os
ifeq ($(WINTOOL),y)
CELFFLAGS += -I"$(shell cygpath -w $(APPDIR))"
CELFFLAGS += -I"$(shell cygpath -w $(SDKDIR)$(DELIM)asmp$(DELIM)worker)"
else
CELFFLAGS += -I$(SDKDIR)/modules/asmp/worker
CELFFLAGS += -I~/GitHubLocal/spresense-v1.2.0/sdk/modules/asmp/worker
endif

ifneq ($(CONFIG_EXAMPLES_ASMP_WORKER_STACKSIZE),)
WORKERSTACKSIZE = $(CONFIG_EXAMPLES_ASMP_WORKER_STACKSIZE)
endif

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

all: $(BIN)
	
$(COBJS): %$(OBJEXT): %.c
	@echo "CC: $<"
	$(Q) $(CC) -c $(CELFFLAGS) $< -o $@

$(AOBJS): %$(OBJEXT): %.S
	@echo "AS: $<"
	$(Q) $(CC) -c $(AFLAGS) $< -o $@

$(BIN): $(COBJS) $(AOBJS)
	@echo "LD: $<"
	$(Q) $(LD) $(LDRAWELFFLAGS) -Map=$(BIN).map $(LDLIBPATH) -o $@ $(ARCHCRT0OBJ) $^ $(LDLIBS)

clean:
	$(call DELFILE, $(BIN))
	$(call CLEAN)

install:
	$(Q) mkdir -p $(ROMFS_DIR)
ifeq ($(WINTOOL),y)
	$(Q) $(STRIP) -d -o "$(shell cygpath -w $(ROMFS_DIR)/$(BIN))" $(BIN)
else
	$(Q) $(STRIP) -d -o $(ROMFS_DIR)/$(BIN) $(BIN)
endif
