.set ARCH, 0
.set MAGIC, 0xe85250d6
.set HEADER_SIZE, (multiboot2_header_end - multiboot2_header)
.set VBE_GRAPHIC_MODE, 0
.set VBE_WIDTH, 1024
.set VBE_HEIGHT, 768
.set VBE_BPP, 32

.set HEADER_TAG_FBREQ, 0x05
.set HEADER_TAG_OPTIONAL, 0x01
.set HEADER_TAG_ALIGN, 0x06

.section .multiboot
.align 8
multiboot2_header:
    .long MAGIC
    .long ARCH
    .long (multiboot2_header_end-multiboot2_header)
    .long -(MAGIC + ARCH + (multiboot2_header_end-multiboot2_header))
    .align 8

multiboot2_tag_framebuffer:
    .short HEADER_TAG_FBREQ
    .short 0
    .long (multiboot2_tag_align_module-multiboot2_tag_framebuffer)
    .long VBE_WIDTH
    .long VBE_HEIGHT
    .long VBE_BPP
    .align 8

multiboot2_tag_align_module:
    .short HEADER_TAG_ALIGN
    .short 0x00
    .long 8
    .align 8

multiboot2_tag_end:
    .short 0
    .short 0
    .long 8
    .align 8

multiboot2_header_end:
