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

#include <stddef.h>
#include <stdint.h>

#include "rtos-acamar.h"
#include "debug.h"

void fn_a(void);
void fn_b(void);
void fn_task_switch_profiling(RtosTaskId from, RtosTaskId to);

void
fn_a(void)
{
    for (;;)
    {
        rtos_yield_to(1);
        debug_println("task a");
    }
}

void
fn_b(void)
{
    for (;;)
    {
        rtos_yield_to(0);
        debug_println("task b");
    }
}

void
fn_task_switch_profiling(RtosTaskId from, RtosTaskId to)
{
    debug_print("\nTask switch from ");
    debug_printhex8(from);
    debug_print(" to ");
    debug_printhex8(to);
    debug_print("\n");
}

int
main(void)
{
    rtos_start();
    for (;;) ;
}
