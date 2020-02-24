include	$(AUDIO_CONFIG)

##### Build configuration file (TBD) #####
# Layout name
LAYOUT_NAME := syseva

# Base os type ('uitron' or 'freertos')
export CONFIG_OS_TYPE := freertos

# Flash Boot
#CDEFS += -DENABLE_FLASH_BOOT

# Compiler type (RVDS or GCC)
export CONFIG_COMPILER := RVDS

# C Library type (microlib or standardlib / newlib or newlib-nano)
export C_LIB_TYPE := standardlib

export CHIP_NAME := SPRITZER_ES
export BOARD_NAME = SYSEVA
export CONFIG_XOSC = 26M
export BOARD_SUB_NAME ?= SYS5

# Configuration type
CDEFS += -DCONFIG_OS_PM_SUPPORT
CDEFS += -DCONFIG_PM_CPUFREQ_ENABLE

##### Debug Level Setting #####
CDEFS += -DDBG_LEVEL_STATIC_AS=DBG_LEVEL_DEBUG
CDEFS += -DDBG_LEVEL_DYNAMIC_AS=DBG_LEVEL_DEBUG
##### Debug Log Buffer Size Setting #####
CDEFS += -DDBG_LOG_BUF_SIZE=2048

CDEFS += -DSUPPORT_MP3_VOICE_RECORDER

# Configuration type
export CONFIG_DBG_TRACE = y
export CONFIG_RUNTIME_STATS = y

