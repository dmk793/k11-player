############################################
#
# TODO dependencies
#
############################################
TARGET = dport

ROOT_DIR      = ../..
SRC_PATH      = src
DARM_LIB_PATH = ./$(SRC_PATH)/darm
TCL_SHELL     = tclsh

DEPFILE       = depfile.mk
UTIL_PATH     = $(ROOT_DIR)/util
SCRIPTS_PATH  = $(UTIL_PATH)

############################################
CC = gcc
LD = gcc

############################################
CFLAGS  = -g
CFLAGS += -Wall
CFLAGS += -I$(SRC_PATH)
CFLAGS += -I$(DARM_LIB_PATH)

LDFLAGS += -L./$(SRC_PATH)/darm

############################################
C_FILES += $(SRC_PATH)/dport.c
C_FILES += $(SRC_PATH)/debug.c
C_FILES += $(SRC_PATH)/comm.c
C_FILES += $(SRC_PATH)/commdev.c
C_FILES += $(SRC_PATH)/linenoise.c
C_FILES += $(SRC_PATH)/tcl_interface.c

C_OBJS  = $(foreach obj,$(C_FILES),$(patsubst %c,%o, $(obj)))
OBJS += $(C_OBJS)


DARM_LIB = $(DARM_LIB_PATH)/libdarm.so

#VPATH += /usr/spkg/lib

LIBS += -ltcl
#LIBS += -ldwarf
#LIBS += -lelf
LIBS += $(DARM_LIB)

############################################
$(TARGET): $(OBJS) $(LIBS)
	$(LD) $(LDFLAGS) -o $(TARGET) $^

$(OBJS): $(DARM_LIB)

$(DARM_LIB):
	cd $(DARM_LIB_PATH) && make

-include $(DEPFILE)

.PHONY: clean depend
depend:
	$(CC) $(CFLAGS) -MM $(C_FILES) > $(DEPFILE)
	$(TCL_SHELL) $(SCRIPTS_PATH)/depdir.tcl $(DEPFILE) $(C_OBJS)

clean:
	rm -f $(OBJS) $(TARGET)
	cd $(DARM_LIB_PATH) && make clean

