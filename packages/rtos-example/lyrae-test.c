/*
 * Copyright (c) 2019, Breakaway Consulting Pty. Ltd.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rtos-lyrae.h"
#include "debug.h"
#include "machine-timer.h"
#include "system.h"

void tick_irq(void);
void fatal(RtosErrorId error_id);

uint32_t ticks[2];

void
tick_irq(void)
{
    ticks[get_core_id()]++;
    machine_timer_tick_isr();
    rtos_timer_tick();
}

void
fatal(const RtosErrorId error_id)
{
    interrupt_disable();

    debug_print("FATAL ERROR: ");
    debug_printhex32(error_id);
    for (;;)
    {
    }
}

void
fn_tg1_a(void)
{
    debug_println("[TG1] task a: starting");
    for (;;) {
        debug_println("[TG1] task a: sleeping for 50");
        rtos_sleep(50);
        debug_println("[TG1] task a: done sleeping");

        rtos_mutex_lock(RTOS_MUTEX_ID_TG1_TEST);
        rtos_sleep(1);
        rtos_mutex_unlock(RTOS_MUTEX_ID_TG1_TEST);
    }

    rtos_signal_wait(RTOS_SIGNAL_SET_EMPTY);
}

void
fn_tg2_a(void)
{
    debug_print("[TG2]: fn_a started - ");
    debug_print("task group id: ");
    debug_printhex8(rtos_taskgroup_current());
    debug_println("");

    rtos_task_start(RTOS_TASK_ID_TG2_B);
    rtos_task_start(RTOS_TASK_ID_TG2_C);

    for (;;)
    {
        volatile int i;
        for (i = 0; i < 10000000; i++) {}
        debug_print("[TG2] task a: yield (");
        debug_printhex32(ticks[get_core_id()]);
        debug_println(")");
        rtos_yield();
        rtos_sleep(20);
    }
}

void
fn_tg2_b(void)
{
    debug_println("[TG2] task b: started");
    for (;;)
    {
        debug_println("[TG2] task b: taking lock & sleeping for 2");
        rtos_mutex_lock(RTOS_MUTEX_ID_TG2_TEST);
        rtos_sleep(2);
        rtos_mutex_unlock(RTOS_MUTEX_ID_TG2_TEST);
        rtos_yield();
        rtos_sleep(20);
    }
}

void
fn_tg2_c(void)
{
    debug_println("[TG2] task c: started");
    for (;;)
    {
        rtos_mutex_lock(RTOS_MUTEX_ID_TG2_TEST);
        debug_println("[TG2] task c: got lock!");
        rtos_mutex_unlock(RTOS_MUTEX_ID_TG2_TEST);
        rtos_yield();
        rtos_sleep(20);
    }
}

void
fn_tg3_a(void)
{
    debug_println("[TG3] task a: starting");
    for (;;)
    {
        debug_println("[TG3] task a: sleeping for 50");
        rtos_sleep(50);
        debug_println("[TG3] task a: done sleeping");

        rtos_mutex_lock(RTOS_MUTEX_ID_TG3_TEST);
        rtos_sleep(1);
        rtos_mutex_unlock(RTOS_MUTEX_ID_TG3_TEST);
    }

    rtos_signal_wait(RTOS_SIGNAL_SET_EMPTY);
}

void
fn_tg4_a(void)
{
    debug_print("[TG4]: fn_a started - ");
    debug_print("task group id: ");
    debug_printhex8(rtos_taskgroup_current());
    debug_println("");

    rtos_task_start(RTOS_TASK_ID_TG4_B);
    rtos_task_start(RTOS_TASK_ID_TG4_C);

    for (;;)
    {
        volatile int i;
        for (i = 0; i < 10000000; i++) {}
        debug_print("[TG4] task a: yield (");
        debug_printhex32(ticks[get_core_id()]);
        debug_println(")");
        rtos_yield();
        rtos_sleep(10);
    }
}

void
fn_tg4_b(void)
{
    debug_println("[TG4] task b: started");
    for (;;)
    {
        debug_println("[TG4] task b: taking lock & sleeping for 2");
        rtos_mutex_lock(RTOS_MUTEX_ID_TG4_TEST);
        rtos_sleep(2);
        rtos_mutex_unlock(RTOS_MUTEX_ID_TG4_TEST);
        rtos_yield();
        rtos_sleep(10);
    }
}

void
fn_tg4_c(void)
{
    debug_println("[TG4] task c: started");
    for (;;)
    {
        rtos_mutex_lock(RTOS_MUTEX_ID_TG4_TEST);
        debug_println("[TG4] task c: got lock!");
        rtos_mutex_unlock(RTOS_MUTEX_ID_TG4_TEST);
        rtos_yield();
        rtos_sleep(5);
    }
}

int
main(void)
{
    machine_timer_start(tick_irq);
    debug_println("Starting RTOS");

    rtos_start();

    for (;;) {}
}
