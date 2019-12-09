/* Copyright (c) 2019, Breakaway Consulting Pty. Ltd. */

typedef uint8_t CoreId;
typedef bool Spinlock;

#define CORE_ID_0 ((CoreId) UINT8_C(0))
#define CORE_ID_INVALID ((CoreId) UINT8_C(0xFF))

#define CORE_ID_MASK 0x7

static inline CoreId get_core_id(void)
{
    uint64_t r;
    asm volatile ("mrs %0, mpidr_el1" : "=r"(r));
    return r & CORE_ID_MASK;
}

static inline void write_tcr_el2(uint64_t r) {
    asm volatile ("msr tcr_el2, %0" : : "r"(r));
}

static inline void write_mair_el2(uint64_t r) {
    asm volatile ("msr mair_el2, %0" : : "r"(r));
}

static inline void write_ttbr0_el2(uint64_t r) {
    asm volatile ("msr ttbr0_el2, %0" : : "r"(r));
}

static inline uint64_t read_sctlr_el2(void) {
    uint64_t r;
    asm volatile ("mrs %0, sctlr_el2" : "=r"(r));
    asm volatile("dsb sy");
    asm volatile("isb");

    return r;
}

static inline void write_sctlr_el2(uint64_t r) {
    asm volatile ("msr sctlr_el2, %0" : : "r"(r));
}

static inline void write_vbar_el2(uint64_t r)
{
    asm volatile ("msr vbar_el2, %0" : : "r"(r));
}

static inline void write_hcr_el2(uint64_t r)
{
    asm volatile ("msr hcr_el2, %0" : : "r"(r));
}

static inline void write_daif(uint64_t r) {
    asm volatile ("msr daif, %0" : : "r"(r));
}

void spinlock_acquire(Spinlock *l);
void spinlock_release(Spinlock *l);