################################################################################
#
# 
#
################################################################################
ROOT_DIR     = ../..

TARGET = main
#################################
#
# Tools config
#
#################################
PROJECT_DIR  = board/sk-mlpc1788
BLOADER_DIR  = $(ROOT_DIR)/board/lpc17xx_boot
DPORT_DIR    = $(ROOT_DIR)/board/dport

SCRIPTS_PATH      = $(ROOT_DIR)/$(UTIL_PATH)
OPENOCD_DIR       = $(ROOT_DIR)/$(UTIL_PATH)/openocd
DPORT_UTIL_DIR    = $(ROOT_DIR)/$(UTIL_PATH)/dport
MLPC17XX_LIB_PATH = $(ROOT_DIR)/lib/mlpc17xx
OS_LIB_PATH       = $(ROOT_DIR)/lib/os
MISC_LIB_PATH     = $(ROOT_DIR)/lib/misc

USE_OPENOCD_PROG ?= 0
DPORT_TTY        ?= /dev/ttyUSB0

include $(ROOT_DIR)/tcl.mk

#################################
#
# Compiller flags
#
#################################
BLOADER_CONFIG = $(BLOADER_DIR)/bloader.mk
include $(BLOADER_CONFIG)

CFLAGS += -O2
CFLAGS += -I$(ROOT_DIR)/lib/lpc17xx
CFLAGS += -I$(MLPC17XX_LIB_PATH)/src
CFLAGS += -I$(OS_LIB_PATH)/src
CFLAGS += -I$(MISC_LIB_PATH)/src

LDFLAGS += -L$(MLPC17XX_LIB_PATH)
LDFLAGS += -L$(OS_LIB_PATH)
LDFLAGS += -L$(MISC_LIB_PATH)
LDFLAGS += -Wl,--defsym=LOAD_OFFSET=$(MAIN_LOAD_OFFSET)
LDFLAGS += -Wl,-Map=$(TARGET).map 

#################################
#
# Objects to build
#
#################################

TARGET_BIN = $(TARGET).bin
TARGET_OUT = $(TARGET).out
TARGET_HEX = $(TARGET).hex

DEPFILE    = depfile.mk
IMAGE_BIN  = image.bin

SRC_DIR = src

C_FILES  = $(SRC_DIR)/main.c
C_FILES += $(SRC_DIR)/it.c
C_FILES += $(SRC_DIR)/startup.c
C_FILES += $(SRC_DIR)/gtask.c
C_FILES += $(SRC_DIR)/osw_objects.c
C_FILES += $(SRC_DIR)/eth.c
C_FILES += $(SRC_DIR)/net/net.c
C_FILES += $(SRC_DIR)/net/net_def.c
C_FILES += $(SRC_DIR)/net/udp.c
C_FILES += $(SRC_DIR)/net/arp.c
C_FILES += $(SRC_DIR)/net/ipv4.c
C_FILES += $(SRC_DIR)/net/icmp.c
C_FILES += $(SRC_DIR)/upload.c
C_FILES += $(SRC_DIR)/dma.c
C_FILES += $(SRC_DIR)/gs/gs.c
C_FILES += $(SRC_DIR)/gs/gsp.c
C_FILES += $(SRC_DIR)/gs/gs_font.c
C_FILES += $(SRC_DIR)/gs/gs_text.c
C_FILES += $(SRC_DIR)/gs/gs_image.c
C_FILES += $(SRC_DIR)/gs/gs_prim.c
C_FILES += $(SRC_DIR)/gs/gs_widget.c
C_FILES += $(SRC_DIR)/gs/gs_util.c
C_FILES += $(SRC_DIR)/gs/widget/gs_wlist.c
C_FILES += $(SRC_DIR)/gs/widget/gs_wpbar.c
C_FILES += $(SRC_DIR)/gs/widget/gs_wtext.c
C_FILES += $(SRC_DIR)/gs/widget/gs_wpixmap.c
C_FILES += $(SRC_DIR)/gs/widget/gs_wvolume.c
C_FILES += $(SRC_DIR)/testgs.c
C_FILES += $(SRC_DIR)/buttons.c
C_FILES += $(SRC_DIR)/gpioirq.c
#C_FILES += $(SRC_DIR)/slog.c
C_FILES += $(SRC_DIR)/nvram.c
#C_FILES += $(SRC_DIR)/gui/gslog.c
C_FILES += $(SRC_DIR)/usbdev/usbdev.c
C_FILES += $(SRC_DIR)/usbdev/usbdev_hw.c
C_FILES += $(SRC_DIR)/usbdev/usbdev_proto.c
C_FILES += $(SRC_DIR)/player/player.c
C_FILES += $(SRC_DIR)/player/player_bartist.c
C_FILES += $(SRC_DIR)/player/player_balbum.c
C_FILES += $(SRC_DIR)/player/player_btrack.c
C_FILES += $(SRC_DIR)/vs1053b/decoder.c
C_FILES += $(SRC_DIR)/vs1053b/vs1053b.c
C_FILES += $(SRC_DIR)/vs1053b/vs1053b_hw.c
C_FILES += $(SRC_DIR)/sdcard/sdcard.c
C_FILES += $(SRC_DIR)/sdcard/sdcard_hw.c
#C_FILES += $(SRC_DIR)/touch/touch.c
#C_FILES += $(SRC_DIR)/touch/touch_hw.c
C_FILES += $(SRC_DIR)/fat_io_lib/fat_access.c
C_FILES += $(SRC_DIR)/fat_io_lib/fat_cache.c
C_FILES += $(SRC_DIR)/fat_io_lib/fat_filelib.c
C_FILES += $(SRC_DIR)/fat_io_lib/fat_format.c
C_FILES += $(SRC_DIR)/fat_io_lib/fat_misc.c
C_FILES += $(SRC_DIR)/fat_io_lib/fat_string.c
C_FILES += $(SRC_DIR)/fat_io_lib/fat_table.c
C_FILES += $(SRC_DIR)/fat_io_lib/fat_write.c
C_FILES += $(SRC_DIR)/image/pcx.c
C_FILES += $(SRC_DIR)/image/image.c
C_FILES += $(SRC_DIR)/image/jpeg.c
C_FILES += $(SRC_DIR)/image/tjpgd/tjpgd.c

