TOOLDIR=~/sysroot/bin/
SRCDIR=src/
INCLUDEDIR=$(SRCDIR)/include/
BACKUPDIR=$(SRCDIR)/../../BACKUP/
GRUBDIR=
SCRIPTSDIR=$(SRCDIR)/scripts/
FILESDIR=files

OSTOOL_DIR=~/sysroot/usr/bin/

CC=$(TOOLDIR)/i686-elf-gcc
CFLAGS= -I$(INCLUDEDIR) -I/usr/include -nostdlib -lgcc -fno-builtin -fno-exceptions -fno-leading-underscore -ffreestanding -Wall -g -O0

CXX=$(TOOLDIR)/i686-elf-g++
CXXFLAGS=

LD=$(TOOLDIR)/i686-elf-ld
LINKERFILE=src/boot/link.ld
LDFLAGS= -T$(LINKERFILE)

AS=$(TOOLDIR)/i686-elf-as
ASFLAGS=

NASM=nasm
NASMFLAGS=-f elf32 -O0 -w+zeroing -g

OBJCOPY = i686-elf-objcopy
OBJDUMP = i686-elf-objdump

OBJCOPYFLAGS = --strip-debug --strip-unneeded

QEMU=qemu-system-i386
QEMUFLAGS=-cdrom $(ISOFILE) -m 3098M -boot d -hda sorhd -serial stdio
QEMUDFLAGS= -s -S -daemonize -m 64M

PROJECT=SectorOS-RW4

EXECUTABLE=$(PROJECT).elf
ISOFILE=$(PROJECT).iso
IMAGEFILE=sorhd

OBJECTS= 	$(SRCDIR)/boot/multiboot.o \
			$(SRCDIR)/boot/boot.o \
			$(SRCDIR)/drivers/io/ports.o \
			$(SRCDIR)/drivers/io/pci.o \
			$(SRCDIR)/drivers/io/stdout.o \
			$(SRCDIR)/drivers/power/reboot.o \
			$(SRCDIR)/drivers/cpu/gdt.o \
			$(SRCDIR)/drivers/cpu/gdt_helper.o \
			$(SRCDIR)/drivers/cpu/idt.o \
			$(SRCDIR)/drivers/cpu/idt_helper.o \
			$(SRCDIR)/drivers/cpu/tss.o \
			$(SRCDIR)/drivers/cpu/tss_helper.o \
			$(SRCDIR)/drivers/cpu/pic.o \
			$(SRCDIR)/drivers/cpu/pit.o \
			$(SRCDIR)/drivers/cpu/rdtsc.o \
			$(SRCDIR)/drivers/cpu/sse.o \
			$(SRCDIR)/drivers/cpu/cpuinfo.o \
			$(SRCDIR)/drivers/storage/atapio.o \
			$(SRCDIR)/drivers/storage/mbr.o \
			$(SRCDIR)/drivers/storage/ramdisk.o \
			$(SRCDIR)/interrupts/interrupt.o \
			$(SRCDIR)/interrupts/interrupt_helper.o \
			$(SRCDIR)/interrupts/exception.o \
			$(SRCDIR)/interrupts/exception_helper.o \
			$(SRCDIR)/interrupts/syscall.o \
			$(SRCDIR)/common/libs/debug.o \
			$(SRCDIR)/common/libs/string.o \
			$(SRCDIR)/common/libs/printf.o \
			$(SRCDIR)/common/libs/bit.o \
			$(SRCDIR)/common/libs/math.o \
			$(SRCDIR)/common/libs/rng.o \
			$(SRCDIR)/common/libs/list.o \
			$(SRCDIR)/common/libs/tree.o \
			$(SRCDIR)/common/libs/fast_memcpy.o \
			$(SRCDIR)/fs/vfs.o \
			$(SRCDIR)/fs/devfs.o \
			$(SRCDIR)/fs/mount.o \
			$(SRCDIR)/fs/sorfs.o \
			$(SRCDIR)/fs/kernelfs.o \
			$(SRCDIR)/fs/stat.o \
			$(SRCDIR)/gui/draw.o \
			$(SRCDIR)/gui/blend.o \
			$(SRCDIR)/gui/targa.o \
			$(SRCDIR)/gui/compositor.o \
			$(SRCDIR)/drivers/video/ifb.o \
			$(SRCDIR)/drivers/io/commanddev.o \
			$(SRCDIR)/gui/bitmap.o \
			$(SRCDIR)/gui/font/font.o \
			$(SRCDIR)/gui/font/font_parser.o \
			$(SRCDIR)/drivers/video/vga_text.o \
			$(SRCDIR)/drivers/video/vesa.o \
			$(SRCDIR)/drivers/video/vidtext.o \
			$(SRCDIR)/drivers/io/serial.o \
			$(SRCDIR)/drivers/time/rtc.o \
			$(SRCDIR)/drivers/input/keyboard.o \
			$(SRCDIR)/memory/kheap.o \
			$(SRCDIR)/memory/paging.o \
			$(SRCDIR)/memory/pmm.o \
			$(SRCDIR)/process/usermode.o \
			$(SRCDIR)/process/process.o \
			$(SRCDIR)/process/context_switch.o \
			$(SRCDIR)/process/spinlock.o \
			$(SRCDIR)/process/elf_loader.o \
			$(SRCDIR)/process/filedescriptor.o \
			$(SRCDIR)/kernel/shell.o \
			$(SRCDIR)/kernel/kernel.o

