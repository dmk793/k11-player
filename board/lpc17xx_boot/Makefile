################################################################################
#
# Generic bootloader for LPC177x and LPC178x based boards.
#
################################################################################
ROOT_DIR    = ../..

TARGET = bloader
#################################
#
# Tools config
#
#################################

SCRIPTS_PATH       = $(ROOT_DIR)/$(UTIL_PATH)
OPENOCD_DIR        = $(ROOT_DIR)/$(UTIL_PATH)/openocd
MLPC17XX_LIB_PATH  = $(ROOT_DIR)/lib/mlpc17xx
MISC_LIB_PATH      = $(ROOT_DIR)/lib/misc

include $(ROOT_DIR)/tcl.mk

#################################
#
# Compiller flags
#
#################################
include bloader.mk

CFLAGS += -O2
CFLAGS += -I$(ROOT_DIR)/lib/lpc17xx
CFLAGS += -I$(MLPC17XX_LIB_PATH)/src
CFLAGS += -I$(MISC_LIB_PATH)/src
CFLAGS += -DIMAGE_OFFSET=$(MAIN_IMAGE_OFFSET)
CFLAGS += -DLOAD_OFFSET=$(MAIN_LOAD_OFFSET)
CFLAGS += -DSIZE_OFFSET=$(MAIN_SIZE_OFFSET)
CFLAGS += -DCRC_OFFSET=$(MAIN_CRC_OFFSET)

LDFLAGS += -L$(MLPC17XX_LIB_PATH)
LDFLAGS += -L$(MISC_LIB_PATH)
LDFLAGS += -Wl,-Map=$(TARGET).map 

#################################
#
# Objects to build
#
#################################

TARGET_BIN = $(TARGET).bin
TARGET_OUT = $(TARGET).out
TARGET_HEX = $(TARGET).hex

DEPFILE = depfile.mk


SRC_DIR = src

C_FILES  = $(SRC_DIR)/main.c
C_FILES += $(SRC_DIR)/it.c
C_FILES += $(SRC_DIR)/startup.c
C_FILES += $(SRC_DIR)/sdram.c
C_FILES += $(SRC_DIR)/mpu.c
C_FILES += $(SRC_DIR)/dport.c
C_FILES += $(SRC_DIR)/dport_hw.c

C_OBJS  = $(foreach obj,$(C_FILES),$(patsubst %c,%o, $(obj)))
AS_OBJS = $(foreach obj,$(AS_FILES),$(patsubst %s,%o, $(obj)))
OBJS += $(AS_OBJS)
OBJS += $(C_OBJS)

LDSCRIPT = src/lpc17xx.ld

LIBS += -lc
LIBS += -lmlpc17xx
LIBS += -lmisc

# TODO VPATH
VPATH += $(MLPC17XX_LIB_PATH)
VPATH += $(MISC_LIB_PATH)

#################################
#
# Build rules
#
#################################
.PHONY: all list

all: $(TARGET_BIN)

$(TARGET_BIN) $(TARGET_HEX): $(TARGET_OUT)
	$(CP) -Obinary $(TARGET_OUT) $(TARGET_BIN)
	$(CP) -Oihex $(TARGET_OUT) $(TARGET_HEX)
	$(TCL_SHELL) insvectcs.tcl $(TARGET_BIN)

$(TARGET_OUT): $(OBJS) $(LDSCRIPT) $(LIBS)
	$(LD) $(LDFLAGS) -T$(LDSCRIPT) -o $(TARGET_OUT) $(OBJS) $(LIBS)
	$(SZ) -A $(TARGET_OUT)

-include $(DEPFILE)

depend: $(C_FILES)
	$(CC) $(CFLAGS) -MM $(C_FILES) > $(DEPFILE)
	$(TCL_SHELL) $(SCRIPTS_PATH)/depdir.tcl $(DEPFILE) $(C_OBJS)

list: $(TARGET_OUT)
	$(OD) -S $(TARGET_OUT) > $(TARGET).lst

.PHONY: clean prog

clean:
	$(RM) $(OBJS) $(TARGET_BIN) $(TARGET_HEX) $(TARGET_OUT) $(TARGET).lst

#prog: $(TARGET_BIN)
#	-cd $(OPENOCD_DIR) && openocd -f openocd.cfg -c "upload_image ../../$(PROJECT_DIR)/$(TARGET_BIN) 0x0;shutdown"

