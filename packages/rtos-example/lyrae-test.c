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

/* GPIO block register definitions */
#define GPFSEL (*(volatile uint32_t (*)[5])(0x3F200000))
#define GPLEV (*(volatile uint32_t (*)[2])(0x3F200034))
#define GPEDS (*(volatile uint32_t (*)[2])(0x3F200040))
#define GPREN (*(volatile uint32_t (*)[2])(0x3F20004C))
#define GPFEN (*(volatile uint32_t (*)[2])(0x3F200058))
#define GPHEN (*(volatile uint32_t (*)[2])(0x3F200064))
#define GPLEN (*(volatile uint32_t (*)[2])(0x3F200070))
#define GPAREN (*(volatile uint32_t (*)[2])(0x3F20007C))
#define GPAFEN (*(volatile uint32_t (*)[2])(0x3F200088))

#define GPIO_PIN_23_MASK ((uint32_t) 0x00800000)
#define GPIO_PIN_23_FSEL_MASK ((uint32_t) 0x00000e00)

/* Interrupt block register definitions */
#define IRQ_BASIC_PENDING (*(volatile uint32_t *)(0x3F00B200))
#define IRQ_PENDING_1 (*(volatile uint32_t *)(0x3F00B204))
#define IRQ_PENDING_2 (*(volatile uint32_t *)(0x3F00B208))
#define FIQ_CONTROL (*(volatile uint32_t *)(0x3F00B20C))
#define ENABLE_IRQS_1 (*(volatile uint32_t *)(0x3F00B210))
#define ENABLE_IRQS_2 (*(volatile uint32_t *)(0x3F00B214))
#define ENABLE_BASIC_IRQS (*(volatile uint32_t *)(0x3F00B218))
#define DISABLE_IRQS_1 (*(volatile uint32_t *)(0x3F00B21C))
#define DISABLE_IRQS_2 (*(volatile uint32_t *)(0x3F00B220))
#define DISABLE_BASIC_IRQS (*(volatile uint32_t *)(0x3F00B224))

#define BASIC_PENDING_ARM_TIMER_IRQ (1U << 0)
#define BASIC_PENDING_ARM_MAILBOX_IRQ (1U << 1)
#define BASIC_PENDING_ARM_DOORBELL_0_IRQ (1U << 2)
#define BASIC_PENDING_ARM_DOORBELL_1_IRQ (1U << 3)
#define BASIC_PENDING_GPU0_HALTED_IRQ (1U << 4)
#define BASIC_PENDING_GPU1_HALTED_IRQ (1U << 5)
#define BASIC_PENDING_ILLEGAL_ACCESS_TYPE1_IRQ (1U << 6)
#define BASIC_PENDING_ILLEGAL_ACCESS_TYPE0_IRQ (1U << 7)
#define BASIC_PENDING_PENDING_REGISTER_1 (1U << 8)
#define BASIC_PENDING_PENDING_REGISTER_2 (1U << 9)

#define IRQ_MASK_GPIO_0 (1U << 17)
#define IRQ_MASK_GPIO_1 (1U << 18)
#define IRQ_MASK_GPIO_2 (1U << 19)
#define IRQ_MASK_GPIO_3 (1U << 20)


void tick_irq(void);
void fatal(RtosErrorId error_id);

uint32_t ticks[2];

uint32_t gpio_count;
volatile uint32_t local_timer_count;

void
tick_irq(void)
{
    ticks[get_core_id()]++;
    machine_timer_tick_isr();
    rtos_timer_tick();
}

void
gpu_irq(void)
{
    uint32_t basic_pending = IRQ_BASIC_PENDING;

    if (basic_pending & BASIC_PENDING_PENDING_REGISTER_2)
    {
        uint32_t pending_2 = IRQ_PENDING_2;
        if (pending_2 & IRQ_MASK_GPIO_0)
        {
            uint32_t event = GPEDS[0];
            if (event & GPIO_PIN_23_MASK)
            {
                gpio_count++;
                rtos_interrupt_event_raise(RTOS_INTERRUPT_EVENT_ID_TG2_GPIO);
            }
            GPEDS[0] = event;
        }
    }
}

void
local_timer_irq(void)
{
    local_timer_count += 1;
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

static void
gpio_dump_registers(void)
{
    /* Display the values of all the GPIO registers */
    int i;

    for (i = 0; i < 5; i++)
    {
        debug_print("GPFSEL_");
        debug_printhex8(i);
        debug_print(" = ");
        debug_printhex32(GPFSEL[i]);
        debug_println("");
    }

    for (i = 0; i < 2; i++)
    {
        debug_print("GPREN_");
        debug_printhex8(i);
        debug_print(" = ");
        debug_printhex32(GPREN[i]);
        debug_println("");
    }

    for (i = 0; i < 2; i++)
    {
        debug_print("GPFEN_");
        debug_printhex8(i);
        debug_print(" = ");
        debug_printhex32(GPFEN[i]);
        debug_println("");
    }

    for (i = 0; i < 2; i++)
    {
        debug_print("GPHEN_");
        debug_printhex8(i);
        debug_print(" = ");
        debug_printhex32(GPHEN[i]);
        debug_println("");
    }

    for (i = 0; i < 2; i++)
    {
        debug_print("GPLEN_");
        debug_printhex8(i);
        debug_print(" = ");
        debug_printhex32(GPLEN[i]);
        debug_println("");
    }

    for (i = 0; i < 2; i++)
    {
        debug_print("GPAREN_");
        debug_printhex8(i);
        debug_print(" = ");
        debug_printhex32(GPAREN[i]);
        debug_println("");
    }

    for (i = 0; i < 2; i++)
    {
        debug_print("GPAFEN_");
        debug_printhex8(i);
        debug_print(" = ");
        debug_printhex32(GPAFEN[i]);
        debug_println("");
    }

    for (i = 0; i < 2; i++)
    {
        debug_print("GPLEV_");
        debug_printhex8(i);
        debug_print(" = ");
        debug_printhex32(GPLEV[i]);
        debug_println("");
    }

    for (i = 0; i < 2; i++)
    {
        debug_print("GPEDS_");
        debug_printhex8(i);
        debug_print(" = ");
        debug_printhex32(GPEDS[i]);
        debug_println("");
    }
}

static void
gpio_init(void)
{
    /* Initialize GPIO subsystem to configure for button presses */
    gpio_dump_registers();

    /* set pin 23 as an input */
    GPFSEL[2] &= ~GPIO_PIN_23_FSEL_MASK;

    /* Enable rising edge detection for pin 23 (and disable other events) */
    GPREN[0] |= GPIO_PIN_23_MASK;
    GPFEN[0] &= ~GPIO_PIN_23_MASK;
    GPHEN[0] &= ~GPIO_PIN_23_MASK;
    GPLEN[0] &= ~GPIO_PIN_23_MASK;
    GPAREN[0] &= ~GPIO_PIN_23_MASK;
    GPAFEN[0] &= ~GPIO_PIN_23_MASK;

    /* enable interrupts for first GPIO block */
    ENABLE_IRQS_2 |= IRQ_MASK_GPIO_0;
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

    gpio_init();
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

        rtos_signal_wait(RTOS_SIGNAL_ID_TG2_GPIO);
        debug_print("[TG2] task b: GPIO count: ");
        debug_printhex32(gpio_count);
        debug_println("");
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
