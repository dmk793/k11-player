src/main.o: src/main.c ../../lib/misc/src/debug.h ../../lib/lpc17xx/types.h \
 ../../lib/mlpc17xx/src/stimer.h ../../lib/mlpc17xx/src/uart.h \
 ../../lib/lpc17xx/LPC177x_8x.h ../../lib/lpc17xx/core_cm3.h \
 ../../lib/lpc17xx/clk_cfg.h ../../lib/mlpc17xx/src/hwcrc.h \
 ../../lib/mlpc17xx/src/gpio.h ../../lib/mlpc17xx/src/gpio.h \
 ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/osw_objects.h src/net/net.h \
 src/net/net_def.h src/net/../eth_def.h src/net/net_def.h src/version.h
src/it.o: src/it.c
src/startup.o: src/startup.c src/startup.h
src/gtask.o: src/gtask.c ../../lib/os/src/os.h ../../lib/lpc17xx/types.h \
 ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h ../../lib/misc/src/debug.h src/gtask.h
src/osw_objects.o: src/osw_objects.c ../../lib/os/src/os.h \
 ../../lib/lpc17xx/types.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h ../../lib/os/src/os_private.h \
 ../../lib/os/src/port/ARMv7-M/port.h \
 ../../lib/os/src/port/ARMv7-M/port.h ../../lib/misc/src/debug.h \
 src/osw_objects.h src/net/net.h src/net/net_def.h src/net/../eth_def.h \
 src/net/net_def.h src/gs/gs.h src/gs/gs_font.h src/gs/gs_config.h \
 src/gs/gs_text.h src/gs/gs_image.h src/gs/gs.h src/gs/gs_prim.h \
 src/gs/gs_util.h src/gs/gs_widget.h src/gs/widget/gs_wlist.h \
 src/gs/widget/../gs.h src/gs/widget/gs_wpbar.h src/gs/widget/gs_wtext.h \
 src/gs/widget/gs_wpixmap.h src/gs/widget/gs_wvolume.h src/testgs.h \
 src/slog.h src/gui/gslog.h src/upload.h src/usbdev/usbdev.h \
 src/usbdev/usbdev_hw.h src/upload_shared.h src/gtask.h src/dma.h \
 src/player/player.h src/player/../fat_io_lib/fat_filelib.h \
 src/player/../fat_io_lib/fat_opts.h \
 src/player/../fat_io_lib/fat_access.h \
 src/player/../fat_io_lib/fat_defs.h src/player/../fat_io_lib/fat_types.h \
 src/player/../fat_io_lib/fat_list.h src/sdcard/sdcard.h \
 src/sdcard/sdcard_hw.h src/buttons.h src/vs1053b/decoder.h src/gpioirq.h
src/eth.o: src/eth.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/lpc17xx/LPC177x_8x_bits.h \
 ../../lib/misc/src/debug.h ../../lib/lpc17xx/types.h \
 ../../lib/misc/src/util.h ../../lib/mlpc17xx/src/stimer.h \
 ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/osw_objects.h src/net/net.h \
 src/net/net_def.h src/net/../eth_def.h src/net/net_def.h src/irqp.h \
 src/eth.h src/eth_def.h
src/net/net.o: src/net/net.c ../../lib/mlpc17xx/src/stimer.h \
 ../../lib/lpc17xx/types.h ../../lib/misc/src/debug.h src/net/../eth.h \
 src/net/net.h src/net/net_def.h src/net/../eth_def.h \
 ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/net/arp.h src/net/udp.h src/net/ipv4.h \
 src/net/icmp.h src/net/../osw_objects.h src/net/../net/net.h \
 src/net/../net/net_def.h src/net/../upload.h src/net/../usbdev/usbdev.h \
 src/net/../usbdev/usbdev_hw.h src/net/../upload_shared.h
src/net/net_def.o: src/net/net_def.c src/net/net_def.h ../../lib/lpc17xx/types.h \
 src/net/../eth_def.h
src/net/udp.o: src/net/udp.c ../../lib/misc/src/debug.h ../../lib/lpc17xx/types.h \
 src/net/net.h src/net/net_def.h src/net/../eth_def.h \
 ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/net/udp.h src/net/ipv4.h
src/net/arp.o: src/net/arp.c ../../lib/misc/src/debug.h ../../lib/lpc17xx/types.h \
 src/net/arp.h src/net/net_def.h src/net/../eth_def.h src/net/net.h \
 ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/net/../osw_objects.h \
 src/net/../net/net.h src/net/../net/net_def.h
src/net/ipv4.o: src/net/ipv4.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/net/ipv4.h src/net/net_def.h \
 src/net/../eth_def.h src/net/udp.h src/net/net.h ../../lib/os/src/os.h \
 ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/net/arp.h src/net/icmp.h \
 src/net/../osw_objects.h src/net/../net/net.h src/net/../net/net_def.h
src/net/icmp.o: src/net/icmp.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/net/icmp.h src/net/ipv4.h \
 src/net/net_def.h src/net/../eth_def.h src/net/net.h \
 ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h
src/upload.o: src/upload.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/upload.h src/usbdev/usbdev.h \
 ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/usbdev/usbdev_hw.h src/upload_shared.h \
 src/net/net.h src/net/net_def.h src/net/../eth_def.h \
 ../../lib/misc/src/crc.h ../../lib/misc/src/crc32.h \
 ../../lib/misc/src/crc16.h
src/dma.o: src/dma.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/lpc17xx/LPC177x_8x_bits.h \
 ../../lib/mlpc17xx/src/stimer.h ../../lib/lpc17xx/types.h \
 ../../lib/misc/src/debug.h ../../lib/os/src/os.h \
 ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/irqp.h src/dma.h
src/gs/gs.o: src/gs/gs.c ../../lib/misc/src/debug.h ../../lib/lpc17xx/types.h \
 ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h ../../lib/mlpc17xx/src/stimer.h src/gs/gs.h \
 src/gs/gs_font.h src/gs/gs_config.h src/gs/gs_text.h src/gs/gs_image.h \
 src/gs/gs_prim.h src/gs/gs_util.h src/gs/gs_widget.h \
 src/gs/widget/gs_wlist.h src/gs/widget/../gs.h src/gs/widget/gs_wpbar.h \
 src/gs/widget/gs_wtext.h src/gs/widget/gs_wpixmap.h \
 src/gs/widget/gs_wvolume.h src/gs/gsp.h
src/gs/gsp.o: src/gs/gsp.c ../../lib/lpc17xx/clk_cfg.h \
 ../../lib/lpc17xx/LPC177x_8x.h ../../lib/lpc17xx/core_cm3.h \
 ../../lib/lpc17xx/LPC177x_8x_bits.h ../../lib/mlpc17xx/src/stimer.h \
 ../../lib/lpc17xx/types.h ../../lib/misc/src/debug.h \
 ../../lib/mlpc17xx/src/gpio.h ../../lib/mlpc17xx/src/gpio.h \
 ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/gs/gsp.h src/gs/gs.h src/gs/gs_font.h \
 src/gs/gs_config.h src/gs/gs_text.h src/gs/gs_image.h src/gs/gs_prim.h \
 src/gs/gs_util.h src/gs/gs_widget.h src/gs/widget/gs_wlist.h \
 src/gs/widget/../gs.h src/gs/widget/gs_wpbar.h src/gs/widget/gs_wtext.h \
 src/gs/widget/gs_wpixmap.h src/gs/widget/gs_wvolume.h src/gs/../dma.h \
 src/gs/../irqp.h
src/gs/gs_font.o: src/gs/gs_font.c src/gs/gs.h ../../lib/lpc17xx/types.h \
 src/gs/gs_font.h src/gs/gs_config.h src/gs/gs_text.h src/gs/gs_image.h \
 src/gs/gs_prim.h src/gs/gs_util.h src/gs/gs_widget.h \
 src/gs/widget/gs_wlist.h src/gs/widget/../gs.h src/gs/widget/gs_wpbar.h \
 src/gs/widget/gs_wtext.h src/gs/widget/gs_wpixmap.h \
 src/gs/widget/gs_wvolume.h
