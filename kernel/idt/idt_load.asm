
global idt_load
global isr0_stub
global isr13_stub
global isr14_stub

global irq0_stub
global irq1_stub
global irq2_stub
global irq12_stub

extern isr_handler

idt_load:
    mov eax, [esp + 4]  ; get the gdtr pointer argument from the stack
    lidt [eax]          
    ret
; --- exception stubs ---
isr0_stub: ; Division by zero
    cli
    push 0
    push 0
    jmp isr_common

isr13_stub: ; GPF
    cli
    push 13
    jmp isr_common

isr14_stub: ; Page fault
    cli
    push 14
    jmp isr_common

; --- IRQ stubs ---
irq0_stub:
    cli
    push 0
    push 32
    jmp isr_common
irq1_stub: ; Keyboard hardware interrupt
    cli
    push 0
    push 33
    jmp isr_common


irq12_stub:
    cli
    push 0
    push 44
    jmp isr_common

; --- shared handler ---
isr_common:
    pusha
    mov eax, [esp + 32]   ; int_number is 2nd push = 8 bytes past pusha (8 regs × 4 = 32)
    push eax
    call isr_handler
    add esp, 4            ; clean up the push eax
    popa
    add esp, 8            ; clean up push int_number + push error_code
    iret