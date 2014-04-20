/*
 * TODO PINSEL and PINMODE select function
 */
#include <LPC177x_8x.h>
#include "gpio.h"

#define PCONP_PCGPIO  (1 << 15)


void gpio_init()
{
    LPC_SC->PCONP |= PCONP_PCGPIO;

    LPC_GPIO0->MASK = 0;
    LPC_GPIO1->MASK = 0;
    LPC_GPIO2->MASK = 0;
    LPC_GPIO3->MASK = 0;
    LPC_GPIO4->MASK = 0;
    LPC_GPIO5->MASK = 0;
}

/*
 * set gpio direciton
 */
void gpio_setdir(const struct gpio_t *pg, uint8 dir)
{
    if (dir == GPIO_DIR_OUTPUT)
        pg->pm->DIR |= pg->mask;
    else
        pg->pm->DIR &= ~(pg->mask);
}

/*
 * turn on/off gpio dependant of 'state' value
 */
void gpio_drive(const struct gpio_t *pg, uint8 state)
{
    if (state)
        pg->pm->SET = pg->mask;
    else
        pg->pm->CLR = pg->mask;
}

/*
 * read GPIO state
 *
 * RETURN
 *       0  if GPIO is low
 *       >0 if GPIO is high
 */
uint8 gpio_read(const struct gpio_t *pg)
{
    if (pg->pm->PIN & pg->mask)
        return 1;
    return 0;
}

/*
 * switch GPIO state
 */
void gpio_switch(const struct gpio_t *pg)
{
    if (pg->pm->PIN & pg->mask)
        pg->pm->CLR = pg->mask;
    else
        pg->pm->SET = pg->mask;
}

