#ifndef DPORT_DEBUG_H
#define DPORT_DEBUG_H

#include <types.h>
#include <tcl.h>
#include "commdev.h"
#include "dport_proto.h"

#define DEBUG_BUF_SIZE    4096
struct debug_buffer_t {
    uint8 buf[DEBUG_BUF_SIZE];
    int c;
};

struct debug_t {
    struct commdev_t *commdev;
    Tcl_Interp *interp;
    uint16 bufsize; /* size of target's buffer */

    union dport_req_t req; 

    struct debug_buffer_t tx;
    struct debug_buffer_t rx;

    int run;
    int cont;
};

extern struct debug_t debug;

#define PRINT_TABSTOP "    "
#define PRINT_NEWLINE "\n"

int debug_loop();
int debug_cmd_get_bufsize(struct debug_t *debug);
int debug_cmd_cont(struct debug_t *debug);
int debug_cmd_step(struct debug_t *debug);
int debug_cmd_reset(struct debug_t *debug);
int debug_cmd_read_reg(struct debug_t *debug, int n, uint32 *value);
int debug_cmd_write_reg(struct debug_t *debug, int n, uint32 value);
int debug_cmd_read_mem(struct debug_t *debug, uint32 addr, uint32 len, uint8 *buf);
int debug_cmd_write_mem(struct debug_t *debug, uint32 addr, uint32 len, uint8 *buf);
int debug_cmd_disasm(struct debug_t *debug, uint32 addr, int ninst);
int debug_cmd_get_flash_bufsize(struct debug_t *debug, uint32 *fbsize);
int debug_cmd_write_image(struct debug_t *debug, uint32 offset, char *file, int length);
int debug_cmd_write_flash(struct debug_t *debug, char *data, uint32 offset, uint32 length);
int debug_cmd_userf(struct debug_t *debug, int nfunc);

#endif

