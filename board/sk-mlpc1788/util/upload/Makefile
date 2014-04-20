CC = gcc
LD = gcc

TARGET = upload

OBJS    = upload.o
#OBJS   += transport.o
#OBJS   += udp.o
OBJS   += commdev.o
OBJS   += crc16.o

CFLAGS += -g
CFLAGS += -O2
CFLAGS += -I/usr/include/libusb-1.0
CFLAGS += -I../../src
CFLAGS += -I.
CFLAGS += -Wall


$(TARGET): $(OBJS)
	$(LD) $(LDFALGS) -o $(TARGET) $^ -lusb-1.0

$(OBJS):

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