src/gs/gs_text.o: src/gs/gs_text.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/gs/gs.h src/gs/gs_font.h \
 src/gs/gs_config.h src/gs/gs_text.h src/gs/gs_image.h src/gs/gs_prim.h \
 src/gs/gs_util.h src/gs/gs_widget.h src/gs/widget/gs_wlist.h \
 src/gs/widget/../gs.h src/gs/widget/gs_wpbar.h src/gs/widget/gs_wtext.h \
 src/gs/widget/gs_wpixmap.h src/gs/widget/gs_wvolume.h src/gs/gsp.h
src/gs/gs_image.o: src/gs/gs_image.c src/gs/gs.h ../../lib/lpc17xx/types.h \
 src/gs/gs_font.h src/gs/gs_config.h src/gs/gs_text.h src/gs/gs_image.h \
 src/gs/gs_prim.h src/gs/gs_util.h src/gs/gs_widget.h \
 src/gs/widget/gs_wlist.h src/gs/widget/../gs.h src/gs/widget/gs_wpbar.h \
 src/gs/widget/gs_wtext.h src/gs/widget/gs_wpixmap.h \
 src/gs/widget/gs_wvolume.h src/gs/gsp.h
src/gs/gs_prim.o: src/gs/gs_prim.c src/gs/gs.h ../../lib/lpc17xx/types.h \
 src/gs/gs_font.h src/gs/gs_config.h src/gs/gs_text.h src/gs/gs_image.h \
 src/gs/gs_prim.h src/gs/gs_util.h src/gs/gs_widget.h \
 src/gs/widget/gs_wlist.h src/gs/widget/../gs.h src/gs/widget/gs_wpbar.h \
 src/gs/widget/gs_wtext.h src/gs/widget/gs_wpixmap.h \
 src/gs/widget/gs_wvolume.h
src/gs/gs_widget.o: src/gs/gs_widget.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/gs/gs.h src/gs/gs_font.h \
 src/gs/gs_config.h src/gs/gs_text.h src/gs/gs_image.h src/gs/gs_prim.h \
 src/gs/gs_util.h src/gs/gs_widget.h src/gs/widget/gs_wlist.h \
 src/gs/widget/../gs.h src/gs/widget/gs_wpbar.h src/gs/widget/gs_wtext.h \
 src/gs/widget/gs_wpixmap.h src/gs/widget/gs_wvolume.h src/gs/gsp.h
src/gs/gs_util.o: src/gs/gs_util.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/gs/gs.h src/gs/gs_font.h \
 src/gs/gs_config.h src/gs/gs_text.h src/gs/gs_image.h src/gs/gs_prim.h \
 src/gs/gs_util.h src/gs/gs_widget.h src/gs/widget/gs_wlist.h \
 src/gs/widget/../gs.h src/gs/widget/gs_wpbar.h src/gs/widget/gs_wtext.h \
 src/gs/widget/gs_wpixmap.h src/gs/widget/gs_wvolume.h
src/gs/widget/gs_wlist.o: src/gs/widget/gs_wlist.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h ../../lib/misc/src/pearson.h \
 src/gs/widget/../gs.h src/gs/widget/../gs_font.h \
 src/gs/widget/../gs_config.h src/gs/widget/../gs_text.h \
 src/gs/widget/../gs_image.h src/gs/widget/../gs.h \
 src/gs/widget/../gs_prim.h src/gs/widget/../gs_util.h \
 src/gs/widget/../gs_widget.h src/gs/widget/../widget/gs_wlist.h \
 src/gs/widget/../widget/../gs.h src/gs/widget/../widget/gs_wpbar.h \
 src/gs/widget/../widget/gs_wtext.h src/gs/widget/../widget/gs_wpixmap.h \
 src/gs/widget/../widget/gs_wvolume.h src/gs/widget/../gsp.h
