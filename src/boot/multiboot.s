.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set ELFSHDR, 1<<4
.set FLAGS, 0x03
.set MAGIC, 0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM