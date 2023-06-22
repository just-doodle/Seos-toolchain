TOOLDIR=~/sysroot/bin/
PREFIX=$(TOOLDIR)
SRCDIR=src/
INCLUDEDIR=$(SRCDIR)/include/
BACKUPDIR=$(SRCDIR)/../../BACKUP/
GRUBDIR=
SCRIPTSDIR=$(SRCDIR)/scripts/
FILESDIR=files

OSTOOL_DIR=~/sysroot/usr/bin/

CC=i686-elf-gcc
CCVERSION = $(shell echo $(CC) $(shell $(PREFIX)$(CC) --version | grep $(CC) | sed 's/^.* //g'))
CFLAGS=
KCFLAGS= -I$(INCLUDEDIR) -I/usr/include -nostdlib -DKERNEL_COMPILER="\"$(CCVERSION)\"" -lgcc -fno-builtin -fno-exceptions -fno-leading-underscore -ffreestanding -Wall -Wpedantic -ggdb -O0 -D__ENABLE_DEBUG_SYMBOL_LOADING__=1 -D__COMPOSITOR_LOW_END__=0 $(CFLAGS)


CXX=$(TOOLDIR)/i686-elf-g++
CXXFLAGS=

LD=$(TOOLDIR)/i686-elf-ld
LINKERFILE=src/boot/link.ld
LDFLAGS= -T$(LINKERFILE)

AS=$(TOOLDIR)/i686-elf-as
ASFLAGS=

NASM=nasm
NASMFLAGS=-f elf32 -g -F dwarf -O0 -w+zeroing

OBJCOPY = i686-elf-objcopy
OBJDUMP = i686-elf-objdump

OBJCOPYFLAGS = --strip-debug --strip-unneeded

QEMU=qemu-system-i386
QEMUFLAGS=-cdrom $(ISOFILE) -m 4096M -boot d -hda sorhd
QEMUDFLAGS= -s -S -serial file:k.log -daemonize -m 512M

PROJECT=SectorOS-RW4

EXECUTABLE=$(PROJECT).elf
ISOFILE=$(PROJECT).iso
IMAGEFILE=sorhd

NETWORK_INTERFACE = enp2s0

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
			$(SRCDIR)/drivers/vminterface/vboxguest.o \
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
			$(SRCDIR)/common/libs/charbuffer.o \
			$(SRCDIR)/fs/vfs.o \
			$(SRCDIR)/fs/ext2.o \
			$(SRCDIR)/fs/devfs.o \
			$(SRCDIR)/fs/mount.o \
			$(SRCDIR)/fs/sorfs.o \
			$(SRCDIR)/fs/kernelfs.o \
			$(SRCDIR)/fs/tmpfs.o \
			$(SRCDIR)/fs/stat.o \
			$(SRCDIR)/gui/draw.o \
			$(SRCDIR)/gui/blend.o \
			$(SRCDIR)/gui/targa.o \
			$(SRCDIR)/gui/compositor.o \
			$(SRCDIR)/drivers/video/ifb.o \
			$(SRCDIR)/drivers/hal/timer.o \
			$(SRCDIR)/drivers/hal/net.o \
			$(SRCDIR)/gui/bitmap.o \
			$(SRCDIR)/gui/font/font.o \
			$(SRCDIR)/gui/font/font_parser.o \
			$(SRCDIR)/drivers/video/vga_text.o \
			$(SRCDIR)/drivers/video/vesa.o \
			$(SRCDIR)/drivers/video/se_term.o \
			$(SRCDIR)/drivers/io/serial.o \
			$(SRCDIR)/drivers/storage/nulldev.o \
			$(SRCDIR)/drivers/storage/logdisk.o \
			$(SRCDIR)/drivers/io/portdev.o \
			$(SRCDIR)/drivers/io/mmio.o \
			$(SRCDIR)/drivers/time/rtc.o \
			$(SRCDIR)/drivers/input/keyboard.o \
			$(SRCDIR)/drivers/ethernet/rtl8139.o \
			$(SRCDIR)/drivers/ethernet/pcnet.o \
			$(SRCDIR)/drivers/ethernet/loopback.o \
			$(SRCDIR)/network/ethernet.o \
			$(SRCDIR)/network/netutils.o \
			$(SRCDIR)/network/arp.o \
			$(SRCDIR)/network/ipv4.o \
			$(SRCDIR)/network/udp.o \
			$(SRCDIR)/network/dhcp.o \
			$(SRCDIR)/network/tcp.o \
			$(SRCDIR)/limine-terminal/flanterm/backends/fb.o \
			$(SRCDIR)/limine-terminal/flanterm/flanterm.o \
			$(SRCDIR)/limine-terminal/stb_image.o \
			$(SRCDIR)/limine-terminal/image.o \
			$(SRCDIR)/limine-terminal/term.o \
			$(SRCDIR)/memory/kheap.o \
			$(SRCDIR)/gui/stb_image_write.o \
			$(SRCDIR)/memory/paging.o \
			$(SRCDIR)/memory/pmm.o \
			$(SRCDIR)/process/usermode.o \
			$(SRCDIR)/process/process.o \
			$(SRCDIR)/process/context_switch.o \
			$(SRCDIR)/process/spinlock.o \
			$(SRCDIR)/process/elf_loader.o \
			$(SRCDIR)/process/filedescriptor.o \
			$(SRCDIR)/kernel/shell.o \
			$(SRCDIR)/kernel/modules.o \
			$(SRCDIR)/kernel/kernel.o