all: kernel iso

kernel: $(EXECUTABLE)
iso: $(ISOFILE)

$(EXECUTABLE): $(OBJECTS)
	@echo '[LD] $@'
	@$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

compile_objs: $(OBJECTS)

$(ISOFILE): $(IMAGEFILE) $(EXECUTABLE)
	@echo '[GRUB] $@'
	@mkdir -p $(PROJECT)/boot/grub
	@cp $(EXECUTABLE) $(PROJECT)/boot/
	@cp sorhd $(PROJECT)/boot/
	@echo 'set timeout=3' >> $(PROJECT)/boot/grub/grub.cfg
	@echo 'set default=0' >> $(PROJECT)/boot/grub/grub.cfg
	@echo 'set root=(cd)' >> $(PROJECT)/boot/grub/grub.cfg
	@echo '' >> $(PROJECT)/boot/grub/grub.cfg
	@echo 'menuentry "$(PROJECT)" { '>> $(PROJECT)/boot/grub/grub.cfg
	@echo 'multiboot /boot/$(EXECUTABLE) --root /dev/apio0' >> $(PROJECT)/boot/grub/grub.cfg
	@echo 'module /boot/sorhd' >> $(PROJECT)/boot/grub/grub.cfg
	@echo 'boot' >> $(PROJECT)/boot/grub/grub.cfg
	@echo '}' >> $(PROJECT)/boot/grub/grub.cfg
	@$(GRUBDIR)grub-mkrescue -o $(ISOFILE) $(PROJECT) --product-name=$(PROJECT)
	@rm -rf $(PROJECT)/


%.o : %.c
	@echo '[CC] $@'
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.s
	@echo '[GAS] $@'
	@$(AS) $(ASFLAGS) -o $@ $<

%.o : %.asm
	@echo '[NASM] $@'
	@$(NASM) $(NASMFLAGS) -o $@ $<

sorfs_compile:
	@echo '[CC] $(SRCDIR)/tools/sorfs.c => sorfs'
	@/usr/bin/gcc $(SRCDIR)/tools/sorfs.c -o sorfs -lm

$(IMAGEFILE): sorfs_compile create_test_program
	@echo '[SORFS] $@'
	./sorfs -c $@ $(wildcard $(FILESDIR)/*)

run: $(ISOFILE)
	$(QEMU) $(QEMUFLAGS) -enable-kvm -cpu host

rund: $(ISOFILE)
	$(QEMU) $(QEMUFLAGS) $(QEMUDFLAGS)

runkvmd: $(ISOFILE)
	$(QEMU) $(QEMUFLAGS) $(QEMUDFLAGS) -enable-kvm -cpu host

stripd: $(EXECUTABLE)
	@$(TOOLDIR)$(OBJCOPY) --only-keep-debug $(EXECUTABLE) debug.sym
	@$(TOOLDIR)$(OBJCOPY) $(OBJCOPYFLAGS) $(EXECUTABLE)

create_test_program:
	@echo "[i686-sectoros-gcc] files/user.elf"
	@$(OSTOOL_DIR)/i686-sectoros-gcc $(SRCDIR)/tools/user.c -o files/user.elf -g

forcerun: clean iso run
forcerund: clean iso rund

PHONY: clean kernel
clean:
	@echo 'Cleaning the source directory...'
	@rm -f $(OBJECTS) $(EXECUTABLE) $(ISOFILE) sorfs $(IMAGEFILE)

clean_objs:
	@rm -f $(OBJECTS)
