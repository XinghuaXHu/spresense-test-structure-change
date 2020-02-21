############################################################################
# test/lte_socket/api/Makefile
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

-include $(TOPDIR)/Make.defs
-include $(SDKDIR)/Make.defs

-include $(SDKDIR)/.config

ifeq ($(WINTOOL),y)
LTEINC = "${shell cygpath -w $(SDKDIR)$(DELIM)..$(DELIM)lte$(DELIM)library$(DELIM)include}"
else
LTEINC = "$(SDKDIR)$(DELIM)..$(DELIM)lte$(DELIM)library$(DELIM)include"
endif

CFLAGS += -I$(LTEINC)

# lte built-in application info

CONFIG_LTE_SOCKTEST_API_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_LTE_SOCKTEST_API_STACKSIZE ?= 2048

APPNAME = apitest
PRIORITY = $(CONFIG_LTE_SOCKTEST_API_PRIORITY)
STACKSIZE = $(CONFIG_LTE_SOCKTEST_API_STACKSIZE)

# socktest Example

ASRCS =
MAINSRC = apitest_main.c
MAINSRC += fdset_test.c
MAINSRC += getsockopt_test.c
MAINSRC += socket_test.c
MAINSRC += bind_test.c
MAINSRC += accept_test.c
MAINSRC += listen_test.c
MAINSRC += close_test.c
MAINSRC += setsockopt_test.c
MAINSRC += getsockname_test.c
MAINSRC += fcntl_test.c
MAINSRC += shutdown_test.c
MAINSRC += connect_test.c
MAINSRC += recvfrom_test.c
MAINSRC += sendto_test.c
MAINSRC += select_test.c
MAINSRC += send_test.c
MAINSRC += recv_test.c
MAINSRC += read_test.c
MAINSRC += write_test.c
CONFIG_LTE_SOCKTEST_API_PROGNAME ?= apitest$(EXEEXT)
PROGNAME = $(CONFIG_LTE_SOCKTEST_API_PROGNAME)

include $(APPDIR)/Application.mk
