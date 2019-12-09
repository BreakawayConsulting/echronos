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

   The module can be configured to support multicore debug.
   When multicore is enabled a spinlock is used to ensure that only
   one core is printing to the debug console at a time.
   The locking/unlocking occurs automatically.
   The spinlock is acquired when attempting to print any character
   and is automatically released when a newline (\n) is printed.
   Of course, this means it is possible for a print that is never
   newline terminated to block any other cores from printing.

   The spinlock from system is used with the addition of keeping
   track of the current holder to track when a core first takes
   the spinlock.
 */

 /*<module>
  <code_gen>template</code_gen>
  <schema>
    <entry name="multicore" type="bool" default="false" />
  </schema>
</module>*/

{{#multicore}}
#include <stdbool.h>
#include <stdint.h>

#include "system.h"
{{/multicore}}

#define AUX_MU_IO_REG ((volatile unsigned int*)(0x3F215040))
#define AUX_MU_LSR_REG ((volatile unsigned int*)(0x3F215054))

{{#multicore}}
static const CoreId core_id_invalid = CORE_ID_INVALID;

static Spinlock spinlock;
static CoreId spinlock_holder = CORE_ID_INVALID;

/* Return true if this core currently has the UART lock */
static bool lock_have(void)
{
    CoreId holder;
    __atomic_load(&spinlock_holder, &holder, __ATOMIC_ACQUIRE);
    return holder == get_core_id();
}

/* Take the lock */
static void lock_acquire(void)
{
    CoreId core = get_core_id();

    spinlock_acquire(&spinlock);
    __atomic_store(&spinlock_holder, &core, __ATOMIC_RELEASE);
}

/* Release the lock */
static void lock_release(void)
{
    __atomic_store(&spinlock_holder, &core_id_invalid, __ATOMIC_RELEASE);
    spinlock_release(&spinlock);
}
{{/multicore}}

static void putc(unsigned int c)
{
{{#multicore}}
    if (!lock_have()) {
        lock_acquire();
    }
{{/multicore}}

    /* Busy loop until "Transmitter empty" bit is set */
    asm volatile("": : :"memory");
    do {
        asm volatile("": : :"memory");
    } while(!(*AUX_MU_LSR_REG & 0x20));
    asm volatile("": : :"memory");

    /* Write character to the output FIFO */
    *AUX_MU_IO_REG = c;
    asm volatile("": : :"memory");

{{#multicore}}
    if (c == '\n') {
        lock_release();
    }
{{/multicore}}
}

void debug_puts(char *s)
{
    while (*s != '\0') {
        putc(*s++);
    }
}
