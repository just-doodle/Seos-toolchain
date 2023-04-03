[GLOBAL rdrand]
[GLOBAL check_rdrand]

check_rdrand:
    mov eax, 1     ; set EAX to request function 1
    mov ecx, 0     ; set ECX to request subfunction 0
    cpuid
    shr ecx, 30    ; the result we want is in ECX...
    and ecx, 1     ; ...test for the flag of interest...
    cmp ecx, 1
    je rdrand_avail
    xor eax, eax
    ret

rdrand_avail:
    mov eax, 1
    ret

rdrand:
    mov ecx, 100
    .retry:
        rdrand eax
        jc .done
        loop .retry
    .fail:
    .done:
        ret