########################
#
# Root Makefile
#
########################

include config.mk

########################
#
# Get board's configuration file. Configuration can
# contain custom build targets.
#
########################

.PHONY: all
all: default

export BOARD_DIR = board/$(BOARD)
BOARD_MK  = $(BOARD_DIR)/$(BOARD).mk
include $(BOARD_MK)

########################
#
# Utilities config
#
########################

export UTIL_PATH = util
ROOT_DIR = ./

include tcl.mk

########################
#
# Generic targets
#
########################

.PHONY: default clean depend

default: $(BOARD_MK)
	$(TCL_SHELL) $(UTIL_PATH)/mforeach.tcl $(if $(findstring 1, $(BUILD_DOC)), doc) $(SUBDIRS) all

clean: $(BOARD_MK)
	$(TCL_SHELL) $(UTIL_PATH)/mforeach.tcl $(SUBDIRS) $(if $(findstring 1, $(BUILD_DOC)), doc) clean

depend: $(BOARD_MK)
	$(TCL_SHELL) $(UTIL_PATH)/mforeach.tcl $(SUBDIRS) depend

