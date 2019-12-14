/* Copyright (c) 2017, Commonwealth Scientific and Research Organisation (CSIRO) */
/* Copyright (c) 2019, Breakaway Consulting Pty. Ltd. */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "system.h"

#include "rtos-lyrae.h"

#define CONTEXT_SIZE 12
#define CONTEXT_X19_IDX 0
#define CONTEXT_X20_IDX 1
#define CONTEXT_X21_IDX 2
#define CONTEXT_X22_IDX 3
#define CONTEXT_X23_IDX 4
#define CONTEXT_X24_IDX 5
#define CONTEXT_X25_IDX 6
#define CONTEXT_X26_IDX 7
#define CONTEXT_X27_IDX 8
#define CONTEXT_X28_IDX 9
#define CONTEXT_X29_IDX 10
#define CONTEXT_X30_IDX 11

#define TICKS_ABSOLUTE_MAX (({{prefix_type}}TicksAbsolute) UINT32_MAX)

#define TASK_ID_NONE ((TaskIdOption) UINT8_MAX)

#define MUTEX_ID_NONE ((MutexIdOption) UINT8_MAX)

#define ERROR_ID_NONE (({{prefix_type}}ErrorId) UINT8_C(0))
#define ERROR_ID_TICK_OVERFLOW (({{prefix_type}}ErrorId) UINT8_C(1))
#define ERROR_ID_INVALID_ID (({{prefix_type}}ErrorId) UINT8_C(2))
#define ERROR_ID_NOT_HOLDING_MUTEX (({{prefix_type}}ErrorId) UINT8_C(3))
#define ERROR_ID_DEADLOCK (({{prefix_type}}ErrorId) UINT8_C(4))
#define ERROR_ID_TASK_FUNCTION_RETURNS (({{prefix_type}}ErrorId) UINT8_C(5))
#define ERROR_ID_INTERNAL_CURRENT_TASK_INVALID (({{prefix_type}}ErrorId) UINT8_C(6))
#define ERROR_ID_INTERNAL_INVALID_ID (({{prefix_type}}ErrorId) UINT8_C(7))
#define ERROR_ID_MESSAGE_QUEUE_BUFFER_OVERLAP (({{prefix_type}}ErrorId) UINT8_C(8))
#define ERROR_ID_MESSAGE_QUEUE_ZERO_TIMEOUT (({{prefix_type}}ErrorId) UINT8_C(9))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_ZERO_TIMEOUT (({{prefix_type}}ErrorId) UINT8_C(10))
#define ERROR_ID_MESSAGE_QUEUE_INVALID_POINTER (({{prefix_type}}ErrorId) UINT8_C(11))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_TICK_OVERFLOW (({{prefix_type}}ErrorId) UINT8_C(12))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_INCORRECT_INITIALIZATION (({{prefix_type}}ErrorId) UINT8_C(13))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_VIOLATED_INVARIANT_CONFIGURATION (({{prefix_type}}ErrorId) UINT8_C(14))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_VIOLATED_INVARIANT_INVALID_HEAD (({{prefix_type}}ErrorId) UINT8_C(15))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_VIOLATED_INVARIANT_INVALID_AVAILABLE (({{prefix_type}}ErrorId) UINT8_C(16))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_VIOLATED_INVARIANT_INVALID_ID_IN_WAITERS (({{prefix_type}}ErrorId) UINT8_C(17))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_VIOLATED_INVARIANT_TASKS_BLOCKED_DESPITE_AVAILABLE_MESSAGES (({{prefix_type}}ErrorId) UINT8_C(18))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_VIOLATED_INVARIANT_WAITING_TASK_IS_NOT_BLOCKED (({{prefix_type}}ErrorId) UINT8_C(19))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_VIOLATED_INVARIANT_INVALID_MESSAGES_POINTER (({{prefix_type}}ErrorId) UINT8_C(20))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_VIOLATED_INVARIANT_INVALID_MESSAGE_SIZE (({{prefix_type}}ErrorId) UINT8_C(21))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_VIOLATED_INVARIANT_INVALID_QUEUE_LENGTH (({{prefix_type}}ErrorId) UINT8_C(22))
#define ERROR_ID_INTERNAL_PRECONDITION_VIOLATED (({{prefix_type}}ErrorId) UINT8_C(23))
#define ERROR_ID_INTERNAL_POSTCONDITION_VIOLATED (({{prefix_type}}ErrorId) UINT8_C(24))
#define ERROR_ID_SEMAPHORE_MAX_INVALID (({{prefix_type}}ErrorId) UINT8_C(25))
#define ERROR_ID_SEMAPHORE_MAX_USE_BEFORE_INIT (({{prefix_type}}ErrorId) UINT8_C(26))
#define ERROR_ID_SEMAPHORE_MAX_ALREADY_INIT (({{prefix_type}}ErrorId) UINT8_C(27))
#define ERROR_ID_SEMAPHORE_MAX_EXCEEDED (({{prefix_type}}ErrorId) UINT8_C(28))
#define ERROR_ID_MESSAGE_QUEUE_INTERNAL_VIOLATED_INVARIANT_TIMER_IS_ENABLED (({{prefix_type}}ErrorId) UINT8_C(29))
#define ERROR_ID_SCHED_PRIO_CEILING_TASK_LOCKING_LOWER_PRIORITY_MUTEX (({{prefix_type}}ErrorId) UINT8_C(30))
#define ERROR_ID_SCHED_PRIO_CEILING_MUTEX_ALREADY_LOCKED (({{prefix_type}}ErrorId) UINT8_C(31))
#define ERROR_ID_TIMER_SIGNAL_SET_IS_EMPTY (({{prefix_type}}ErrorId) UINT8_C(32))
#define ERROR_ID_MPU_INTERNAL_MISALIGNED_ADDR (({{prefix_type}}ErrorId) UINT8_C(33))
#define ERROR_ID_MPU_INTERNAL_INVALID_PTR (({{prefix_type}}ErrorId) UINT8_C(34))
#define ERROR_ID_MPU_VIOLATION (({{prefix_type}}ErrorId) UINT8_C(35))
#define ERROR_ID_MPU_ALREADY_ENABLED (({{prefix_type}}ErrorId) UINT8_C(36))
#define ERROR_ID_MPU_ALREADY_DISABLED (({{prefix_type}}ErrorId) UINT8_C(37))
#define ERROR_ID_MPU_INVALID_REGION_SIZE (({{prefix_type}}ErrorId) UINT8_C(38))
#define ERROR_ID_MPU_NON_STANDARD (({{prefix_type}}ErrorId) UINT8_C(39))
#define ERROR_ID_MPU_SANITATION_FAILURE (({{prefix_type}}ErrorId) UINT8_C(40))
#define ERROR_ID_MPU_INTERNAL_INVALID_REGION_INDEX (({{prefix_type}}ErrorId) UINT8_C(41))
#define ERROR_ID_INTERNAL_CURRENT_TASKGROUP_INVALID (({{prefix_type}}ErrorId) UINT8_C(42))

