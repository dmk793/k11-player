################################################################################
#
# Generic library for LPC177x and LPC178x based devices
#
################################################################################
ROOT_DIR = ../..

#################################
#
# Tools config
#
#################################
SCRIPTS_PATH      = $(ROOT_DIR)/$(UTIL_PATH)

include $(ROOT_DIR)/tcl.mk

#################################
#
# Compiller flags
#
#################################
CFLAGS += -O2


#################################
#
# Objects to build
#
#################################
TARGET = libos.a

DEPFILE = depfile.mk

SRC_DIR = src

C_FILES += $(SRC_DIR)/os.c
C_FILES += $(SRC_DIR)/os_sched.c
C_FILES += $(SRC_DIR)/os_bitobj.c
C_FILES += $(SRC_DIR)/os_mem.c
C_FILES += $(SRC_DIR)/os_mem1.c
C_FILES += $(SRC_DIR)/os_mem2.c
C_FILES += $(SRC_DIR)/os_mem3.c
C_FILES += $(SRC_DIR)/os_queue.c
C_FILES += $(SRC_DIR)/os_multi.c
ifeq ($(PORT), ARMV7M)
    C_FILES  += $(SRC_DIR)/port/ARMv7-M/port.c

    CFLAGS += -I$(ROOT_DIR)/lib/lpc17xx
endif
CFLAGS += -O2

# XXX remove
CFLAGS += -I$(ROOT_DIR)/lib/mlpc17xx/src
CFLAGS += -I$(ROOT_DIR)/lib/misc/src

C_OBJS  = $(foreach obj,$(C_FILES),$(patsubst %c,%o, $(obj)))
AS_OBJS = $(foreach obj,$(AS_FILES),$(patsubst %s,%o, $(obj)))
OBJS += $(AS_OBJS)
OBJS += $(C_OBJS)

#################################
#
# Build rules
#
#################################

.PHONY: all
all: $(TARGET)

-include $(DEPFILE)

depend: $(C_FILES)
	$(CC) $(CFLAGS) -MM $(C_FILES) > $(DEPFILE)
	echo $(C_OBJS)
	$(TCL_SHELL) $(SCRIPTS_PATH)/depdir.tcl $(DEPFILE) $(C_OBJS)

$(TARGET): $(OBJS)
	$(AR) r $@ $^

.PHONY: clean
clean:
	$(RM) $(OBJS) $(TARGET)

