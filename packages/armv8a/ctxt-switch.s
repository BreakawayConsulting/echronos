/*
 * eChronos Real-Time Operating System
 * Copyright (c) 2017, Commonwealth Scientific and Industrial Research
 * Organisation (CSIRO) ABN 41 687 119 230.
 *
 * All rights reserved. CSIRO is willing to grant you a licence to the eChronos
 * real-time operating system under the terms of the CSIRO_BSD_MIT license. See
 * the file "LICENSE_CSIRO_BSD_MIT.txt" for details.
 *
 * @TAG(CSIRO_BSD_MIT)
 */

/*<module>
  <code_gen>template</code_gen>
</module>*/

.section .text

/*
 * A subroutine must preserve the contents of the registers x19-x30
 */

.global rtos_internal_context_switch
.type rtos_internal_context_switch,#function
/* void rtos_internal_context_switch(context_t *to, context_t *from); */
rtos_internal_context_switch:
        /* push callee saved registers and link register to stack */
        stp     x29, x30, [sp, #-16]!
        stp     x27, x28, [sp, #-16]!
        stp     x25, x26, [sp, #-16]!
        stp     x23, x24, [sp, #-16]!
        stp     x21, x22, [sp, #-16]!
        stp     x19, x20, [sp, #-16]!

        /* save the stack-pointer to the context object */
        mov     x3, sp
        str     x3, [x1]
        /* fallthrough */

.global rtos_internal_context_switch_first
.type rtos_internal_context_switch_first,#function
/* void rtos_internal_context_switch_first(context_t *to); */
rtos_internal_context_switch_first:
        /* load new stack-pointer from context object */
        ldr     x3, [x0]
        mov     sp, x3

        /* restore callee saved registers and link register from stack */
        ldp     x19, x21, [sp], #16
        ldp     x21, x23, [sp], #16
        ldp     x23, x25, [sp], #16
        ldp     x25, x26, [sp], #16
        ldp     x27, x28, [sp], #16
        ldp     x29, x30, [sp], #16

        ret
.size rtos_internal_context_switch_first, .-rtos_internal_context_switch_first
.size rtos_internal_context_switch, .-rtos_internal_context_switch

.global rtos_internal_trampoline
.type rtos_internal_trampoline,#function
/*
 * This function does not really obey a standard C abi.
 * It is designed to be used in conjunction with the context
 * switch code for the initial switch to a particular task.
 * The tasks entry point is stored in 'x19'.
 */
rtos_internal_trampoline:
        blr x19
.size rtos_internal_trampoline, .-rtos_internal_trampoline
