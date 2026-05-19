/*
 * linker.ld — Linker script
 *
 * This tells the linker how to arrange all the pieces of your kernel in memory.
 * Without this, the linker wouldn't know where to place code, data, or the
 * multiboot header — and GRUB wouldn't be able to find your kernel.
 *
 * The kernel is loaded at 1MB (0x100000). This is the standard load address
 * for x86 kernels — the first 1MB is reserved for BIOS and hardware.
 */

/* _start is our entry point — defined in boot.asm */
ENTRY(_start)

SECTIONS {
    /*
     * Start placing sections at 1MB (0x100000).
     * The first 1MB of memory is reserved:
     *   0x00000 - 0x9FFFF  = conventional memory / BIOS data area
     *   0xA0000 - 0xBFFFF  = VGA memory (including 0xB8000 text mode)
     *   0xC0000 - 0xFFFFF  = BIOS ROM
     * So 0x100000 (1MB) is the first safe place to load a kernel.
     */
    . = 1M;

    /*
     * .text — executable code section
     * All compiled functions and the bootloader code end up here.
     * KEEP(.multiboot) ensures the multiboot header isn't stripped out
     * by the linker even though nothing "calls" it directly.
     */
    .text BLOCK(4K) : ALIGN(4K) {
        KEEP(*(.multiboot))   /* multiboot header MUST come first */
        *(.text)              /* all code from all object files */
    }

    /*
     * .rodata — read-only data
     * String literals, const globals, and lookup tables go here.
     * Marked read-only so any accidental write will fault (once you have paging).
     */
    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }

    /*
     * .data — initialized read-write data
     * Global and static variables that have an initial value go here.
     * Example: int x = 5; — the value 5 is stored in .data
     */
    .data BLOCK(4K) : ALIGN(4K) {
        *(.data)
    }

    /*
     * .bss — uninitialized data
     * Global and static variables with no initial value go here.
     * The kernel stack (defined in boot.asm with 'resb') lives here.
     * The OS is responsible for zeroing this region on boot.
     * Example: int x; — no value, just reserved space in .bss
     */
    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)   /* common symbols (uninitialized globals from C) */
        *(.bss)     /* explicitly declared BSS variables */
    }

    /* Calculated the kernel end, avoids overwritting the kernel itself */
    kernel_end = .;
}
