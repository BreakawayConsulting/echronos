/* Copyright (c) 2019, Breakaway Consulting Pty. Ltd. */
.global _asm_entry

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
	For CPUs other than zero, jump to the infinite loop.
	Current implementation status only supports a single
	CPU.
	*/
	CBNZ    x0, 4f
	
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
	LDR     x0, =_stack_top
	MOV     sp, x0
{{#platform_init}}
	BL      {{platform_init}}
{{/platform_init}}
	BL      main

	/*
	Infinite loop - reached if main() returns, or for CPUs other
	than zero
	*/
4:
	WFE
	B       4b
   
