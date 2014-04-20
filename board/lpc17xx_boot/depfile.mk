src/main.o: src/main.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/mlpc17xx/src/uart.h \
 ../../lib/lpc17xx/types.h ../../lib/lpc17xx/clk_cfg.h \
 ../../lib/misc/src/crc.h ../../lib/misc/src/crc32.h \
 ../../lib/misc/src/crc16.h ../../lib/misc/src/debug.h \
 ../../lib/mlpc17xx/src/stimer.h ../../lib/mlpc17xx/src/gpio.h \
 ../../lib/mlpc17xx/src/gpio.h src/sdram.h src/version.h src/mpu.h \
 src/dport.h
src/it.o: src/it.c
src/startup.o: src/startup.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/lpc17xx/LPC177x_8x_bits.h \
 ../../lib/lpc17xx/clk_cfg.h
src/sdram.o: src/sdram.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h ../../lib/mlpc17xx/src/stimer.h src/sdram.h \
 ../../lib/lpc17xx/clk_cfg.h
src/mpu.o: src/mpu.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h src/mpu.h src/sdram.h \
 ../../lib/lpc17xx/clk_cfg.h ../../lib/lpc17xx/types.h
src/dport.o: src/dport.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/mlpc17xx/src/stimer.h \
 ../../lib/lpc17xx/types.h src/dport.h src/dport_hw.h src/dport_proto.h
src/dport_hw.o: src/dport_hw.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/lpc17xx/LPC177x_8x_bits.h \
 ../../lib/lpc17xx/clk_cfg.h src/dport_hw.h ../../lib/lpc17xx/types.h

