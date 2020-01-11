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
#include "platform.h"

void tick_irq(void);
void fatal(RtosErrorId error_id);

uint32_t ticks[3];

volatile uint32_t local_timer_count;

void
tick_irq(void)
{
    ticks[get_core_id()]++;
    machine_timer_tick_isr();
    rtos_timer_tick();
}

void
local_timer_irq(void)
{
    debug_print("\nTICK\n");
    local_timer_count += 1;
    rtos_interrupt_event_raise(RTOS_INTERRUPT_EVENT_ID_TG2_LOCAL_TIMER);
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
    volatile int i;
    debug_println("[TG1] task a: starting");
    for (;;) {
        debug_println("[TG1] task a: waiting for taskgroup wakeup event");
        rtos_signal_wait(RTOS_SIGNAL_ID_WAKEUP);
        debug_println("[TG1] task a: done waiting waking up lo and tg5");
        rtos_taskgroup_event_raise(RTOS_TASKGROUP_EVENT_ID_WAKEUPLO);
        rtos_taskgroup_event_raise(RTOS_TASKGROUP_EVENT_ID_WAKEUP_TG5);
        debug_println("[TG1] task a: spinning for a bit");
        for (i = 0; i < 100000; i++) {}
        debug_println("[TG1] task a: finished spinning");


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

    bcm2837_local_timer_init();

    rtos_task_start(RTOS_TASK_ID_TG2_B);
    rtos_task_start(RTOS_TASK_ID_TG2_C);
    rtos_task_start(RTOS_TASK_ID_TG2_D);

    for (;;)
    {
        volatile int i;
        for (i = 0; i < 10000000; i++) {}
        debug_print("[TG2] task a: yield (");
        debug_printhex32(ticks[get_core_id()]);
        debug_println(")");
        rtos_yield();
        debug_println("[TG2] task a: raising taskgroup wakeup event");
        rtos_taskgroup_event_raise(RTOS_TASKGROUP_EVENT_ID_WAKEUP);
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

        rtos_signal_wait(RTOS_SIGNAL_ID_TG2_LOCAL_TIMER);

        debug_print("[TG2] task b: local timer count: ");
        debug_printhex32(local_timer_count);
        debug_println("");
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
fn_tg2_d(void)
{
   debug_println("[TG2] task d: started");
    for (;;)
    {
        debug_println("[TG2] task d: waiting for taskgroup wakeuplo event");
        rtos_signal_wait(RTOS_SIGNAL_ID_WAKEUPLO);
        debug_println("[TG2] task d: done waiting");
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

void
fn_tg5_a(void)
{
    debug_println("[TG5] task a: started");
    for (;;)
    {
        debug_println("[TG5] task a: waiting for taskgroup wakeup_tg5 event");
        rtos_signal_wait(RTOS_SIGNAL_ID_WAKEUP_TG5);
        debug_println("[TG5] task a: done waiting");
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
