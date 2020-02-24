##### Build configuration file (TBD) #####
# Layout name
LAYOUT_NAME := syseva

# Base os type ('uitron' or 'freertos')
export CONFIG_OS_TYPE := freertos

# Compiler type (RVDS or GCC)
export CONFIG_COMPILER := RVDS

# C Library type (microlib or standardlib / newlib or newlib-nano)
export C_LIB_TYPE := standardlib

export CCOPTS_RVDS += --preinclude=$$(APP_PATH)/include/test_define.h
export CXXOPTS_RVDS += --preinclude=$$(APP_PATH)/include/test_define.h

# Configuration type
CDEFS += -DCONFIG_OS_PM_SUPPORT

INCLUDES += -Iinclude

# Add this define to enable hi-res function
#CDEFS += -DSUPPORT_HIGHRES_AUDIO


##### Debug Level Setting #####
CDEFS += -DDBG_LEVEL_STATIC_SAMPLE=DBG_LEVEL_DEBUG
CDEFS += -DDBG_LEVEL_DYNAMIC_SAMPLE=DBG_LEVEL_DEBUG

export CONFIG_DBG_TRACE = y
export CONFIG_RUNTIME_STATS = y
export CONFIG_POWER_STATUS = y
export CONFIG_MERLOT_HEAP = y
