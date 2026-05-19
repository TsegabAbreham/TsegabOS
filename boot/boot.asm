bits 32

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0x00)

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text
global _start
_start:
    mov esp, stack_top
    push ebx        ; multiboot info pointer
    push eax        ; multiboot magic
    extern kernel_main
    call kernel_main
.hang:
    cli
    hlt
    jmp .hang

; For frame buffer mode go to guiboot.asm