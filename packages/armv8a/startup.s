/* Copyright (c) 2019, Breakaway Consulting Pty. Ltd. */
.global _asm_entry

{{^cpu_single_core}}
.section .rodata
_per_cpu_stack:
{{#cpu_init_stacks}}
        .dword _stack_top_{{idx}}
{{/cpu_init_stacks}}

{{/cpu_single_core}}


.section .text.startup
.type entry,#function

_asm_entry:

	/*
	Different bootloaders start the cluster up
	in different ways. Usually only CPU#0 would be
	active, but in case other CPUs are active we don't
	want them to actually do anything, so they are
	put into an idle loop at this point.

	Note: Assumption is made that bootloader has put
	the CPU into exception level 2 (EL2).
	*/

	/*
	Determine current CPU# (bottom 8-bits of MPIDR)
	*/
	MRS     x0, mpidr_el1
	AND     x0, x0, #255

	/*
	For CPUs greater than CPU count jump to the infinite loop.
	The number of supported CPUs is controlled by the configuration
	For single CPU fast-path is used.
	*/
{{#cpu_single_core}}
	CBNZ    x0, 4f
	LDR     x2, =_stack_top
{{/cpu_single_core}}

{{^cpu_single_core}}
	/* Check number of supported CPUs from the setup */
	CMP		x0, {{cpu_count}}
	BGE    	4f

	/*
	For supported cores set x2 to be the top of the
	CPU specific core.
	*/
	LDR     x1, =_per_cpu_stack
	LDR     x2, [x1, x0, LSL#3]

	/*
	Only perform BSS setup on Core #0 - other CPUs jump
	immediately to stack setup
	*/
	CBNZ    x0, 3f
{{/cpu_single_core}}

1:
	/*
	Clear the bss. The firmware image is usually not ELF loaded
	but raw binary loaded, so can't trust that memory is
	zeroed.
	*/
	LDR     x0, =__bss_start
	LDR     w1, =__bss_size
	CBZ     w1, 3f
2:
	STR     xzr, [x0], #8
	SUB     w1, w1, #1
	CBNZ    w1, 2b

	/* initialise stack pointer and call main() */
3:
	MOV     sp, x2
{{#platform_init}}
	BL      {{platform_init}}
{{/platform_init}}
	BL      main

	/*
	Infinite loop - reached if main() returns, or for non-configured
	cores.
	*/
4:
	WFE
	B       4b
