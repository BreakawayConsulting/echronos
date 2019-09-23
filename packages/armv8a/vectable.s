/*
 * This module provides a valid vector table for ARMv8A. It should be installed
 * by platform code by writing to the appropriate VBAR register.
 *
 * The behaviour of the vector table is currently hard-coded.
 *
 * An EL2 IRQ exception shall call the irq_handler function (to be provided by
 * platform code).
 *
 * All other exception shall call an abort_handler function (to be provided by
 * platform code).
 *
 * The aborts are non-recoverable (the abort_handler must not return). The C
 * function descriptor for the abort handler is:
 *
 * void abort_handler(uint64_t type, uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far) __attribute__ ((noreturn))
 *
 * The esr, elr, spsr, and far as passed from the EL2 version of these system registers.
 * The type value identifies which exception occurred (see source for details).
 *
 * The irq_handler function is recoverable. Prior to the irq_handler being called all
 * register state is pushed to the stack. The irq_handler should not return direclty, but
 * instead tail call the provided _asm_return_from_irq function.
 *
 * The C function description for irq_handler is:
 *
 * void irq_handler(uint64_t spsr, uint64_t elr, uint64_t sp) __attribute__ ((noreturn))
 *
 * The spsr, elr and sp parameters contain the values from the equivalent registers.
 * These values should be passed to _asm_return_from_irq to correctly restore state.
 * The _asm_return_from_irq function description is:
 *
 * void _asm_return_from_irq(uint64_t spsr, uint64_t elr, uint64_t sp) __attribute__ ((noreturn));
 *
 * This function will restore the state of the code running prior to the interrupt. It
 * restores state from the stack and restors the SPSR and ELR registers directly.
 */

.global _vector_table

        .align 11
_vector_table:
        /* EL0 exception entry points.
           These can't happen as we never enable EL0, but just in case something weird
           happens we call the abort handler. Note: The abort handler never returns.
        */

        /* syncrhonous */
        MOV     x0, #0
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align  7

        /* IRQ */
        MOV     x0, #1
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align 7

        /* FIQ */
        .align  7
        MOV     x0, #2
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align  7

        /* SError */
        MOV     x0, #3
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align  7

        /* EL2 exception entry points. These exceptions can occur. Only the IRQ
           exception is hanlded; all others route to abort_handler. */

        /* synchronous */
        MOV     x0, #4
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align  7

        /* IRQ */

        /* Save register state to the stack */
        SUB     sp, sp, #256
        STP     x0, x1, [sp]
        STP     x2, x3, [sp, #16]
        STP     x4, x5, [sp, #32]
        STP     x6, x7, [sp, #48]
        STP     x8, x9, [sp, #64]
        STP     x10, x11, [sp, #80]
        STP     x12, x13, [sp, #96]
        STP     x14, x15, [sp, #112]
        STP     x16, x17, [sp, #128]
        STP     x18, x19, [sp, #144]
        STP     x20, x21, [sp, #160]
        STP     x22, x23, [sp, #176]
        STP     x24, x25, [sp, #192]
        STP     x26, x27, [sp, #208]
        STP     x28, x29, [sp, #224]
        STP     x30, xzr, [sp, #240]

        MRS     x0, spsr_el2
        MRS     x1, elr_el2
        MOV     x2, sp

        BL      irq_handler
        .align  7

        /* FIQ */
        MOV     x0, #6
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align  7

        /* SError */
        MOV     x0, #7
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align  7

        /* Lower level exception from AArch64. These are unexpected, routed to the abort handler. */

        /* synchronous */
        MOV     x0, #8
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align  7

        /* IRQ */
        MOV     x0, #9
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align 7

        /* FIQ */
        .align  7
        MOV     x0, #10
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align  7

        /* SError */
        MOV     x0, #11
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align  7

        /* Lower level exception from AArch32. These are unexpected; routed to the abort handler */
        /* syncrhonous */
        MOV     x0, #12
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align  7

        /* IRQ */
        MOV     x0, #13
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align 7

        /* FIQ */
        .align  7
        MOV     x0, #14
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align  7

        /* SError */
        MOV     x0, #15
        MRS     x1, esr_el2
        MRS     x2, elr_el2
        MRS     x3, spsr_el2
        MRS     x4, far_el2
        B       abort_handler
        .align  7


.global _asm_return_from_irq
.type entry,#function
_asm_return_from_irq:
        MSR     spsr_el2, x0
        MSR     elr_el2, x1
        MOV     sp, x2

        LDP     x0, x1, [sp]
        LDP     x2, x3, [sp, #16]
        LDP     x4, x5, [sp, #32]
        LDP     x6, x7, [sp, #48]
        LDP     x8, x9, [sp, #64]
        LDP     x10, x11, [sp, #80]
        LDP     x12, x13, [sp, #96]
        LDP     x14, x15, [sp, #112]
        LDP     x16, x17, [sp, #128]
        LDP     x18, x19, [sp, #144]
        LDP     x20, x21, [sp, #160]
        LDP     x22, x23, [sp, #176]
        LDP     x24, x25, [sp, #192]
        LDP     x26, x27, [sp, #208]
        LDP     x28, x29, [sp, #224]
        LDP     x30, xzr, [sp, #240]

        ADD     sp, sp, #256

        ERET
