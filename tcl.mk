
ifeq ($(findstring Win,$(OS)), Win)
    export TCL_SHELL = $(ROOT_DIR)/$(UTIL_PATH)/tclkitsh
else
    export TCL_SHELL = tclsh
endif

