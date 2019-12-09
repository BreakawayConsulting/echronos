/* Copyright (c) 2019, Breakaway Consulting Pty. Ltd. */

.global _asm_taskgroup_switch
.type entry,#function
_asm_taskgroup_switch:
      /* Save current task group first */
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

        MOV     x2, #0x349
        MOV     x3, x30
        MOV     x4, sp

        STR     x2, [x0]
        STR     x3, [x0, #8]
        STR     x4, [x0, #16]

        LDR     x2, [x1]
        LDR     x3, [x1, #8]
        LDR     x4, [x1, #16]
        MSR     spsr_el2, x2
        MSR     elr_el2, x3
        MOV     sp, x4

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
