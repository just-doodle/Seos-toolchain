ENTRY(kernel_entry)

paddr = 0x100000;
off = 0xC0000000;
vaddr = off + paddr;

SECTIONS
{
    . = vaddr;
    .text ALIGN(4K) : AT(ADDR(.text)-0xC0000000)
    {
        *(.multiboot)
        *(.text)
    }
    .rodata ALIGN (4K) : AT(ADDR(.rodata)-0xC0000000)
    {
        *(.rodata)
    }
    .bss ALIGN(4K) : AT(ADDR(.bss)-0xC0000000)
    {
        *(.bss)
        *(.initial_stack)
    }
    .data ALIGN(4K) : AT(ADDR(.data)-0xC0000000)
    {
        *(.data)
    }

    k_end = .;
    /DISCARD/ : { *(.fini_array*) *(.comment) }
}
