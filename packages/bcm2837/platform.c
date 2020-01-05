/* Copyright (c) 2019, Breakaway Consulting Pty. Ltd. */

/*
 * The module provides basic platform support for the BCM2837 chip.
 *
 * Primarily this means support for interrupt handling and routing, pagetable
 * initialization, MMU configuration and multicore startup.
 *
 * The bcm2837_init function should be called prior to the main entry
 * point to correctly initialize the vector table, interrupt handling, pagetables,
 * MMU and secondary cores.
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
 * Although it is possible to execute with the MMU disabled (and indeed the boot-loader and init
 * code successfully execute with the MMU disabled) there are some important CPU operations that
 * will only work in cases where the CPU is running with the MMU enabled. One important example
 * of this is the ability to execute atomic instructions.
 *
 * The platform initialization creates a direct mapped pagetable for both memory and peripherals.
 * (initialize_pagetables).
 *
 * The pagetable is installed and the MMU enabled (initialize_mmu).
 */

/*<module>
  <code_gen>template</code_gen>
  <schema>
    <entry name="tick_handler" type="c_ident" optional="true" />
    <entry name="gpu_handler" type="c_ident" optional="true" />
    <entry name="irq_return" type="c_ident" default="_asm_return_from_irq" />
  </schema>
</module>*/

#include <stdbool.h>
#include <stdint.h>

#include "system.h"
#include "debug.h"

#define IRQ_SOURCE (*(volatile uint32_t (*)[4])(0x40000060))
#define GPU_INTERRUPT_ROUTING (*(volatile uint32_t *)0x4000000C)

#define TIMER_IRQ_SOURCE 0x002
#define GPU_IRQ_SOURCE 0x100

#define CORE_COUNT 4

#define PAGE_TABLE_2_ENTRY_COUNT 512
#define PAGE_TABLE_2_DEVICE_ENTRY_IDX 400 //504
#define PAGE_TABLE_BLOCK_SIZE 0x200000 /* 2MB */
#define PAGE_TABLE_2A_ENTRY_COUNT 1 /* See Quad A7 control document, first 2MB is ARM timer/IRQs/mailboxes */

/* Export from the vectable assembly module. */
extern uint64_t _vector_table;
extern void _asm_entry(void) __attribute__ ((noreturn));
extern void {{irq_return}}(uint64_t spsr, uint64_t elr, uint64_t sp) __attribute__ ((noreturn));

{{#tick_handler}}
extern void {{tick_handler}}(void);
{{/tick_handler}}

{{#gpu_handler}}
extern void {{gpu_handler}}(void);
{{/gpu_handler}}


/* Level 1 page table has a single entry covering 1GB. Points
   to level2 pagetable with 512 2MB entries.
   This covers all the useful physical address space.
   Both page tables must be 4KB aligned.
 */
static uint64_t pagetable1[2] __attribute__ ((aligned(4096)));
static uint64_t pagetable2a[PAGE_TABLE_2_ENTRY_COUNT] __attribute__ ((aligned(4096)));
static uint64_t pagetable2b[PAGE_TABLE_2_ENTRY_COUNT] __attribute__ ((aligned(4096)));

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

    for (CoreId i = CORE_ID_0 + 1; i < CORE_COUNT; i++) {
        spinboot[i] = (uint64_t)_asm_entry;
    }
}

/*
 * Create direct mapped pagetables; this allows in-place execution to
 * occur.
 *
 * Memory and peripherals are mapped appropriately.
 */
static void
initialize_pagetables(void)
{
    uint64_t addr = 0;
    int i;

    /* Make PTE in level 1 page table point to level 2 page table
     *  (3L marks it as a page table pointer)
     */
    pagetable1[0] = (uint64_t)&pagetable2a | 3L;
    pagetable1[1] = (uint64_t)&pagetable2b | 3L;

    for (i = 0; i < PAGE_TABLE_2_DEVICE_ENTRY_IDX; i++) {
        pagetable2a[i] = addr | 0x705;
        addr += PAGE_TABLE_BLOCK_SIZE;
    }

    for (; i < PAGE_TABLE_2_ENTRY_COUNT; i++) {
        pagetable2a[i] = addr | 0x701;
        addr += PAGE_TABLE_BLOCK_SIZE;
    }

    for (i = 0; i < PAGE_TABLE_2A_ENTRY_COUNT; i++) {
        pagetable2b[i] = addr | 0x701;
        addr += PAGE_TABLE_BLOCK_SIZE;
    }
    /* rest are left zeroed  - no access */
}

/*
 * Enable the MMU on the core.
 *
 * This assumes the shared pagetable structues have already been
 * initialised correctly.
 */
static void initialize_mmu(void)
{
    write_tcr_el2((1L << 31) | (1L << 23) | 0x351fL);
    write_mair_el2(0xFF00);
    write_ttbr0_el2((uint64_t)pagetable1);

    /* Actually turn on MMU for this core */
    uint64_t sctrl_el2 = read_sctlr_el2();
    sctrl_el2 |= (1 << 0) | (1 << 2) | (1 << 1) | (1 << 12);
    write_sctlr_el2(sctrl_el2);
}

static void initialize_irq_routing(void)
{
    /* All GPU interrupts (IRQ and FIQ) go to core zero */
    GPU_INTERRUPT_ROUTING = 0x0;
}

void bcm2837_init(void)
{
    CoreId core = get_core_id();

    if (core == CORE_ID_0)
    {
        initialize_smp_startpen();
        initialize_pagetables();
    }

    initialize_vector_table();
    initialize_exceptions();
    initialize_mmu();

    if (core == CORE_ID_0)
    {
        initialize_irq_routing();
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
    /* Note: Currently assumes at least one handler has been set -- if not
    this line of code will trigger a compiler warning. */
    uint32_t source = IRQ_SOURCE[get_core_id()];

{{#tick_handler}}
    if (source & TIMER_IRQ_SOURCE)
    {
        {{tick_handler}}();
    }
{{/tick_handler}}

{{#gpu_handler}}
    if (source & GPU_IRQ_SOURCE)
    {
        {{gpu_handler}}();
    }
{{/gpu_handler}}

    {{irq_return}}(spsr, elr, sp);
}
