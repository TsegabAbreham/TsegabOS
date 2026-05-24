bits 32

section .multiboot
align 4
    dd 0x1BADB002           ; magic
    dd 0x00000007           ; flags: bit0 (align) + bit1 (meminfo) + bit2 (video)
    dd -(0x1BADB002 + 0x00000007) ; checksum

    ; These are only used when bit 2 of flags is set (video mode request)
    dd 0    ; header_addr   (0 = use ELF info)
    dd 0    ; load_addr
    dd 0    ; load_end_addr
    dd 0    ; bss_end_addr
    dd 0    ; entry_addr

    ; Video mode request
    dd 0        ; mode_type: 0 = linear graphics
    dd 1024     ; width
    dd 768      ; height
    dd 32       ; depth (bpp)

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text
global _start
_start:
    mov esp, stack_top
    push ebx
    push eax
    extern kernel_main
    call kernel_main
.hang:
    cli
    hlt
    jmp .hang