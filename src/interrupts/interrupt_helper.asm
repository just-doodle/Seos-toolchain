%macro ISR 2
    [GLOBAL isr%1]
    isr%1:
        cli
        push byte 0
        push byte %2
        jmp isr_common
%endmacro

ISR 0, 32
ISR 1, 33
ISR 2, 34
ISR 3, 35
ISR 4, 36
ISR 5, 37
ISR 6, 38
ISR 7, 39
ISR 8, 40
ISR 9, 41
ISR 10, 42
ISR 11, 43
ISR 12, 44
ISR 13, 45
ISR 14, 46
ISR 15, 47


[extern isr_handler]

isr_common:
    pusha
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler

    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popa

    add esp, 8
    sti
    iret