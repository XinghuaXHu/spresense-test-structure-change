
-include $(TOPDIR)/Make.defs
-include $(SDKDIR)/Make.defs

CONFIG_TEST_ACA_UTIL_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_TEST_ACA_UTIL_STACKSIZE ?= 2048

APPNAME = aca
PRIORITY = $(CONFIG_TEST_ACA_UTIL_PRIORITY)
STACKSIZE = $(CONFIG_TEST_ACA_UTIL_STACKSIZE)

ASRCS =
CSRCS =
MAINSRC = aca_util_main.c

CONFIG_TEST_ACA_UTIL_PROGNAME ?= aca_util$(EXEEXT)
PROGNAME = $(CONFIG_TEST_ACA_UTIL_PROGNAME)

include $(APPDIR)/Application.mk