#define tasks rtos_internal_tasks

#define current_taskgroup rtos_internal_current_taskgroup

typedef uint64_t* context_t;
typedef {{prefix_type}}TaskId SchedIndex;

typedef uint32_t TicksTimeout;

{{#mutexes.length}}
typedef {{prefix_type}}MutexId MutexIdOption;
{{/mutexes.length}}

typedef {{prefix_type}}TaskId TaskIdOption;

struct sched_task {
    bool runnable;
};

struct sched {
    SchedIndex cur[{{taskgroups.length}}]; /* The index of the currently scheduled task */
    SchedIndex base[{{taskgroups.length}}];
    SchedIndex max[{{taskgroups.length}}];
    struct sched_task tasks[{{tasks.length}}];
};

struct signal_task {
    {{prefix_type}}SignalSet signals;
};

struct signal {
    struct signal_task tasks[{{tasks.length}}];
};

{{#timers.length}}
struct timer
{
    bool enabled;
    bool overflow;
    TicksTimeout expiry;
    {{prefix_type}}TicksRelative reload;

    /*
     * when error_id is not ERROR_ID_NONE, the timer calls
     * the application error function with this error_id.
     */
    {{prefix_type}}ErrorId error_id;

    {{prefix_type}}TaskId task_id;
    {{prefix_type}}SignalSet signal_set;
};
{{/timers.length}}

{{#mutexes.length}}
struct mutex {
    TaskIdOption holder;
};

{{#mutex.stats}}
struct mutex_stat {
    uint32_t mutex_lock_counter;
    uint32_t mutex_lock_contended_counter;
    {{prefix_type}}TicksRelative mutex_lock_max_wait_time;
};
{{/mutex.stats}}
{{/mutexes.length}}

struct task
{
    context_t ctx;
};

extern void rtos_internal_context_switch(context_t *, context_t *);
extern void rtos_internal_context_switch_first(context_t *);
extern void rtos_internal_trampoline(void);

extern /*@noreturn@*/ void fatal({{prefix_type}}ErrorId error_id);

{{#tasks}}
extern void {{function}}(void);
{{/tasks}}

extern void record_task_switch({{prefix_type}}TaskId from, {{prefix_type}}TaskId to);


/**
 * Set up the initial execution context of a task.
 * This function is invoked exactly once forcd ../ each task in the system.
  *
 * @param ctx An output parameter interpreted by the RTOS as the initial context for each task.
 *  After this function returns, the RTOS uses the value of ctx for task/context/stack switching.
 *  The concept of a context and of the context_t type is abstract and may have different implementations on
 *  different platforms.
 *  It can be, e.g., a stack pointer or a data structure for user-level task switching as on POSIX.
 *  This function is expected to set ctx to a value that the RTOS can pass to this platform's implementation of
 *  context_switch() and context_switch_first().
 *  The context must be set up such that the destination task of a task switch executes the code at the address fn
 *  using the memory region defined by stack_base and stack_size as its stack.
 *  For hardware platforms, this typically requires the following set up steps:
 *  - The value of ctx points to either the beginning or the end of the stack area.
 *  - The stack area contains fn so that the context-switch functions can pop it off the stack as a return address to
 *    begin execution at.
 *  - Optionally reserve additional stack space if the context-switch functions depend on it.
 * @param fn Points to a code address at which the given execution context shall start executing.
 *  This is typically a pointer to a parameter-less function that is assumed to never return.
 * @param stack_base Points to the lowest address of the memory area this execution context shall use as a stack.
 * @param stack_size The size in bytes of the stack memory area reserved for this execution context.
 */
static void context_init(context_t *const ctx, void (*const fn)(void), uint64_t *const stack_base, const size_t stack_size);

static void sched_set_runnable(const {{prefix_type}}TaskId task_id);
static void sched_set_blocked(const {{prefix_type}}TaskId task_id);
static TaskIdOption sched_get_next(void);

static {{prefix_type}}SignalSet signal_recv({{prefix_type}}SignalSet *pending_signals, {{prefix_type}}SignalSet requested_signals);
static void signal_send_set({{prefix_type}}TaskId task_id, {{prefix_type}}SignalSet signals);
static {{prefix_type}}SignalSet signal_wait_set({{prefix_type}}SignalSet requested_signals);

{{#timers.length}}
static void timer_process_one(struct timer *timer);
static void timer_enable({{prefix_type}}TimerId timer_id);
static void timer_oneshot({{prefix_type}}TimerId timer_id, {{prefix_type}}TicksRelative timeout);
{{/timers.length}}

static void timer_tick_process(void);

static {{prefix_type}}TaskId get_current_task_check(void);
static void yield_to({{prefix_type}}TaskId to);
static void block(void);
static void unblock({{prefix_type}}TaskId task);

{{#tasks}}
static uint64_t stack_{{idx}}[{{stack_size}}] __attribute__((aligned(16)));
{{/tasks}}

static struct sched sched_tasks;
static struct signal signal_tasks;
static volatile uint8_t timer_pending_ticks;
{{prefix_type}}TicksAbsolute {{prefix_func}}timer_current_ticks[{{cpus.length}}];

{{#timers.length}}
static struct timer timers[{{timers.length}}] = {
{{#timers}}
    {
        {{#enabled}}true{{/enabled}}{{^enabled}}false{{/enabled}},
        false,
        {{#enabled}}{{reload}}{{/enabled}}{{^enabled}}0{{/enabled}},
        {{reload}},
        {{error}},
        {{#task}}{{prefix_const}}TASK_ID_{{name|u}}{{/task}}{{^task}}TASK_ID_NONE{{/task}},
        {{#sig_set}}{{prefix_const}}SIGNAL_SET_{{.|u}}{{/sig_set}}{{^sig_set}}{{prefix_const}}SIGNAL_SET_EMPTY{{/sig_set}}
    },
{{/timers}}
};
{{/timers.length}}

{{#mutexes.length}}
static struct mutex mutexes[{{mutexes.length}}] = {
{{#mutexes}}
    {TASK_ID_NONE},
{{/mutexes}}
};
static MutexIdOption mutex_waiters[{{tasks.length}}] = {
{{#tasks}}
    MUTEX_ID_NONE,
{{/tasks}}
};
{{/mutexes.length}}

{{#mutexes.length}}
{{#mutex.stats}}
bool {{prefix_func}}mutex_stats_enabled;
static struct mutex_stat mutex_stats[{{mutexes.length}}];
{{/mutex.stats}}
{{/mutexes.length}}

{{#profiling}}
{{#profiling.task_uptime}}
static uint32_t profiling_task_uptimes[{{tasks.length}} + 1];
{{/profiling.task_uptime}}
{{/profiling}}

struct task rtos_internal_tasks[{{tasks.length}}];

{{#timers.length}}
static {{prefix_type}}TimerId task_timers[{{tasks.length}}] = {
{{#tasks}}
    {{prefix_const}}TIMER_ID_{{timer.name|u}},
{{/tasks}}
};
{{/timers.length}}

{{prefix_type}}TaskGroupId cpu_current_taskgroup[{{cpus.length}}];

#define rtos_internal_current_taskgroup cpu_current_taskgroup[get_core_id()]

#define context_switch(from, to) rtos_internal_context_switch(to, from)
#define context_switch_first(to) rtos_internal_context_switch_first(to)
#define sched_runnable(task_id) (SCHED_OBJ(task_id).runnable)

#define sched_get_cur_index() (sched_tasks.cur[rtos_internal_current_taskgroup])
#define sched_set_cur_index(idx) sched_tasks.cur[rtos_internal_current_taskgroup] = (idx)
#define sched_max_index() sched_tasks.max[rtos_internal_current_taskgroup]
#define sched_base_index() sched_tasks.base[rtos_internal_current_taskgroup]
#define sched_index_to_taskid(sched_index) ({{prefix_type}}TaskId)(sched_index)
#define SCHED_OBJ(task_id) sched_tasks.tasks[task_id]

static inline SchedIndex sched_next_index(SchedIndex cur)
{
    if (cur == sched_max_index())
    {
        return sched_base_index();
    }
    else
    {
        return (SchedIndex)(cur + 1);
    }
}

#define signal_wait(requested_signals) (void)signal_wait_set(requested_signals)
#define signal_peek(pending_signals, requested_signals) (((pending_signals) & (requested_signals)) != {{prefix_const}}SIGNAL_SET_EMPTY)
#define signal_pending(task_id, mask) ((PENDING_SIGNALS(task_id) & mask) == mask)
#define PENDING_SIGNALS(task_id) signal_tasks.tasks[task_id].signals
#define timer_pending_ticks_check() ((bool)timer_pending_ticks)

{{#timers.length}}
#define timer_expired(timer, timeout) ((timer)->enabled && (timer)->expiry <= timeout)
#define timer_is_periodic(timer) ((timer)->reload > 0)
#define timer_reload_set(timer_id, ticks) timers[timer_id].reload = ticks
#define timer_disable(timer_id) timers[timer_id].enabled = false
#define assert_timer_valid(timer) api_assert(timer_id >= taskgroup_timer_base[current_taskgroup] && timer_id < taskgroup_timer_end[current_taskgroup], ERROR_ID_INVALID_ID)
#define TIMER_PTR(timer_id) (&timers[timer_id])
{{/timers.length}}

#define current_timeout() ((TicksTimeout) {{prefix_func}}timer_current_ticks[get_core_id()])

{{#mutexes.length}}
#define assert_mutex_valid(mutex) api_assert(mutex >= taskgroup_mutex_base[current_taskgroup] && mutex < taskgroup_mutex_end[current_taskgroup], ERROR_ID_INVALID_ID)
{{/mutexes.length}}

{{#api_asserts}}
#define api_error(error_id) {{fatal_error}}(error_id)
#define api_assert(expression, error_id) do { if (!(expression)) { api_error(error_id); } } while(0)
{{/api_asserts}}
{{^api_asserts}}
#define api_error(error_id)
#define api_assert(expression, error_id)
{{/api_asserts}}

{{#internal_asserts}}
#define internal_error(error_id) {{fatal_error}}(error_id)
#define internal_assert(expression, error_id) do { if (!(expression)) { internal_error(error_id); } } while(0)
{{/internal_asserts}}
{{^internal_asserts}}
#define internal_error(error_id)
#define internal_assert(expression, error_id)
{{/internal_asserts}}

#define rtos_internal_api_begin()
#define rtos_internal_api_end()

#define get_current_task() get_current_task_check()
#define get_current_taskgroup() get_current_taskgroup_check()
#define get_task_context(task_id) &tasks[task_id].ctx
#define internal_assert_task_valid(task) api_assert(task >= taskgroup_task_base[current_taskgroup] && task < taskgroup_task_end[current_taskgroup], ERROR_ID_INTERNAL_INVALID_ID)
#define assert_task_valid(task) api_assert(task >= taskgroup_task_base[current_taskgroup] && task < taskgroup_task_end[current_taskgroup], ERROR_ID_INVALID_ID)
#define yield() {{prefix_func}}yield()
#define mutex_core_block_on(unused_task) {{prefix_func}}signal_wait({{prefix_const}}SIGNAL_ID__TASK_TIMER)
#define mutex_core_unblock(task) {{prefix_func}}signal_send(task, {{prefix_const}}SIGNAL_ID__TASK_TIMER)

static bool taskgroup_runnable[{{taskgroups.length}}];
static {{prefix_type}}TicksAbsolute taskgroup_wakeuptick[{{taskgroups.length}}];
static {{prefix_type}}TimerId taskgroup_timer_base[{{taskgroups.length}}];
static {{prefix_type}}TimerId taskgroup_timer_end[{{taskgroups.length}}];
static {{prefix_type}}MutexId taskgroup_mutex_base[{{taskgroups.length}}];
static {{prefix_type}}MutexId taskgroup_mutex_end[{{taskgroups.length}}];
static {{prefix_type}}TaskId taskgroup_task_base[{{taskgroups.length}}];
static {{prefix_type}}TaskId taskgroup_task_end[{{taskgroups.length}}];

static {{prefix_type}}TaskGroupId taskgroup_map[{{tasks.length}}];

static {{prefix_type}}TaskGroupId cpu_taskgroup_base[{{cpus.length}}];
static {{prefix_type}}TaskGroupId cpu_taskgroup_end[{{cpus.length}}];

extern void _asm_return_from_irq(uint64_t spsr, uint64_t elr, uint64_t sp) __attribute__ ((noreturn));
extern void rtos_internal_taskgroup_switch(uint64_t *to, uint64_t *from);
extern void rtos_internal_taskgroup_switch_first(uint64_t *to);

struct taskgroup_context {
    uint64_t spsr;
    uint64_t elr;
    uint64_t sp;
};

static struct taskgroup_context taskgroup_contexts[{{taskgroups.length}}];

void __attribute__ ((noreturn)) rtos_return_from_irq(uint64_t spsr, uint64_t elr, uint64_t sp)
{
    {{prefix_type}}TaskGroupId tg;
    {{prefix_type}}TaskGroupId next_tg = current_taskgroup;

    CoreId core = get_core_id();
    for (tg = cpu_taskgroup_base[core]; tg < cpu_taskgroup_end[core]; tg++)
    {
        if (taskgroup_runnable[tg])
        {
            next_tg = tg;
            break;
        }
    }

    if (current_taskgroup != next_tg)
    {
        taskgroup_contexts[current_taskgroup].spsr = spsr;
        taskgroup_contexts[current_taskgroup].elr = elr;
        taskgroup_contexts[current_taskgroup].sp = sp;

        current_taskgroup = next_tg;

        spsr = taskgroup_contexts[current_taskgroup].spsr;
        elr = taskgroup_contexts[current_taskgroup].elr;
        sp = taskgroup_contexts[current_taskgroup].sp;
    }

    _asm_return_from_irq(spsr, elr, sp);
}

static void
context_init(context_t *const ctx, void (*const fn)(void), uint64_t *const stack_base, const size_t stack_size)
{
    uint64_t *context;
    context = stack_base + stack_size - CONTEXT_SIZE;
    context[CONTEXT_X19_IDX] = (uint64_t) fn;
    context[CONTEXT_X30_IDX] = (uint64_t) rtos_internal_trampoline;
    *ctx = context;
}

static void
sched_set_runnable(const {{prefix_type}}TaskId task_id)
{
    SCHED_OBJ(task_id).runnable = true;
    taskgroup_runnable[taskgroup_map[task_id]] = true;
}

static void
sched_set_blocked(const {{prefix_type}}TaskId task_id)
{
    SCHED_OBJ(task_id).runnable = false;
}

static TaskIdOption
sched_get_next(void)
{
    TaskIdOption task;
    SchedIndex next = sched_get_cur_index();
    bool found = false;

    do
    {
        next = sched_next_index(next);
        found = sched_runnable(sched_index_to_taskid(next));
    } while (
        (!found)
        && (next != (sched_get_cur_index()))
        );

    if (found)
    {
        task = sched_index_to_taskid(next);
    }
    else
    {
        next = sched_max_index();
        task = TASK_ID_NONE;
    }

    return task;
}

static {{prefix_type}}SignalSet
signal_recv({{prefix_type}}SignalSet *const pending_signals, const {{prefix_type}}SignalSet requested_signals)
{
    const {{prefix_type}}SignalSet received_signals = *pending_signals & requested_signals;
    *pending_signals &= ~received_signals;

    return received_signals;
}

static void
signal_send_set(const {{prefix_type}}TaskId task_id, const {{prefix_type}}SignalSet signals)
{
    PENDING_SIGNALS(task_id) |= signals;
    unblock(task_id);
}

static {{prefix_type}}SignalSet
signal_wait_set(const {{prefix_type}}SignalSet requested_signals)
{
    {{prefix_type}}SignalSet received_signals;

    {
        {{prefix_type}}SignalSet *const pending_signals = &PENDING_SIGNALS(get_current_task());

        if (signal_peek(*pending_signals, requested_signals))
        {
            yield();
        }
        else
        {
            do
            {
                block();
            } while (!signal_peek(*pending_signals, requested_signals));
        }

        received_signals = signal_recv(pending_signals, requested_signals);
    }

    return received_signals;
}

{{#timers.length}}

static void
timer_set_expiry(struct timer *const timer, {{prefix_type}}TicksAbsolute timeout)
{
    timer->expiry = timeout;
    interrupt_disable();
    if (timeout < taskgroup_wakeuptick[current_taskgroup])
    {
        taskgroup_wakeuptick[current_taskgroup] = timeout;
    }
    interrupt_enable();
}

static void
timer_process_one(struct timer *const timer)
{
    if (timer_is_periodic(timer))
    {
        timer_set_expiry(timer, timer->expiry + timer->reload);
        timer->expiry += timer->reload;
    }
    else
    {
        timer->enabled = false;
    }

    if (timer->error_id != ERROR_ID_NONE)
    {
        fatal(timer->error_id);
    }
    else
    {
        if (signal_pending(timer->task_id, timer->signal_set))
        {
            timer->overflow = true;
        }
        RtosTaskId t = timer->task_id;
        signal_send_set(t, timer->signal_set);
    }
}

static void
timer_enable(const {{prefix_type}}TimerId timer_id)
{
    if (timers[timer_id].reload == 0)
    {
        timer_process_one(&timers[timer_id]);
    }
    else
    {
        timer_set_expiry(&timers[timer_id], current_timeout() + timers[timer_id].reload);
        timers[timer_id].enabled = true;
    }
}

static void
timer_oneshot(const {{prefix_type}}TimerId timer_id, const {{prefix_type}}TicksRelative timeout)
{
    timer_reload_set(timer_id, timeout);
    timer_enable(timer_id);
    timer_reload_set(timer_id, 0);
}

/*
 * Process all timers for the current task group to determine if any have expired
 */
static void
timers_process(void)
{
    {{prefix_type}}TicksAbsolute timeout = current_timeout();
    {{prefix_type}}TimerId timer_id;
    struct timer *timer;

    for (timer_id = taskgroup_timer_base[current_taskgroup]; timer_id < taskgroup_timer_end[current_taskgroup]; timer_id++)
    {
        timer = TIMER_PTR(timer_id);
        if (timer_expired(timer, timeout))
        {
            timer_process_one(timer);
        }
        interrupt_disable();
        if (timer->enabled && timer->expiry < taskgroup_wakeuptick[current_taskgroup])
        {
            taskgroup_wakeuptick[current_taskgroup] = timer->expiry;
        }
        interrupt_enable();
    }
}
{{/timers.length}}


static void
taskgroup_schedule(void)
{
    uint64_t *context_to;
    uint64_t *context_from = (uint64_t*)&taskgroup_contexts[current_taskgroup];

    {{prefix_type}}TaskGroupId tg;
    taskgroup_runnable[current_taskgroup] = false;
    bool found = false;

    for (tg = current_taskgroup + 1; tg < cpu_taskgroup_end[get_core_id()]; tg++)
    {
        if (taskgroup_runnable[tg])
        {
            current_taskgroup = tg;
            found = true;
            break;
        }
    }

    if (found)
    {
        context_to = (uint64_t*)&taskgroup_contexts[current_taskgroup];
        rtos_internal_taskgroup_switch(context_to, context_from);
    }
    else
    {
        interrupt_disable();
        asm volatile("wfi");
        interrupt_enable();
    }
}

static {{prefix_type}}TaskId
get_next(void)
{
    TaskIdOption next = TASK_ID_NONE;

    for (;;)
    {
{{#timers.length}}
        timers_process();
{{/timers.length}}
        next = sched_get_next();

        if (next == TASK_ID_NONE)
        {
            taskgroup_schedule();
        }
        else
        {
            break;
        }
    }

    internal_assert_task_valid(next);

    return next;
}

{{#mutexes.length}}
static bool
mutex_try_lock(const {{prefix_type}}MutexId m)
{
    const bool r = mutexes[m].holder == TASK_ID_NONE;

    if (r)
    {
        mutexes[m].holder = get_current_task();
    }

    return r;
}

{{#mutex.stats}}
static void
mutex_stats_update(const {{prefix_type}}MutexId m, const bool contended, const {{prefix_type}}TicksAbsolute
        wait_start_ticks)
{
    if ({{prefix_func}}mutex_stats_enabled) {
        mutex_stats[m].mutex_lock_counter += 1;
        if (contended) {
            {{prefix_type}}TicksRelative wait_time = ({{prefix_type}}TicksRelative)({{prefix_func}}timer_current_ticks[get_core_id()] - wait_start_ticks);

            mutex_stats[m].mutex_lock_contended_counter += 1;
            if (wait_time > mutex_stats[m].mutex_lock_max_wait_time)
            {
                mutex_stats[m].mutex_lock_max_wait_time = wait_time;
            }
        }
    }
}
{{/mutex.stats}}
{{/mutexes.length}}

static {{prefix_type}}TaskId
get_current_task_check(void)
{
    internal_assert(sched_get_cur_index() >= sched_base_index(), ERROR_ID_INTERNAL_CURRENT_TASK_INVALID);
    internal_assert(sched_get_cur_index() <= sched_max_index(), ERROR_ID_INTERNAL_CURRENT_TASK_INVALID);
    return sched_get_cur_index();
}

static {{prefix_type}}TaskGroupId
get_current_taskgroup_check(void)
{
    internal_assert(current_taskgroup < {{taskgroups.length}}, ERROR_ID_INTERNAL_CURRENT_TASKGROUP_INVALID);
    return current_taskgroup;
}

static void
yield_to(const {{prefix_type}}TaskId to)
{
    const {{prefix_type}}TaskId from = get_current_task();

    internal_assert_task_valid(to);

    {{#profiling}}
    {{#profiling.hook_for_task_switch}}
    {{hook_for_task_switch}}(from, to);
    {{/profiling.hook_for_task_switch}}
    {{/profiling}}

    sched_set_cur_index(to);
    context_switch(get_task_context(from), get_task_context(to));
}

static void
block(void)
{
    sched_set_blocked(get_current_task());
    {{prefix_func}}yield();
}

static void
unblock(const {{prefix_type}}TaskId task)
{
    sched_set_runnable(task);
}

/* entry point trampolines */
{{#tasks}}
static void
entry_{{name}}(void)
{
    {{#start}}{{prefix_func}}yield();{{/start}}
    {{^start}}{{prefix_func}}signal_wait({{prefix_const}}SIGNAL_ID__RTOS_UTIL);{{/start}}

    {{function}}();

    api_error(ERROR_ID_TASK_FUNCTION_RETURNS);
}

{{/tasks}}


{{prefix_type}}SignalSet
{{prefix_func}}signal_wait_set(const {{prefix_type}}SignalSet requested_signals)
{
    return signal_wait_set(requested_signals);
}

{{prefix_type}}SignalSet
{{prefix_func}}signal_poll_set(const {{prefix_type}}SignalSet requested_signals)
{
    {{prefix_type}}SignalSet *const pending_signals = &PENDING_SIGNALS(get_current_task());
    {{prefix_type}}SignalSet received_signals;

    received_signals = signal_recv(pending_signals, requested_signals);

    return received_signals;
}

{{prefix_type}}SignalSet
{{prefix_func}}signal_peek_set(const {{prefix_type}}SignalSet requested_signals)
{
    const {{prefix_type}}SignalSet pending_signals = PENDING_SIGNALS(get_current_task());
    return pending_signals & requested_signals;
}

void
{{prefix_func}}signal_send_set(const {{prefix_type}}TaskId task_id, const {{prefix_type}}SignalSet signals)
{
    assert_task_valid(task_id);
    signal_send_set(task_id, signals);
}

void
{{prefix_func}}sleep(const {{prefix_type}}TicksRelative ticks)
{
    timer_oneshot(task_timers[get_current_task()], ticks);
    signal_wait({{prefix_const}}SIGNAL_ID__TASK_TIMER);
}

{{#timers.length}}
void
{{prefix_func}}timer_enable(const {{prefix_type}}TimerId timer_id)
{
    assert_timer_valid(timer_id);
    timer_enable(timer_id);
}

void
{{prefix_func}}timer_disable(const {{prefix_type}}TimerId timer_id)
{
    assert_timer_valid(timer_id);

    timer_disable(timer_id);
}

void
{{prefix_func}}timer_oneshot(const {{prefix_type}}TimerId timer_id, const {{prefix_type}}TicksRelative timeout)
{
    assert_timer_valid(timer_id);
    timer_oneshot(timer_id, timeout);
}

bool
{{prefix_func}}timer_check_overflow(const {{prefix_type}}TimerId timer_id)
{
    bool r;

    assert_timer_valid(timer_id);
    r = timers[timer_id].overflow;
    timers[timer_id].overflow = false;

    return r;
}

{{prefix_type}}TicksRelative
{{prefix_func}}timer_remaining(const {{prefix_type}}TimerId timer_id)
{
    {{prefix_type}}TicksRelative remaining;

    assert_timer_valid(timer_id);

    remaining = timers[timer_id].enabled ? timers[timer_id].expiry - current_timeout() : 0;

    return remaining;
}

/* Configuration functions */
void
{{prefix_func}}timer_reload_set(const {{prefix_type}}TimerId timer_id, const {{prefix_type}}TicksRelative reload)
{
    assert_timer_valid(timer_id);

    timer_reload_set(timer_id, reload);
}

void
{{prefix_func}}timer_signal_set(const {{prefix_type}}TimerId timer_id, const {{prefix_type}}TaskId task_id, const {{prefix_type}}SignalSet signal_set)
{
    assert_timer_valid(timer_id);
    assert_task_valid(task_id);
    api_assert(signal_set != {{prefix_const}}SIGNAL_SET_EMPTY, ERROR_ID_TIMER_SIGNAL_SET_IS_EMPTY);

    timers[timer_id].error_id = ERROR_ID_NONE;
    timers[timer_id].task_id = task_id;
    timers[timer_id].signal_set = signal_set;
}

void
{{prefix_func}}timer_error_set(const {{prefix_type}}TimerId timer_id, const {{prefix_type}}ErrorId error_id)
{
    assert_timer_valid(timer_id);

    timers[timer_id].error_id = error_id;
}
{{/timers.length}}

{{#mutexes.length}}
void
{{prefix_func}}mutex_lock(const {{prefix_type}}MutexId m)
{
    bool contended = false;
    const {{prefix_type}}TicksAbsolute wait_start_ticks = {{prefix_func}}timer_current_ticks[get_core_id()];

    assert_mutex_valid(m);
    api_assert(mutexes[m].holder != get_current_task(), ERROR_ID_DEADLOCK);

    while (!mutex_try_lock(m))
    {
        contended = true;
        mutex_waiters[get_current_task()] = m;
        mutex_core_block_on(mutexes[m].holder);
    }

    mutex_stats_update(m, contended, wait_start_ticks);
}

void
{{prefix_func}}mutex_unlock(const {{prefix_type}}MutexId m)
{
    {{prefix_type}}TaskId t;

    assert_mutex_valid(m);
    api_assert(mutexes[m].holder == get_current_task(), ERROR_ID_NOT_HOLDING_MUTEX);

    for (t = taskgroup_mutex_base[current_taskgroup]; t < taskgroup_mutex_end[current_taskgroup]; t++)
    {
        if (mutex_waiters[t] == m)
        {
            mutex_waiters[t] = MUTEX_ID_NONE;
            mutex_core_unblock(t);
        }
    }

    mutexes[m].holder = TASK_ID_NONE;
}

bool
{{prefix_func}}mutex_try_lock(const {{prefix_type}}MutexId m)
{
    assert_mutex_valid(m);

    return mutex_try_lock(m);
}

/* A macro implementation would be preferable to eliminate function call overhead when compilers don't support implicit
 * inlining, but at present this would involve exposing too many implementation internals in the public API header. */
bool
{{prefix_func}}mutex_holder_is_current(const {{prefix_type}}MutexId m)
{
    assert_mutex_valid(m);
    return mutexes[m].holder == get_current_task();
}

{{#mutex.stats}}
void
{{prefix_func}}mutex_stats_clear(void)
{
    /* memset would be preferable, but string.h is not available on all platforms */
    uint8_t mutex_index;
    for (mutex_index = 0; mutex_index < 1; mutex_index += 1)
    {
        mutex_stats[mutex_index].mutex_lock_counter = 0;
        mutex_stats[mutex_index].mutex_lock_contended_counter = 0;
        mutex_stats[mutex_index].mutex_lock_max_wait_time = 0;
    }
}
{{/mutex.stats}}
{{/mutexes.length}}

{{prefix_type}}TaskId
{{prefix_func}}task_current(void)
{
    {{prefix_type}}TaskId t;
    rtos_internal_api_begin();
    t = get_current_task();
    rtos_internal_api_end();
    return t;
}

{{prefix_type}}TaskGroupId
{{prefix_func}}taskgroup_current(void)
{
    {{prefix_type}}TaskGroupId t;
    rtos_internal_api_begin();
    t = get_current_taskgroup();
    rtos_internal_api_end();
    return t;
}

void
{{prefix_func}}task_start(const {{prefix_type}}TaskId task)
{
    assert_task_valid(task);
    {{prefix_func}}signal_send(task, {{prefix_const}}SIGNAL_ID__RTOS_UTIL);
}

void
{{prefix_func}}yield(void)
{
    {{prefix_type}}TaskId to = get_next();
    yield_to(to);
}

void
{{prefix_func}}timer_tick(void)
{
    /*
        This function executes in interrupt mode.
        It does the minimum amount of work required to determine
        if a pre-emptive context switch to a different task group
        is required.
    */
    {{prefix_type}}TaskGroupId tg;

    /* First the current ticks is atomically updated */
    /* NOTE: Potential optimisation is to use atomic increment operation here,
     * which avoids requiring interrupt disable around the read
     */
    {{prefix_func}}timer_current_ticks[get_core_id()]++;

    /* Determine if any of the taskgroups should become runnable */
    /* NOTE: Could optimise by not considering taskgroups with lower priority than
     * the current task group (and then handling at taskgroup_schedule time
     */
    for (tg = cpu_taskgroup_base[get_core_id()]; tg <= cpu_taskgroup_end[get_core_id()]; tg++) {
        if ({{prefix_func}}timer_current_ticks[get_core_id()] > taskgroup_wakeuptick[tg])
        {
            taskgroup_runnable[tg] = true;
            taskgroup_wakeuptick[tg] = TICKS_ABSOLUTE_MAX;
        }
    }
}

void taskgroup_entry(void)
{
    sched_set_cur_index(sched_base_index());
    context_switch_first(get_task_context(sched_base_index()));
}

{{#taskgroups}}
uint64_t tg_stack_{{idx}}[256];
{{/taskgroups}}

{{#cpus}}
static void
start_{{idx}}(void)
{
    uint64_t *context_to;

    cpu_taskgroup_base[{{idx}}] = {{taskgroup_id_base}};
    cpu_taskgroup_end[{{idx}}] = {{taskgroup_id_end}};

{{#taskgroups}}
    taskgroup_contexts[{{idx}}].spsr = DEFAULT_SPSR;
    taskgroup_contexts[{{idx}}].elr = (uint64_t) &taskgroup_entry;
    taskgroup_contexts[{{idx}}].sp = (uint64_t) &tg_stack_{{idx}}[256];

    taskgroup_runnable[{{idx}}] = true;
    taskgroup_wakeuptick[{{idx}}] = TICKS_ABSOLUTE_MAX;

    sched_tasks.base[{{idx}}] = {{task_id_base}};
    sched_tasks.max[{{idx}}] = {{task_id_max}};

    taskgroup_timer_base[{{idx}}] = {{timer_id_base}};
    taskgroup_timer_end[{{idx}}] = {{timer_id_end}};

    taskgroup_mutex_base[{{idx}}] = {{mutex_id_base}};
    taskgroup_mutex_end[{{idx}}] = {{mutex_id_end}};

    taskgroup_task_base[{{idx}}] = {{task_id_base}};
    taskgroup_task_end[{{idx}}] = {{task_id_end}};

{{/taskgroups}}

{{#tasks}}
    context_init(get_task_context({{idx}}), entry_{{name}}, stack_{{idx}}, {{stack_size}});
    sched_set_runnable({{idx}});
    taskgroup_map[{{idx}}] = {{taskgroup_id}};

{{/tasks}}

    current_taskgroup = {{taskgroup_id_base}};

    context_to = (uint64_t*)&taskgroup_contexts[current_taskgroup];
    rtos_internal_taskgroup_switch_first(context_to);
}
{{/cpus}}

void
{{prefix_func}}start(void)
{
    switch(get_core_id()) {
{{#cpus}}
        case {{idx}}: return start_{{idx}}();
{{/cpus}}
    }
}