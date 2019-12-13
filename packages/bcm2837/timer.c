/* Copyright (c) 2019, Breakaway Consulting Pty. Ltd. */

/*<module>
    <code_gen>template</code_gen>
    <headers>
        <header path="../rtos-example/machine-timer.h" code_gen="template" />
    </headers>
</module>*/
#include <stdbool.h>
#include <stdint.h>

#include "system.h"
#include "machine-timer.h"

#define TIMERS_INTERRUPT_CONTROL (*(volatile uint32_t (*)[4])(0x40000040))

/* Currently this is an arbitrary time. Future would be to configure this is
 * appropriate units based on reading from CPU registers.
 */
#define TIMER_INTERVAL 0x100000

static inline void write_cntp_ctl_el0(uint64_t r) {
    asm volatile ("msr cntp_ctl_el0, %0" : : "r"(r));
}

static inline void write_cntp_tval_el0(uint64_t r) {
    asm volatile ("msr cntp_tval_el0, %0" : : "r"(r));
}

void
machine_timer_start(__attribute__((unused)) void (*application_timer_isr)(void))
{
    /* Enable the timer interrupt (on the current core) */
    TIMERS_INTERRUPT_CONTROL[get_core_id()] = 0x2;

    write_cntp_ctl_el0(1); /* enabled, not masked */
    write_cntp_tval_el0(TIMER_INTERVAL);
}


void
machine_timer_stop(void)
{
    /* not implemented because unused */
}

void
machine_timer_tick_isr(void)
{
    write_cntp_tval_el0(TIMER_INTERVAL);
}
