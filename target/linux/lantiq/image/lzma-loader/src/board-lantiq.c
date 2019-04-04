/*
 * Arch specific code for Lantiq based boards
 *
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include "config.h"


#define READREG(r)		*(volatile unsigned int *)(r)
#define READREG32(r)		*(volatile uint32_t *)(r)
#define WRITEREG(r,v)		*(volatile unsigned int *)(r) = v
#define WRITEREG32(r,v)		*(volatile uint32_t *)(r) = v

#define UART_BASE		0xbe100c00
#define ASC_TBUF		(UART_BASE | 0x20)
#define ASC_FSTAT		(UART_BASE | 0x48)

// Watchdog

#define LTQ_WDT_MEMBASE_CR	0xbf8803f0 // Membase + control register

#define LTQ_WDT_PW1             0x00BE0000
#define LTQ_WDT_PW2             0x00DC0000

#define LTQ_WDT_SR_EN           (0x1 << 31)     /* enable bit */
#define LTQ_WDT_SR_PWD          (0x3 << 26)     /* turn on power */
#define LTQ_WDT_SR_CLKDIV       (0x3 << 24)     /* turn on clock and set */
                                                /* divider to 0x40000 */
#define LTQ_WDT_DIVIDER         0x40000
#define LTQ_MAX_TIMEOUT         ((1 << 16) - 1) /* the reload field is 16 bit */

#define RCU_MEMBASE			0xBF203000
/* reboot bit */
#define RCU_RD_GPHY0_XRX200     (1<<31)
#define RCU_RD_SRST             (1<<30)
#define RCU_RD_GPHY1_XRX200     (1<<29)
#define RCU_RST_REQ             0x0010

#define CGU_BASE_ADDR		0xBF103000

#define PMU_BASE		0xBF102000
#define BSP_PMU_PWDCR		0x001C
#define PMU_PWDCR 		0xBF10201C
#define PMU_SR			0xBF102020

#define TXMASK          0x3F00
#define TXOFFSET        8

void board_putc(char c)
{
	while ((READREG(ASC_FSTAT) & TXMASK) >> TXOFFSET);

	WRITEREG(ASC_TBUF, c);
}

void board_init(void)
{

	// WDT enable and reset timer
        WRITEREG(LTQ_WDT_MEMBASE_CR, LTQ_WDT_PW1); // First magic word
	WRITEREG(LTQ_WDT_MEMBASE_CR, LTQ_WDT_PW2); //Power off WDT
//        WRITEREG(LTQ_WDT_MEMBASE_CR, LTQ_WDT_SR_EN | LTQ_WDT_SR_PWD | LTQ_WDT_SR_CLKDIV | LTQ_WDT_PW2 | LTQ_MAX_TIMEOUT);

}
