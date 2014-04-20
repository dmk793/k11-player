src/os.o: src/os.c ../../lib/misc/src/debug.h ../../lib/lpc17xx/types.h \
 src/os.h src/os_config.h src/../../../board/sk-mlpc1788/src/os_config.h \
 src/os_flags.h src/os_bitobj.h src/os_queue.h src/os_mem.h \
 src/os_multi.h src/os_private.h src/port/ARMv7-M/port.h src/os_sched.h
src/os_sched.o: src/os_sched.c src/os_sched.h src/os_private.h \
 ../../lib/lpc17xx/types.h src/os_config.h \
 src/../../../board/sk-mlpc1788/src/os_config.h src/os_flags.h \
 src/port/ARMv7-M/port.h src/os_queue.h src/os_multi.h src/os_bitobj.h
src/os_bitobj.o: src/os_bitobj.c src/os_private.h ../../lib/lpc17xx/types.h \
 src/os_config.h src/../../../board/sk-mlpc1788/src/os_config.h \
 src/os_flags.h src/port/ARMv7-M/port.h src/os_bitobj.h src/os_sched.h
src/os_mem.o: src/os_mem.c src/os_private.h ../../lib/lpc17xx/types.h \
 src/os_config.h src/../../../board/sk-mlpc1788/src/os_config.h \
 src/os_flags.h src/port/ARMv7-M/port.h
src/os_mem1.o: src/os_mem1.c src/os_private.h ../../lib/lpc17xx/types.h \
 src/os_config.h src/../../../board/sk-mlpc1788/src/os_config.h \
 src/os_flags.h src/port/ARMv7-M/port.h src/os_mem.h
src/os_mem2.o: src/os_mem2.c src/os_private.h ../../lib/lpc17xx/types.h \
 src/os_config.h src/../../../board/sk-mlpc1788/src/os_config.h \
 src/os_flags.h src/port/ARMv7-M/port.h src/os_mem.h src/os_bitobj.h
src/os_mem3.o: src/os_mem3.c src/os_private.h ../../lib/lpc17xx/types.h \
 src/os_config.h src/../../../board/sk-mlpc1788/src/os_config.h \
 src/os_flags.h src/port/ARMv7-M/port.h src/os_mem.h
src/os_queue.o: src/os_queue.c src/os_private.h ../../lib/lpc17xx/types.h \
 src/os_config.h src/../../../board/sk-mlpc1788/src/os_config.h \
 src/os_flags.h src/port/ARMv7-M/port.h src/os_queue.h src/os_bitobj.h \
 src/os_sched.h src/os_mem.h
src/os_multi.o: src/os_multi.c src/os_private.h ../../lib/lpc17xx/types.h \
 src/os_config.h src/../../../board/sk-mlpc1788/src/os_config.h \
 src/os_flags.h src/port/ARMv7-M/port.h src/os_multi.h src/os_bitobj.h \
 src/os_queue.h src/os_sched.h src/os_mem.h
src/port/ARMv7-M/port.o: src/port/ARMv7-M/port.c ../../lib/lpc17xx/cm3.h \
 ../../lib/lpc17xx/LPC177x_8x.h ../../lib/lpc17xx/core_cm3.h \
 ../../lib/lpc17xx/clk_cfg.h ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/port/ARMv7-M/port.h \
 src/port/ARMv7-M/../../os.h src/port/ARMv7-M/../../os_config.h \
 src/port/ARMv7-M/../../../../../board/sk-mlpc1788/src/os_config.h \
 src/port/ARMv7-M/../../os_flags.h src/port/ARMv7-M/../../os_bitobj.h \
 src/port/ARMv7-M/../../os_queue.h src/port/ARMv7-M/../../os_mem.h \
 src/port/ARMv7-M/../../os_multi.h src/port/ARMv7-M/../../os_private.h \
 src/port/ARMv7-M/../../port/ARMv7-M/port.h \
 src/port/ARMv7-M/../../os_config.h

