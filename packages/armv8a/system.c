/* Copyright (c) 2019, Breakaway Consulting Pty. Ltd. */
/*
 * The system module provides access to the low-level registers on the ARMv8A,
 * and provide other system specific constants.
 *
 * The functionality in this module is primarily provided through the header
 * file.
 */
/*<module>
  <code_gen>template</code_gen>
  <headers>
      <header path="system.h" code_gen="template" />
  </headers>
  <schema>
  </schema>
</module>*/

#include <stdbool.h>
#include <stdint.h>

#include "system.h"

void spinlock_acquire(Spinlock *l)
{
    while (__atomic_test_and_set(l, __ATOMIC_ACQUIRE)) {
        asm volatile("wfe");
    }
}

/* Release the lock */
void spinlock_release(Spinlock *l)
{
    __atomic_clear(l, __ATOMIC_RELEASE);
    asm volatile("sev");
}
