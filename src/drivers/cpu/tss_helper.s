.global flush_tss

flush_tss:
    mov $0x2B, %ax
    ltr %ax
    ret
