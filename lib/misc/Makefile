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
CFLAGS += -I$(ROOT_DIR)/lib/lpc17xx

#################################
#
# Objects to build
#
#################################
TARGET = libmisc.a

DEPFILE = depfile.mk

SRC_DIR = src

C_FILES += $(SRC_DIR)/util.c
C_FILES += $(SRC_DIR)/debug.c
C_FILES += $(SRC_DIR)/crc32.c
C_FILES += $(SRC_DIR)/crc16.c
C_FILES += $(SRC_DIR)/pearson.c

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
	$(CC) $(CFLAGS) -M $(C_FILES) > $(DEPFILE)
	$(TCL_SHELL) $(SCRIPTS_PATH)/depdir.tcl $(DEPFILE) $(C_OBJS)

$(TARGET): $(OBJS)
	$(AR) r $@ $^

.PHONY: clean
clean:
	$(RM) $(OBJS) $(TARGET)

