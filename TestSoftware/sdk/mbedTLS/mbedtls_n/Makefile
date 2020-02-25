############################################################################
# test/mbedtls/Makefile
#
#   Copyright 2019 Sony Corporation
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

-include $(SDKDIR)/.config

# lte built-in application info

CONFIG_TEST_MBEDTLS_N_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_SQA_TEST_MBEDTLS_STACKSIZE ?= 5120

APPNAME = mbedtls_n
PRIORITY = $(CONFIG_TEST_MBEDTLS_N_PRIORITY)
STACKSIZE = $(CONFIG_TEST_MBEDTLS_N_STACKSIZE)

# mbedtls test

ASRCS =
CSRCS += lte_sub_n.c ssl_sub_n.c
MAINSRC = mbedtls_n_main.c

CONFIG_TEST_MBEDTLS_N_PROGNAME ?= mbedtls_n$(EXEEXT)
PROGNAME = $(CONFIG_TEST_MBEDTLS_N_PROGNAME)

include $(APPDIR)/Application.mk
