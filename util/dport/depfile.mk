src/dport.o: src/dport.c src/commdev.h src/types.h src/version.h \
 src/errcode.h src/debug.h src/dport_proto.h src/tcl_interface.h
src/debug.o: src/debug.c src/linenoise.h src/comm.h src/types.h src/debug.h \
 src/commdev.h src/dport_proto.h src/errcode.h src/tcl_interface.h \
 src/darm/darm.h src/darm/armv7-tbl.h src/darm/darm-tbl.h
src/comm.o: src/comm.c src/errcode.h src/comm.h src/types.h src/debug.h \
 src/commdev.h src/dport_proto.h
src/commdev.o: src/commdev.c src/commdev.h src/types.h src/errcode.h
src/linenoise.o: src/linenoise.c src/linenoise.h
src/tcl_interface.o: src/tcl_interface.c src/tcl_interface.h src/debug.h \
 src/types.h src/commdev.h src/dport_proto.h src/tcl_def.h