src/gs/widget/gs_wpbar.o: src/gs/widget/gs_wpbar.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/gs/widget/../gs.h \
 src/gs/widget/../gs_font.h src/gs/widget/../gs_config.h \
 src/gs/widget/../gs_text.h src/gs/widget/../gs_image.h \
 src/gs/widget/../gs.h src/gs/widget/../gs_prim.h \
 src/gs/widget/../gs_util.h src/gs/widget/../gs_widget.h \
 src/gs/widget/../widget/gs_wlist.h src/gs/widget/../widget/../gs.h \
 src/gs/widget/../widget/gs_wpbar.h src/gs/widget/../widget/gs_wtext.h \
 src/gs/widget/../widget/gs_wpixmap.h \
 src/gs/widget/../widget/gs_wvolume.h src/gs/widget/../gsp.h
src/gs/widget/gs_wtext.o: src/gs/widget/gs_wtext.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/gs/widget/../gs.h \
 src/gs/widget/../gs_font.h src/gs/widget/../gs_config.h \
 src/gs/widget/../gs_text.h src/gs/widget/../gs_image.h \
 src/gs/widget/../gs.h src/gs/widget/../gs_prim.h \
 src/gs/widget/../gs_util.h src/gs/widget/../gs_widget.h \
 src/gs/widget/../widget/gs_wlist.h src/gs/widget/../widget/../gs.h \
 src/gs/widget/../widget/gs_wpbar.h src/gs/widget/../widget/gs_wtext.h \
 src/gs/widget/../widget/gs_wpixmap.h \
 src/gs/widget/../widget/gs_wvolume.h src/gs/widget/../gsp.h
src/gs/widget/gs_wpixmap.o: src/gs/widget/gs_wpixmap.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/gs/widget/../gs.h \
 src/gs/widget/../gs_font.h src/gs/widget/../gs_config.h \
 src/gs/widget/../gs_text.h src/gs/widget/../gs_image.h \
 src/gs/widget/../gs.h src/gs/widget/../gs_prim.h \
 src/gs/widget/../gs_util.h src/gs/widget/../gs_widget.h \
 src/gs/widget/../widget/gs_wlist.h src/gs/widget/../widget/../gs.h \
 src/gs/widget/../widget/gs_wpbar.h src/gs/widget/../widget/gs_wtext.h \
 src/gs/widget/../widget/gs_wpixmap.h \
 src/gs/widget/../widget/gs_wvolume.h src/gs/widget/../gsp.h
src/gs/widget/gs_wvolume.o: src/gs/widget/gs_wvolume.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/gs/widget/../gs.h \
 src/gs/widget/../gs_font.h src/gs/widget/../gs_config.h \
 src/gs/widget/../gs_text.h src/gs/widget/../gs_image.h \
 src/gs/widget/../gs.h src/gs/widget/../gs_prim.h \
 src/gs/widget/../gs_util.h src/gs/widget/../gs_widget.h \
 src/gs/widget/../widget/gs_wlist.h src/gs/widget/../widget/../gs.h \
 src/gs/widget/../widget/gs_wpbar.h src/gs/widget/../widget/gs_wtext.h \
 src/gs/widget/../widget/gs_wpixmap.h \
 src/gs/widget/../widget/gs_wvolume.h src/gs/widget/../gsp.h
src/testgs.o: src/testgs.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h ../../lib/os/src/os.h \
 ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h ../../lib/mlpc17xx/src/stimer.h src/gs/gs.h \
 src/gs/gs_font.h src/gs/gs_config.h src/gs/gs_text.h src/gs/gs_image.h \
 src/gs/gs.h src/gs/gs_prim.h src/gs/gs_util.h src/gs/gs_widget.h \
 src/gs/widget/gs_wlist.h src/gs/widget/../gs.h src/gs/widget/gs_wpbar.h \
 src/gs/widget/gs_wtext.h src/gs/widget/gs_wpixmap.h \
 src/gs/widget/gs_wvolume.h src/testgs.h src/net/net.h src/net/net_def.h \
 src/net/../eth_def.h
