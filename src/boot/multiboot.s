.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set VIDEO, 1<<2
.set ELFSHDR, 1<<4
.set FLAGS, ALIGN | MEMINFO | VIDEO
.set MAGIC, 0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)
.set VBE_GRAPHIC_MODE, 0
.set VBE_WIDTH, 1024
.set VBE_HEIGHT, 768
.set VBE_BPP, 32

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.long 0x00000000
.long 0x00000000
.long 0x00000000
.long 0x00000000
.long 0x00000000

.long VBE_GRAPHIC_MODE
.long VBE_WIDTH
.long VBE_HEIGHT
.long VBE_BPP
