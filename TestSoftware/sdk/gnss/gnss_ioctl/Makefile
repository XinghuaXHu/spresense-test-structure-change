############################################################################
# test/gnss_ioctl/Makefile
#
#   Copyright (C) 2016 Sony. All rights reserved.
#   Author: Sony Spritzer support team <dsbg-spritzer-sw-support@jp.sony.com>
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

-include $(TOPDIR)/Make.defs

# GNSS_IOCTL(simple) built-in application info

CONFIG_EXAMPLES_GNSS_IOCTL_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_EXAMPLES_GNSS_IOCTL_STACKSIZE ?= 2048

APPNAME = gnss_ioctl
PRIORITY = $(CONFIG_EXAMPLES_GNSS_IOCTL_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_GNSS_IOCTL_STACKSIZE)

# GNSS_IOCTL(simple) Example

ASRCS =
CSRCS = gnss_test_ope_mode.c gnss_test_select_satellite_system.c gnss_test_start.c gnss_test_set_receiver_position.c gnss_test_set_time.c gnss_test_backup_data.c gnss_test_set_cep_data.c gnss_test_set_tcxo_offset.c gnss_test_set_almanac.c gnss_test_set_ephemeris.c
MAINSRC = gnss_ioctl_main.c

CONFIG_EXAMPLES_GNSS_IOCTL_PROGNAME ?= gnss_ioctl$(EXEEXT)
PROGNAME = $(CONFIG_EXAMPLES_GNSS_IOCTL_PROGNAME)

include $(APPDIR)/Application.mk