src/buttons.o: src/buttons.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/mlpc17xx/src/stimer.h \
 ../../lib/lpc17xx/types.h ../../lib/misc/src/debug.h \
 ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h ../../lib/mlpc17xx/src/gpio.h \
 ../../lib/mlpc17xx/src/gpio.h src/buttons.h src/irqp.h \
 src/player/player.h src/player/../fat_io_lib/fat_filelib.h \
 src/player/../fat_io_lib/fat_opts.h \
 src/player/../fat_io_lib/fat_access.h \
 src/player/../fat_io_lib/fat_defs.h src/player/../fat_io_lib/fat_types.h \
 src/player/../fat_io_lib/fat_list.h src/gpioirq.h
src/gpioirq.o: src/gpioirq.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/lpc17xx/LPC177x_8x_bits.h \
 ../../lib/misc/src/debug.h ../../lib/lpc17xx/types.h \
 ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/gpioirq.h src/irqp.h src/buttons.h \
 src/vs1053b/vs1053b_hw.h
src/nvram.o: src/nvram.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/lpc17xx/LPC177x_8x_bits.h \
 ../../lib/misc/src/debug.h ../../lib/lpc17xx/types.h src/nvram.h
src/usbdev/usbdev.o: src/usbdev/usbdev.c ../../lib/os/src/os.h \
 ../../lib/lpc17xx/types.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h ../../lib/misc/src/debug.h \
 src/usbdev/../upload.h src/usbdev/../usbdev/usbdev.h \
 src/usbdev/../usbdev/usbdev_hw.h src/usbdev/../upload_shared.h \
 src/usbdev/usbdev.h src/usbdev/usbdev_proto.h
src/usbdev/usbdev_hw.o: src/usbdev/usbdev_hw.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/lpc17xx/LPC177x_8x_bits.h \
 ../../lib/mlpc17xx/src/gpio.c ../../lib/mlpc17xx/src/gpio.h \
 ../../lib/lpc17xx/types.h ../../lib/lpc17xx/clk_cfg.h \
 ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h ../../lib/misc/src/debug.h \
 src/usbdev/../irqp.h src/usbdev/usbdev.h src/usbdev/usbdev_hw.h \
 src/usbdev/usbdev_proto.h
src/usbdev/usbdev_proto.o: src/usbdev/usbdev_proto.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/usbdev/usbdev.h ../../lib/os/src/os.h \
 ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/usbdev/usbdev_hw.h \
 src/usbdev/usbdev_proto.h
src/player/player.o: src/player/player.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h ../../lib/os/src/os.h \
 ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h ../../lib/misc/src/pearson.h \
 ../../lib/mlpc17xx/src/stimer.h src/player/player.h \
 src/player/../fat_io_lib/fat_filelib.h \
 src/player/../fat_io_lib/fat_opts.h \
 src/player/../fat_io_lib/fat_access.h \
 src/player/../fat_io_lib/fat_defs.h src/player/../fat_io_lib/fat_types.h \
 src/player/../fat_io_lib/fat_list.h src/player/player_bartist.h \
 src/player/player_balbum.h src/player/player_btrack.h \
 src/player/player_layout.h src/player/../gs/gs.h \
 src/player/../gs/gs_font.h src/player/../gs/gs_config.h \
 src/player/../gs/gs_text.h src/player/../gs/gs_image.h \
 src/player/../gs/gs.h src/player/../gs/gs_prim.h \
 src/player/../gs/gs_util.h src/player/../gs/gs_widget.h \
 src/player/../gs/widget/gs_wlist.h src/player/../gs/widget/../gs.h \
 src/player/../gs/widget/gs_wpbar.h src/player/../gs/widget/gs_wtext.h \
 src/player/../gs/widget/gs_wpixmap.h \
 src/player/../gs/widget/gs_wvolume.h src/player/../sdcard/sdcard.h \
 src/player/../sdcard/sdcard_hw.h src/player/../image/image.h \
 src/player/../buttons.h src/player/../vs1053b/vs1053b.h \
 src/player/../vs1053b/decoder.h src/player/../nvram.h
