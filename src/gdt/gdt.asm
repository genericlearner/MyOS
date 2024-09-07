section .asm 
global gdt_load 

gdt_load:
    mov eax, [esp + 4] ; Load the address of the GDT
    mov [gdt_descriptor + 2], eax ; Load the address of the GDT
    mov ax, [esp + 8] ; Load the size of the GDT
    mov [gdt_descriptor], ax ; Load the size of the GDT
    lgdt [gdt_descriptor] ; Load the GDT
    ret 

section .data
gdt_descriptor:
    dw 0x00 ; Size of the GDT
    dd 0x00 ; Address start of the GDT
