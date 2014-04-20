#ifndef CLOCK_CONFIG_H
#define CLOCK_CONFIG_H

/*
 * PLL_OUT = M * PLL_IN = Fcco / (2 * P)
 *
 * Fcco = PLL_OUT * 2 * P = PLL_IN * M * 2 * P
 *
 */
#define CLK_PLL_IN     ( 12 * 1000000)
#define CLK_PLL_OUT    ( 72 * 1000000)

#if (CLK_PLL_OUT % CLK_PLL_IN)
    #error "CLK_PLL_OUT should me multiple of CLK_PLL_IN"
#endif

#define CLK_M          (CLK_PLL_OUT / CLK_PLL_IN)       /* 1..32 */
#define CLK_P          2                                /* 1, 2, 4, 8 */
#define CLK_FCCO       (CLK_PLL_IN * CLK_M * 2 * CLK_P)

#if (CLK_M < 1) || (CLK_M > 32)
    #error "CLK_M should be in range 1..32"
#endif

#if (CLK_FCCO < (156 * 1000000)) || (CLK_FCCO > (320 * 1000000))
#error "check CLK_M and CLK_P for FCCO"
#endif

#define CLK_CCLKDIV    1  /* 1..31 */
#define CLK_PCLKDIV    3  /* 0..31 */
#define CLK_EMCDIV     1  /* 1, 2 */

#define CLK_CCLK (CLK_PLL_OUT / CLK_CCLKDIV)
#define CLK_PCLK (CLK_CCLK / CLK_PCLKDIV)
#define CLK_EMC  (CLK_CCLK / CLK_EMCDIV)

#if (CLK_CCLK > 120000000)
    #error "check CPU clock multiplier"
#endif

/*
 * PLL1 and USB clocks
 */
#define CLK_PLL1_OUT    (48 * 1000000)
/* XXX M should be ceil */
#define CLK_PLL1_M      (CLK_PLL1_OUT / CLK_PLL_IN)       /* 1..32 */
#define CLK_PLL1_P      2                                 /* 1, 2, 4, 8 */
#define CLK_PLL1_FCCO   (CLK_PLL_IN * CLK_PLL1_M * 2 * CLK_PLL1_P)

#if (CLK_PLL1_FCCO < (156 * 1000000)) || (CLK_PLL1_FCCO > (320 * 1000000))
#error "check M and P for FCCO of PLL1"
#endif

#define CLK_USBDIV    1  /* 1, 2 or 3 */

#define CLK_USB (CLK_PLL1_OUT / CLK_USBDIV)

#if (CLK_USB != (48 * 1000000))
#error "USB clock != 48 MHz"
#endif

#endif