src/player/player_bartist.o: src/player/player_bartist.c ../../lib/lpc17xx/types.h \
 ../../lib/misc/src/debug.h src/player/player_bartist.h \
 src/player/player.h ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/player/../fat_io_lib/fat_filelib.h \
 src/player/../fat_io_lib/fat_opts.h \
 src/player/../fat_io_lib/fat_access.h \
 src/player/../fat_io_lib/fat_defs.h src/player/../fat_io_lib/fat_types.h \
 src/player/../fat_io_lib/fat_list.h src/player/player_layout.h \
 src/player/../buttons.h src/player/../gs/gs.h src/player/../gs/gs_font.h \
 src/player/../gs/gs_config.h src/player/../gs/gs_text.h \
 src/player/../gs/gs_image.h src/player/../gs/gs.h \
 src/player/../gs/gs_prim.h src/player/../gs/gs_util.h \
 src/player/../gs/gs_widget.h src/player/../gs/widget/gs_wlist.h \
 src/player/../gs/widget/../gs.h src/player/../gs/widget/gs_wpbar.h \
 src/player/../gs/widget/gs_wtext.h src/player/../gs/widget/gs_wpixmap.h \
 src/player/../gs/widget/gs_wvolume.h
src/player/player_balbum.o: src/player/player_balbum.c ../../lib/lpc17xx/types.h \
 ../../lib/misc/src/debug.h src/player/player_balbum.h \
 src/player/player.h ../../lib/os/src/os.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/player/../fat_io_lib/fat_filelib.h \
 src/player/../fat_io_lib/fat_opts.h \
 src/player/../fat_io_lib/fat_access.h \
 src/player/../fat_io_lib/fat_defs.h src/player/../fat_io_lib/fat_types.h \
 src/player/../fat_io_lib/fat_list.h src/player/player_layout.h \
 src/player/../buttons.h src/player/../gs/gs.h src/player/../gs/gs_font.h \
 src/player/../gs/gs_config.h src/player/../gs/gs_text.h \
 src/player/../gs/gs_image.h src/player/../gs/gs.h \
 src/player/../gs/gs_prim.h src/player/../gs/gs_util.h \
 src/player/../gs/gs_widget.h src/player/../gs/widget/gs_wlist.h \
 src/player/../gs/widget/../gs.h src/player/../gs/widget/gs_wpbar.h \
 src/player/../gs/widget/gs_wtext.h src/player/../gs/widget/gs_wpixmap.h \
 src/player/../gs/widget/gs_wvolume.h
src/player/player_btrack.o: src/player/player_btrack.c ../../lib/lpc17xx/types.h \
 ../../lib/misc/src/debug.h ../../lib/misc/src/util.h \
 src/player/player_btrack.h src/player/player.h ../../lib/os/src/os.h \
 ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/player/../fat_io_lib/fat_filelib.h \
 src/player/../fat_io_lib/fat_opts.h \
 src/player/../fat_io_lib/fat_access.h \
 src/player/../fat_io_lib/fat_defs.h src/player/../fat_io_lib/fat_types.h \
 src/player/../fat_io_lib/fat_list.h src/player/player_layout.h \
 src/player/../buttons.h src/player/../gs/gs.h src/player/../gs/gs_font.h \
 src/player/../gs/gs_config.h src/player/../gs/gs_text.h \
 src/player/../gs/gs_image.h src/player/../gs/gs.h \
 src/player/../gs/gs_prim.h src/player/../gs/gs_util.h \
 src/player/../gs/gs_widget.h src/player/../gs/widget/gs_wlist.h \
 src/player/../gs/widget/../gs.h src/player/../gs/widget/gs_wpbar.h \
 src/player/../gs/widget/gs_wtext.h src/player/../gs/widget/gs_wpixmap.h \
 src/player/../gs/widget/gs_wvolume.h src/player/../image/image.h
src/vs1053b/decoder.o: src/vs1053b/decoder.c ../../lib/os/src/os.h \
 ../../lib/lpc17xx/types.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h ../../lib/misc/src/debug.h \
 ../../lib/mlpc17xx/src/stimer.h ../../lib/misc/src/util.h \
 src/vs1053b/vs1053b.h src/vs1053b/decoder.h \
 src/vs1053b/../player/player.h \
 src/vs1053b/../player/../fat_io_lib/fat_filelib.h \
 src/vs1053b/../player/../fat_io_lib/fat_opts.h \
 src/vs1053b/../player/../fat_io_lib/fat_access.h \
 src/vs1053b/../player/../fat_io_lib/fat_defs.h \
 src/vs1053b/../player/../fat_io_lib/fat_types.h \
 src/vs1053b/../player/../fat_io_lib/fat_list.h \
 src/vs1053b/../fat_io_lib/fat_filelib.h
