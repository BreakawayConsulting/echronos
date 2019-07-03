/*
 * Copyright (c) 2019, Breakaway Consulting Pty. Ltd.
 */

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-bigaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_asm_entry)

SECTIONS
{
        . = {{code_addr}};

        .text :
        {
            *(.text.startup)
            *(.text)
            *(.text.unlikely)
            . = ALIGN(16);
        }

        .rodata :
        {
            *(.rodata*)
            . = ALIGN(16);
        }

        .data :
        {
            *(.data*)
            . = ALIGN(16);
        }

        __bss_start = .;
        .bss :
        {
            *(.bss*)
            *(COMMON)
            . = ALIGN(16);
        }
        __bss_size = . - __bss_start;

         .stacks : {
            . = . + {{stack_size}};
            _stack_top = .;
        }
 
        /* Debug sections */

        .debug_aranges  0 : { *(.debug_aranges) }
        .debug_pubnames 0 : { *(.debug_pubnames) }

        .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
        .debug_abbrev   0 : { *(.debug_abbrev) }
        .debug_line     0 : { *(.debug_line) }
        .debug_frame    0 : { *(.debug_frame) }
        .debug_str      0 : { *(.debug_str) }
        .debug_loc      0 : { *(.debug_loc) }
        .debug_macinfo  0 : { *(.debug_macinfo) }

        .debug_pubtypes 0 : { *(.debug_pubtypes) }
        .debug_ranges   0 : { *(.debug_ranges) }

        .debug_macro    0 : { *(.debug_macro) }

        /DISCARD/ : { *(*) }
}