all: kernel iso

kernel: $(EXECUTABLE)
iso: $(ISOFILE)

echocc:
	@echo $(CCVERSION)

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
	@echo 'multiboot /boot/$(EXECUTABLE) --root /dev/apio0 --loglevel 2' >> $(PROJECT)/boot/grub/grub.cfg
	@#echo 'module /boot/sorhd' >> $(PROJECT)/boot/grub/grub.cfg
	@#echo 'set gfxpayload=800x600x32' >> $(PROJECT)/boot/grub/grub.cfg
	@echo 'boot' >> $(PROJECT)/boot/grub/grub.cfg
	@echo '}' >> $(PROJECT)/boot/grub/grub.cfg
	@$(GRUBDIR)grub-mkrescue -o $(ISOFILE) $(PROJECT) --product-name=$(PROJECT)
	@rm -rf $(PROJECT)/


%.o : %.c
	@echo '[CC] $@'
	@$(PREFIX)/$(CC) $(KCFLAGS) -c -o $@ $<

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
	$(QEMU) $(QEMUFLAGS) -enable-kvm -cpu host -serial stdio | tee k.log

runnkvm: $(ISOFILE)
	$(QEMU) $(QEMUFLAGS) -serial stdio | tee k.log

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
	@$(OSTOOL_DIR)/i686-sectoros-gcc $(SRCDIR)/tools/uname.c -o files/uname -g

forcerun: clean iso run
forcerund: clean iso rund

PHONY: clean kernel
clean:
	@echo 'Cleaning the source directory...'
	@rm -f $(OBJECTS) $(EXECUTABLE) $(ISOFILE) sorfs $(IMAGEFILE)

clean_objs:
	@rm -f $(OBJECTS) $(SRCDIR)

setupTAP:
	@printf '$(BLU)Setting up tap\n$(RESET)'
	sudo brctl addbr br0
	sudo ip addr flush dev $(NETWORK_INTERFACE)
	sudo brctl addif br0 $(NETWORK_INTERFACE)
	sudo tunctl -t tap0 -u `whoami`
	sudo brctl addif br0 tap0
	sudo ifconfig $(NETWORK_INTERFACE) up
	sudo ifconfig tap0 up
	sudo ifconfig br0 up
	sudo dhclient -v br0

stopTAP:
	@sudo brctl delif br0 tap0
	@sudo tunctl -d tap0
	@sudo brctl delif br0 $(NETWORK_INTERFACE)
	@sudo ifconfig br0 down
	@sudo brctl delbr br0
	@sudo ifconfig $(NETWORK_INTERFACE) up
	@sudo dhclient -v $(NETWORK_INTERFACE)

stopTAP1:
	@sudo ifconfig br0 down
	@sudo brctl delbr br0
	@sudo ifconfig $(NETWORK_INTERFACE) up
	@sudo dhclient -v $(NETWORK_INTERFACE)

modules:
	$(CC) -c -g -pedantic -ffreestanding -static -I$(INCLUDEDIR) $(SRCDIR)/modules/test.c -o files/test.ko 

runnet: $(ISOFILE) setupTAP
	sudo $(QEMU) $(QEMUFLAGS) -accel kvm -serial stdio -netdev tap,id=mynet0,ifname=tap0,script=no,downscript=no -device rtl8139,netdev=mynet0,mac=52:55:00:d1:55:01 | tee k.log
	make stopTAP

runnet2: $(ISOFILE)
	sudo $(QEMU) $(QEMUFLAGS) -accel kvm -serial stdio -netdev tap,id=mynet0,ifname=tap0,script=no,downscript=no -device rtl8139,netdev=mynet0,mac=52:55:00:d1:55:01 | tee k.log
