TOOLDIR=/opt/cross/bin/
SRCDIR=src/
INCLUDEDIR=$(SRCDIR)/include/
BACKUPDIR=$(SRCDIR)/../../BACKUP/
GRUBDIR=

CC=$(TOOLDIR)/i686-elf-gcc
CFLAGS= -I$(INCLUDEDIR) -I/usr/include -nostdlib -lgcc -fno-builtin -fno-exceptions -fno-leading-underscore -Wno-write-strings -m32 -g

CXX=$(TOOLDIR)/i686-elf-g++
CXXFLAGS=

LD=$(TOOLDIR)/i686-elf-ld
LINKERFILE=src/boot/link.ld
LDFLAGS= -T$(LINKERFILE)

AS=$(TOOLDIR)/i686-elf-as
ASFLAGS=

NASM=nasm
NASMFLAGS=-f elf32 -O0 -w+zeroing

OBJCOPY = i686-elf-objcopy
OBJDUMP = i686-elf-objdump

OBJCOPYFLAGS = --strip-debug --strip-unneeded

QEMU=qemu-system-i386
QEMUFLAGS=-cdrom SEOS.iso
QEMUDFLAGS=-serial file:serial.log -s -S -daemonize

PROJECT=SEOS

EXECUTABLE=$(PROJECT).elf
ISOFILE=$(PROJECT).iso

OBJECTS= 	$(SRCDIR)/boot/boot.o \
			$(SRCDIR)/drivers/io/ports.o \
			$(SRCDIR)/drivers/power/reboot.o \
			$(SRCDIR)/drivers/cpu/gdt.o \
			$(SRCDIR)/drivers/cpu/gdt_helper.o \
			$(SRCDIR)/drivers/cpu/idt.o \
			$(SRCDIR)/drivers/cpu/idt_helper.o \
			$(SRCDIR)/drivers/cpu/pic.o \
			$(SRCDIR)/drivers/cpu/pit.o \
			$(SRCDIR)/interrupts/interrupt.o \
			$(SRCDIR)/interrupts/interrupt_helper.o \
			$(SRCDIR)/interrupts/exception.o \
			$(SRCDIR)/interrupts/exception_helper.o \
			$(SRCDIR)/common/libs/debug.o \
			$(SRCDIR)/common/libs/string.o \
			$(SRCDIR)/common/libs/printf.o \
			$(SRCDIR)/common/libs/bit.o \
			$(SRCDIR)/drivers/video/vga_text.o \
			$(SRCDIR)/drivers/io/serial.o \
			$(SRCDIR)/kernel/kernel.o

all: kernel iso

kernel: $(EXECUTABLE)
iso: $(ISOFILE)

$(EXECUTABLE): $(OBJECTS)
	@echo '[LD] $@'
	@$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

$(ISOFILE): $(EXECUTABLE)
	@echo '[GRUB] $@'
	@mkdir -p $(PROJECT)/boot/grub
	@cp $(EXECUTABLE) $(PROJECT)/boot/
	@echo 'set timeout=3' >> $(PROJECT)/boot/grub/grub.cfg
	@echo 'set default=0' >> $(PROJECT)/boot/grub/grub.cfg
	@echo 'set root=(cd)' >> $(PROJECT)/boot/grub/grub.cfg
	@echo '' >> $(PROJECT)/boot/grub/grub.cfg
	@echo 'menuentry "$(PROJECT)" { '>> $(PROJECT)/boot/grub/grub.cfg
	@echo 'multiboot /boot/$(EXECUTABLE)' >> $(PROJECT)/boot/grub/grub.cfg
	@echo 'boot' >> $(PROJECT)/boot/grub/grub.cfg
	@echo '}' >> $(PROJECT)/boot/grub/grub.cfg
	@$(GRUBDIR)grub-mkrescue -o $(ISOFILE) $(PROJECT)
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

run: iso
	$(QEMU) $(QEMUFLAGS)

rund: iso
	$(QEMU) $(QEMUFLAGS) $(QEMUDFLAGS)

stripd: $(EXECUTABLE)
	@$(TOOLDIR)$(OBJCOPY) --only-keep-debug $(EXECUTABLE) debug.sym
	@$(TOOLDIR)$(OBJCOPY) $(OBJCOPYFLAGS) $(EXECUTABLE)

forcerun: clean iso run
forcerund: clean iso rund stripd

PHONY: clean
clean:
	@echo 'Cleaning the source directory...'
	@rm $(OBJECTS) $(EXECUTABLE) $(ISOFILE)
