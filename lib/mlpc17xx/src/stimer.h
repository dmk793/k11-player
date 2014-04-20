#ifndef STIMER_LPC17XX_H
#define STIMER_LPC17XX_H

#include <types.h>

void   stimer_init();
void   stimer_settime(uint32 *val);
uint32 stimer_deltatime(uint32 time);
void   stimer_wait(uint32 time);

void   stimer_settime_us(uint32 *val);
uint32 stimer_deltatime_us(uint32 time);
void   stimer_wait_us(uint32 time);

#endif

