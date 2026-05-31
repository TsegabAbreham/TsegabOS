global gdt_load

gdt_load:
    mov eax, [esp + 4]  ; get the gdtr pointer argument from the stack
    lgdt [eax]          ; load into GDTR reg

    ; reload data segment registers with our new kernel data selector
    ; 0x10 = entry 2 in GDT (index 2 * 8 bytes = 16 = 0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; reload code segment 
    jmp 0x08:flush

flush:
    ret