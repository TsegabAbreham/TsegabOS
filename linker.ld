OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
    . = 1M;
    kernel_start = .;

    .multiboot :
    {
        KEEP(*(.multiboot))
    }

    .text ALIGN(4K) :
    {
        *(.text*)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata*)
    }

    .data ALIGN(4K) :
    {
        *(.data*)
    }

    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss*)
    }

    kernel_end = .;

    /DISCARD/ :
    {
        *(.eh_frame)
        *(.comment)
    }
}