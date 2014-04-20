###############################################################################
# 
# This file is included from root Makefile.
# Build rules for SK-MLPC1788 dev. board (http://starterkit.ru).
#
###############################################################################
#
# Compiller    GCC
# MCU          LPC1788
#
# GCC from CodeSourcery was used (http://www.codesourcery.com/sgpp/lite/arm).
# CodeSourcery is now Mentor Graphics Sourcery Tools
# (http://www.mentor.com/embedded-software/sourcery-tools/sourcery-codebench/
# lite-edition).
#
###############################################################################

#########################
#
# Environment variables used by project itself and libraries.
#
#########################
export PORT = ARMV7M
export MCU  = LPC1788

#########################
#
# Toolchain config
#
#########################
TOOLCHAIN = arm-none-eabi

export CCACHE_DIR = /dev/shm/ccache
export CC = ccache $(TOOLCHAIN)-gcc
export LD = $(TOOLCHAIN)-gcc
export AR = $(TOOLCHAIN)-ar
export AS = $(TOOLCHAIN)-as
export CP = $(TOOLCHAIN)-objcopy
export OD = $(TOOLCHAIN)-objdump
export SZ = $(TOOLCHAIN)-size
export RM = rm -f

# XXX
export PATH+=:$(ARM_TOOLCHAIN_PATH)

CFLAGS += -fno-common
CFLAGS += -mthumb 
CFLAGS += -mcpu=cortex-m3
CFLAGS += -g
CFLAGS += -Wall
CFLAGS += -DPORT_$(PORT)
CFLAGS += -DMCU_$(MCU)
CFLAGS += -DOS_CONFIGURATION_HEADER=\"../../../$(BOARD_DIR)/src/os_config.h\"

LDFLAGS += -nostartfiles
LDFLAGS += -mcpu=cortex-m3
LDFLAGS += -mthumb

export CFLAGS
export LDFLAGS

#########################
#
# Directories required to build project. SUBDIRS relative to root directory.
# NOTE sub-directory with BOARD_DIR should be last (for proper dependencies
# build).
#
#########################

# directories used by bootloader and main program
SUBDIRS += lib/misc
SUBDIRS += lib/mlpc17xx
# directories used by bootloader
SUBDIRS += board/lpc17xx_boot
# directories used by main program
SUBDIRS += lib/os
SUBDIRS += $(BOARD_DIR)

#########################
#
# Custom build targets
#
#########################

.PHONY: prog list image debug
prog: all
	$(TCL_SHELL) $(UTIL_PATH)/mforeach.tcl $(BOARD_DIR) prog

list: all
	$(TCL_SHELL) $(UTIL_PATH)/mforeach.tcl $(BOARD_DIR) $(BOARD_DIR)/../lpc17xx_boot list

image: all
	$(TCL_SHELL) $(UTIL_PATH)/mforeach.tcl $(BOARD_DIR) image

debug: all
	$(TCL_SHELL) $(UTIL_PATH)/mforeach.tcl $(BOARD_DIR) debug

