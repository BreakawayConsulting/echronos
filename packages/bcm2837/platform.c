/* Copyright (c) 2019, Breakaway Consulting Pty. Ltd. */

/*
 * The module provides basic platform support for the BCM2837 chip.
 *
 * Primarily this means support for interrupt handling and routing.
 *
 * The bcm2837_init function should be called prior to the main entry
 * point to correctly initialize the vector table and and interrupt handling.
 * This is normally done by setting the platform_init configuration point in
 * the ARMv8A startup module.
 *
 * The boot-loader on the BCM2837 starts executing code in EL2 mode, so it is
 * necessary to configure it appropriately at start up.
 *
 * Currently this requires:
 *  1/ Setting up a vector table that will take any IRQs. (initialize_vector_table)
 *  2/ Initialize the EL2 environment to appropriately route exceptions (initialize_exceptions).
 *
 * The vector table routes EL2 IRQ exceptions to the irq_handler function. The irq_handler function
 * in the current implementation has a hard-coded handlingof the timer tick, which is routed to
 * tick_irq.
 *
 * All other exceptions are routed to the abort_handler function. The abort_handler implementation
 * relies on the debug module to print appropriate useful information about the exception received.
 *
 */

/*<module>
  <code_gen>template</code_gen>
  <schema>
    <entry name="tick_handler" type="c_ident" optional="true" />
  </schema>
</module>*/

#include <stdint.h>

#include "system.h"
#include "debug.h"

typedef uint8_t CoreId;

#define IRQ_SOURCE (*(volatile uint32_t (*)[4])(0x40000060))

#define TIMER_IRQ_SOURCE 2
#define CORE_ID_MASK 0x7

/*
 * Note: The number of cores on the BCM platform is fixed at 4, so it is
 * reasonable to hard-code here.
 */
#define CORE_ID_0 ((CoreId) UINT8_C(0))
#define CORE_ID_1 ((CoreId) UINT8_C(1))
#define CORE_ID_2 ((CoreId) UINT8_C(2))
#define CORE_ID_3 ((CoreId) UINT8_C(3))

#define CORE_COUNT 4

/* Export from the vectable assembly module. */
extern uint64_t _vector_table;
extern void _asm_entry(void) __attribute__ ((noreturn));
extern void _asm_return_from_irq(uint64_t spsr, uint64_t elr, uint64_t sp) __attribute__ ((noreturn));

extern void tick_irq(void);

static inline CoreId get_core_id(void)
{
    uint64_t r;
    asm volatile ("mrs %0, mpidr_el1" : "=r"(r));
    return r & CORE_ID_MASK;
}

static void initialize_vector_table(void)
{
    write_vbar_el2((uint64_t) &_vector_table);
}

static void initialize_exceptions(void)
{
    /* Initialize EL2 hypervisor configuration register */
    /* Bits sets:
     * E2H [34] Facilities to host an OS are enabled in EL2.
     * RW  [31] EL1 is AArch64
     * TGE [27] Trap exceptions (ensure we get interrupts)
     * IMO [ 4] Physical interrupts routed to EL2.
     */
    write_hcr_el2(1L << 31 | 1L << 27 | 1L << 34 | 1L << 4);

    /* Mask all debug Debug, SError and FIQ. Leave IRQ enabled */
    write_daif(1 << 9 | 1 << 8 | 1 << 6);
}

static void start_secondary_cores(void)
{
    asm volatile("sev");
}

/* The bootloader holds cores other than 0 in a WFE (wait-for-event) state. The cores
 * can be released using the SEV instruction. After waking the core jumps to the address
 * in the spinboot variable (which is at a fixed 0xd8 address in memory.
 *
 * The spinboot variable is initialized so that all cores start at _asm_entry
 * which is the standard entry point.
 */
static void
initialize_smp_startpen(void)
{
    /* Initialize spinboot variable *before* enabling
       cache. This makes life simpler than having to flush/invalidate
       cache later */
    uint64_t *spinboot = (uint64_t*)0xd8;

    for (CoreId i = CORE_ID_1; i < CORE_COUNT; i++) {
        spinboot[i] = (uint64_t)_asm_entry;
    }
}

void bcm2837_init(void)
{
    CoreId core = get_core_id();

    initialize_vector_table();
    initialize_exceptions();

    if (core == CORE_ID_0)
    {
        initialize_smp_startpen();
        start_secondary_cores();
    }
}

__attribute__ ((noreturn)) void abort_handler(uint64_t type, uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far)
{
    debug_print("Abort Handler: type=");
    debug_printhex64(type);
    debug_print(" esr=");
    debug_printhex64(esr);
    debug_print(" elr=");
    debug_printhex64(elr);
    debug_print(" spsr=");
    debug_printhex64(spsr);
    debug_print(" far=");
    debug_printhex64(far);
    debug_println("");
    for (;;) {}
}

 __attribute__ ((noreturn)) void irq_handler(uint64_t spsr, uint64_t elr, uint64_t sp)
{
{{#tick_handler}}
    uint32_t source = IRQ_SOURCE[0];

    if (source == TIMER_IRQ_SOURCE)
    {
        {{tick_handler}}();
    }
{{/tick_handler}}
    /* Other interrupt sources are currently ignored. */

    _asm_return_from_irq(spsr, elr, sp);
}