src/vs1053b/vs1053b.o: src/vs1053b/vs1053b.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h ../../lib/os/src/os.h \
 ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h ../../lib/mlpc17xx/src/stimer.h \
 src/vs1053b/vs1053b.h src/vs1053b/vs1053b_hw.h \
 src/vs1053b/../player/player.h \
 src/vs1053b/../player/../fat_io_lib/fat_filelib.h \
 src/vs1053b/../player/../fat_io_lib/fat_opts.h \
 src/vs1053b/../player/../fat_io_lib/fat_access.h \
 src/vs1053b/../player/../fat_io_lib/fat_defs.h \
 src/vs1053b/../player/../fat_io_lib/fat_types.h \
 src/vs1053b/../player/../fat_io_lib/fat_list.h
src/vs1053b/vs1053b_hw.o: src/vs1053b/vs1053b_hw.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/lpc17xx/LPC177x_8x_bits.h \
 ../../lib/lpc17xx/clk_cfg.h ../../lib/os/src/os.h \
 ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h ../../lib/mlpc17xx/src/gpio.h \
 ../../lib/mlpc17xx/src/gpio.h ../../lib/mlpc17xx/src/stimer.h \
 src/vs1053b/../irqp.h src/vs1053b/vs1053b_hw.h
src/sdcard/sdcard.o: src/sdcard/sdcard.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h ../../lib/os/src/os.h \
 ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h src/sdcard/sdcard.h src/sdcard/sdcard_hw.h \
 src/sdcard/../fat_io_lib/fat_filelib.h \
 src/sdcard/../fat_io_lib/fat_opts.h \
 src/sdcard/../fat_io_lib/fat_access.h \
 src/sdcard/../fat_io_lib/fat_defs.h src/sdcard/../fat_io_lib/fat_types.h \
 src/sdcard/../fat_io_lib/fat_list.h
src/sdcard/sdcard_hw.o: src/sdcard/sdcard_hw.c ../../lib/lpc17xx/LPC177x_8x.h \
 ../../lib/lpc17xx/core_cm3.h ../../lib/lpc17xx/LPC177x_8x_bits.h \
 ../../lib/lpc17xx/clk_cfg.h ../../lib/os/src/os.h \
 ../../lib/lpc17xx/types.h ../../lib/os/src/os_config.h \
 ../../lib/os/src/../../../board/sk-mlpc1788/src/os_config.h \
 ../../lib/os/src/os_flags.h ../../lib/os/src/os_bitobj.h \
 ../../lib/os/src/os_queue.h ../../lib/os/src/os_mem.h \
 ../../lib/os/src/os_multi.h ../../lib/misc/src/debug.h \
 ../../lib/mlpc17xx/src/gpio.h ../../lib/mlpc17xx/src/gpio.h \
 src/sdcard/sdcard.h src/sdcard/sdcard_hw.h src/sdcard/../dma.h \
 src/sdcard/../irqp.h
src/fat_io_lib/fat_access.o: src/fat_io_lib/fat_access.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/fat_io_lib/fat_defs.h \
 src/fat_io_lib/fat_opts.h src/fat_io_lib/fat_types.h \
 src/fat_io_lib/fat_access.h src/fat_io_lib/fat_table.h \
 src/fat_io_lib/fat_misc.h src/fat_io_lib/fat_write.h \
 src/fat_io_lib/fat_string.h
src/fat_io_lib/fat_cache.o: src/fat_io_lib/fat_cache.c src/fat_io_lib/fat_cache.h \
 src/fat_io_lib/fat_filelib.h src/fat_io_lib/fat_opts.h \
 src/fat_io_lib/fat_access.h src/fat_io_lib/fat_defs.h \
 src/fat_io_lib/fat_types.h ../../lib/lpc17xx/types.h \
 src/fat_io_lib/fat_list.h