C_OBJS  = $(foreach obj,$(C_FILES),$(patsubst %c,%o, $(obj)))
AS_OBJS = $(foreach obj,$(AS_FILES),$(patsubst %s,%o, $(obj)))
OBJS += $(AS_OBJS)
OBJS += $(C_OBJS)

LDSCRIPT = src/lpc17xx.ld

LIBS += -lc
LIBS += -lmlpc17xx
LIBS += -los
LIBS += -lmisc

VPATH += $(MLPC17XX_LIB_PATH)
VPATH += $(OS_LIB_PATH)
VPATH += $(MISC_LIB_PATH)

#################################
#
# Build rules
#
#################################
.PHONY: all list image

all: $(TARGET_BIN)

$(TARGET_BIN) $(TARGET_HEX): $(TARGET_OUT)
	$(CP) -Obinary $(TARGET_OUT) $(TARGET_BIN)
	$(CP) -Oihex $(TARGET_OUT) $(TARGET_HEX)

$(TARGET_OUT): $(OBJS) $(LDSCRIPT) $(LIBS) $(BLOADER_CONFIG)
	$(LD) $(LDFLAGS) -T$(LDSCRIPT) -o $(TARGET_OUT) $(OBJS) $(LIBS)
	$(SZ) -A $(TARGET_OUT)

image: $(IMAGE_BIN)

-include $(DEPFILE)

depend: $(C_FILES)
	$(CC) $(CFLAGS) -MM $(C_FILES) > $(DEPFILE)
	$(TCL_SHELL) $(SCRIPTS_PATH)/depdir.tcl $(DEPFILE) $(C_OBJS)

$(IMAGE_BIN): $(TARGET_BIN) $(BLOADER_DIR)/bloader.bin mkimg.tcl
	$(TCL_SHELL) mkimg.tcl \
	    $(BLOADER_DIR)/bloader.bin $(BLOADER_MAXIMAGE_SIZE) $(TARGET_BIN) $(IMAGE_BIN) \
	    $(MAIN_SIZE_OFFSET) $(MAIN_CRC_OFFSET) $(MAIN_IMAGE_OFFSET)

list: $(TARGET_OUT)
	$(OD) -S $(TARGET_OUT) > $(TARGET).lst

.PHONY: clean prog debug

clean:
	$(RM) $(OBJS) $(TARGET_BIN) $(TARGET_HEX) $(TARGET_OUT) $(TARGET).lst $(TARGET).map $(IMAGE_BIN)

prog: image
	-cd $(OPENOCD_DIR) && openocd -f openocd.cfg -c "upload_image ../../$(PROJECT_DIR)/$(IMAGE_BIN) 0x0;shutdown"

debug: $(TARGET_OUT)
	-cd $(OPENOCD_DIR) && openocd

