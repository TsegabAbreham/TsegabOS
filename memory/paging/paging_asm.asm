global paging_load_directory
global paging_enable

paging_load_directory:
    mov eax, [esp + 4]  ; get the first argument (physical address of page directory)
    mov cr3, eax        ; load into CR3
    ret

paging_enable:
    mov eax, cr0         ; read current CR0
    or  eax, 0x80000000  ; set bit 31 (PG bit)
    mov cr0, eax         ; write it back — paging is NOW on
    ret