/* Copyright (c) 2019, Breakaway Consulting Pty. Ltd. */

/* 
   This module provides the standard debug interface for
   the mini-uart hardware block on the BCM2837.

   This is designed for very simple low-level debug output,
   so is just a simple polling implementation.

   Note: Per https://github.com/raspberrypi/documentation/issues/325
   there is no officially release documentation for the BCM2837,
   instead we rely on the release documentation for the BCM2835.
   https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
   and the knowledge shared here:
   https://www.raspberrypi.org/forums/viewtopic.php?p=942349
   which indicates the peripheral base address is at 0x3F000000 
   rather than 0x7E000000 per the BCM2835 documentation.
 */


#define AUX_MU_IO_REG ((volatile unsigned int*)(0x3F215040))
#define AUX_MU_LSR_REG ((volatile unsigned int*)(0x3F215054))

static void putc(unsigned int c)
{
    /* Busy loop until "Transmitter empty" bit is set */
    asm volatile("": : :"memory");
    
    do {
        asm volatile("": : :"memory");
    } while(!(*AUX_MU_LSR_REG & 0x20));
    
    asm volatile("": : :"memory");

    /* Write character to the output FIFO */
    *AUX_MU_IO_REG = c;
    
    asm volatile("": : :"memory");
}

void debug_puts(char *s)
{
    while (*s != '\0') {
        putc(*s++);
    }
}