src/fat_io_lib/fat_filelib.o: src/fat_io_lib/fat_filelib.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/fat_io_lib/fat_defs.h \
 src/fat_io_lib/fat_opts.h src/fat_io_lib/fat_types.h \
 src/fat_io_lib/fat_access.h src/fat_io_lib/fat_table.h \
 src/fat_io_lib/fat_misc.h src/fat_io_lib/fat_write.h \
 src/fat_io_lib/fat_string.h src/fat_io_lib/fat_filelib.h \
 src/fat_io_lib/fat_list.h src/fat_io_lib/fat_cache.h \
 src/fat_io_lib/fat_list2.h
src/fat_io_lib/fat_format.o: src/fat_io_lib/fat_format.c src/fat_io_lib/fat_defs.h \
 src/fat_io_lib/fat_opts.h src/fat_io_lib/fat_types.h \
 ../../lib/lpc17xx/types.h src/fat_io_lib/fat_access.h \
 src/fat_io_lib/fat_table.h src/fat_io_lib/fat_misc.h \
 src/fat_io_lib/fat_write.h src/fat_io_lib/fat_string.h \
 src/fat_io_lib/fat_format.h
src/fat_io_lib/fat_misc.o: src/fat_io_lib/fat_misc.c src/fat_io_lib/fat_misc.h \
 src/fat_io_lib/fat_defs.h src/fat_io_lib/fat_opts.h \
 src/fat_io_lib/fat_types.h ../../lib/lpc17xx/types.h
src/fat_io_lib/fat_string.o: src/fat_io_lib/fat_string.c src/fat_io_lib/fat_string.h
src/fat_io_lib/fat_table.o: src/fat_io_lib/fat_table.c src/fat_io_lib/fat_defs.h \
 src/fat_io_lib/fat_opts.h src/fat_io_lib/fat_types.h \
 ../../lib/lpc17xx/types.h src/fat_io_lib/fat_access.h \
 src/fat_io_lib/fat_table.h src/fat_io_lib/fat_misc.h
src/fat_io_lib/fat_write.o: src/fat_io_lib/fat_write.c src/fat_io_lib/fat_defs.h \
 src/fat_io_lib/fat_opts.h src/fat_io_lib/fat_types.h \
 ../../lib/lpc17xx/types.h src/fat_io_lib/fat_access.h \
 src/fat_io_lib/fat_table.h src/fat_io_lib/fat_misc.h \
 src/fat_io_lib/fat_write.h src/fat_io_lib/fat_string.h
src/image/pcx.o: src/image/pcx.c ../../lib/lpc17xx/types.h \
 ../../lib/misc/src/debug.h src/image/pcx.h
src/image/image.o: src/image/image.c ../../lib/misc/src/debug.h \
 ../../lib/lpc17xx/types.h src/image/../gs/gs.h src/image/../gs/gs_font.h \
 src/image/../gs/gs_config.h src/image/../gs/gs_text.h \
 src/image/../gs/gs_image.h src/image/../gs/gs.h \
 src/image/../gs/gs_prim.h src/image/../gs/gs_util.h \
 src/image/../gs/gs_widget.h src/image/../gs/widget/gs_wlist.h \
 src/image/../gs/widget/../gs.h src/image/../gs/widget/gs_wpbar.h \
 src/image/../gs/widget/gs_wtext.h src/image/../gs/widget/gs_wpixmap.h \
 src/image/../gs/widget/gs_wvolume.h src/image/pcx.h src/image/jpeg.h \
 src/image/image.h
src/image/jpeg.o: src/image/jpeg.c ../../lib/lpc17xx/types.h \
 ../../lib/misc/src/debug.h src/image/jpeg.h src/image/tjpgd/tjpgd.h \
 src/image/tjpgd/integer.h
src/image/tjpgd/tjpgd.o: src/image/tjpgd/tjpgd.c src/image/tjpgd/tjpgd.h \
 src/image/tjpgd/integer.h

