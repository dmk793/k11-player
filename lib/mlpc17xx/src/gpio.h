#ifndef GPIO_H
#define GPIO_H

#include <types.h>
#include "gpio.h"

#define GPIO_DIR_OUTPUT   0x01
#define GPIO_DIR_INPUT    0x00

struct gpio_t {
    LPC_GPIO_TypeDef *pm;
    uint32 mask;
};

void  gpio_init();
void  gpio_drive(const struct gpio_t *pg, uint8 state);
void  gpio_setdir(const struct gpio_t *pg, uint8 dir);
uint8 gpio_read(const struct gpio_t *pg);
void  gpio_switch(const struct gpio_t *pg);

//#define led_on(pg) gpio_drive(pg, 1)
//#define led_off(pg) gpio_drive(pg, 0)

#endif

