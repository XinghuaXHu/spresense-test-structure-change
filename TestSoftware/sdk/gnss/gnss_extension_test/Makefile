############################################################################
# test/sqa/singlefunction/gnss_extension_test/Makefile
############################################################################

-include $(TOPDIR)/Make.defs
-include $(SDKDIR)/Make.defs

# GNSS(simple) built-in application info

CONFIG_EXAMPLES_GNSS_EXTENSION_TEST_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_EXAMPLES_GNSS_EXTENSION_TEST_STACKSIZE ?= 81920

ASRCS =
CSRCS =
CXXSRCS = libgetopt.cxx gnss_api.cxx
MAINSRC = gnss_extension_main.cxx

CONFIG_EXAMPLES_GNSS_EXTENSION_TEST_PROGNAME ?= gnss_extension$(EXEEXT)
PROGNAME = $(CONFIG_EXAMPLES_GNSS_EXTENSION_TEST_PROGNAME)

# gnss_extension_test built-in application info

APPNAME = $(CONFIG_EXAMPLES_GNSS_EXTENSION_TEST_PROGNAME)
PRIORITY = $(CONFIG_EXAMPLES_GNSS_EXTENSION_TEST_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_GNSS_EXTENSION_TEST_STACKSIZE)



include $(APPDIR)/Application.mk

CXXFLAGS += -I$(SDKDIR)/system/include
