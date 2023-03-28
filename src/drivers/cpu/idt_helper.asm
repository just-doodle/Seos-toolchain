[global flush_idt]

flush_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret