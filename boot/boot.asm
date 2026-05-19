; =============================================================================
; boot.asm — Multiboot bootloader
;
; This is the FIRST code that runs when your OS boots. GRUB (or QEMU directly)
; looks for a "multiboot header" in the first 8KB of your kernel binary. When
; it finds it, it loads your kernel into memory and jumps here.
;
; We're using the Multiboot 1 standard which is simpler than Multiboot 2.
; =============================================================================

; -----------------------------------------------------------------------------
; Multiboot header constants
; GRUB scans for these magic numbers to identify a multiboot-compliant kernel.
; -----------------------------------------------------------------------------
MBALIGN     equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO     equ  1 << 1            ; provide memory map from BIOS
FLAGS       equ  MBALIGN | MEMINFO ; combine both flags
MAGIC       equ  0x1BADB002        ; magic number GRUB looks for
CHECKSUM    equ -(MAGIC + FLAGS)   ; checksum: magic + flags + checksum must = 0

; -----------------------------------------------------------------------------
; Multiboot header section
; Must be in the first 8KB of the binary and 4-byte aligned.
; -----------------------------------------------------------------------------
section .multiboot
align 4
    dd MAGIC       ; magic number — tells GRUB this is a multiboot kernel
    dd FLAGS       ; feature flags we want from GRUB
    dd CHECKSUM    ; checksum to verify the header isn't corrupted

; -----------------------------------------------------------------------------
; Stack setup
; The CPU needs a stack before we can call any C functions.
; C functions use the stack for local variables, return addresses, etc.
; We reserve 16KB here — plenty for a simple kernel.
; -----------------------------------------------------------------------------
section .bss
align 16
stack_bottom:
    resb 16384     ; reserve 16384 bytes (16KB) — uninitialized, filled with 0
stack_top:         ; stack grows DOWNWARD in x86, so top = high address

; -----------------------------------------------------------------------------
; Kernel entry point
; This is where GRUB jumps to. We must set up the stack then call our C kernel.
; -----------------------------------------------------------------------------
section .text
global _start      ; make _start visible to the linker
_start:
    ; Point the stack pointer (ESP) to the top of our reserved stack space.
    ; x86 stack grows downward, so we start at the TOP (high address).
    mov esp, stack_top

    ; Call our C kernel entry point — kernel_main() in kernel.c
    ; At this point:
    ;   - We're in 32-bit protected mode (GRUB set this up for us)
    ;   - Interrupts are disabled
    ;   - The stack is set up and ready
    extern kernel_main
    call kernel_main

    ; If kernel_main ever returns (it shouldn't), halt the CPU.
    ; This is a safety net — we disable interrupts then loop forever.
.hang:
    cli            ; disable interrupts so we can't be woken up
    hlt            ; halt the CPU — stops execution until next interrupt
    jmp .hang      ; if an NMI wakes us, halt again
