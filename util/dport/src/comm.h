#ifndef DP_PROTOCOL_H
#define DP_PROTOCOL_H

#include <types.h>
#include "debug.h"
#include "commdev.h"

int comm_txrx(struct debug_t *debug);
void comm_mkreq(struct debug_buffer_t *buf, uint8 cmd, uint8 *payload, int len);

#endif

